/*
 * Copyright (C) 2015 Carlos Garnacho
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * Author: Carlos Garnacho <carlosg@gnome.org>
 */

#include "config.h"

#include "client-factory.h"
#include "observer.h"
#include "text-event.h"
#include "contact.h"
#include "room.h"
#include "conversation.h"
#include "miner.h"

#ifdef HAVE_MIGRATION
#include "logger-dumper.h"
#endif

#define GRAPH_URN "urn:uuid:5f15c02c-bede-06c7-413f-7bae48712d3a"
#define TRANSACTION_LIMIT 10000
#define TP_MIGRATION_FILENAME "tracker-miner-chatlog.tp-migrated"

typedef struct _TmcMinerPrivate TmcMinerPrivate;
typedef struct _Transaction Transaction;
typedef struct _Operation Operation;

enum {
	PHASE_NONE,
	PHASE_CONTACTS,
	PHASE_CHANNELS,
	PHASE_MESSAGES
};

static gchar *phases[] = {
	"none",
	"contacts",
	"channels",
	"messages"
};

struct _Operation {
	GPtrArray *elements;
	Transaction *transaction;
};

struct _Transaction {
	guint phase;
	GPtrArray *events;
	TmcMiner *miner;
};

struct _TmcMinerPrivate {
	TpDBusDaemon *dbus;
	TmcClientFactory *client_factory;
	TmcObserver *observer;

#ifdef HAVE_MIGRATION
	TmcLoggerDumper *dumper;
#endif

	GHashTable *room_urn_cache;
	GHashTable *contact_urn_cache;

	GPtrArray *events;

	guint flush_idle_id;
	guint flushing : 1;
};

typedef TrackerSparqlBuilder * (*TmcInsertFunc) (TmcEntity *entity,
                                                 TmcMiner  *miner);

static void transaction_next_phase (Transaction *transaction);
static void tmc_miner_check_flush (TmcMiner *miner);
static void tmc_miner_initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (TmcMiner, tmc_miner, TRACKER_TYPE_MINER,
                         G_ADD_PRIVATE (TmcMiner)
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                tmc_miner_initable_iface_init))

static GQuark quark_urn = 0;

static void
tmc_miner_finalize (GObject *object)
{
	TmcMiner *miner = TMC_MINER (object);
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);

	g_hash_table_unref (priv->room_urn_cache);
	g_hash_table_unref (priv->contact_urn_cache);

	if (priv->events)
		g_ptr_array_unref (priv->events);

	if (priv->flush_idle_id)
		g_source_remove (priv->flush_idle_id);

	g_object_unref (priv->observer);
	g_object_unref (priv->client_factory);
	g_object_unref (priv->dbus);

#ifdef HAVE_MIGRATION
	g_object_unref (priv->dumper);
#endif

	G_OBJECT_CLASS (tmc_miner_parent_class)->finalize (object);
}

static void
tmc_miner_class_init (TmcMinerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_miner_finalize;

	quark_urn = g_quark_from_static_string ("tmc-quark-urn");
}

static Operation *
operation_new (GHashTable  *elements,
               Transaction *transaction)
{
	GHashTableIter iter;
	Operation *op;
	gpointer data;

	op = g_slice_new0 (Operation);
	op->transaction = transaction;
	op->elements = g_ptr_array_new ();
	g_hash_table_iter_init (&iter, elements);

	while (g_hash_table_iter_next (&iter, &data, NULL))
		g_ptr_array_add (op->elements, data);

	return op;
}

static void
operation_free (Operation *op)
{
	g_ptr_array_unref (op->elements);
	g_slice_free (Operation, op);
}

static void
miner_set_entity_urn (TmcMiner    *miner,
                      TmcEntity   *entity,
                      const gchar *urn)
{
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);

	if (TMC_IS_CONVERSATION (entity)) {
		g_object_set_qdata_full (G_OBJECT (entity), quark_urn,
		                         g_strdup (urn),
		                         (GDestroyNotify) g_free);
	} else {
		GHashTable *ht;
		gchar *copy;

		if (TMC_IS_ROOM (entity)) {
			ht = priv->room_urn_cache;
		} else {
			ht = priv->contact_urn_cache;
		}

		copy = g_strdup (urn);
		g_hash_table_insert (ht, (gpointer) tmc_entity_get_name (entity), copy);
		g_object_set_qdata (G_OBJECT (entity), quark_urn, copy);
	}
}

static const gchar *
miner_get_entity_urn (TmcMiner  *miner,
                      TmcEntity *entity)
{
	TmcMinerPrivate *priv;
	GHashTable *ht;
	gchar *urn;

	if (entity == tmc_contact_self_get ())
		return "nco:default-contact-me";

	urn = g_object_get_qdata (G_OBJECT (entity), quark_urn);

	if (urn)
		return urn;

	if (TMC_IS_CONVERSATION (entity))
		return NULL;

	priv = tmc_miner_get_instance_private (miner);

	if (TMC_IS_ROOM (entity)) {
		ht = priv->room_urn_cache;
	} else {
		ht = priv->contact_urn_cache;
	}

	urn = g_hash_table_lookup (ht, tmc_entity_get_name (entity));

	if (urn) {
		g_object_set_qdata (G_OBJECT (entity), quark_urn, urn);
	}

	return urn;
}

static TrackerSparqlBuilder *
contact_builder_new (TmcEntity *entity,
                     TmcMiner  *miner)
{
	TrackerSparqlBuilder *builder;
	const gchar *name = tmc_entity_get_name (entity);

	builder = tracker_sparql_builder_new_update ();
	tracker_sparql_builder_insert_open (builder, GRAPH_URN);

	tracker_sparql_builder_subject (builder, "_:contact");
	tracker_sparql_builder_predicate (builder, "a");
	tracker_sparql_builder_object (builder, "nco:Contact");

	if (name) {
		tracker_sparql_builder_predicate (builder, "nco:nickname");
		tracker_sparql_builder_object_unvalidated (builder, name);
	}

	/* FIXME: nco:fullname */

	/* IM address */
	tracker_sparql_builder_predicate (builder, "nco:hasIMAddress");

	tracker_sparql_builder_object_blank_open (builder);
	tracker_sparql_builder_predicate (builder, "a");
	tracker_sparql_builder_object (builder, "nco:IMAddress");

	tracker_sparql_builder_predicate (builder, "nco:imProtocol");
	tracker_sparql_builder_object_unvalidated (builder, tmc_entity_get_protocol (entity));

	if (name) {
		tracker_sparql_builder_predicate (builder, "nco:imNickname");
		tracker_sparql_builder_object_unvalidated (builder, name);
	}

	tracker_sparql_builder_object_blank_close (builder);
	tracker_sparql_builder_insert_close (builder);

	return builder;
}

static TrackerSparqlBuilder *
channel_builder_new (TmcEntity *channel,
                     TmcMiner  *miner)
{
	TrackerSparqlBuilder *builder;

	builder = tracker_sparql_builder_new_update ();
	tracker_sparql_builder_insert_open (builder, GRAPH_URN);

	tracker_sparql_builder_subject (builder, "_:channel");
	tracker_sparql_builder_predicate (builder, "a");

	if (TMC_IS_ROOM (channel)) {
		tracker_sparql_builder_object (builder, "nmo:PermanentChannel");

		tracker_sparql_builder_predicate (builder, "nie:title");
		tracker_sparql_builder_object_unvalidated (builder,
							   tmc_entity_get_name (channel));
	} else if (TMC_IS_CONVERSATION (channel)) {
		TmcContact *peer;

		peer = tmc_conversation_get_peer (TMC_CONVERSATION (channel));
		tracker_sparql_builder_object (builder, "nmo:TransientChannel");

		tracker_sparql_builder_predicate (builder, "nmo:hasParticipant");
		tracker_sparql_builder_object (builder, "nco:default-contact-me");

		tracker_sparql_builder_predicate (builder, "nmo:hasParticipant");
		tracker_sparql_builder_object_iri (builder, miner_get_entity_urn (miner, TMC_ENTITY (peer)));
	}

	tracker_sparql_builder_insert_close (builder);

	return builder;
}

static TrackerSparqlBuilder *
message_builder_new (TmcTextEvent *event,
                     TmcMiner     *miner)
{
	TrackerSparqlBuilder *builder;
	TmcEntity *from, *channel, *peer = NULL;
	gint64 timestamp;
	GList *to, *l;

	channel = tmc_text_event_get_channel (event);
	from = tmc_text_event_get_from (event);
	to = tmc_text_event_get_to (event);
	timestamp = tmc_text_event_get_timestamp (event);

	builder = tracker_sparql_builder_new_update ();
	tracker_sparql_builder_insert_open (builder, GRAPH_URN);

	tracker_sparql_builder_subject (builder, "_:message");
	tracker_sparql_builder_predicate (builder, "a");
	tracker_sparql_builder_object (builder, "nmo:IMMessage");

	/* Channel */
	tracker_sparql_builder_predicate (builder, "nmo:communicationChannel");
	tracker_sparql_builder_object_iri (builder, miner_get_entity_urn (miner, channel));

	/* From */
	tracker_sparql_builder_predicate (builder, "nmo:from");

	if (from == tmc_contact_self_get ()) {
		tracker_sparql_builder_object (builder, "nco:default-contact-me");
	} else {
		tracker_sparql_builder_object_iri (builder, miner_get_entity_urn (miner, from));
	}

	/* To */
	for (l = to; l; l = l->next) {
		tracker_sparql_builder_predicate (builder, "nmo:to");

		if (l->data == tmc_contact_self_get ()) {
			tracker_sparql_builder_object (builder, "nco:default-contact-me");
		} else {
			tracker_sparql_builder_object_iri (builder, miner_get_entity_urn (miner, l->data));
		}
	}

	/* Message content */
	tracker_sparql_builder_predicate (builder, "nie:plainTextContent");
	tracker_sparql_builder_object_unvalidated (builder,
						   tmc_text_event_get_text (event));

	if (from == tmc_contact_self_get ()) {
		tracker_sparql_builder_predicate (builder, "nmo:sentDate");
		tracker_sparql_builder_object_date (builder, (time_t*) &timestamp);
	} else {
		tracker_sparql_builder_predicate (builder, "nmo:receivedDate");
		tracker_sparql_builder_object_date (builder, (time_t*) &timestamp);
	}

	tracker_sparql_builder_predicate (builder, "nie:contentCreated");
	tracker_sparql_builder_object_date (builder, (time_t*) &timestamp);

#if 0
	/*FIXME: Must be deleted before */
	/* Update communication channel timestamps */
	tracker_sparql_builder_subject_iri (builder, miner_get_entity_urn (miner, channel));
	tracker_sparql_builder_predicate (builder, "nmo:lastMessageDate");
	tracker_sparql_builder_object_date (builder, (time_t*) &timestamp);
#endif

	tracker_sparql_builder_insert_close (builder);

	return builder;
}

static void
transaction_free (Transaction *transaction)
{
	g_ptr_array_unref (transaction->events);
	g_free (transaction);
}

static void
transaction_finish (Transaction *transaction,
                    GError      *error)
{
	TmcMiner *miner = transaction->miner;
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);

	if (error) {
		g_critical ("Transaction error in phase '%s': %s",
		            phases[transaction->phase], error->message);
		g_error_free (error);
	}

	priv->flushing = FALSE;
	tmc_miner_check_flush (transaction->miner);

	transaction_free (transaction);
}

static void
add_if_missing (GHashTable *ht,
                TmcEntity  *entity,
                TmcMiner   *miner)
{
	if (!entity)
		return;

	if (miner_get_entity_urn (miner, entity))
		return;

	g_hash_table_add (ht, entity);
}

static void
map_results (TmcMiner  *miner,
             GPtrArray *entities,
             GVariant  *result)
{
	guint i, n_children;
	GVariant *child;

	n_children = g_variant_n_children (result);
	g_assert (n_children == entities->len);

	for (i = 0; i < n_children; i++) {
		TmcEntity *entity;
		gchar *name, *urn;
		GVariant *elem;

		entity = g_ptr_array_index (entities, i);

		child = g_variant_get_child_value (result, i);
		elem = g_variant_get_child_value (child, 0);
		g_variant_get_child (elem, 0, "{ss}", &name, &urn);

		miner_set_entity_urn (miner, entity, urn);

		g_variant_unref (elem);
		g_variant_unref (child);
		g_free (name);
		g_free (urn);
	}
}

static void
process_messages_cb (GObject      *source,
                     GAsyncResult *res,
                     gpointer      user_data)
{
	TrackerSparqlConnection *conn = TRACKER_SPARQL_CONNECTION (source);
	Transaction *transaction = user_data;
	GError *error = NULL;

	tracker_sparql_connection_update_finish (conn, res, &error);
	transaction_finish (transaction, error);
}

static void
insert_entities_cb (GObject      *source,
                    GAsyncResult *res,
                    gpointer      user_data)
{
	TrackerSparqlConnection *conn = TRACKER_SPARQL_CONNECTION (source);
	Operation *op = user_data;
	GError *error = NULL;
	GVariant *result;

	result = tracker_sparql_connection_update_blank_finish (conn, res, &error);

	if (error) {
		transaction_finish (op->transaction, error);
		operation_free (op);
		return;
	}

	map_results (op->transaction->miner, op->elements, result);
	transaction_next_phase (op->transaction);
	operation_free (op);
	g_variant_unref (result);
}

static void
insert_entities (Transaction   *transaction,
                 GHashTable    *entities,
                 TmcInsertFunc  func)
{
	TrackerSparqlConnection *conn;
	TrackerSparqlBuilder *builder;
	GString *query = g_string_new ("");
	TmcEntity *elem;
	Operation *op;
	guint i;

	op = operation_new (entities, transaction);

	for (i = 0; i < op->elements->len; i++) {
		elem = g_ptr_array_index (op->elements, i);
		builder = (func) (elem, transaction->miner);
		g_string_append (query, tracker_sparql_builder_get_result (builder));
		g_object_unref (builder);
	}

	conn = tracker_miner_get_connection (TRACKER_MINER (transaction->miner));
	tracker_sparql_connection_update_blank_async (conn,
	                                              query->str,
	                                              G_PRIORITY_DEFAULT,
	                                              NULL,
	                                              (GAsyncReadyCallback) insert_entities_cb,
	                                              op);
	g_string_free (query, TRUE);
}

static void
transaction_process_messages (Transaction *transaction)
{
	TrackerSparqlConnection *conn;
	TrackerSparqlBuilder *builder;
	GString *query = g_string_new ("");
	guint i;

	for (i = 0; i < transaction->events->len; i++) {
		TmcTextEvent *event = g_ptr_array_index (transaction->events, i);

		builder = message_builder_new (event, transaction->miner);
		g_string_append (query, tracker_sparql_builder_get_result (builder));
		g_object_unref (builder);
	}

	conn = tracker_miner_get_connection (TRACKER_MINER (transaction->miner));
	tracker_sparql_connection_update_async (conn,
	                                        query->str,
	                                        G_PRIORITY_DEFAULT,
	                                        NULL,
	                                        (GAsyncReadyCallback) process_messages_cb,
	                                        transaction);
	g_string_free (query, TRUE);
}

static void
transaction_process_missing_channels (Transaction *transaction)
{
	GHashTable *channels;
	guint i;

	channels = g_hash_table_new (NULL, NULL);

	for (i = 0; i < transaction->events->len; i++) {
		TmcTextEvent *event = g_ptr_array_index (transaction->events, i);
		TmcEntity *channel = tmc_text_event_get_channel (event);

		add_if_missing (channels, channel, transaction->miner);
	}

	if (g_hash_table_size (channels) > 0) {
		insert_entities (transaction, channels, channel_builder_new);
	} else {
		transaction_next_phase (transaction);
	}

	g_hash_table_unref (channels);
}

static void
transaction_process_missing_contacts (Transaction *transaction)
{
	GHashTable *contacts;
	GList *to, *l;
	guint i;

	contacts = g_hash_table_new (NULL, NULL);

	for (i = 0; i < transaction->events->len; i++) {
		TmcTextEvent *event = g_ptr_array_index (transaction->events, i);
		to = tmc_text_event_get_to (event);

		add_if_missing (contacts, tmc_text_event_get_from (event),
		                transaction->miner);

		for (l = to; l; l = l->next)
			add_if_missing (contacts, l->data, transaction->miner);
	}

	if (g_hash_table_size (contacts) > 0) {
		insert_entities (transaction, contacts, contact_builder_new);
	} else {
		transaction_next_phase (transaction);
	}

	g_hash_table_unref (contacts);
}

static void
transaction_next_phase (Transaction *transaction)
{
	transaction->phase++;

	if (transaction->phase == PHASE_CONTACTS)
		transaction_process_missing_contacts (transaction);
	else if (transaction->phase == PHASE_CHANNELS)
		transaction_process_missing_channels (transaction);
	else if (transaction->phase == PHASE_MESSAGES)
		transaction_process_messages (transaction);
	else
		g_assert_not_reached ();
}

static gboolean
idle_flush_cb (Transaction *transaction)
{
	TmcMiner *miner = transaction->miner;
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);
	gint i;

	g_debug ("Starting transaction, %ld elements remaining", priv->events->len);

	if (priv->events->len < TRANSACTION_LIMIT) {
		/* Transfer events to the transaction */
		transaction->events = priv->events;
		priv->events = NULL;
	} else {
		/* Make a partial copy */
		transaction->events =
			g_ptr_array_new_with_free_func (g_object_unref);

		for (i = 0; i < TRANSACTION_LIMIT; i++) {
			g_ptr_array_add (transaction->events,
					 g_object_ref (g_ptr_array_index (priv->events, i)));
		}

		g_ptr_array_remove_range (priv->events, 0, TRANSACTION_LIMIT);
	}

	priv->flushing = TRUE;
	priv->flush_idle_id = 0;

	transaction_next_phase (transaction);

	return G_SOURCE_REMOVE;
}

static void
tmc_miner_check_flush (TmcMiner *miner)
{
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);
	Transaction *transaction;

	if (!priv->events || priv->flush_idle_id || priv->flushing)
		return;

	transaction = g_new0 (Transaction, 1);
	transaction->miner = miner;
	priv->flush_idle_id =
		g_idle_add ((GSourceFunc) idle_flush_cb, transaction);
}

static void
handle_text_event (TmcMiner     *miner,
		   TmcTextEvent *event,
		   TmcObserver  *observer)
{
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);
	TrackerSparqlBuilder *builder;

	/* FIXME: Ensure message is not empty (full of spaces) */

	if (!priv->events) {
		priv->events = g_ptr_array_new_with_free_func (g_object_unref);
	}

	g_ptr_array_add (priv->events, g_object_ref (event));
	tmc_miner_check_flush (miner);
}

static void
tmc_miner_init (TmcMiner *miner)
{
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);

	priv->room_urn_cache = g_hash_table_new_full (g_str_hash, g_str_equal,
	                                              (GDestroyNotify) g_free,
	                                              (GDestroyNotify) g_free);
	priv->contact_urn_cache = g_hash_table_new_full (g_str_hash, g_str_equal,
	                                                 (GDestroyNotify) g_free,
	                                                 (GDestroyNotify) g_free);
}

static gboolean
populate_room_urn_cache (TmcMiner      *miner,
                         GCancellable  *cancellable,
                         GError       **error)
{
	TrackerSparqlConnection *conn;
	TrackerSparqlCursor *cursor;
	TmcMinerPrivate *priv;

	priv = tmc_miner_get_instance_private (miner);
	conn = tracker_miner_get_connection (TRACKER_MINER (miner));
	cursor = tracker_sparql_connection_query (conn,
	                                          "SELECT ?title ?urn {"
	                                          "  GRAPH <" GRAPH_URN "> {"
	                                          "    ?urn a nmo:PermanentChannel;"
	                                          "         nie:title ?title"
	                                          "  }"
	                                          "}",
	                                          cancellable, error);
	if (!cursor)
		return FALSE;

	while (tracker_sparql_cursor_next (cursor, cancellable, error)) {
		const gchar *name, *urn;

		name = tracker_sparql_cursor_get_string (cursor, 0, NULL);
		urn = tracker_sparql_cursor_get_string (cursor, 1, NULL);
		g_hash_table_insert (priv->room_urn_cache,
		                     g_strdup (name), g_strdup (urn));
	}

	g_object_unref (cursor);
	return (*error) == NULL;
}

static gboolean
populate_contact_urn_cache (TmcMiner      *miner,
                            GCancellable  *cancellable,
                            GError       **error)
{
	TrackerSparqlConnection *conn;
	TrackerSparqlCursor *cursor;
	TmcMinerPrivate *priv;

	priv = tmc_miner_get_instance_private (miner);
	conn = tracker_miner_get_connection (TRACKER_MINER (miner));
	cursor = tracker_sparql_connection_query (conn,
	                                          "SELECT ?name ?urn {"
	                                          "  GRAPH <" GRAPH_URN "> {"
	                                          "    ?urn a nco:Contact ;"
	                                          "         nco:nickname ?name"
	                                          "  }"
	                                          "}",
	                                          cancellable, error);
	if (!cursor)
		return FALSE;

	while (tracker_sparql_cursor_next (cursor, cancellable, error)) {
		const gchar *name, *urn;

		name = tracker_sparql_cursor_get_string (cursor, 0, NULL);
		urn = tracker_sparql_cursor_get_string (cursor, 1, NULL);
		g_hash_table_insert (priv->contact_urn_cache,
		                     g_strdup (name), g_strdup (urn));
	}

	g_object_unref (cursor);
	return (*error) == NULL;
}

#ifdef HAVE_MIGRATION
static void
observer_account_added (TmcMiner    *miner,
			TpAccount   *account,
			TmcObserver *observer)
{
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);

	tmc_logger_dumper_add_account (priv->dumper, account);
}
#endif

static gboolean
tmc_miner_initable_init (GInitable     *initable,
                         GCancellable  *cancellable,
                         GError       **error)
{
	TmcMiner *miner = TMC_MINER (initable);
	TmcMinerPrivate *priv = tmc_miner_get_instance_private (miner);
	GInitableIface *parent_iface;
#ifdef HAVE_MIGRATION
	gchar *filename;
#endif

	parent_iface = g_type_interface_peek_parent (G_INITABLE_GET_IFACE (initable));

	if (!parent_iface->init (initable, cancellable, error))
		return FALSE;

	priv->dbus = tp_dbus_daemon_dup (error);

	if (!priv->dbus)
		return FALSE;

	priv->client_factory = tmc_client_factory_new (priv->dbus);
	priv->observer = tmc_observer_new (priv->client_factory);
	g_signal_connect_swapped (priv->observer, "text-event",
				  G_CALLBACK (handle_text_event), miner);

#ifdef HAVE_MIGRATION
	filename = g_build_filename (g_get_user_cache_dir (), "tracker",
				     "tracker-miner-chatlog.tp-migration", NULL);

	if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
		priv->dumper = tmc_logger_dumper_new ();
		g_signal_connect_swapped (priv->dumper, "text-event",
					  G_CALLBACK (handle_text_event), miner);
		g_signal_connect_swapped (priv->observer, "account",
					  G_CALLBACK (observer_account_added), miner);

		g_file_set_contents (filename, "", -1, NULL);
	}

	g_free (filename);
#endif

	if (!tp_base_client_register (TP_BASE_CLIENT (priv->observer), error))
		return FALSE;

	if (!populate_room_urn_cache (miner, cancellable, error))
		return FALSE;
	if (!populate_contact_urn_cache (miner, cancellable, error))
		return FALSE;

	return TRUE;
}

static void
tmc_miner_initable_iface_init (GInitableIface *iface)
{
	iface->init = tmc_miner_initable_init;
}

TrackerMiner *
tmc_miner_new (GError **error)
{
	return g_initable_new (TMC_TYPE_MINER, NULL, error,
	                       "name", "Chatlogs",
	                       NULL);
}

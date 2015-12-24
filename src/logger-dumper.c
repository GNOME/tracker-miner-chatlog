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

#include <telepathy-logger/telepathy-logger.h>

#include "logger-dumper.h"
#include "text-event.h"
#include "contact.h"
#include "room.h"
#include "conversation.h"
#include "entity-pool.h"

typedef struct _TmcLoggerDumperPrivate TmcLoggerDumperPrivate;
typedef struct _QueryOperation QueryOperation;

struct _TmcLoggerDumper {
	GObject parent_instance;
};

struct _QueryOperation {
	TmcLoggerDumper *dumper;
	TpAccount *account;
	TplEntity *entity;
	GList *dates;
};

struct _TmcLoggerDumperPrivate {
	TplLogManager *log_manager;
	GList *operations;
	GList *accounts;

	guint querying : 1;
};

enum {
	TEXT_EVENT,
	N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

static gboolean tmc_logger_dumper_next_operation (TmcLoggerDumper *dumper);

G_DEFINE_TYPE_WITH_PRIVATE (TmcLoggerDumper, tmc_logger_dumper, G_TYPE_OBJECT)

static void
tmc_logger_dumper_finalize (GObject *object)
{
	TmcLoggerDumper *dumper;
	TmcLoggerDumperPrivate *priv;

	dumper = TMC_LOGGER_DUMPER (object);
	priv = tmc_logger_dumper_get_instance_private (dumper);

	g_object_unref (priv->log_manager);

	g_list_foreach (priv->accounts, (GFunc) g_object_unref, NULL);
	g_list_free (priv->accounts);

	G_OBJECT_CLASS (tmc_logger_dumper_parent_class)->finalize (object);
}

static void
tmc_logger_dumper_class_init (TmcLoggerDumperClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_logger_dumper_finalize;

	signals[TEXT_EVENT] =
		g_signal_new ("text-event",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0, NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, TMC_TYPE_TEXT_EVENT);
}

static void
tmc_logger_dumper_init (TmcLoggerDumper *dumper)
{
	TmcLoggerDumperPrivate *priv;

	priv = tmc_logger_dumper_get_instance_private (dumper);
	priv->log_manager = tpl_log_manager_dup_singleton ();
}

TmcLoggerDumper *
tmc_logger_dumper_new (void)
{
	return g_object_new (TMC_TYPE_LOGGER_DUMPER, NULL);
}

static QueryOperation *
query_operation_new (TmcLoggerDumper *dumper,
		     TpAccount       *account,
		     TplEntity       *entity)
{
	QueryOperation *op;

	op = g_new0 (QueryOperation, 1);
	op->dumper = dumper;
	op->account = g_object_ref (account);
	op->entity = g_object_ref (entity);

	return op;
}

static void
query_operation_free (QueryOperation *op)
{
	g_object_unref (op->account);
	g_object_unref (op->entity);
	g_list_foreach (op->dates, (GFunc) g_date_free, NULL);
	g_list_free (op->dates);

	g_free (op);
}

static TmcEntity *
translate_contact (TplEntity *entity)
{
	TmcEntityPool *pool = tmc_entity_pool_contacts_get ();
	TmcEntity *contact;

	switch (tpl_entity_get_entity_type (entity)) {
	case TPL_ENTITY_UNKNOWN:
	case TPL_ENTITY_ROOM:
		return NULL;
	case TPL_ENTITY_SELF:
		return tmc_contact_self_get ();
	case TPL_ENTITY_CONTACT:
		contact = tmc_entity_pool_lookup (pool, tpl_entity_get_alias (entity));

		if (!contact) {
			contact = tmc_contact_new (tpl_entity_get_alias (entity),
						   "irc");
			tmc_entity_pool_add (pool, contact);
		}

		return contact;
	}
}

static TmcEntity *
translate_channel (QueryOperation *op)
{
	TmcEntityPool *pool = tmc_entity_pool_channels_get ();
	TmcEntity *channel;

	channel = tmc_entity_pool_lookup (pool, tpl_entity_get_alias (op->entity));

	if (!channel) {
		switch (tpl_entity_get_entity_type (op->entity)) {
		case TPL_ENTITY_UNKNOWN:
		case TPL_ENTITY_SELF:
			return NULL;
		case TPL_ENTITY_CONTACT:
			channel = tmc_conversation_new (TMC_CONTACT (translate_contact (op->entity)));
			break;
		case TPL_ENTITY_ROOM:
			channel = tmc_room_new (tpl_entity_get_alias (op->entity),
					"irc");
			break;
		}

		tmc_entity_pool_add (pool, channel);
	}

	return channel;
}

static void
tmc_logger_dumper_emit_event (TmcLoggerDumper *dumper,
			      TpAccount       *account,
			      TplEntity       *entity,
			      TplTextEvent    *event)
{
	TmcLoggerDumperPrivate *priv;
	QueryOperation *op;
	TmcTextEvent *text_event;
	TmcEntity *from, *to, *channel;
	GList *to_list = NULL;

	priv = tmc_logger_dumper_get_instance_private (dumper);
	op = priv->operations->data;

	channel = translate_channel (op);
	from = translate_contact (tpl_event_get_sender (TPL_EVENT (event)));
	to = translate_contact (tpl_event_get_receiver (TPL_EVENT (event)));

	if (to) {
		to_list = g_list_prepend (to_list, to);
	}

	text_event = tmc_text_event_new (channel, from, to_list,
					 tpl_text_event_get_message (event),
					 tpl_event_get_timestamp (TPL_EVENT (event)));
	g_signal_emit (dumper, signals[TEXT_EVENT], 0, text_event);
	g_object_unref (text_event);
}

static void
get_date_events_async_cb (GObject      *object,
			  GAsyncResult *res,
			  gpointer      user_data)
{
	TmcLoggerDumper *dumper = user_data;
	TmcLoggerDumperPrivate *priv = tmc_logger_dumper_get_instance_private (dumper);
	QueryOperation *op = priv->operations->data;
	GError *error = NULL;
	GList *events, *l;

	if (!tpl_log_manager_get_events_for_date_finish (TPL_LOG_MANAGER (object),
							 res, &events, &error)) {
		g_warning ("Could not get events for date: %s",
			   error->message);
	} else {
		for (l = events; l; l = l->next) {
			tmc_logger_dumper_emit_event (dumper, op->account,
						      op->entity, l->data);
			g_object_unref (l->data);
		}

		g_list_free (events);
	}

	tmc_logger_dumper_next_operation (dumper);
}

static void
get_entity_dates_async_cb (GObject      *object,
			   GAsyncResult *res,
			   gpointer      user_data)
{
	TmcLoggerDumper *dumper = user_data;
	TmcLoggerDumperPrivate *priv = tmc_logger_dumper_get_instance_private (dumper);
	GError *error = NULL;
	QueryOperation *op;

	op = priv->operations->data;

	if (!tpl_log_manager_get_dates_finish (TPL_LOG_MANAGER (object),
					       res, &op->dates, &error)) {
		g_warning ("Could not get dates for entity: %s",
			   error->message);
	}

	tmc_logger_dumper_next_operation (dumper);
}

static void
get_account_entities_async_cb (GObject      *object,
			       GAsyncResult *res,
			       gpointer      user_data)
{
	TmcLoggerDumper *dumper = user_data;
	TmcLoggerDumperPrivate *priv;
	GList *l, *entities = NULL;
	GError *error = NULL;
	TpAccount *account;

	priv = tmc_logger_dumper_get_instance_private (dumper);
	account = priv->accounts->data;
	priv->accounts = g_list_remove (priv->accounts, account);

	if (!tpl_log_manager_get_entities_finish (TPL_LOG_MANAGER (object),
						  res, &entities, &error)) {
		g_warning ("Could not get entities for account: %s",
			   error->message);
	} else {
		for (l = entities; l; l = l->next) {
			TplEntity *entity = l->data;
			QueryOperation *op;

			op = query_operation_new (dumper, account, entity);
			priv->operations = g_list_prepend (priv->operations, op);
			g_object_unref (entity);
		}

		g_list_free (entities);
	}

	tmc_logger_dumper_next_operation (dumper);
	g_object_unref (account);
}

static gboolean
tmc_logger_dumper_query_date_events (TmcLoggerDumper *dumper)
{
	TmcLoggerDumperPrivate *priv = tmc_logger_dumper_get_instance_private (dumper);
	QueryOperation *op;
	GDate *date;
	GList *elem;

	if (!priv->operations)
		return FALSE;

	op = priv->operations->data;

	if (!op->dates)
		return FALSE;

	elem = op->dates;
	date = elem->data;
	op->dates = op->dates->next;
	g_list_free1 (elem);

	tpl_log_manager_get_events_for_date_async (priv->log_manager,
						   op->account, op->entity,
						   TPL_EVENT_MASK_TEXT, date,
						   get_date_events_async_cb,
						   dumper);
	g_date_free (date);

	return TRUE;
}

static gboolean
tmc_logger_dumper_query_next_entity (TmcLoggerDumper *dumper)
{
	TmcLoggerDumperPrivate *priv = tmc_logger_dumper_get_instance_private (dumper);
	QueryOperation *op;

	if (!priv->operations)
		return FALSE;

	op = priv->operations->data;
	tpl_log_manager_get_dates_async (priv->log_manager,
					 op->account, op->entity,
					 TPL_EVENT_MASK_TEXT,
					 get_entity_dates_async_cb,
					 dumper);
	return TRUE;
}

static gboolean
tmc_logger_dumper_query_next_account (TmcLoggerDumper *dumper)
{
	TmcLoggerDumperPrivate *priv;

	priv = tmc_logger_dumper_get_instance_private (dumper);
	priv->querying = priv->accounts != NULL;

	if (priv->accounts) {
		tpl_log_manager_get_entities_async (priv->log_manager,
						    priv->accounts->data,
						    get_account_entities_async_cb,
						    dumper);
	}

	return priv->querying;
}

static gboolean
tmc_logger_dumper_next_operation (TmcLoggerDumper *dumper)
{
	TmcLoggerDumperPrivate *priv = tmc_logger_dumper_get_instance_private (dumper);
	QueryOperation *op;

	if (!priv->operations)
		return tmc_logger_dumper_query_next_account (dumper);

	op = priv->operations->data;

	if (op->dates) {
		return tmc_logger_dumper_query_date_events (dumper);
	} else {
		priv->operations = g_list_remove (priv->operations, op);
		query_operation_free (op);

		return tmc_logger_dumper_query_next_entity (dumper);
	}
}

void
tmc_logger_dumper_add_account (TmcLoggerDumper *dumper,
			       TpAccount       *account)
{
	TmcLoggerDumperPrivate *priv;

	priv = tmc_logger_dumper_get_instance_private (dumper);

	if (g_list_find (priv->accounts, account))
		return;

	priv->accounts = g_list_prepend (priv->accounts,
					 g_object_ref (account));

	if (!priv->querying)
		tmc_logger_dumper_next_operation (dumper);
}

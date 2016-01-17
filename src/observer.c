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

#include <glib.h>
#include "observer.h"
#include "text-channel.h"
#include "text-event.h"
#include "entity-pool.h"
#include "contact.h"
#include "conversation.h"
#include "room.h"
#include "helpers.h"

typedef struct _TmcObserverPrivate TmcObserverPrivate;

struct _TmcObserverPrivate {
	GList *channels;
	GRegex *recipients_regex;
	GList *accounts;
};

enum {
	ACCOUNT,
	TEXT_EVENT,
	N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE (TmcObserver, tmc_observer, TP_TYPE_BASE_CLIENT)

static void
tmc_observer_finalize (GObject *object)
{
	TmcObserver *observer = TMC_OBSERVER (object);
	TmcObserverPrivate *priv = tmc_observer_get_instance_private (observer);

	g_list_foreach (priv->channels, (GFunc) g_object_unref, NULL);
	g_list_free (priv->channels);
	g_regex_unref (priv->recipients_regex);

	g_list_foreach (priv->accounts, (GFunc) g_object_unref, NULL);
	g_list_free (priv->accounts);

	G_OBJECT_CLASS (tmc_observer_parent_class)->finalize (object);
}

static GList *
message_guess_recipients (TmcObserver *observer,
			  const gchar *text,
			  TpChannel   *channel)
{
	TmcObserverPrivate *priv = tmc_observer_get_instance_private (observer);
	GList *recipients = NULL;
	GMatchInfo *match_info;
	gboolean first = TRUE;

	if (!g_regex_match (priv->recipients_regex, text, 0, &match_info)) {
		g_match_info_free (match_info);
		return NULL;
	}

	while (g_match_info_matches (match_info)) {
		TmcEntity *contact;
		gint start, end;
		gchar *name;

		if (!g_match_info_fetch_pos (match_info, 1, &start, &end))
			break;

		if (first && start != 0)
			break;

		first = FALSE;
		name = g_match_info_fetch (match_info, 1);
		g_match_info_next (match_info, NULL);
		contact = tmc_text_channel_lookup_contact (TMC_TEXT_CHANNEL (channel),
							   name);
		if (contact)
			recipients = g_list_prepend (recipients, contact);

		g_free (name);
	}

	/* FIXME: find pings to self on all the string */

	g_match_info_free (match_info);

	return recipients;
}

static void
emit_text_event (TmcObserver *observer,
		 TmcEntity   *from,
		 TmcEntity   *to,
		 TpMessage   *message,
		 TpChannel   *channel)
{
	TmcTextEvent *event;
	gint64 timestamp;
	GList *to_list;
	gchar *text;

	if (tp_message_is_rescued (message))
		return;
	if (tp_message_is_delivery_report (message))
		return;
	if (tp_message_is_scrollback (message))
		return;

	/* FIXME: check the message is not older than our miner */

	text = tp_message_to_text (message, NULL);
	timestamp = tp_message_get_received_timestamp (message);

	if (timestamp == 0)
		timestamp = tp_message_get_sent_timestamp (message);

	to_list = message_guess_recipients (observer, text, channel);

	if (to) {
		to_list = g_list_prepend (to_list, to);
	}

	event = tmc_text_event_new (tmc_ensure_channel (channel),
				    from, to_list, text, timestamp);

	g_signal_emit (observer, signals[TEXT_EVENT], 0, event);

	g_list_free (to_list);
	g_object_unref (event);
	g_free (text);
}

static void
message_received (TmcObserver *observer,
		  TpMessage   *message,
		  TpChannel   *channel)
{
	TmcEntity *from, *to = NULL;

	from = tmc_ensure_contact (tp_signalled_message_get_sender (message));

	if (tp_channel_get_target_contact (channel)) {
		/* Private conversation */
		to = tmc_contact_self_get ();
	}

	emit_text_event (observer, from, to, message, channel);
}

static void
message_sent (TmcObserver           *observer,
	      TpMessage             *message,
	      TpMessageSendingFlags  flags,
	      const gchar           *token,
	      TpChannel             *channel)
{
	TmcEntity *from, *to = NULL;
	TpContact *peer;

	from = tmc_contact_self_get ();
	peer = tp_channel_get_target_contact (channel);

	if (peer) {
		/* Private conversation */
		to = tmc_ensure_contact (peer);
	}

	emit_text_event (observer, from, to, message, channel);
}

static void
channel_invalidated (TmcObserver *observer,
		     guint        domain,
		     guint        code,
		     gchar       *message,
		     TpChannel   *channel)
{
	TmcObserverPrivate *priv;

	priv = tmc_observer_get_instance_private (observer);
	priv->channels = g_list_remove (priv->channels, channel);

	g_signal_handlers_disconnect_by_data (channel, observer);
	g_object_unref (channel);
}

static void
register_channel (TmcObserver *observer,
		  TpChannel   *channel)
{
	TmcObserverPrivate *priv;
	GList *pending;

	priv = tmc_observer_get_instance_private (observer);
	priv->channels = g_list_prepend (priv->channels,
					 g_object_ref (channel));

	g_signal_connect_swapped (channel, "message-received",
				  G_CALLBACK (message_received), observer);
	g_signal_connect_swapped (channel, "message-sent",
				  G_CALLBACK (message_sent), observer);
	g_signal_connect_swapped (channel, "invalidated",
				  G_CALLBACK (channel_invalidated), observer);

	pending = tp_text_channel_dup_pending_messages (TP_TEXT_CHANNEL (channel));

	while (pending) {
		message_received (observer, pending->data, channel);
		g_object_unref (pending->data);
		pending = g_list_delete_link (pending, pending);
	}
}

static void
tmc_observer_manage_account (TmcObserver *observer,
			     TpAccount   *account)
{
	TmcObserverPrivate *priv = tmc_observer_get_instance_private (observer);

	if (g_list_find (priv->accounts, account))
		return;

	priv->accounts = g_list_prepend (priv->accounts,
					 g_object_ref (account));
	g_signal_emit (observer, signals[ACCOUNT], 0, account);
}

static void
tmc_observer_observe_channels (TpBaseClient               *client,
			       TpAccount                  *account,
			       TpConnection               *connection,
			       GList                      *channels,
			       TpChannelDispatchOperation *dispatch_operation,
			       GList                      *requests,
			       TpObserveChannelsContext   *context)
{
	TmcObserver *self = TMC_OBSERVER (client);
	GList *l;

	tmc_observer_manage_account (self, account);

	for (l = channels; l != NULL; l = l->next)
		register_channel (self, l->data);

	tp_observe_channels_context_accept (context);
}


static void
tmc_observer_class_init (TmcObserverClass *klass)
{
	TpBaseClientClass *base_client_class = TP_BASE_CLIENT_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_observer_finalize;

	tp_base_client_implement_observe_channels (base_client_class,
						   tmc_observer_observe_channels);

	signals[TEXT_EVENT] =
		g_signal_new ("text-event",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0, NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, TMC_TYPE_TEXT_EVENT);
	signals[ACCOUNT] =
		g_signal_new ("account",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0, NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, TP_TYPE_ACCOUNT);
}

static void
tmc_observer_init (TmcObserver *self)
{
	TmcObserverPrivate *priv = tmc_observer_get_instance_private (self);

	/* Observe contact text channels */
	tp_base_client_take_observer_filter (TP_BASE_CLIENT (self),
					     tp_asv_new (TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING,
							 TP_IFACE_CHANNEL_TYPE_TEXT,
							 TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT,
							 TP_HANDLE_TYPE_CONTACT,
							 NULL));
	/* Observe room text channels */
	tp_base_client_take_observer_filter (TP_BASE_CLIENT (self),
					     tp_asv_new (TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING,
							 TP_IFACE_CHANNEL_TYPE_TEXT,
							 TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT,
							 TP_HANDLE_TYPE_ROOM,
							 NULL));

	tp_base_client_set_observer_recover (TP_BASE_CLIENT (self), TRUE);

	priv->recipients_regex = g_regex_new ("(\\S+)\\s*[,:]", G_REGEX_OPTIMIZE, 0, NULL);
}

TmcObserver *
tmc_observer_new (TmcClientFactory *factory)
{
	return g_object_new (TMC_TYPE_OBSERVER,
			     "factory", factory,
			     "name", "TrackerMinerChatlog",
			     "uniquify-name", FALSE,
			     NULL);
}

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

#include "text-channel.h"
#include "entity-pool.h"
#include "contact.h"
#include "helpers.h"

typedef struct _TmcTextChannelPrivate TmcTextChannelPrivate;

struct _TmcTextChannelPrivate {
	GHashTable *contacts;
	guint contacts_changed_id;
};

G_DEFINE_TYPE_WITH_PRIVATE (TmcTextChannel, tmc_text_channel, TP_TYPE_TEXT_CHANNEL)

static void
tmc_text_channel_finalize (GObject *object)
{
	TmcTextChannel *channel = TMC_TEXT_CHANNEL (object);
	TmcTextChannelPrivate *priv = tmc_text_channel_get_instance_private (channel);

	if (priv->contacts)
		g_hash_table_destroy (priv->contacts);

	G_OBJECT_CLASS (tmc_text_channel_parent_class)->finalize (object);
}

static void
tmc_text_channel_class_init (TmcTextChannelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_text_channel_finalize;
}

static void
handle_contacts_added (TmcTextChannel *channel,
                       GPtrArray      *contacts)
{
	TmcTextChannelPrivate *priv = tmc_text_channel_get_instance_private (channel);
	TmcEntityPool *pool = tmc_entity_pool_contacts_get ();
	TmcEntity *contact;
	guint i;

	for (i = 0; i < contacts->len; i++) {
		contact = tmc_ensure_contact (g_ptr_array_index (contacts, i));
		g_hash_table_insert (priv->contacts,
		                     (gpointer) tmc_entity_get_name (contact),
		                     contact);
	}
}

static void
handle_contacts_removed (TmcTextChannel *channel,
                         GPtrArray      *contacts)
{
	TmcTextChannelPrivate *priv = tmc_text_channel_get_instance_private (channel);
	TmcEntityPool *pool = tmc_entity_pool_contacts_get ();
	TpContact *contact;
	guint i;

	for (i = 0; i < contacts->len; i++) {
		contact = g_ptr_array_index (contacts, i);
		g_hash_table_remove (priv->contacts,
		                     tp_contact_get_alias (contact));
	}
}

static void
channel_contacts_changed (TmcTextChannel *channel,
			  GPtrArray      *added,
			  GPtrArray      *removed,
			  GPtrArray      *local_pending,
			  GPtrArray      *remote_pending,
			  TpContact      *actor,
			  gpointer        user_data)
{
	handle_contacts_added (channel, added);
	handle_contacts_removed (channel, removed);
}

static void
tmc_text_channel_init (TmcTextChannel *channel)
{
}

TmcTextChannel *
tmc_text_channel_new (TpSimpleClientFactory  *factory,
		      TpConnection           *conn,
		      const gchar            *object_path,
		      const GHashTable       *tp_chan_props,
		      GError                **error)
{
	g_return_val_if_fail (TP_IS_CONNECTION (conn), NULL);
	g_return_val_if_fail (object_path && *object_path, NULL);
	g_return_val_if_fail (tp_chan_props != NULL, NULL);

	if (!tp_dbus_check_valid_object_path (object_path, error))
		return NULL;

	return g_object_new (TMC_TYPE_TEXT_CHANNEL,
			     "factory", factory,
			     "connection", conn,
			     "dbus-daemon", tp_proxy_get_dbus_daemon (conn),
			     "bus-name", tp_proxy_get_bus_name (conn),
			     "object-path", object_path,
			     "handle-type", TP_UNKNOWN_HANDLE_TYPE,
			     "channel-properties", tp_chan_props,
			     NULL);
}

TmcEntity *
tmc_text_channel_lookup_contact (TmcTextChannel *channel,
				 const gchar    *name)
{
	TmcTextChannelPrivate *priv;

	g_return_val_if_fail (TMC_IS_TEXT_CHANNEL (channel), NULL);
	g_return_val_if_fail (name && *name, NULL);

	priv = tmc_text_channel_get_instance_private (channel);

	if (!priv->contacts) {
		GPtrArray *contacts;

		g_signal_connect (channel, "group-contacts-changed",
				  G_CALLBACK (channel_contacts_changed), channel);
		priv->contacts = g_hash_table_new (g_str_hash, g_str_equal);

		contacts = tp_channel_group_dup_members_contacts (TP_CHANNEL (channel));

		if (contacts) {
			handle_contacts_added (channel, contacts);
			g_ptr_array_unref (contacts);
		}
	}

	return g_hash_table_lookup (priv->contacts, name);
}

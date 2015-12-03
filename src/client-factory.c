/*
 * Copyright (C) 2015 Carlos Garnacho
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * Author: Carlos Garnacho <carlosg@gnome.org>
 */

#include "client-factory.h"
#include "text-channel.h"

G_DEFINE_TYPE (TmcClientFactory, tmc_client_factory, TP_TYPE_AUTOMATIC_CLIENT_FACTORY)

static TpChannel *
create_channel (TpSimpleClientFactory  *self,
		TpConnection           *conn,
		const gchar            *object_path,
		const GHashTable       *properties,
		GError                **error)
{
	const gchar *channel_type;

	channel_type = tp_asv_get_string (properties, TP_PROP_CHANNEL_CHANNEL_TYPE);

	if (g_strcmp0 (channel_type, TP_IFACE_CHANNEL_TYPE_TEXT) == 0) {
		return TP_CHANNEL (tmc_text_channel_new (self, conn, object_path, properties, error));
	} else {
		g_debug ("Unknown channel type: %s", channel_type);

		return TP_SIMPLE_CLIENT_FACTORY_CLASS (tmc_client_factory_parent_class)->create_channel (self, conn, object_path, properties, error);
	}
}

static void
tmc_client_factory_init (TmcClientFactory *self)
{
	tp_simple_client_factory_add_contact_features_varargs (TP_SIMPLE_CLIENT_FACTORY (self),
							       TP_CONTACT_FEATURE_ALIAS,
							       TP_CONTACT_FEATURE_PRESENCE,
							       TP_CONTACT_FEATURE_INVALID);
#if 0
	tp_simple_client_factory_add_channel_features_varargs (TP_SIMPLE_CLIENT_FACTORY (self),
							       TP_CHANNEL_FEATURE_CONTACTS,
							       TP_TEXT_CHANNEL_FEATURE_INCOMING_MESSAGES,
							       TP_CONTACT_FEATURE_INVALID);
#endif
}

static void
tmc_client_factory_class_init (TmcClientFactoryClass *cls)
{
	TpSimpleClientFactoryClass *simple_class = (TpSimpleClientFactoryClass *) cls;

	simple_class->create_channel = create_channel;
}

TmcClientFactory *
tmc_client_factory_new (TpDBusDaemon *dbus)
{
	return g_object_new (TMC_TYPE_CLIENT_FACTORY,
			     "dbus-daemon", dbus,
			     NULL);
}

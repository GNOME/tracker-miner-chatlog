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

#include "entity-pool.h"
#include "contact.h"
#include "room.h"
#include "conversation.h"
#include "helpers.h"

TmcEntity *
tmc_ensure_contact (TpContact *contact)
{
	TmcEntityPool *contacts = tmc_entity_pool_contacts_get ();
	TmcEntity *entity;
	const gchar *name;

	name = tp_contact_get_alias (contact);
	entity = tmc_entity_pool_lookup (contacts, name);

	if (!entity) {
		TpConnection *connection;
		const gchar *protocol;

		connection = tp_contact_get_connection (contact);
		protocol = tp_connection_get_protocol_name (connection);
		entity = tmc_contact_new (name, protocol);
		tmc_entity_pool_add (contacts, entity);
	}

	return entity;
}

TmcEntity *
tmc_ensure_channel (TpChannel *channel)
{
	TmcEntityPool *channels = tmc_entity_pool_channels_get ();
	TmcEntity *entity;
	const gchar *name;

	name = tp_channel_get_identifier (channel);
	entity = tmc_entity_pool_lookup (channels, name);

	if (!entity) {
		TpConnection *connection;
		const gchar *protocol;
		TpContact *peer;

		peer = tp_channel_get_target_contact (channel);

		if (peer) {
			/* Private conversation */
			entity = tmc_conversation_new (TMC_CONTACT (tmc_ensure_contact (peer)));
		} else {
			/* Room */
			connection = tp_channel_get_connection (channel);
			protocol = tp_connection_get_protocol_name (connection);
			entity = tmc_room_new (name, protocol);
		}

		tmc_entity_pool_add (channels, entity);
	}

	return entity;
}

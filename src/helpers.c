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

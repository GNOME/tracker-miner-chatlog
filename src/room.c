#include "room.h"

typedef struct _TmcRoom TmcRoom;

struct _TmcRoom {
	TmcEntity parent_instance;
};

G_DEFINE_TYPE (TmcRoom, tmc_room, TMC_TYPE_ENTITY)

static void
tmc_room_class_init (TmcRoomClass *klass)
{
}

static void
tmc_room_init (TmcRoom *room)
{
}

TmcEntity *
tmc_room_new (const gchar *name,
              const gchar *protocol)
{
	return g_object_new (TMC_TYPE_ROOM,
			     "name", name,
	                     "protocol", protocol,
			     NULL);
}

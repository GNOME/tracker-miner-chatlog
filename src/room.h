#ifndef __TMC_ROOM_H__
#define __TMC_ROOM_H__

#include "entity.h"

#define TMC_TYPE_ROOM (tmc_room_get_type())

G_DECLARE_FINAL_TYPE (TmcRoom, tmc_room, TMC, ROOM, TmcEntity)

TmcEntity * tmc_room_new (const gchar *name,
                          const gchar *protocol);

#endif /* __TMC_ROOM_H__ */

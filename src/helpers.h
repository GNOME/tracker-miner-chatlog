#ifndef __TMC_HELPERS_H__
#define __TMC_HELPERS_H__

#include <telepathy-glib/telepathy-glib.h>
#include "entity.h"

TmcEntity * tmc_ensure_contact (TpContact *contact);
TmcEntity * tmc_ensure_channel (TpChannel *channel);

#endif /* __TMC_HELPERS_H__ */

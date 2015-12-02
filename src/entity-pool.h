#ifndef __TMC_ENTITY_POOL_H__
#define __TMC_ENTITY_POOL_H__

#include <glib-object.h>
#include "entity.h"

#define TMC_TYPE_ENTITY_POOL (tmc_entity_pool_get_type())

G_DECLARE_FINAL_TYPE (TmcEntityPool, tmc_entity_pool, TMC, ENTITY_POOL, GObject)

TmcEntityPool * tmc_entity_pool_contacts_get (void);
TmcEntityPool * tmc_entity_pool_channels_get (void);

void            tmc_entity_pool_add          (TmcEntityPool *pool,
					      TmcEntity     *entity);
void            tmc_entity_pool_remove       (TmcEntityPool *pool,
					      TmcEntity     *entity);

TmcEntity     * tmc_entity_pool_lookup       (TmcEntityPool *pool,
					      const gchar   *name);

#endif /* __TMC_ENTITY_POOL_H__ */

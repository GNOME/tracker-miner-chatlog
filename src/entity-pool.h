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

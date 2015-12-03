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

#ifndef __TMC_ENTITY_H__
#define __TMC_ENTITY_H__

#include <glib-object.h>

#define TMC_TYPE_ENTITY (tmc_entity_get_type())

G_DECLARE_DERIVABLE_TYPE (TmcEntity, tmc_entity, TMC, ENTITY, GObject)

struct _TmcEntityClass {
	GObjectClass parent_class;
};

const gchar * tmc_entity_get_name     (TmcEntity *entity);
const gchar * tmc_entity_get_protocol (TmcEntity *entity);

#endif /* __TMC_ENTITY_H__ */

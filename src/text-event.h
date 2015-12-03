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

#ifndef __TMC_TEXT_EVENT_H__
#define __TMC_TEXT_EVENT_H__

#include "glib-object.h"
#include "entity.h"

#define TMC_TYPE_TEXT_EVENT (tmc_text_event_get_type())

G_DECLARE_FINAL_TYPE (TmcTextEvent, tmc_text_event, TMC, TEXT_EVENT, GObject)

TmcTextEvent * tmc_text_event_new           (TmcEntity    *channel,
					     TmcEntity    *from,
					     GList        *to,
					     const gchar  *text,
					     gint64        timestamp);

TmcEntity    * tmc_text_event_get_channel (TmcTextEvent *event);
TmcEntity    * tmc_text_event_get_from    (TmcTextEvent *event);
GList        * tmc_text_event_get_to      (TmcTextEvent *event);

const gchar  * tmc_text_event_get_text      (TmcTextEvent *event);
gint64         tmc_text_event_get_timestamp (TmcTextEvent *event);

#endif /* __TMC_ROOM_H__ */

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

#ifndef __TMC_OBSERVER_H__
#define __TMC_OBSERVER_H__

#include <telepathy-glib/telepathy-glib.h>
#include "client-factory.h"

#define TMC_TYPE_OBSERVER         (tmc_observer_get_type())
#define TMC_OBSERVER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TMC_TYPE_OBSERVER, TmcObserver))
#define TMC_OBSERVER_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c), TMC_TYPE_OBSERVER, TmcObserverClass))
#define TMC_IS_OBSERVER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TMC_TYPE_OBSERVER))
#define TMC_IS_OBSERVER_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c),  TMC_TYPE_OBSERVER))
#define TMC_OBSERVER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TMC_TYPE_OBSERVER, TmcObserverClass))

typedef struct TmcObserver TmcObserver;
typedef struct TmcObserverClass TmcObserverClass;

struct TmcObserver {
	TpBaseClient parent_instance;
};

struct TmcObserverClass {
	TpBaseClientClass parent_class;
};

GType         tmc_observer_get_type (void) G_GNUC_CONST;

TmcObserver * tmc_observer_new      (TmcClientFactory *factory);

#endif /* __TMC_OBSERVER_H__ */

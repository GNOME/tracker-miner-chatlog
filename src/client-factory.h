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

#ifndef __TMC_CLIENT_FACTORY_H__
#define __TMC_CLIENT_FACTORY_H__

#include <telepathy-glib/telepathy-glib.h>

#define TMC_TYPE_CLIENT_FACTORY         (tmc_client_factory_get_type())
#define TMC_CLIENT_FACTORY(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TMC_TYPE_CLIENT_FACTORY, TmcClientFactory))
#define TMC_CLIENT_FACTORY_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c), TMC_TYPE_CLIENT_FACTORY, TmcClientFactoryClass))
#define TMC_IS_CLIENT_FACTORY(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TMC_TYPE_CLIENT_FACTORY))
#define TMC_IS_CLIENT_FACTORY_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c),  TMC_TYPE_CLIENT_FACTORY))
#define TMC_CLIENT_FACTORY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TMC_TYPE_CLIENT_FACTORY, TmcClientFactoryClass))

typedef struct TmcClientFactory TmcClientFactory;
typedef struct TmcClientFactoryClass TmcClientFactoryClass;
typedef struct TmcClientFactoryPrivate TmcClientFactoryPrivate;

struct TmcClientFactory {
	TpSimpleClientFactory parent_instance;
};

struct TmcClientFactoryClass {
	TpSimpleClientFactoryClass parent_class;
};

GType              tmc_client_factory_get_type (void) G_GNUC_CONST;

TmcClientFactory * tmc_client_factory_new      (TpDBusDaemon *dbus);

#endif /* __TMC_CLIENT_FACTORY_H__ */

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

#include "contact.h"

typedef struct _TmcContact TmcContact;

struct _TmcContact {
	TmcEntity parent_instance;
};

G_DEFINE_TYPE (TmcContact, tmc_contact, TMC_TYPE_ENTITY)

static void
tmc_contact_class_init (TmcContactClass *klass)
{
}

static void
tmc_contact_init (TmcContact *contact)
{
}

TmcEntity *
tmc_contact_new (const gchar *nickname,
                 const gchar *protocol,
                 const gchar *identifier)
{
	return g_object_new (TMC_TYPE_CONTACT,
			     "name", nickname,
			     "protocol", protocol,
			     "identifier", identifier,
			     NULL);
}

TmcEntity *
tmc_contact_self_get (void)
{
	static TmcEntity *self = NULL;

	if (G_UNLIKELY (!self)) {
		self = g_object_new (TMC_TYPE_CONTACT, NULL);
	}

	return self;
}

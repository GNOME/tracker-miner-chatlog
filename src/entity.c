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

#include "entity.h"

typedef struct _TmcEntityPrivate TmcEntityPrivate;

struct _TmcEntityPrivate {
	gchar *name;
	gchar *protocol;
	gchar *identifier;
};

enum {
	PROP_0,
	PROP_NAME,
	PROP_PROTOCOL,
	PROP_IDENTIFIER,
	PROP_LAST
};

static GParamSpec *props[PROP_LAST] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE (TmcEntity, tmc_entity, G_TYPE_OBJECT)

static void
tmc_entity_finalize (GObject *object)
{
	TmcEntity *entity = TMC_ENTITY (object);
	TmcEntityPrivate *priv = tmc_entity_get_instance_private (entity);

	g_clear_pointer (&priv->name, g_free);
	g_clear_pointer (&priv->protocol, g_free);
	g_clear_pointer (&priv->identifier, g_free);

	G_OBJECT_CLASS (tmc_entity_parent_class)->finalize (object);
}

static void
tmc_entity_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	TmcEntity *entity = TMC_ENTITY (object);
	TmcEntityPrivate *priv = tmc_entity_get_instance_private (entity);

	switch (prop_id) {
	case PROP_NAME:
		g_clear_pointer (&priv->name, g_free);
		priv->name = g_value_dup_string (value);
		break;
	case PROP_PROTOCOL:
		g_clear_pointer (&priv->protocol, g_free);
		priv->protocol = g_value_dup_string (value);
		break;
	case PROP_IDENTIFIER:
		g_clear_pointer (&priv->identifier, g_free);
		priv->identifier = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
tmc_entity_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	TmcEntity *entity = TMC_ENTITY (object);
	TmcEntityPrivate *priv = tmc_entity_get_instance_private (entity);

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_PROTOCOL:
		g_value_set_string (value, priv->protocol);
		break;
	case PROP_IDENTIFIER:
		g_value_set_string (value, priv->identifier);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
tmc_entity_class_init (TmcEntityClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_entity_finalize;
	object_class->set_property = tmc_entity_set_property;
	object_class->get_property = tmc_entity_get_property;

	props[PROP_NAME] =
		g_param_spec_string ("name",
				     "Name",
				     "Name",
				     NULL,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY);
	props[PROP_PROTOCOL] =
		g_param_spec_string ("protocol",
				     "Protocol",
				     "Protocol",
				     NULL,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY);
	props[PROP_IDENTIFIER] =
		g_param_spec_string ("identifier",
				     "Identifier",
				     "Identifier",
				     NULL,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties (object_class, PROP_LAST, props);
}

static void
tmc_entity_init (TmcEntity *entity)
{
}

const gchar *
tmc_entity_get_name (TmcEntity *entity)
{
	TmcEntityPrivate *priv;

	g_return_val_if_fail (TMC_IS_ENTITY (entity), NULL);

	priv = tmc_entity_get_instance_private (entity);

	return priv->name;
}

const gchar *
tmc_entity_get_protocol (TmcEntity *entity)
{
	TmcEntityPrivate *priv;

	g_return_val_if_fail (TMC_IS_ENTITY (entity), NULL);

	priv = tmc_entity_get_instance_private (entity);

	return priv->protocol;
}

const gchar *
tmc_entity_get_identifier (TmcEntity *entity)
{
	TmcEntityPrivate *priv;

	g_return_val_if_fail (TMC_IS_ENTITY (entity), NULL);

	priv = tmc_entity_get_instance_private (entity);

	return priv->identifier;
}

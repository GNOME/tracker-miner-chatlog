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

#include "text-event.h"

typedef struct _TmcTextEvent TmcTextEvent;
typedef struct _TmcTextEventPrivate TmcTextEventPrivate;

struct _TmcTextEvent {
	GObject parent_instance;
};

struct _TmcTextEventPrivate {
	TmcEntity *channel;
	TmcEntity *from;
	GList *to;
	gchar *text;
	gint64 timestamp;
};

enum {
	PROP_0,
	PROP_CHANNEL,
	PROP_FROM,
	PROP_TEXT,
	PROP_TIMESTAMP,
	PROP_LAST
};

static GParamSpec *props[PROP_LAST] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE (TmcTextEvent, tmc_text_event, G_TYPE_OBJECT)

static void
tmc_text_event_finalize (GObject *object)
{
	TmcTextEvent *event = TMC_TEXT_EVENT (object);
	TmcTextEventPrivate *priv = tmc_text_event_get_instance_private (event);

	g_clear_object (&priv->channel);
	g_clear_object (&priv->from);
	g_clear_pointer (&priv->text, g_free);

	g_list_foreach (priv->to, (GFunc) g_object_unref, NULL);
	g_list_free (priv->to);

	G_OBJECT_CLASS (tmc_text_event_parent_class)->finalize (object);
}

static void
tmc_text_event_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
	TmcTextEvent *event = TMC_TEXT_EVENT (object);
	TmcTextEventPrivate *priv = tmc_text_event_get_instance_private (event);

	switch (prop_id) {
	case PROP_CHANNEL:
		g_set_object (&priv->channel, g_value_dup_object (value));
		break;
	case PROP_FROM:
		g_set_object (&priv->from, g_value_dup_object (value));
		break;
	case PROP_TEXT:
		g_clear_pointer (&priv->text, g_free);
		priv->text = g_value_dup_string (value);
		break;
	case PROP_TIMESTAMP:
		priv->timestamp = g_value_get_int64 (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
tmc_text_event_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
	TmcTextEvent *event = TMC_TEXT_EVENT (object);
	TmcTextEventPrivate *priv = tmc_text_event_get_instance_private (event);

	switch (prop_id) {
	case PROP_CHANNEL:
		g_value_set_object (value, priv->channel);
		break;
	case PROP_FROM:
		g_value_set_object (value, priv->from);
		break;
	case PROP_TEXT:
		g_value_set_string (value, priv->text);
		break;
	case PROP_TIMESTAMP:
		g_value_set_int64 (value, priv->timestamp);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
tmc_text_event_class_init (TmcTextEventClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_text_event_finalize;
	object_class->set_property = tmc_text_event_set_property;
	object_class->get_property = tmc_text_event_get_property;

	props[PROP_CHANNEL] =
		g_param_spec_object ("channel",
				     "Channel",
				     "Channel",
				     TMC_TYPE_ENTITY,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY);
	props[PROP_FROM] =
		g_param_spec_object ("from",
				     "From",
				     "From",
				     TMC_TYPE_ENTITY,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY);
	props[PROP_TEXT] =
		g_param_spec_string ("text",
				     "Text",
				     "Text",
				     NULL,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY);
	props[PROP_TIMESTAMP] =
		g_param_spec_int64 ("timestamp",
				    "Timestamp",
				    "Timestamp",
				    G_MININT64, G_MAXINT64, 0,
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties (object_class, PROP_LAST, props);
}

static void
tmc_text_event_init (TmcTextEvent *event)
{
}

static void
tmc_text_event_set_to (TmcTextEvent *event,
		       GList        *to)
{
	TmcTextEventPrivate *priv;

	priv = tmc_text_event_get_instance_private (event);

	g_list_foreach (to, (GFunc) g_object_ref, NULL);
	priv->to = g_list_copy (to);
}

TmcTextEvent *
tmc_text_event_new (TmcEntity   *channel,
		    TmcEntity   *from,
		    GList       *to,
		    const gchar *text,
		    gint64       timestamp)
{
	TmcTextEvent *event;

	event = g_object_new (TMC_TYPE_TEXT_EVENT,
			      "channel", channel,
			      "from", from,
			      "text", text,
			      "timestamp", timestamp,
			      NULL);

	tmc_text_event_set_to (event, to);

	return event;
}

TmcEntity *
tmc_text_event_get_channel (TmcTextEvent *event)
{
	TmcTextEventPrivate *priv;

	g_return_val_if_fail (TMC_IS_TEXT_EVENT (event), NULL);

	priv = tmc_text_event_get_instance_private (event);

	return priv->channel;
}

TmcEntity *
tmc_text_event_get_from (TmcTextEvent *event)
{
	TmcTextEventPrivate *priv;

	g_return_val_if_fail (TMC_IS_TEXT_EVENT (event), NULL);

	priv = tmc_text_event_get_instance_private (event);

	return priv->from;
}

GList *
tmc_text_event_get_to (TmcTextEvent *event)
{
	TmcTextEventPrivate *priv;

	g_return_val_if_fail (TMC_IS_TEXT_EVENT (event), NULL);

	priv = tmc_text_event_get_instance_private (event);

	return priv->to;
}


const gchar *
tmc_text_event_get_text (TmcTextEvent *event)
{
	TmcTextEventPrivate *priv;

	g_return_val_if_fail (TMC_IS_TEXT_EVENT (event), NULL);

	priv = tmc_text_event_get_instance_private (event);

	return priv->text;
}

gint64
tmc_text_event_get_timestamp (TmcTextEvent *event)
{
	TmcTextEventPrivate *priv;

	g_return_val_if_fail (TMC_IS_TEXT_EVENT (event), 0);

	priv = tmc_text_event_get_instance_private (event);

	return priv->timestamp;
}

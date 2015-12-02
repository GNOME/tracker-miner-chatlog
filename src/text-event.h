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

#ifndef __TMC_TEXT_CHANNEL_H__
#define __TMC_TEXT_CHANNEL_H__

#include "client-factory.h"
#include "entity.h"

#define TMC_TYPE_TEXT_CHANNEL         (tmc_text_channel_get_type())
#define TMC_TEXT_CHANNEL(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TMC_TYPE_TEXT_CHANNEL, TmcTextChannel))
#define TMC_TEXT_CHANNEL_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c), TMC_TYPE_TEXT_CHANNEL, TmcTextChannelClass))
#define TMC_IS_TEXT_CHANNEL(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TMC_TYPE_TEXT_CHANNEL))
#define TMC_IS_TEXT_CHANNEL_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c),  TMC_TYPE_TEXT_CHANNEL))
#define TMC_TEXT_CHANNEL_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TMC_TYPE_TEXT_CHANNEL, TmcTextChannelClass))

typedef struct TmcTextChannel TmcTextChannel;
typedef struct TmcTextChannelClass TmcTextChannelClass;

struct TmcTextChannel {
	TpTextChannel parent_instance;
};

struct TmcTextChannelClass {
	TpTextChannelClass parent_class;
};

GType            tmc_text_channel_get_type (void) G_GNUC_CONST;

TmcTextChannel * tmc_text_channel_new      (TpSimpleClientFactory  *factory,
					    TpConnection           *conn,
					    const gchar            *object_path,
					    const GHashTable       *tp_chan_props,
					    GError                **error);

TmcEntity      * tmc_text_channel_lookup_contact (TmcTextChannel *channel,
						  const gchar    *name);

#endif /* __TMC_TEXT_CHANNEL_H__ */

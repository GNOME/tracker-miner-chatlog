#ifndef __TMC_CONVERSATION_H__
#define __TMC_CONVERSATION_H__

#include "contact.h"

#define TMC_TYPE_CONVERSATION (tmc_conversation_get_type())

G_DECLARE_FINAL_TYPE (TmcConversation, tmc_conversation, TMC, CONVERSATION, TmcEntity)

TmcEntity  * tmc_conversation_new      (TmcContact *peer);
TmcContact * tmc_conversation_get_peer (TmcConversation *conversation);

#endif /* __TMC_CONVERSATION_H__ */

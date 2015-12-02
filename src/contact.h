#ifndef __TMC_CONTACT_H__
#define __TMC_CONTACT_H__

#include "entity.h"

#define TMC_TYPE_CONTACT (tmc_contact_get_type())

G_DECLARE_FINAL_TYPE (TmcContact, tmc_contact, TMC, CONTACT, TmcEntity)

TmcEntity * tmc_contact_new      (const gchar *nickname,
                                  const gchar *protocol);
TmcEntity * tmc_contact_self_get (void);

#endif /* __TMC_CONTACT_H__ */

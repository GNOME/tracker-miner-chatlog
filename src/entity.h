#ifndef __TMC_ENTITY_H__
#define __TMC_ENTITY_H__

#include <glib-object.h>

#define TMC_TYPE_ENTITY (tmc_entity_get_type())

G_DECLARE_DERIVABLE_TYPE (TmcEntity, tmc_entity, TMC, ENTITY, GObject)

struct _TmcEntityClass {
	GObjectClass parent_class;
};

const gchar * tmc_entity_get_name     (TmcEntity *entity);
const gchar * tmc_entity_get_protocol (TmcEntity *entity);

#endif /* __TMC_ENTITY_H__ */

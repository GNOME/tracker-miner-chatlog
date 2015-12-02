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
                 const gchar *protocol)
{
	return g_object_new (TMC_TYPE_CONTACT,
			     "name", nickname,
			     "protocol", protocol,
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

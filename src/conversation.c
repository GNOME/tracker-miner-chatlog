#include "conversation.h"

typedef struct _TmcConversation TmcConversation;
typedef struct _TmcConversationPrivate TmcConversationPrivate;

struct _TmcConversation {
	TmcEntity parent_instance;
};

struct _TmcConversationPrivate {
	TmcContact *peer;
};

enum {
	PROP_0,
	PROP_PEER,
	PROP_LAST
};

static GParamSpec *props[PROP_LAST] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE (TmcConversation, tmc_conversation, TMC_TYPE_ENTITY)

static void
tmc_conversation_finalize (GObject *object)
{
	TmcConversation *conversation = TMC_CONVERSATION (object);
	TmcConversationPrivate *priv = tmc_conversation_get_instance_private (conversation);

	g_clear_object (&priv->peer);

	G_OBJECT_CLASS (tmc_conversation_parent_class)->finalize (object);
}

static void
tmc_conversation_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	TmcConversation *conversation = TMC_CONVERSATION (object);
	TmcConversationPrivate *priv = tmc_conversation_get_instance_private (conversation);

	switch (prop_id) {
	case PROP_PEER:
		g_clear_object (&priv->peer);
		priv->peer = g_value_dup_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
tmc_conversation_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	TmcConversation *conversation = TMC_CONVERSATION (object);
	TmcConversationPrivate *priv = tmc_conversation_get_instance_private (conversation);

	switch (prop_id) {
	case PROP_PEER:
		g_value_set_object (value, priv->peer);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
tmc_conversation_class_init (TmcConversationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_conversation_finalize;
	object_class->set_property = tmc_conversation_set_property;
	object_class->get_property = tmc_conversation_get_property;

	props[PROP_PEER] =
		g_param_spec_object ("peer",
				     "Peer",
				     "Peer",
		                     TMC_TYPE_CONTACT,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties (object_class, PROP_LAST, props);
}

static void
tmc_conversation_init (TmcConversation *conversation)
{
}

TmcEntity *
tmc_conversation_new (TmcContact *peer)
{
	return g_object_new (TMC_TYPE_CONVERSATION,
	                     "name", tmc_entity_get_name (TMC_ENTITY (peer)),
	                     "protocol", tmc_entity_get_protocol (TMC_ENTITY (peer)),
	                     "peer", peer,
	                     NULL);
}

TmcContact *
tmc_conversation_get_peer (TmcConversation *conversation)
{
	TmcConversationPrivate *priv;

	g_return_val_if_fail (TMC_IS_CONVERSATION (conversation), NULL);

	priv = tmc_conversation_get_instance_private (conversation);
	return priv->peer;
}

#include "entity-pool.h"

typedef struct _TmcEntityPool TmcEntityPool;
typedef struct _TmcEntityPoolPrivate TmcEntityPoolPrivate;

struct _TmcEntityPool {
	GObject parent_instance;
};

struct _TmcEntityPoolPrivate {
	GHashTable *entities;
};

G_DEFINE_TYPE_WITH_PRIVATE (TmcEntityPool, tmc_entity_pool, G_TYPE_OBJECT)

static void
tmc_entity_pool_finalize (GObject *object)
{
	TmcEntityPool *pool = TMC_ENTITY_POOL (object);
	TmcEntityPoolPrivate *priv = tmc_entity_pool_get_instance_private (pool);

	g_hash_table_destroy (priv->entities);

	G_OBJECT_CLASS (tmc_entity_pool_parent_class)->finalize (object);
}

static void
tmc_entity_pool_class_init (TmcEntityPoolClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tmc_entity_pool_finalize;
}

static void
tmc_entity_pool_init (TmcEntityPool *pool)
{
	TmcEntityPoolPrivate *priv = tmc_entity_pool_get_instance_private (pool);

	priv->entities = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,
						(GDestroyNotify) g_object_unref);
}

static TmcEntityPool *
tmc_entity_pool_new (void)
{
	return g_object_new (TMC_TYPE_ENTITY_POOL, NULL);
}

void
tmc_entity_pool_add (TmcEntityPool *pool,
		     TmcEntity     *entity)
{
	TmcEntityPoolPrivate *priv;

	g_return_if_fail (TMC_IS_ENTITY_POOL (pool));
	g_return_if_fail (TMC_IS_ENTITY (entity));

	priv = tmc_entity_pool_get_instance_private (pool);

	g_hash_table_insert (priv->entities,
			     (gpointer) tmc_entity_get_name (entity),
			     entity);
}

void
tmc_entity_pool_remove (TmcEntityPool *pool,
			TmcEntity     *entity)
{
	TmcEntityPoolPrivate *priv;

	g_return_if_fail (TMC_IS_ENTITY_POOL (pool));
	g_return_if_fail (TMC_IS_ENTITY (entity));

	priv = tmc_entity_pool_get_instance_private (pool);

	g_hash_table_remove (priv->entities,
			     tmc_entity_get_name (entity));
}

TmcEntity *
tmc_entity_pool_lookup (TmcEntityPool *pool,
			const gchar   *name)
{
	TmcEntityPoolPrivate *priv;

	g_return_val_if_fail (TMC_IS_ENTITY_POOL (pool), NULL);
	g_return_val_if_fail (name != NULL, NULL);

	priv = tmc_entity_pool_get_instance_private (pool);

	return g_hash_table_lookup (priv->entities, name);
}

TmcEntityPool *
tmc_entity_pool_contacts_get (void)
{
	static TmcEntityPool *pool = NULL;

	if (G_UNLIKELY (!pool)) {
		pool = tmc_entity_pool_new ();
	}

	return pool;
}

TmcEntityPool *
tmc_entity_pool_channels_get (void)
{
	static TmcEntityPool *pool = NULL;

	if (G_UNLIKELY (!pool)) {
		pool = tmc_entity_pool_new ();
	}

	return pool;
}

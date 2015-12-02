#ifndef __TMC_MINER_H__
#define __TMC_MINER_H__

#include <libtracker-miner/tracker-miner.h>

#define TMC_TYPE_MINER         (tmc_miner_get_type())
#define TMC_MINER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TMC_TYPE_MINER, TmcMiner))
#define TMC_MINER_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c), TMC_TYPE_MINER, TmcMinerClass))
#define TMC_IS_MINER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TMC_TYPE_MINER))
#define TMC_IS_MINER_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c),  TMC_TYPE_MINER))
#define TMC_MINER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TMC_TYPE_MINER, TmcMinerClass))

typedef struct TmcMiner TmcMiner;
typedef struct TmcMinerClass TmcMinerClass;

struct TmcMiner {
	TrackerMiner parent_instance;
};

struct TmcMinerClass {
	TrackerMinerClass parent_class;
};

GType          tmc_miner_get_type (void) G_GNUC_CONST;

TrackerMiner * tmc_miner_new      (GError **error);

#endif /* __TMC_MINER_H__ */

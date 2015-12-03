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

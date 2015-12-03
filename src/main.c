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

#include "miner.h"

int
main (int   argc,
      char *argv[])
{
	TrackerMiner *miner;
	GMainLoop *loop;
	GError *error = NULL;

	miner = tmc_miner_new (&error);

	if (error) {
		g_critical ("Miner could not be initialized: %s", error->message);
		g_error_free (error);
		return -1;
	}

	loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (loop);
	return 0;
}

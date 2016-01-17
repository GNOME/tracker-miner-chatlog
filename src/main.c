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

#include <glib-unix.h>
#include "miner.h"

static gboolean
signal_handler (gpointer user_data)
{
	GMainLoop *loop = user_data;
	static gboolean in_loop = FALSE;

	/* Die if we get re-entrant signals handler calls */
	if (in_loop) {
		_exit (EXIT_FAILURE);
	}

	in_loop = TRUE;
	g_main_loop_quit (loop);

	return G_SOURCE_CONTINUE;
}

static void
initialize_signal_handler (GMainLoop *loop)
{
#ifndef G_OS_WIN32
	g_unix_signal_add (SIGTERM, signal_handler, loop);
	g_unix_signal_add (SIGINT, signal_handler, loop);
#endif /* G_OS_WIN32 */
}

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
	initialize_signal_handler (loop);
	g_main_loop_run (loop);
	return 0;
}

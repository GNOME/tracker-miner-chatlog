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

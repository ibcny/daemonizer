/*
 * ----------------------------------------------------------------------------
 * daemonizer
 * A General purpose Unix daemon trace
 *
 * Copyright (C) 2012 Ibrahim Can Yuce <canyuce@gmail.com>
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 */

#include "lock.h"
#include "daemon.h"

int main(int argc, char* argv[]) {
	if (getuid() != 0) {
		fprintf(stderr, "%s:%d Error: You must be root to continue!\n", LOG_OPT
				);
		exit(2);
	}

	atexit(finalize);

	initLogging();

	signal(SIGTERM, signalHandler); /* termination signal from kill */
	signal(SIGINT, signalHandler); /* termination signal from kill */
	signal(SIGSEGV, signalHandler); /* termination signal from kill */

	daemonize();

	/*
	 * at most 1 daemon is allowed to execute any time
	 */
	if (tryLock()) {
		syslog(LOG_CRIT, "%s:%d can't lock the file \"%s\": %m", LOG_OPT,
				LOCKFILE);
		exit(3);
	}

	while (1)
		sleep(1); /* run */

	return 0;
}

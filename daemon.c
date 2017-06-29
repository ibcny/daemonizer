/*
 * ----------------------------------------------------------------------------
 * daemonizer 
 * A General purpose Unix daemon trace
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


#include  "daemon.h"

void finalize(void) {
	closelog();
}

void signalHandler(int sig) /*signal handler function */
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
	case SIGSEGV:
		/*finalize the server*/
		syslog(LOG_INFO, "%s:%d Daemon is passing away...", LOG_OPT);
		exit(0);
		break;
	}
}

void initLogging(void) {
	char buffer[70];
	int fd;
	BOOL restartNeeded = FALSE;
	sprintf(buffer, CHECK_SYSLOG_ENTRY);

	/* /bin/sh -c "buffer" */
	if (system(buffer) > 0) {
		/* open syslog conf file */
		if ((fd = open(SYSLOG_CONF_FILE, O_RDWR | O_APPEND | O_SYNC)) < 0) {
			perror("Open Error");
			close(fd);
			exit(1);
		}

		char toBeAppended[] = "\n# Log all daemon messages to file\n"
				"local3.*";

		char buffer[sizeof(toBeAppended) + sizeof(LOG_FILE) + 50];

		sprintf(buffer, "%s \t\t -%s \n\n", toBeAppended, LOG_FILE);

		/* append new entry to syslog.conf */
		if (write(fd, buffer, strlen(buffer)) != strlen(buffer)) {
			perror("Write Error");
			close(fd);
			exit(1);
		}

		restartNeeded = TRUE; // need to restart..
		close(fd);
	}

	/* Check whether the daemon log file exists or not */
	struct stat st;
	if (stat(LOG_FILE, &st) != 0) { //if not create it! 
		if ((fd = creat(LOG_FILE, O_RDWR | O_CREAT)) < 0) {
			perror("File Creation Error");
			close(fd);
			exit(1);
		}
		//successful creation needs the daemon to be restarted
		restartNeeded = TRUE;
		close(fd);
	}

	/* restart finally */
	if (restartNeeded && system(RESTART_SYSLOGD) > 0) {
		perror("Syslogd restart error");
		exit(1);
	}

	openlog("My_Daemon", LOG_CONS | LOG_PID, LOG_LOCAL3);
	syslog(LOG_INFO, "%s:%d Attached to syslogd...", LOG_OPT);

}

void daemonize(void) {
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	syslog(LOG_INFO, "%s:%d Demonizing has started...", LOG_OPT);

	/*
	 * Clear file creation mask.
	 */
	umask(0);

	/*
	 * Get maximum number of file descriptors.
	 */
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		syslog(LOG_ERR,
				"%s:%d get maximum number of file descriptors error: %m",
				LOG_OPT);
		exit(1);
	}

	/*
	 * Become a session leader to lose controlling TTY.
	 */
	if ((pid = fork()) < 0) {
		syslog(LOG_ERR, "%s:%d forking error for : %m", LOG_OPT);
		exit(1);
	}
	/* kill the parent in order to become orphan and to be adopted by a rich stepdad :)) yes it is the "init"*/
	else if (pid != 0)
		exit(0);

	setsid();

	/*
	 * Ensure future opens won't allocate controlling TTYs.
	 */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		syslog(LOG_ERR, "%s:%d signal closure error : %m", LOG_OPT);
		exit(1);
	}

	if ((pid = fork()) < 0) {
		syslog(LOG_ERR, "%s:%d forking error for : %m", LOG_OPT);
		exit(1);
	}
	/* kill the parent in order to become orphan and to be adopted by a rich stepdad :)) yes it is the "init"*/
	else if (pid != 0)
		exit(0);

	syslog(LOG_INFO, "%s:%d Changing current working directory to `/`", LOG_OPT
			);
	/* 
	 *Change the working directory to the root in order to prevent filesystems from being unmounted
	 */
	if (chdir("/") < 0) {
		syslog(LOG_ERR, "%s:%d Directory change error: %m", LOG_OPT);
		exit(1);
	}

	syslog(LOG_INFO, "%s:%d Closing open file descriptors..", LOG_OPT);
	/*
	 * Close all open file descriptors.
	 */
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++)
		close(i);

	/*
	 * Attach file descriptors 0, 1, and 2 to /dev/null.
	 */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

}

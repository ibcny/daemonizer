/*
 * ----------------------------------------------------------------------------
 * daemonizer
 * Unix daemon template implementation
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

static pid_t lock_test(int fd, int type, off_t offset, int whence, off_t len) {
	struct flock lock;
	lock.l_type = type; /* F_RDLCK or F_WRLCK */
	lock.l_start = offset; /* byte offset, relative to l_whence */
	lock.l_whence = whence; /* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len; /* #bytes (0 means to EOF) */

	if (fcntl(fd, F_GETLK, &lock) < 0)
		syslog(LOG_ERR, "%s:%d fcntl error %s: %m", LOG_OPT, LOCKFILE);

	if (lock.l_type == F_UNLCK)
		return (0); /* false, region isn't locked by another proc */

	return (lock.l_pid); /* true, return pid of lock owner */
}

static int lockfile(int fd) {
	if (!lock_test(fd, F_WRLCK, 0, SEEK_SET, 0)) {
		struct flock fl;

		fl.l_type = F_WRLCK;
		fl.l_start = 0;
		fl.l_whence = SEEK_SET;
		fl.l_len = 0;

		return (fcntl(fd, F_SETLK, &fl));
	}

	errno = EAGAIN;
	return -1;
}

int tryLock() {
	int fd;
	char buf[16];

	syslog(LOG_INFO, "%s:%d Trying to lock...", LOG_OPT);

	fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		syslog(LOG_ERR, "%s:%d can't open %s: %m", LOG_OPT, LOCKFILE);
		exit(1);
	}

	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			close(fd);
			return (1);
		}

		syslog(LOG_ERR, "%s:%d another error occured %s: %m", LOG_OPT,
				LOCKFILE);
		exit(1);
	}

	syslog(LOG_INFO, "%s:%d Truncating and printing to/from the log file\n",
			LOG_OPT);

	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long) getpid());
	write(fd, buf, strlen(buf) + 1);

	return (0);
}

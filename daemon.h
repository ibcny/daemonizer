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

#ifndef DAEMON_H
#define DAEMON_H

#include "lock.h"

#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <signal.h>

#define SYSLOG_CONF_FILE "/etc/syslog.conf"
#define IDENT "MyDaemon"
#define LOG_FILE "/var/log/daemon.log"

#define RESTART_SYSLOGD "/etc/init.d/sysklogd restart 2> /dev/null"
#define CHECK_SYSLOG_ENTRY "grep -i %s %s > /dev/null", IDENT, SYSLOG_CONF_FILE 

void
finalize(void);

void
signalHandler(int);

void
initLogging(void);

void
daemonize(void);

#endif

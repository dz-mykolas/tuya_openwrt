#ifndef DAEMON_UTILS_H
#define DAEMON_UTILS_H

#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>

#define LOGS_ERROR   3
#define LOGS_WARNING 4
#define LOGS_NOTICE  5

#define BD_NO_CHDIR	  01 /* Don't chdir ("/") */
#define BD_NO_CLOSE_FILES 02 /* Don't close all open files */
#define BD_NO_REOPEN_STD_FDS                                                                                 \
	04 /* Don't reopen stdin, stdout, and stderr
                                   to /dev/null */
#define BD_NO_UMASK0 010 /* Don't do a umask(0) */
#define BD_MAX_CLOSE                                                                                         \
	8192 /* Max file descriptors to close if
                                   sysconf(_SC_OPEN_MAX) is indeterminate */

// returns 0 on success -1 on error
int become_daemon(int flags);

unsigned long int ram_get_free();
void log_event(int type, const char *format, ...);

#endif
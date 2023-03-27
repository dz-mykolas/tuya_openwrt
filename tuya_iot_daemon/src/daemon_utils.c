#include <syslog.h>
#include "daemon_utils.h"

int become_daemon(int flags)
{
    switch(fork()) {
        case -1: return -1;
        case 0: break;
        default: _exit(EXIT_SUCCESS);
    }

    if(setsid() == -1)
        return -1;

    switch(fork()) {
        case -1: return -1;
        case 0: break;
        default: _exit(EXIT_SUCCESS);
    }
    if(!(flags & BD_NO_UMASK0))
        umask(0);

    if(!(flags & BD_NO_CHDIR))
        chdir("/");

    int fd;
    if(!(flags & BD_NO_CLOSE_FILES)) {
        int maxfd;
        maxfd = sysconf(_SC_OPEN_MAX);
        if(maxfd == -1)
            maxfd = BD_MAX_CLOSE;
        for(fd = 0; fd < maxfd; fd++)
            close(fd);
    }

    if(!(flags & BD_NO_REOPEN_STD_FDS)) {
        close(STDIN_FILENO);
        fd = open("/dev/null", O_RDWR);
        if(fd != STDIN_FILENO)
            return -1;
        if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) 
            return -2;
        if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -3;
    }
    return 0;
}

unsigned long int ram_get_free()
{
    struct sysinfo info;
    sysinfo(&info);
    return (info.freeram/1024/1024);
}

void log_event(int type, char *log)
{   
    setlogmask(LOG_UPTO(LOG_NOTICE));
    openlog("tuya_daemon", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    char buffer[100];
    snprintf(buffer, 100, "%s", log);
    syslog(type, "%s", buffer);
    closelog();
}
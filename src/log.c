#include "../include/log.h"

const char *SYSLOG_NAME = "canreader";

void syslog_init(void) {
    openlog(SYSLOG_NAME, LOG_CONS | LOG_NDELAY, LOG_DAEMON);
    setlogmask(LOG_UPTO(LOG_INFO));
    syslog(LOG_INFO, "[SYSTEM] canreader's log enabled");
}

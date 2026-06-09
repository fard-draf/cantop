#ifndef LOG_H_
#define LOG_H_

#include <errno.h>
#include <string.h>
#include <syslog.h>

extern const char *SYSLOG_NAME;

void syslog_init(void);

#endif // LOG_H_

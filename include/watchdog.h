#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include "common.h"
#include "context.h"
#include "log.h"
#include <sys/timerfd.h>

int watchdog_init(AppContext *ac);
void watchdog_handler(void *ctx);

#endif // WATCHDOG_H_

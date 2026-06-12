#include "../include/watchdog.h"
#include "../include/analyzer.h"
#include "../include/pgns.h"

int watchdog_init(AppContext *ac) {

    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (tfd == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] watchdog/timerfd_create(): %s", strerror(err));
        return -1;
    }

    struct itimerspec ts = {
        .it_interval = {.tv_sec = 1, .tv_nsec = 0},
        .it_value = {.tv_sec = 1, .tv_nsec = 0},
    };

    if (timerfd_settime(tfd, 0, &ts, NULL) == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] watchdog/timerfd_settime(): %s", strerror(err));
        return -1;
    }

    ac->watchdog_fd = tfd;
    return 0;
}

void watchdog_handler(void *ctx) {
    AppContext *ac = ctx;
    uint64_t expirations;
    read(ac->watchdog_fd, &expirations, sizeof(expirations));
    analyzer_update_rate(&ac->an);
    for (uint8_t i = ac->an.pgns.reg.instances_actives_count; i-- > 0;) {
        pgn_update_rate(&ac->an, i);
    }
}

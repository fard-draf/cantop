#include "../include/signals.h"
#include "../include/context.h"
#include "../include/log.h"
#include "sys/signalfd.h"

int sig_init(AppContext *ac) {
    sigset_t mask;

    if (sigemptyset(&mask) == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] sigemptyset(): %s", strerror(err));
        return -1;
    }

    if (sigaddset(&mask, SIGTERM) == -1 || sigaddset(&mask, SIGINT) == -1 ||
        sigaddset(&mask, SIGHUP) == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] sigaddset(): %s", strerror(err));
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] sigprocmask(): %s", strerror(err));
        return -1;
    }

    int sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (sfd == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] signalfd(): %s", strerror(err));
        return -1;
    }

    ac->sig_fd = sfd;

    return 0;
}

void sig_handler(void *ctx) {
    AppContext *ac = ctx;
    struct signalfd_siginfo info;

    while (1) {
        ssize_t n = read(ac->sig_fd, &info, sizeof(info));
        // error handling
        if (n == -1) {
            int err = errno;
            if (err == EAGAIN || err == EWOULDBLOCK) {
                return;
            }
            syslog(LOG_ERR, "[ERR] signalfd/read(): %s", strerror(err));
            running = 0;
            return;
        }
        if (n != sizeof(info)) {
            int err = errno;
            syslog(LOG_ERR, "[ERR] signalfd/partial read(): %s", strerror(err));
            running = 0;
            return;
        }
        // switch
        switch (info.ssi_signo) {
        case SIGTERM: {
            syslog(LOG_INFO, "[INFO] sigterm received, exit..");
            running = 0;
            return;
        }
        case SIGHUP: {
            syslog(LOG_INFO, "[INFO] config toggled, reload..");
            // TODO: id filtering via setsockopt -> reload on runtime
            //  log via stdout too
            return;
        }
        case SIGINT: {
            syslog(LOG_INFO, "[INFO] sigint received, exit..");
            running = 0;
            return;
        }
        default:
            syslog(LOG_WARNING, "[WARN] unexpected signal.. ignore..");
            return;
        }
    }
}

#include "../include/common.h"
#include "../include/context.h"
#include "../include/frame_parser.h"
#include "../include/log.h"
#include "../include/net.h"
#include "../include/poll.h"
#include "../include/signals.h"
#include "../include/ui.h"

volatile sig_atomic_t running = 1;

static void pipeline_handler(void *ctx) {
    AppContext *ac = ctx;
    net_recv(ac);
    frame_reader(ac);
    analyzer_populate(&ac->anlyzr, &ac->cr);
    analyzer_print(&ac->anlyzr);
};

int main(void) {

    int ret = -1, res = 0;
    AppContext ac = {0};

    syslog_init();
    setlogmask(LOG_UPTO(LOG_INFO));

    // net init
    if ((res = net_init(&ac)) < 0) {
        ret = res;
        syslog(LOG_ERR, "can_socket_init error");
        goto cleanup;
    }

    // sig init
    if ((res = sig_init(&ac)) == -1) {
        ret = res;
        syslog(LOG_ERR, "sig_handling_init error");
        goto cleanup;
    }

    // metrics init
    if (analyzer_start_glob(&ac.an) == -1) {
        goto cleanup;
    }

    // ui init
    if ((ui_init(&ac)) == -1) {
        goto cleanup;
    }

    // pollcontxt init
    PollContext pc = {0};

    poll_register(&pc, ac.net_fd, POLL_IN, pipeline_handler, &ac);
    poll_register(&pc, ac.sig_fd, POLL_IN, sig_handler, &ac);
    poll_register(&pc, ac.timer_fd, POLL_IN, ui_handler, &ac);

    if (poll_launcher(&pc) == -1) {
        running = 0;
        goto cleanup;
    }

    ret = 0;

cleanup:
    switch (ret) {
    case 0: {
        __attribute__((fallthrough));
    }
    case -1: {
        if (close(ac.net_fd) == -1) {
            syslog(LOG_ERR, "close socket fail");
        }
        break;
    }
    case -2: {
        syslog(LOG_ERR, "unable to reach can interface.. exit..");
        break;
    }
    default:
        syslog(LOG_ERR, "unexpected err.. exit..");
        break;
    }
    closelog();
    return ret;
}

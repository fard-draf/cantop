#include "../include/common.h"
#include "../include/context.h"
#include "../include/frame_parser.h"
#include "../include/log.h"
#include "../include/net.h"
#include "../include/poll.h"
#include "../include/signals.h"
#include "../include/ui.h"
#include "../include/watchdog.h"

volatile sig_atomic_t running = 1;

static void pipeline_handler(void *ctx) {
    AppContext *ac = ctx;
    net_recv(ac);
    frame_reader(ac);
    analyzer_populate(&ac->an, &ac->cr);
};

int main(void) {
    int ret = -1;
    AppContext ac = context_init();
    UiState us = {.ac = &ac, .paused = 0};
    syslog_init();

    // net init
    if (net_init(&ac) == -1) {
        syslog(LOG_ERR, "[ERR] net_init");
        goto cleanup;
    }

    // sig init
    if ((sig_init(&ac)) == -1) {
        syslog(LOG_ERR, "[ERR] sig_init");
        goto cleanup;
    }

    // ui init
    if ((ui_init(&ac)) == -1) {
        syslog(LOG_ERR, "[ERR] ui_init");
        goto cleanup;
    }

    // watchdog init
    if ((watchdog_init(&ac)) == -1) {
        syslog(LOG_ERR, "[ERR] watchdog_init");
        goto cleanup;
    }

    // metrics init
    if (analyzer_init(&ac.an) == -1) {
        syslog(LOG_ERR, "[ERR] analyzer_init");
        goto cleanup;
    }

    // pollcontxt init
    PollContext pc = {0};

    poll_register(&pc, ac.net_fd, POLL_IN, pipeline_handler, &ac);
    poll_register(&pc, ac.sig_fd, POLL_IN, sig_handler, &ac);
    poll_register(&pc, ac.ui_fd, POLL_IN, ui_handler, &us);
    poll_register(&pc, ac.watchdog_fd, POLL_IN, watchdog_handler, &ac);
    poll_register(&pc, STDIN_FILENO, POLL_IN, ui_stdin_handler, &us);

    if (poll_launcher(&pc) == -1) {
        running = 0;
        syslog(LOG_ERR, "[ERR] poll_launcher");
        goto cleanup;
    }
    ret = 0;

cleanup:
    if (ac.ui_fd >= 0 && close(ac.ui_fd) == -1) {
        syslog(LOG_ERR, "[ERR] close ui fd fail");
    }
    if (ac.sig_fd >= 0 && close(ac.sig_fd) == -1) {
        syslog(LOG_ERR, "[ERR] close sig fd fail");
    }
    if (ac.net_fd >= 0 && close(ac.net_fd) == -1) {
        syslog(LOG_ERR, "[ERR] close net fd fail");
    }
    if (ac.watchdog_fd >= 0 && close(ac.watchdog_fd) == -1) {
        syslog(LOG_ERR, "[ERR] close watchdog fd fail");
    }
    closelog();
    return ret;
}

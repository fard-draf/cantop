#include "../include/context.h"

AppContext context_init(void) {
    AppContext ac = {0};
    ac.net_fd = -1;
    ac.sig_fd = -1;
    ac.ui_fd = -1;
    ac.watchdog_fd = -1;

    return ac;
}

#ifndef APP_H_
#define APP_H_

#include "analyzer.h"
#include "utils.h"
#include <signal.h>

extern volatile sig_atomic_t running;

typedef struct {
    Analyzer an;
    CanReader cr;
    struct can_frame frame;
    int net_fd;
    int sig_fd;
    int timer_fd;
} AppContext;

#endif // APP_H_

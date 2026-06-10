#include "../include/poll.h"
#include "../include/context.h"
#include "../include/log.h"
#include <poll.h>
#include <sys/poll.h>

int poll_launcher(PollContext *p) {
    struct pollfd fds[MAX_FDS];
    while (running) {
        for (int i = 0; i < p->count; i++) {
            fds[i].fd = p->entries[i].fd;
            fds[i].events = p->entries[i].events;
            fds[i].revents = 0;
        }

        int ready = poll(fds, p->count, -1);
        if (ready == -1) {
            int err = errno;
            syslog(LOG_ERR, "[ERR] poll(): %s", strerror(err));
            return -1;
        }

        for (int i = 0; i < p->count; i++) {
            if (fds[i].revents & p->entries[i].events) {
                p->entries[i].handler(p->entries[i].ctx);
            }
        }
    }
    return 0;
}

void poll_register(PollContext *p, int fd, short events, fd_handler_fn handler,
                   void *ctx) {
    p->entries[p->count].fd = fd;
    p->entries[p->count].events = events;
    p->entries[p->count].handler = handler;
    p->entries[p->count].ctx = ctx;
    p->count++;

    syslog(LOG_DEBUG, "[DEBUG] poll_register for fd <%d> successfuly added",
           fd);
}

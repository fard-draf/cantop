#ifndef POLL_H_
#define POLL_H_

#include "app.h"
#include <poll.h>

#define MAX_CLIENT 10
#define BUF_SIZE 256
#define IDX_LISN 0
#define IDX_STDIN 1
#define IDX_SIG 2
#define SYS_FDS 3
#define MAX_FDS (MAX_CLIENT + SYS_FDS)

typedef void (*fd_handler_fn)(void *ctx);

typedef struct {
    int fd;
    short events;
    fd_handler_fn handler;
    void *ctx;
} PollEntry;

typedef struct {
    PollEntry entries[MAX_FDS];
    int count;
} PollContext;

void poll_register(PollContext *p, int fd, short events, fd_handler_fn handler,
                   void *ctx);
int poll_launcher(PollContext *p);

#endif // POLL_H_

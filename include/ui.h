#ifndef UI_H_
#define UI_H_

#include "common.h"
#include "context.h"
#include "log.h"
#include <ncurses.h>
#include <sys/timerfd.h>

typedef struct {
    AppContext *ac;
    int paused;
} UiState;

int ui_init(AppContext *ac);
void ui_handler(void *ctx);
void ui_stdin_handler(void *ctx);

#endif // UI_H_

#ifndef UI_H_
#define UI_H_

#include "common.h"
#include "context.h"
#include "log.h"
#include <ncurses.h>
#include <sys/timerfd.h>

typedef enum {
    TAB_PROVIDERS = 0,
    TAB_PGNS,
    TAB_PGN_DETAIL,
    TAB_COUNT,
} UiTab;

typedef struct {
    AppContext   *ac;
    int           paused;
    UiTab         tab;
    int           selected_pgn;
    uint8_t       prev_data[8];
    uint8_t       prev_data_len;
    int           history_scroll;
    DataSnapshot  frozen_history[PGN_DATA_HISTORY];
    int           frozen_count;
    bool          history_frozen;
    int           history_cursor;
    uint8_t       byte_changed_ticks[8];
} UiState;

int  ui_init(AppContext *ac);
void ui_handler(void *ctx);
void ui_stdin_handler(void *ctx);
void ui_cleanup(void);

#endif // UI_H_

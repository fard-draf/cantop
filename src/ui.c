#include "../include/ui.h"

#define WIN_TITLE_H 3
#define WIN_GLOBAL_H 5
#define WIN_TRAMES_H 6
#define WIN_WIDTH 60
#define WIN_START_X 2

static WINDOW *win_title = NULL;
static WINDOW *win_global = NULL;
static WINDOW *win_trames = NULL;

void ui_draw(Analyzer *an);

int ui_init(AppContext *ac) {
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (tfd == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] ui/timerfd_create(): %s", strerror(err));
        return -1;
    }

    struct itimerspec ts = {
        .it_interval = {.tv_sec = 0, .tv_nsec = 5000000},
        .it_value = {.tv_sec = 0, .tv_nsec = 5000000},
    };

    if (timerfd_settime(tfd, 0, &ts, NULL) == -1) {
        int err = errno;
        syslog(LOG_ERR, "[ERR] ui/timerfd_settime(): %s", strerror(err));
        return -1;
    }

    ac->ui_fd = tfd;

    // TODO: error handling
    initscr();             /* ncruses init */
    cbreak();              /* unset line buffer -> direct input */
    noecho();              /* no echo on stdin */
    keypad(stdscr, TRUE);  /* special keys */
    nodelay(stdscr, TRUE); /* non blocking mode */
    curs_set(0);

    win_title = newwin(WIN_TITLE_H, WIN_WIDTH, 1, WIN_START_X);
    win_global = newwin(WIN_GLOBAL_H, WIN_WIDTH, WIN_TITLE_H + 1, WIN_START_X);
    win_trames = newwin(WIN_TRAMES_H, WIN_WIDTH,
                        1 + WIN_TITLE_H + WIN_GLOBAL_H + 1, WIN_START_X);

    return 0;
}

void ui_handler(void *ctx) {
    UiState *us = ctx;
    uint64_t expirations;
    read(us->ac->ui_fd, &expirations, sizeof(expirations));

    if (!us->paused) {
        ui_draw(&us->ac->an);
    }

    refresh();
}

void ui_stdin_handler(void *ctx) {
    UiState *us = ctx;
    uint64_t expirations;
    read(us->ac->ui_fd, &expirations, sizeof(expirations));

    int ch = getch();
    switch (ch) {
    case ' ':
        us->paused ^= 1;
        ui_draw(&us->ac->an);
        break;
    case 'q':
        running = 0;
    case ERR:
        break;
    }
}

void ui_draw(Analyzer *an) {
    // --- fenêtre global ---
    werase(win_title);
    box(win_title, 0, 0);
    mvwprintw(win_title, 0, 2, "cantop");
    mvwprintw(win_title, 1, 2, "version: 0.1");

    werase(win_global);
    box(win_global, 0, 0);
    mvwprintw(win_global, 0, 2, " Global ");
    mvwprintw(win_global, 1, 2, "tram_count : %d", an->g_metr.tram_count_total);
    mvwprintw(win_global, 2, 2, "since      : %.1fs",
              elapsed_ms(&an->g_metr.start) / 1000.0);
    mvwprintw(win_global, 3, 2, "tram/sec   : %d", an->g_metr.tram_rate);

    // --- fenêtre trames ---
    werase(win_trames);
    box(win_trames, 0, 0);
    mvwprintw(win_trames, 0, 2, " Trames ");

    wnoutrefresh(win_title);
    wnoutrefresh(win_global);
    wnoutrefresh(win_trames);
    doupdate();
}

void ui_cleanup(void) {
    delwin(win_global);
    delwin(win_trames);
    endwin();
}

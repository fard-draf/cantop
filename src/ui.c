#include "../include/ui.h"

#define WIN_TITLE_H 3
#define WIN_GLOBAL_H 5
#define WIN_PROVIDERS_H 8
#define WIN_TRAMES_H 10
#define WIN_WIDTH 60
#define WIN_START_X 2

static WINDOW *win_title = NULL;
static WINDOW *win_global = NULL;
static WINDOW *win_providers = NULL;
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

    // TODO: put this init out of this function. Two handler, maybe ui_fd_init +
    // ui_screen_init and wrap it inside a struct
    // TODO: error handling
    initscr();             /* ncruses init */
    cbreak();              /* unset line buffer -> direct input */
    noecho();              /* no echo on stdin */
    keypad(stdscr, TRUE);  /* special keys */
    nodelay(stdscr, TRUE); /* non blocking mode */
    curs_set(0);

    win_title = newwin(WIN_TITLE_H, WIN_WIDTH, 1, WIN_START_X);
    win_global = newwin(WIN_GLOBAL_H, WIN_WIDTH, WIN_TITLE_H + 1, WIN_START_X);
    win_providers = newwin(WIN_PROVIDERS_H, WIN_WIDTH,
                           WIN_TITLE_H + WIN_GLOBAL_H + 1, WIN_START_X);
    win_trames = newwin(WIN_TRAMES_H, WIN_WIDTH,
                        1 + WIN_TITLE_H + WIN_GLOBAL_H + WIN_PROVIDERS_H + 1,
                        WIN_START_X);

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
    mvwprintw(win_global, 1, 2, "tram_count : %d",
              an->metr.glob_data_rate.tram_count_total);
    mvwprintw(win_global, 2, 2, "since      : %.1fs",
              elapsed_ms(&an->metr.start) / 1000.0);
    mvwprintw(win_global, 3, 2, "tram/sec   : %d",
              an->metr.glob_data_rate.tram_rate);

    // --- fenêtre trames ---
    werase(win_providers);
    box(win_providers, 0, 0);
    mvwprintw(win_providers, 0, 2, " Providers ");
    int j = 1;
    for (uint8_t i = an->prov_mgmt.instances_actives; i-- > 0;) {
        mvwprintw(win_providers, j, 2, "%d", an->prov_inst[i].sa);
        j++;
    }

    werase(win_trames);
    box(win_trames, 0, 0);
    mvwprintw(win_trames, 0, 2, "PGN");
    int l = 1;
    for (uint8_t k = an->pgn_mgmt.instances_actives; k-- > 0;) {
        mvwprintw(win_trames, l, 2, "%d", an->pgn_inst[k].frame.pgn);
        int m = 10;
        for (uint8_t i = 0; i < an->pgn_inst[k].frame.data_len; i++) {
            mvwprintw(win_trames, l, m, "%02X", an->pgn_inst[k].frame.data[i]);
            m += 3;
        }
        l++;
    }

    wnoutrefresh(win_title);
    wnoutrefresh(win_global);
    wnoutrefresh(win_providers);
    wnoutrefresh(win_trames);
    doupdate();
}

void ui_cleanup(void) {
    delwin(win_title);
    delwin(win_global);
    delwin(win_trames);
    delwin(win_providers);
    endwin();
}

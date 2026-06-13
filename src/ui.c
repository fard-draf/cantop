#include "../include/ui.h"
#include <locale.h>

static const char *logo[] = {
    "░█▀▀░█▀█░█▀█░▀█▀░█▀█░█▀█",
    "░█░░░█▀█░█░█░░█░░█░█░█▀▀",
    "░▀▀▀░▀░▀░▀░▀░░▀░░▀▀▀░▀░░",
};

#define LOGO_LINES 3
#define LOGO_WIDTH 27

#define COL_LABEL 2

#define PROV_COL_SA 2
#define PROV_COL_RATE 10
#define PROV_COL_TOTAL 18
#define PROV_COL_PGNS 28

#define PGN_COL_PGN 2
#define PGN_COL_PROV 12
#define PGN_COL_RATE 18
#define PGN_COL_TOTAL 26
#define PGN_COL_DATA 36

#define DETAIL_COL_LABEL 2
#define DETAIL_COL_BYTES 8
#define DETAIL_BYTE_W 5

#define BYTE_HIGHLIGHT_TICKS 40 /* 40 × 5ms = 200ms */

static void ui_draw(UiState *us);
static void freeze_pgn_history(UiState *us) {
    Analyzer *an = &us->ac->an;
    int count = an->pgns.reg.instances_actives_count;
    us->frozen_count = 0;
    if (count == 0 || us->selected_pgn >= count)
        return;
    PgnEntry *e = &an->pgns.entries[us->selected_pgn];
    us->frozen_count = e->history_count;
    for (int i = 0; i < e->history_count; i++) {
        int idx = ((int)e->history_head - 1 - i + PGN_DATA_HISTORY) %
                  PGN_DATA_HISTORY;
        us->frozen_history[i] = e->history[idx];
    }
    us->history_scroll = 0;
    us->prev_data_len = 0;
}

static int draw_header(Analyzer *an, int cols);
static int draw_tabbar(UiTab tab, int cols, int row);
static void draw_footer(int rows, int cols, UiTab tab, bool frozen);
static void draw_providers(Analyzer *an, int row, int cols);
static void draw_pgns(Analyzer *an, int selected, int row, int cols);
static void draw_pgn_detail(UiState *us, int row, int rows, int cols);

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

    setlocale(LC_ALL, "");
    initscr();
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLACK, COLOR_BLUE);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    return 0;
}

void ui_handler(void *ctx) {
    UiState *us = ctx;
    uint64_t expirations;
    read(us->ac->ui_fd, &expirations, sizeof(expirations));

    if (!us->paused)
        ui_draw(us);

    refresh();
}

void ui_stdin_handler(void *ctx) {
    UiState *us = ctx;
    int ch = getch();
    int count;

    switch (ch) {
    case KEY_F(1):
        us->tab = TAB_PROVIDERS;
        us->history_frozen = false;
        break;
    case KEY_F(2):
        us->tab = TAB_PGNS;
        us->history_frozen = false;
        break;
    case KEY_F(3):
        us->tab = TAB_PGN_DETAIL;
        us->history_frozen = false;
        us->history_scroll = 0;
        break;
    case KEY_UP:
        if (us->tab == TAB_PGNS && us->selected_pgn > 0) {
            us->selected_pgn--;
            us->prev_data_len = 0;
            us->history_scroll = 0;
        } else if (us->tab == TAB_PGN_DETAIL) {
            if (us->history_frozen) {
                if (us->history_cursor > 0)
                    us->history_cursor--;
            } else {
                if (us->history_scroll > 0)
                    us->history_scroll--;
            }
        }
        break;
    case KEY_DOWN:
        if (us->tab == TAB_PGNS) {
            count = us->ac->an.pgns.reg.instances_actives_count;
            if (count > 0 && us->selected_pgn < count - 1) {
                us->selected_pgn++;
                us->prev_data_len = 0;
                us->history_scroll = 0;
            }
        } else if (us->tab == TAB_PGN_DETAIL) {
            if (us->history_frozen) {
                if (us->history_cursor < us->frozen_count - 1)
                    us->history_cursor++;
            } else {
                us->history_scroll++;
            }
        }
        break;
    case '\n':
    case KEY_ENTER:
        if (us->tab == TAB_PGNS) {
            us->tab = TAB_PGN_DETAIL;
            us->history_frozen = false;
            us->history_scroll = 0;
        }
        break;
    case ' ':
        if (us->tab == TAB_PGN_DETAIL) {
            us->history_frozen ^= 1;
            if (us->history_frozen) {
                freeze_pgn_history(us);
                us->history_cursor = 0;
            } else {
                us->frozen_count = 0;
                us->history_scroll = 0;
            }
        } else {
            us->paused ^= 1;
        }
        break;
    case 'q':
        running = 0;
        break;
    case ERR:
        break;
    }

    ui_draw(us);
    refresh();
}

static int draw_header(Analyzer *an, int cols) {
    int row = 1;

    attron(COLOR_PAIR(3) | A_BOLD);
    for (int i = 0; i < LOGO_LINES; i++)
        mvprintw(row + i, cols - LOGO_WIDTH, "%s", logo[i]);
    attroff(COLOR_PAIR(3) | A_BOLD);

    attron(COLOR_PAIR(1));
    mvprintw(row, COL_LABEL, "started since:");
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" %.1f secs", elapsed_ms(&an->metr.start) / 1000.0);
    attroff(COLOR_PAIR(3) | A_BOLD);

    row++;
    attron(COLOR_PAIR(1));
    mvprintw(row, COL_LABEL, "tram count:");
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" %u", an->metr.datas.tram_count_total);
    attroff(COLOR_PAIR(3) | A_BOLD);
    attron(COLOR_PAIR(1));
    printw("  tram/sec:");
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" %u", an->metr.datas.tram_rate);
    attroff(COLOR_PAIR(3) | A_BOLD);

    return row + 2;
}

static int draw_tabbar(UiTab tab, int cols, int row) {
    (void)cols;
    static const char *labels[TAB_COUNT] = {
        [TAB_PROVIDERS] = "F1 PROVIDERS",
        [TAB_PGNS] = "F2 PGNS",
        [TAB_PGN_DETAIL] = "F3 PGN",
    };

    move(row, 0);
    for (int t = 0; t < TAB_COUNT; t++) {
        if (t == (int)tab) {
            attron(COLOR_PAIR(4) | A_BOLD);
            printw(" %s ", labels[t]);
            attroff(COLOR_PAIR(4) | A_BOLD);
        } else {
            attron(A_BOLD);
            printw(" %s ", labels[t]);
            attroff(A_BOLD);
        }
        if (t < TAB_COUNT - 1)
            printw("│");
    }

    return row + 1;
}

static void draw_footer(int rows, int cols, UiTab tab, bool frozen) {
    attron(A_REVERSE);
    mvhline(rows - 1, 0, ' ', cols);
    if (tab == TAB_PGN_DETAIL && frozen)
        mvprintw(rows - 1, 0,
                 "  F1 Providers  F2 PGNs  F3 PGN"
                 "  ↑↓ Navigate  SPC Unfreeze  Q Quit");
    else if (tab == TAB_PGN_DETAIL)
        mvprintw(rows - 1, 0,
                 "  F1 Providers  F2 PGNs  F3 PGN"
                 "  ↑↓ Scroll  SPC Freeze  Q Quit");
    else
        mvprintw(rows - 1, 0,
                 "  F1 Providers  F2 PGNs  F3 PGN"
                 "  ↑↓ Navigate  Enter Detail  SPC Pause  Q Quit");
    attroff(A_REVERSE);
}

static void draw_providers(Analyzer *an, int row, int cols) {
    attron(COLOR_PAIR(2) | A_BOLD);
    mvhline(row, 0, ' ', cols);
    mvprintw(row, PROV_COL_SA, "SA");
    mvprintw(row, PROV_COL_RATE, "PGNS/s");
    mvprintw(row, PROV_COL_TOTAL, "TOTAL");
    mvprintw(row, PROV_COL_PGNS, "PGN#");
    attroff(COLOR_PAIR(2) | A_BOLD);
    row++;

    for (uint8_t i = 0; i < an->providers.reg.instances_actives_count; i++) {
        uint8_t pgn_count = 0;
        for (uint8_t j = 0; j < an->pgns.reg.instances_actives_count; j++) {
            if (an->pgns.entries[j].provider == an->providers.entries[i].sa)
                pgn_count++;
        }
        mvprintw(row, PROV_COL_SA, "0x%02X", an->providers.entries[i].sa);
        mvprintw(row, PROV_COL_RATE, "%6u",
                 an->providers.entries[i].metr.datas.tram_rate);
        mvprintw(row, PROV_COL_TOTAL, "%8u",
                 an->providers.entries[i].metr.datas.tram_count_total);
        mvprintw(row, PROV_COL_PGNS, "%4u", pgn_count);
        row++;
    }
}

static void draw_pgns(Analyzer *an, int selected, int row, int cols) {
    attron(COLOR_PAIR(2) | A_BOLD);
    mvhline(row, 0, ' ', cols);
    mvprintw(row, PGN_COL_PGN, "PGN");
    mvprintw(row, PGN_COL_PROV, "PROV");
    mvprintw(row, PGN_COL_RATE, "RATE/s");
    mvprintw(row, PGN_COL_TOTAL, "TOTAL");
    mvprintw(row, PGN_COL_DATA, "DATA");
    attroff(COLOR_PAIR(2) | A_BOLD);
    row++;

    for (uint8_t k = 0; k < an->pgns.reg.instances_actives_count; k++) {
        bool is_sel = (k == (uint8_t)selected);

        if (is_sel)
            attron(A_REVERSE);
        mvhline(row, 0, ' ', cols);
        mvprintw(row, PGN_COL_PGN, "0x%05X", an->pgns.entries[k].frame.pgn);
        mvprintw(row, PGN_COL_PROV, "0x%02X", an->pgns.entries[k].provider);
        mvprintw(row, PGN_COL_RATE, "%6u",
                 an->pgns.entries[k].metr.datas.tram_rate);
        mvprintw(row, PGN_COL_TOTAL, "%8u",
                 an->pgns.entries[k].metr.datas.tram_count_total);
        int col = PGN_COL_DATA;
        for (uint8_t b = 0; b < an->pgns.entries[k].frame.data_len; b++) {
            mvprintw(row, col, "%02X", an->pgns.entries[k].frame.data[b]);
            col += 3;
        }
        if (is_sel)
            attroff(A_REVERSE);

        row++;
    }
}

static void draw_pgn_detail(UiState *us, int row, int rows, int cols) {
    Analyzer *an = &us->ac->an;
    int count = an->pgns.reg.instances_actives_count;

    if (count == 0 || us->selected_pgn >= count) {
        mvprintw(row + 2, COL_LABEL,
                 "No PGN selected — go to F2 PGNS and select one.");
        return;
    }

    PgnEntry *e = &an->pgns.entries[us->selected_pgn];

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(row, COL_LABEL, "PGN");
    attroff(COLOR_PAIR(1) | A_BOLD);
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" 0x%05X", e->frame.pgn);
    attroff(COLOR_PAIR(3) | A_BOLD);
    attron(COLOR_PAIR(1) | A_BOLD);
    printw("   PROV");
    attroff(COLOR_PAIR(1) | A_BOLD);
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" 0x%02X", e->provider);
    attroff(COLOR_PAIR(3) | A_BOLD);
    attron(COLOR_PAIR(1) | A_BOLD);
    printw("   RATE");
    attroff(COLOR_PAIR(1) | A_BOLD);
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" %u/s", e->metr.datas.tram_rate);
    attroff(COLOR_PAIR(3) | A_BOLD);
    attron(COLOR_PAIR(1) | A_BOLD);
    printw("   TOTAL");
    attroff(COLOR_PAIR(1) | A_BOLD);
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" %u", e->metr.datas.tram_count_total);
    attroff(COLOR_PAIR(3) | A_BOLD);
    attron(COLOR_PAIR(1) | A_BOLD);
    printw("   DLC");
    attroff(COLOR_PAIR(1) | A_BOLD);
    attron(COLOR_PAIR(3) | A_BOLD);
    printw(" %u", e->frame.data_len);
    attroff(COLOR_PAIR(3) | A_BOLD);

    row += 2;

    // Mise à jour des compteurs de highlight par byte
    if (us->prev_data_len == e->frame.data_len) {
        for (int b = 0; b < e->frame.data_len; b++)
            if (e->frame.data[b] != us->prev_data[b])
                us->byte_changed_ticks[b] = BYTE_HIGHLIGHT_TICKS;
    } else {
        memset(us->byte_changed_ticks, 0, sizeof(us->byte_changed_ticks));
    }
    memcpy(us->prev_data, e->frame.data, e->frame.data_len);
    us->prev_data_len = e->frame.data_len;

    attron(COLOR_PAIR(2) | A_BOLD);
    mvhline(row, 0, ' ', cols);
    mvprintw(row, DETAIL_COL_LABEL, "BYTE");
    for (int b = 0; b < 8; b++)
        mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "%4d", b);
    attroff(COLOR_PAIR(2) | A_BOLD);
    row++;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(row, DETAIL_COL_LABEL, " HEX");
    attroff(COLOR_PAIR(1) | A_BOLD);
    for (int b = 0; b < 8; b++) {
        if (b < e->frame.data_len) {
            if (us->byte_changed_ticks[b])
                attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "  %02X",
                     e->frame.data[b]);
            if (us->byte_changed_ticks[b])
                attroff(COLOR_PAIR(3) | A_BOLD);
        } else {
            attron(A_DIM);
            mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "  --");
            attroff(A_DIM);
        }
    }
    row++;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(row, DETAIL_COL_LABEL, " DEC");
    attroff(COLOR_PAIR(1) | A_BOLD);
    for (int b = 0; b < 8; b++) {
        if (b < e->frame.data_len) {
            if (us->byte_changed_ticks[b])
                attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "%4u",
                     e->frame.data[b]);
            if (us->byte_changed_ticks[b])
                attroff(COLOR_PAIR(3) | A_BOLD);
        } else {
            attron(A_DIM);
            mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "  --");
            attroff(A_DIM);
        }
    }
    row += 2;

    // Décrémenter les compteurs après le rendu
    for (int b = 0; b < 8; b++)
        if (us->byte_changed_ticks[b])
            us->byte_changed_ticks[b]--;

    // History table
    int available = rows - row - 2;
    if (available <= 1)
        return;

    if (us->history_frozen) {
        // Mode figé : snapshot plat, curseur navigable
        int hcount = us->frozen_count;
        if (hcount == 0)
            return;

        // Auto-scroll pour garder le curseur visible
        if (us->history_cursor < us->history_scroll)
            us->history_scroll = us->history_cursor;
        if (us->history_cursor >= us->history_scroll + available)
            us->history_scroll = us->history_cursor - available + 1;

        attron(COLOR_PAIR(3) | A_BOLD);
        mvhline(row, 0, ' ', cols);
        mvprintw(row, DETAIL_COL_LABEL, "  #FRZ");
        for (int b = 0; b < 8; b++)
            mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "  B%d", b);
        attroff(COLOR_PAIR(3) | A_BOLD);
        row++;

        for (int i = 0; i < available && (i + us->history_scroll) < hcount;
             i++) {
            int age = i + us->history_scroll;
            bool is_cur = (age == us->history_cursor);
            const DataSnapshot *snap = &us->frozen_history[age];
            const DataSnapshot *prev =
                (age + 1 < hcount) ? &us->frozen_history[age + 1] : NULL;

            bool hchanged[8] = {false};
            if (prev && prev->data_len == snap->data_len)
                for (int b = 0; b < snap->data_len; b++)
                    hchanged[b] = (snap->data[b] != prev->data[b]);

            if (is_cur) {
                attron(A_REVERSE);
                mvhline(row, 0, ' ', cols);
            }
            mvprintw(row, DETAIL_COL_LABEL, "%3d", age);
            for (int b = 0; b < 8; b++) {
                if (b < snap->data_len) {
                    if (!is_cur && hchanged[b])
                        attron(COLOR_PAIR(3) | A_BOLD);
                    mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W,
                             "  %02X", snap->data[b]);
                    if (!is_cur && hchanged[b])
                        attroff(COLOR_PAIR(3) | A_BOLD);
                } else {
                    if (!is_cur)
                        attron(A_DIM);
                    mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "  --");
                    if (!is_cur)
                        attroff(A_DIM);
                }
            }
            if (is_cur)
                attroff(A_REVERSE);
            row++;
        }
    } else {
        // Mode live : ring buffer direct
        int hcount = e->history_count;
        if (hcount == 0)
            return;

        int max_scroll = (hcount > available) ? hcount - available : 0;
        if (us->history_scroll > max_scroll)
            us->history_scroll = max_scroll;

        attron(COLOR_PAIR(4) | A_BOLD);
        mvhline(row, 0, ' ', cols);
        mvprintw(row, DETAIL_COL_LABEL, "  #");
        for (int b = 0; b < 8; b++)
            mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "  B%d", b);
        attroff(COLOR_PAIR(4) | A_BOLD);
        row++;

        for (int i = 0; i < available && (i + us->history_scroll) < hcount;
             i++) {
            int age = i + us->history_scroll;
            int idx = ((int)e->history_head - 1 - age + PGN_DATA_HISTORY) %
                      PGN_DATA_HISTORY;
            int prev_idx = ((int)e->history_head - 2 - age + PGN_DATA_HISTORY) %
                           PGN_DATA_HISTORY;
            const DataSnapshot *snap = &e->history[idx];
            const DataSnapshot *prev =
                (age + 1 < hcount) ? &e->history[prev_idx] : NULL;

            bool hchanged[8] = {false};
            if (prev && prev->data_len == snap->data_len)
                for (int b = 0; b < snap->data_len; b++)
                    hchanged[b] = (snap->data[b] != prev->data[b]);

            mvprintw(row, DETAIL_COL_LABEL, "%3d", age);
            for (int b = 0; b < 8; b++) {
                if (b < snap->data_len) {
                    if (hchanged[b])
                        attron(COLOR_PAIR(3) | A_BOLD);
                    mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W,
                             "  %02X", snap->data[b]);
                    if (hchanged[b])
                        attroff(COLOR_PAIR(3) | A_BOLD);
                } else {
                    attron(A_DIM);
                    mvprintw(row, DETAIL_COL_BYTES + b * DETAIL_BYTE_W, "  --");
                    attroff(A_DIM);
                }
            }
            row++;
        }
    }
}

static void ui_draw(UiState *us) {
    Analyzer *an = &us->ac->an;
    erase();
    int rows = getmaxy(stdscr);
    int cols = getmaxx(stdscr);

    int row = draw_header(an, cols);
    row = draw_tabbar(us->tab, cols, row);

    switch (us->tab) {
    case TAB_PROVIDERS:
        draw_providers(an, row, cols);
        break;
    case TAB_PGNS:
        draw_pgns(an, us->selected_pgn, row, cols);
        break;
    case TAB_PGN_DETAIL:
        draw_pgn_detail(us, row, rows, cols);
        break;
    default:
        break;
    }

    draw_footer(rows, cols, us->tab, us->history_frozen);
}

void ui_cleanup(void) { endwin(); }

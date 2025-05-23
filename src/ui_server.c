#include "ui_server.h"
#include <ncurses.h>

static WINDOW *win_map;
static WINDOW *win_status;

void ui_init(void) {
    initscr();
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    cbreak();
    noecho();
    curs_set(0);

    int height, width;
    getmaxyx(stdscr, height, width);

    // Janela do mapa ocupa todo menos 3 linhas de status
    win_map = newwin(height - 3, width, 0, 0);
    win_status = newwin(3, width, height - 3, 0);

    box(win_map, 0, 0);
    box(win_status, 0, 0);
    wrefresh(win_map);
    wrefresh(win_status);
}

void ui_draw_map(const int *treasure_x, const int *treasure_y, int n) {
    werase(win_map);
    box(win_map, 0, 0);

    int max_y = getmaxy(win_map);
    int max_x = getmaxx(win_map);

    for (int i = 0; i < n; ++i) {
        int x = treasure_x[i];
        int y = treasure_y[i];
        if (y > 0 && y < max_y - 1 && x > 0 && x < max_x - 1) {
            wattron(win_map, COLOR_PAIR(1));
            mvwaddch(win_map, y, x, '$');
            wattroff(win_map, COLOR_PAIR(1));
        }
    }
    wrefresh(win_map);
}

void ui_show_status(const char *msg) {
    werase(win_status);
    box(win_status, 0, 0);
    mvwprintw(win_status, 1, 1, "%s", msg);
    wrefresh(win_status);
}

void ui_cleanup(void) {
    delwin(win_map);
    delwin(win_status);
    endwin();
}
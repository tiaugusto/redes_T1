#include "ui_server.h"
#include <ncurses.h>

static WINDOW *win_map;
static WINDOW *win_status;
static WINDOW *win_border;

#define GRID_SIZE 8 

void ui_init(void) {
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_YELLOW, -1);  // tesouro
    init_pair(2, COLOR_CYAN, -1);    // borda
    cbreak();
    noecho();
    curs_set(0);

    int height, width;
    getmaxyx(stdscr, height, width);

    win_border = newwin(GRID_SIZE + 2, GRID_SIZE * 2 + 3, 1, 2);
    win_map = derwin(win_border, GRID_SIZE, GRID_SIZE * 2, 1, 1);
    win_status = newwin(5, width - 4, height - 6, 2);

    wbkgd(win_border, COLOR_PAIR(2));
    box(win_border, 0, 0);
    box(win_status, 0, 0);

    mvprintw(0, 2, "=== MAPA DO SERVIDOR ===");
    mvprintw(height - 1, 2, "Tesouros marcados com '$'");
    refresh();
    wrefresh(win_border);
    wrefresh(win_map);
    wrefresh(win_status);
}

void ui_draw_map(const int *treasure_x, const int *treasure_y, int n) {
    werase(win_map);
    for (int i = 0; i < n; ++i) {
        int x = treasure_x[i], y = treasure_y[i];
        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) continue;
        mvwprintw(win_map, GRID_SIZE-1-x, y*2, "$ ");
    }
    wrefresh(win_map);
    box(win_border, 0, 0);
    wrefresh(win_border);
}

void ui_show_status(const char *msg) {
    werase(win_status);
    box(win_status, 0, 0);
    mvwprintw(win_status, 2, 2, "%s", msg);
    wrefresh(win_status);
}

void ui_cleanup(void) {
    delwin(win_map);
    delwin(win_status);
    delwin(win_border);
}

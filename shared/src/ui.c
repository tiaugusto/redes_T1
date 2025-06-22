#include "ui.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

static WINDOW *win_map;
static WINDOW *win_status;
static WINDOW *win_border;

static char visited[GRID_SIZE][GRID_SIZE] = {0};

void ui_client_init(void) {
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);  // jogador
    init_pair(2, COLOR_RED, -1);    // erro
    init_pair(3, COLOR_YELLOW, -1); // trilha
    init_pair(4, COLOR_CYAN, -1);   // borda
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    int height, width;
    getmaxyx(stdscr, height, width);

    win_border = newwin(GRID_SIZE + 2, GRID_SIZE * 2 + 3, 1, 2);
    win_map = derwin(win_border, GRID_SIZE, GRID_SIZE * 2, 1, 1);
    win_status = newwin(5, width - 4, height - 6, 2);

    wbkgd(win_border, COLOR_PAIR(4));
    box(win_border, 0, 0);
    box(win_status, 0, 0);

    mvprintw(0, 2, "=== CACA AO TESOURO ===");
    refresh();
    wrefresh(win_border);
    wrefresh(win_map);
    wrefresh(win_status);
}


void ui_client_draw_map(const int *x, const int *y, int n) {
    werase(win_map);

    for (int i = 0; i < n; ++i) {
        if (x[i] < 0 || x[i] >= GRID_SIZE ||
            y[i] < 0 || y[i] >= GRID_SIZE)                 /* <-- GUARDA */
            continue;
        visited[y[i]][x[i]] = 2;
    }

    /* desenha do topo (linha 0) até a base (linha GRID_SIZE-1) */
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            if (visited[row][col] == 2) {
                wattron(win_map, COLOR_PAIR(1));
                mvwprintw(win_map, row, col * 2, "X ");
                wattroff(win_map, COLOR_PAIR(1));
                visited[row][col] = 1;
            } else if (visited[row][col] == 1) {
                wattron(win_map, COLOR_PAIR(3));
                mvwprintw(win_map, row, col * 2, ". ");
                wattroff(win_map, COLOR_PAIR(3));
            } else {
                mvwprintw(win_map, row, col * 2, "  ");
            }
        }
    }

    wrefresh(win_map);
    box(win_border, 0, 0);
    wrefresh(win_border);
}


void ui_client_show_status(const char *msg) {
    werase(win_status);
    box(win_status, 0, 0);
    mvwprintw(win_status, 2, 2, "%s", msg);
    wrefresh(win_status);
}

void ui_client_show_error(const char *msg) {
    werase(win_status);
    box(win_status, 0, 0);
    wattron(win_status, COLOR_PAIR(2));
    mvwprintw(win_status, 2, 2, "ERRO: %s", msg);
    wattroff(win_status, COLOR_PAIR(2));
    wrefresh(win_status);
}

char ui_client_read_command(void) {
    int ch = getch();
    return (char)ch;
}

void ui_server_init(void) {
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

void ui_server_draw_map(const int *treasure_x, const int *treasure_y, int n) {
    werase(win_map);
    for (int i = 0; i < n; ++i) {
        int x = treasure_x[i];
        int y = treasure_y[i];
         if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
            mvwprintw(win_map, y, x * 2, "$ ");
        }
    }
    wrefresh(win_map);
    box(win_border, 0, 0);
    wrefresh(win_border);
}

void ui_server_show_status(const char *msg) {
    werase(win_status);
    box(win_status, 0, 0);
    mvwprintw(win_status, 2, 2, "%s", msg);
    wrefresh(win_status);
}

void ui_cleanup(void) {
    if (win_map)    delwin(win_map);
    if (win_status) delwin(win_status);
    if (win_border) delwin(win_border);
    endwin();    
}

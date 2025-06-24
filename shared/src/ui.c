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

/*
  x[] = linhas (0..7 de baixo pra cima)
  y[] = colunas (0..7 esquerda pra direita)
*/
void ui_client_draw_map(const int *x, const int *y, int n) {
    werase(win_map);

    for (int i = 0; i < n; ++i) {
        int row = x[i], col = y[i];
        if (row<0||row>=GRID_SIZE||col<0||col>=GRID_SIZE) continue;
        visited[row][col] = 2;
    }

    for (int vis_r = 0; vis_r < GRID_SIZE; ++vis_r) {
        // vis_r=0 é topo da janela, corresponde a grid_row = GRID_SIZE-1
        int grid_row = GRID_SIZE - 1 - vis_r;
        for (int col = 0; col < GRID_SIZE; ++col) {
            char buf[3] = "  ";
            int color = 0;
            if (visited[grid_row][col] == 2) {
                buf[0] = 'X'; buf[1] = ' '; color = 1;
                visited[grid_row][col] = 1;
            }
            else if (visited[grid_row][col] == 1) {
                buf[0] = '.'; buf[1] = ' '; color = 3;
            }
            if (color) wattron(win_map, COLOR_PAIR(color));
            mvwprintw(win_map, vis_r, col*2, "%s", buf);
            if (color) wattroff(win_map, COLOR_PAIR(color));
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
        int x = treasure_x[i], y = treasure_y[i];
        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) continue;
        mvwprintw(win_map, GRID_SIZE-1-x, y*2, "$ ");
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

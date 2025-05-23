#include "ui_client.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>



static WINDOW *win_map;
static WINDOW *win_status;

void ui_init(void) {
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    cbreak();
    noecho();
    curs_set(0);

    int height, width;
    getmaxyx(stdscr, height, width);
    win_map = newwin(height - 3, width, 0, 0);
    win_status = newwin(3, width, height - 3, 0);

    box(win_map, 0, 0);
    box(win_status, 0, 0);
    wrefresh(win_map);
    wrefresh(win_status);
}

void ui_draw_map(const int *x, const int *y, int n) {
    werase(win_map);
    box(win_map, 0, 0);
    int height = getmaxy(win_map);
    int width  = getmaxx(win_map);
    for (int i = 0; i < n; ++i) {
        int px = x[i];
        int py = y[i];
        // valida coordenadas dentro do grid
        if (px >= 0 && px < GRID_SIZE && py >= 0 && py < GRID_SIZE) {
            // posição de desenho: offset de 1 para contornar a borda
            int wy = py + 1;
            int wx = px + 1;
            if (wy > 0 && wy < height - 1 && wx > 0 && wx < width - 1) {
                wattron(win_map, COLOR_PAIR(1));
                mvwaddch(win_map, wy, wx, 'X');
                wattroff(win_map, COLOR_PAIR(1));
            }
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

void ui_show_error(const char *msg) {
    werase(win_status);
    box(win_status, 0, 0);
    wattron(win_status, COLOR_PAIR(2));
    mvwprintw(win_status, 1, 1, "ERROR: %s", msg);
    wattroff(win_status, COLOR_PAIR(2));
    wrefresh(win_status);
}

char ui_read_command(void) {
    int ch = wgetch(win_status);
    return (char)ch;
}

void ui_handle_text(const char *data, size_t len) {
    // Exibe texto recebido na janela de status
    char buf[256];
    size_t copy_len = (len < sizeof(buf)-1) ? len : sizeof(buf)-1;
    memcpy(buf, data, copy_len);
    buf[copy_len] = '\0';
    ui_show_status(buf);
}

void ui_handle_image(const uint8_t *data, size_t len) {
    // Stub: pode salvar imagem em disco ou exibir metadados
    ui_show_status("[Imagem recebida]");
}

void ui_handle_video(const uint8_t *data, size_t len) {
    ui_show_status("[Vídeo recebido]");
}

void ui_handle_data_chunk(const uint8_t *data, size_t len) {
    ui_show_status("[Chunk de dados recebido]");
}

void ui_complete_file(void) {
    ui_show_status("[Recebimento de arquivo concluído]");
}

void ui_cleanup(void) {
    delwin(win_map);
    delwin(win_status);
    endwin();
}
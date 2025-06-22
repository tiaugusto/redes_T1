#ifndef UI_H
#define UI_H

#include <stddef.h>

#define GRID_SIZE 8

/* ---------- SERVIDOR ---------- */
void ui_server_init(void);
void ui_server_draw_map(const int *treasure_x, const int *treasure_y, int n);
void ui_server_show_status(const char *msg);


/* ---------- CLIENTE ---------- */
void ui_client_init(void);
void ui_client_draw_map(const int *x, const int *y, int n);
void ui_client_show_status(const char *msg);
void ui_client_show_error(const char *msg);
char ui_client_read_command(void);


/* ---------- COMUM ---------- */
void ui_cleanup(void);


/** Handlers para diferentes tipos de dados recebidos */
void ui_handle_text(const char *data, size_t len);
void ui_handle_image(const unsigned char *data, size_t len);
void ui_handle_video(const unsigned char *data, size_t len);
void ui_handle_data_chunk(const unsigned char *data, size_t len);
void ui_complete_file(void);


#endif
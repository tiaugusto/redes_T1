#ifndef UI_CLIENT_H
#define UI_CLIENT_H

#include <stddef.h>
#include <stdint.h>

#define GRID_SIZE 8

void ui_init(void);

void ui_draw_map(const int *x, const int *y, int n);

void ui_show_status(const char *msg);

void ui_show_error(const char *msg);

char ui_read_command(void);

void ui_handle_text(const char *data, size_t len);
void ui_handle_image(const unsigned char *data, size_t len);
void ui_handle_video(const unsigned char *data, size_t len);
void ui_handle_data_chunk(const unsigned char *data, size_t len);
void ui_complete_file(void);

void ui_cleanup(void);

#endif
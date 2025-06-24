#ifndef UI_SERVER_H
#define UI_SERVER_H

#define GRID_SIZE 8

void ui_init(void);

void ui_draw_map(const int *treasure_x, const int *treasure_y, int n);

void ui_show_status(const char *msg);

void ui_cleanup(void);

#endif
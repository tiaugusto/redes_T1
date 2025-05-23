#ifndef UI_CLIENT_H
#define UI_CLIENT_H

#include <stddef.h>
#include <stdint.h>

#define GRID_SIZE 8

/** Inicializa a UI do cliente */
void ui_init(void);

/** Desenha o "mapa" do cliente (ex: posição do jogador) */
void ui_draw_map(const int *x, const int *y, int n);

/** Exibe status ou mensagens de erros */
void ui_show_status(const char *msg);

/** Exibe mensagens genéricas (erros/buffer de arquivo/etc.) */
void ui_show_error(const char *msg);

/** Lê o comando do usuário (tecla) */
char ui_read_command(void);

/** Handlers para diferentes tipos de dados recebidos */
void ui_handle_text(const char *data, size_t len);
void ui_handle_image(const uint8_t *data, size_t len);
void ui_handle_video(const uint8_t *data, size_t len);
void ui_handle_data_chunk(const uint8_t *data, size_t len);
void ui_complete_file(void);

/** Fecha a UI do cliente */
void ui_cleanup(void);

#endif
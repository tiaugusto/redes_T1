#ifndef UI_SERVER_H
#define UI_SERVER_H

/** Inicializa a UI do servidor */
void ui_init(void);

/**
 * Desenha o mapa de tesouros
 * @param treasure_x vetor de coordenadas X dos tesouros
 * @param treasure_y vetor de coordenadas Y dos tesouros
 * @param n número de tesouros
 */
void ui_draw_map(const int *treasure_x, const int *treasure_y, int n);

/**
 * Exibe mensagem de status (envio, recebimento, conexões, etc.)
 * @param msg texto para exibir
 */
void ui_show_status(const char *msg);

/** Limpa a UI e restaura o terminal */
void ui_cleanup(void);

#endif
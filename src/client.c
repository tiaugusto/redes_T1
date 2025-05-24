#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include "protocol.h"
#include "protocol_net.h"
#include "net.h"
#include "ui_client.h"

#define GRID_SIZE 8
#define MAX_RETRANSMISSIONS 5

static int sock_fd;
static uint8_t seq_num = 0;
static int player_x = 0, player_y = 0;

void send_move(msg_type_t mov_type) {
    int attempts = 0;
    uint8_t ack_seq, dummy_len;
    msg_type_t ack_type;
    int rv;

    while (attempts < MAX_RETRANSMISSIONS) {
        rv = protocol_send(sock_fd, seq_num, mov_type, NULL, 0);
        if (rv < 0) {
            ui_show_error("Erro ao enviar frame");
            return;
        }
        rv = protocol_recv(sock_fd, &ack_seq, &ack_type, NULL, &dummy_len);
        if (rv == 1 && ack_type == MSG_ACK && ack_seq == seq_num) {
            //printf("[DEBUG] Recebido: seq=%d type=%d  (esperado seq=%d type=%d)\n",
            seq_num = (seq_num + 1) % SEQ_MODULO;
            // atualiza posição
            switch (mov_type) {
                case MSG_MOV_UP:    if (player_y > 0) player_y--; break;
                case MSG_MOV_DOWN:  if (player_y < GRID_SIZE-1) player_y++; break;
                case MSG_MOV_LEFT:  if (player_x > 0) player_x--; break;
                case MSG_MOV_DIR:   if (player_x < GRID_SIZE-1) player_x++; break;
                default: break;
            }
            ui_draw_map(&player_x, &player_y, 1);
            ui_show_status("Movimento confirmado");
            return;
        }
        attempts++;
    }
    ui_show_error("Falha: sem ACK após várias tentativas");
}

int main(int argc, char **argv) {
    char *iface = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        if (opt == 'i') iface = optarg;
    }
    if (!iface) {
        fprintf(stderr, "Usage: %s -i <interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sock_fd = net_init(iface);
    ui_init();
    
    // desenha posição inicial e status
    ui_draw_map(&player_x, &player_y, 1);
    ui_show_status("Aguardando servidor...");

    // loop de comandos
    while (1) {
        char cmd = ui_read_command();
        switch (cmd) {
            case 'w': send_move(MSG_MOV_UP); break;
            case 'a': send_move(MSG_MOV_LEFT); break;
            case 's': send_move(MSG_MOV_DOWN); break;
            case 'd': send_move(MSG_MOV_DIR); break;
            default: ui_show_status("Use W/A/S/D para mover");
        }
    }

    close(sock_fd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <ncurses.h>
#include "protocol.h"
#include "protocol_net.h"
#include "net.h"
#include "ui_client.h"

#define GRID_SIZE 8
#define MAX_RETRANSMISSIONS 5

static int sock_fd;
static uint8_t seq_num = 0;
static int player_x = 0, player_y = 0;

/* ---------- estado para download de arquivo ------------------------ */
static FILE      *rx_file  = NULL;
static char       rx_name[64];
static msg_type_t rx_kind;

static void process_packet(uint8_t seq, msg_type_t type,
                         const uint8_t *payload, uint8_t len)
{
    if (type == MSG_NACK) return;                 /* ignora */

    /* ---------- início de arquivo ---------- */
    if (type == MSG_TEXTO_ACK_NOME ||
        type == MSG_VIDEO_ACK_NOME ||
        type == MSG_IMAGEM_ACK_NOME) {
        if (len == 0 || len >= sizeof(rx_name)) {              /* nome mal-formado */
            protocol_send(sock_fd, seq, MSG_NACK, NULL, 0);
            return;
        }
        for (int i = 0; i < len; ++i)                          /* só ASCII visível */
            if (payload[i] < 0x20 || payload[i] > 0x7E) return;
        
        memcpy(rx_name, payload, len);
        rx_name[len] = '\0';

        char path[96];
        snprintf(path,sizeof(path),"tesouros/%s",rx_name);
        rx_file = fopen(path,"wb");
        if (!rx_file) {
            uint8_t code = 0;
            protocol_send(sock_fd, seq, MSG_ERRO, &code, 1);
            return;
        }
        rx_kind = type;
        protocol_send(sock_fd, seq, MSG_ACK, NULL, 0);
        return;
    }

    /* ---------- dados ---------- */
    if (type == MSG_DADOS && rx_file) {
        fwrite(payload,1,len,rx_file);
        protocol_send(sock_fd, seq, MSG_ACK, NULL, 0);
        return;
    }

    /* ---------- fim ---------- */
    if (type == MSG_FIM_ARQ && rx_file) {
        fclose(rx_file); rx_file = NULL;
        protocol_send(sock_fd, seq, MSG_ACK, NULL, 0);


            char path[96];
            snprintf(path, sizeof(path), "tesouros/%s", rx_name);
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "xdg-open '%s' >/dev/null 2>&1 &", path);
            system(cmd);

            ui_show_status("Tesouro aberto.");

    }
}

static void confirm_move(msg_type_t mov)
{
    switch (mov) {
        case MSG_MOV_UP:    if (player_y > 0)           player_y--; break;
        case MSG_MOV_DOWN:  if (player_y < GRID_SIZE-1) player_y++; break;
        case MSG_MOV_LEFT:  if (player_x > 0)           player_x--; break;
        case MSG_MOV_DIR:   if (player_x < GRID_SIZE-1) player_x++; break;
        default: break;
    }
    ui_draw_map(&player_x,&player_y,1);
    ui_show_status("Movimento confirmado");
}

void send_move(msg_type_t mov_type)
{
    uint8_t buf[MAX_DATA_LEN];

    for (int att = 0; att < MAX_RETRANSMISSIONS; ++att) {

        if (protocol_send(sock_fd, seq_num, mov_type, NULL, 0) < 0) {
            ui_show_error("Falha ao enviar");
            return;
        }

        while (1) {
            uint8_t seq,len; msg_type_t type;
            int rv = protocol_recv(sock_fd,&seq,&type,buf,&len);

            if (rv == 0) break;          /* timeout → retransmitir   */
            if (rv < 0) { ui_show_error("recv falhou"); return; }

            if (type == MSG_ACK && seq == seq_num) {
                seq_num = (seq_num + 1) % SEQ_MODULO;
                confirm_move(mov_type);
                return;
            }
            /* qualquer outro → processa, continua esperando ACK     */
            process_packet(seq,type,buf,len);
        }
    }
    ui_show_error("Sem ACK após várias tentativas");
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
    nodelay(stdscr, TRUE);
    
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

        uint8_t seq,len; msg_type_t type;
        uint8_t buf[MAX_DATA_LEN];
        while (protocol_recv(sock_fd,&seq,&type,buf,&len)==1)
            process_packet(seq,type,buf,len);
    }

    close(sock_fd);
    return 0;
}

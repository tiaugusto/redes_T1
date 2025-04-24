#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"
#include "net.h"
#include "ui_server.h"
#include "utils.h"

#define RECV_BUF_SIZE 1500
#define NUM_TREASURES 8

static int sock_fd;
static uint8_t seq_num = 0;
static int treasure_x[NUM_TREASURES];
static int treasure_y[NUM_TREASURES];
static int sent_flags[NUM_TREASURES];

void init_treasures() {
    // TODO: randomizar 8 posições únicas em [0..7]
}

void send_file(int id) {
    // 1) enviar frame inicial com nome e tipo
    // 2) enviar pedaços com PROTO_MSG_DADOS
    // 3) enviar PROTO_MSG_FIM_ARQ
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
    ui_server_init();
    init_treasures();
    memset(sent_flags, 0, sizeof(sent_flags));

    uint8_t in_buf[RECV_BUF_SIZE];
    while (1) {
        int n = net_recv(sock_fd, in_buf, sizeof(in_buf));
        if (n <= 0) continue;

        uint8_t recv_seq, pl_len;
        protocol_msg_t type;
        uint8_t payload[MAX_PAYLOAD_SIZE];

        if (protocol_unpack_frame(in_buf, n, &recv_seq, &type, payload, &pl_len) == 0) {
            // Se for movimento
            if (type >= PROTO_MSG_MOV_DIR && type <= PROTO_MSG_MOV_LEFT) {
                int px, py;
                ui_server_update_player(type, &px, &py);
                ui_server_log_move(px, py);
                // Verificar se há tesouro
                for (int i = 0; i < NUM_TREASURES; i++) {
                    if (!sent_flags[i] && px == treasure_x[i] && py == treasure_y[i]) {
                        send_file(i);
                        sent_flags[i] = 1;
                    }
                }
                // TODO: enviar ACK
            }
        }
    }

    close(sock_fd);
    return 0;
}
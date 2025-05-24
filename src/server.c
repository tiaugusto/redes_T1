#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "net.h"
#include "protocol_net.h"
#include "protocol.h"
#include "ui_server.h"
#include "utils.h"

#define NUM_TREASURES 8
#define GRID_SIZE     8
#define MAX_RETRANS   5

static int sock_fd;
static uint8_t seq_num = 0;
static int treasure_x[NUM_TREASURES];
static int treasure_y[NUM_TREASURES];
static bool sent_flags[NUM_TREASURES];

static void init_treasures(void) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < NUM_TREASURES; ) {
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;
        bool exists = false;
        for (int j = 0; j < i; j++) {
            if (treasure_x[j] == x && treasure_y[j] == y) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            treasure_x[i] = x;
            treasure_y[i] = y;
            i++;
        }
    }
}

static int send_reliable(msg_type_t type,
                         const uint8_t *payload,
                         uint8_t len) {
    int attempts = 0;
    msg_type_t ack_type;
    uint8_t ack_seq, dummy_len;

    while (attempts < MAX_RETRANS) {
        fprintf(stderr, "[SEND_RELIABLE] seq=%u type=%u len=%u\n",
                seq_num, type, len);
        if (protocol_send(sock_fd, seq_num, type, payload, len) < 0) {
            fprintf(stderr, "[SEND_RELIABLE] envio falhou\n");
            return -1;
        }

        int rv = protocol_recv(sock_fd, &ack_seq, &ack_type, NULL, &dummy_len);
        if (rv == 1 && ack_type == MSG_ACK && ack_seq == seq_num) {
            seq_num = (seq_num + 1) % SEQ_MODULO;
            return 0;
        }
        attempts++;
    }
    return -1;
}

static void send_file(int id) {
    const char *filename = get_treasure_filename(id);
    uint8_t fname_len = (uint8_t)strlen(filename);

    if (send_reliable(MSG_INI_ARQ,
                      (const uint8_t *)filename,
                      fname_len) < 0) {
        ui_show_status("Erro: não enviou nome de arquivo");
        return;
    }

    FILE *f = fopen(filename, "rb");
    if (!f) {
        ui_show_status("Erro: não abriu arquivo");
        return;
    }

    uint8_t buf[MAX_DATA_LEN];
    size_t n;
    while ((n = fread(buf, 1, MAX_DATA_LEN, f)) > 0) {
        if (send_reliable(MSG_DADOS, buf, (uint8_t)n) < 0) {
            ui_show_status("Erro: falha ao enviar dados");
            fclose(f);
            return;
        }
    }
    fclose(f);

    if (send_reliable(MSG_FIM_ARQ, NULL, 0) < 0) {
        ui_show_status("Erro: falha fim de arquivo");
    }
}

int main(int argc, char **argv) {
    char *iface = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "i:")) != -1) {
        if (opt == 'i') iface = optarg;
    }
    if (!iface) {
        fprintf(stderr, "Uso: %s -i <interface>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sock_fd = net_init(iface);
    ui_init();

    init_treasures();
    ui_draw_map(treasure_x, treasure_y, NUM_TREASURES);
    memset(sent_flags, 0, sizeof(sent_flags));

    while (1) {
        uint8_t recv_seq, len;
        msg_type_t type;
        uint8_t payload[MAX_DATA_LEN];

        int rv = protocol_recv(sock_fd, &recv_seq, &type, payload, &len);
        if (rv <= 0) {
            continue;
        }

        if (type >= MSG_MOV_DIR && type <= MSG_MOV_LEFT) {
            
            static int px = 0, py = 0;
            switch (type) {
                case MSG_MOV_UP:   if (py > 0) py--; break;
                case MSG_MOV_DOWN: if (py < GRID_SIZE - 1) py++; break;
                case MSG_MOV_LEFT: if (px > 0) px--; break;
                case MSG_MOV_DIR:  if (px < GRID_SIZE - 1) px++; break;
                default: break;
            }
            //fprintf(stderr, "[SERVER] ACK para seq=%d (tipo %d recebido)\n", recv_seq, type);

            // ACK do movimento
            //fprintf(stderr, "[SERVER] Enviando ACK: seq=%d\n", recv_seq);
            protocol_send(sock_fd, recv_seq, MSG_ACK, NULL, 0);

            ui_draw_map(treasure_x, treasure_y, NUM_TREASURES);
            char status[64];
            snprintf(status, sizeof(status),
                     "Jogador em (%d,%d)", px, py);
            ui_show_status(status);


            // Verifica tesouro
            for (int i = 0; i < NUM_TREASURES; i++) {
                if (!sent_flags[i] && px == treasure_x[i] && py == treasure_y[i]) {
                    send_file(i);
                    sent_flags[i] = true;
                    ui_show_status("Enviando arquivo do tesouro");
                    ui_draw_map(treasure_x, treasure_y, NUM_TREASURES);
                }
            }
        }
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
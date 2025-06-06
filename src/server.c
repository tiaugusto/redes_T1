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
        //fprintf(stderr, "[SEND_RELIABLE] seq=%u type=%u len=%u\n",
                //seq_num, type, len);
        //printf("[SERVER] Respondendo ACK para seq=%d\n", recv_seq);
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

static int send_file(int id) {
    const char *filename = get_treasure_filename(id);
    if (!filename) {
        ui_show_status("Erro: nome de arquivo nulo");
        return -1;
    }

    uint8_t fname_len = (uint8_t)strlen(filename);

    msg_type_t ini_type;
    if (ends_with(filename, ".txt"))
        ini_type = MSG_TEXTO_ACK_NOME;
    else if (ends_with(filename, ".mp4"))
        ini_type = MSG_VIDEO_ACK_NOME;
    else
        ini_type = MSG_IMAGEM_ACK_NOME;

    if (send_reliable(ini_type, (const uint8_t *)filename, fname_len) < 0) {
        ui_show_status("Erro: não enviou nome de arquivo");
        return -1;
    }

    char path[96];
    snprintf(path, sizeof(path), "objetos/%s", filename);
    FILE *f = fopen(path, "rb");
    if (!f) {
        ui_show_status("Erro: não abriu arquivo");
        return -1;
    }

    uint8_t buf[MAX_DATA_LEN];
    size_t n;
    while ((n = fread(buf, 1, MAX_DATA_LEN, f)) > 0) {
        if (send_reliable(MSG_DADOS, buf, (uint8_t)n) < 0) {
            ui_show_status("Erro: falha ao enviar dados");
            fclose(f);
            return -1;
        }
    }
    fclose(f);

    if (send_reliable(MSG_FIM_ARQ, NULL, 0) < 0) {
        ui_show_status("Erro: falha fim de arquivo");
        return -1;
    }

    return 0;  // sucesso!
}

static void desenhar_tesouros_restantes(void) {
    int n_visiveis = 0;
    int tx[NUM_TREASURES], ty[NUM_TREASURES];

    for (int i = 0; i < NUM_TREASURES; ++i) {
        if (!sent_flags[i]) {
            tx[n_visiveis] = treasure_x[i];
            ty[n_visiveis] = treasure_y[i];
            n_visiveis++;
        }
    }

    ui_draw_map(tx, ty, n_visiveis);
}


int main(int argc, char **argv) {
    char *iface = NULL;
    int opt;

    static int px = 0, py = 0;

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

    // Conta quantos tesouros ainda não foram enviados
    int n_visiveis = 0;
    int tx[NUM_TREASURES], ty[NUM_TREASURES];

    for (int i = 0; i < NUM_TREASURES; ++i) {
        if (!sent_flags[i]) {
            tx[n_visiveis] = treasure_x[i];
            ty[n_visiveis] = treasure_y[i];
            n_visiveis++;
        }
    }

    desenhar_tesouros_restantes();
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
            //fprintf(stderr, "[SERVER] Respondendo ACK para seq=%d\n", recv_seq);
            protocol_send(sock_fd, recv_seq, MSG_ACK, NULL, 0);

            desenhar_tesouros_restantes();
            char status[64];
            snprintf(status, sizeof(status),
                     "Jogador em (%d,%d)", px, py);
            ui_show_status(status);


            // Verifica tesouro
            for (int i = 0; i < NUM_TREASURES; i++) {
                if (!sent_flags[i] && px == treasure_x[i] && py == treasure_y[i]) {
                    if (send_file(i) == 0) {
                        sent_flags[i] = true;
                        ui_show_status("Enviando arquivo do tesouro");
                        desenhar_tesouros_restantes();
                    }
                }
            }
        }
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>

#include "socket.h"
#include "protocol_net.h"
#include "protocol.h"
#include "ui_server.h"
#include "utils.h"

#define NUM_TREASURES 8
#define GRID_SIZE 8

static int sock_fd;
static unsigned char seq_num = 0;

// Posições dos tesouros e flags indicando se já foram enviados
static int treasure_x[NUM_TREASURES];
static int treasure_y[NUM_TREASURES];
static bool sent_flags[NUM_TREASURES];

static void init_treasures(void) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < NUM_TREASURES; ) {
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;

        // Não permite tesouro na origem.
        if (x == 0 && y == 0)
            continue;
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

// Envia pacote e espera confirmação (ACK) com mesmo número de sequência.
static int send_reliable(msg_type_t type, const unsigned char *payload, unsigned char len) {
    msg_type_t ack_type;
    unsigned char ack_seq;
    unsigned char dummy_len = 0;

    while (1) {
        if (protocol_send(sock_fd, seq_num, type, payload, len) < 0) {
            fprintf(stderr, "[SEND_RELIABLE] envio falhou\n");
            return -1;
        }

        int rv = protocol_recv(sock_fd, &ack_seq, &ack_type, NULL, &dummy_len);

        // Confirma recebimento correto com mesmo número de sequência
        if (rv == 1 && ack_type == MSG_ACK && ack_seq == seq_num) {
            seq_num = (seq_num + 1) % SEQ_MODULO;
            return 0;
        }
    }
    
    return -1;
}

// Envia o arquivo do tesouro identificado por 'id'
static int send_file(int id) {
    const char *filename = get_treasure_filename(id);
    if (!filename) {
        return -1;
    }

    unsigned char fname_len = (unsigned char)strlen(filename);

    msg_type_t ini_type;
    if (ends_with(filename, ".txt"))
        ini_type = MSG_TEXTO_ACK_NOME;
    else if (ends_with(filename, ".mp4"))
        ini_type = MSG_VIDEO_ACK_NOME;
    else
        ini_type = MSG_IMAGEM_ACK_NOME;

    // Envia nome do arquivo
    if (send_reliable(ini_type, (const unsigned char *)filename, fname_len) < 0) {
        ui_show_status("Erro: não enviou nome de arquivo");
        return -1;
    }

    // Monta caminho e abre arquivo
    char path[96];
    snprintf(path, sizeof(path), "objetos/%s", filename);
    FILE *f = fopen(path, "rb");
    if (!f) {
        ui_show_status("Erro: não abriu arquivo");
        return -1;
    }

    // Verifica permissões e tipo do arquivo
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        unsigned char code = ERR_PERMISSAO;
        send_reliable(MSG_ERRO, &code, 1);
        return -1;
    }

    // Envia o tamanho do arquivo em 4 bytes
    uint32_t fsize = (uint32_t)st.st_size;
    unsigned char size_payload[4];
    size_payload[0] = (fsize >> 24) & 0xFF;
    size_payload[1] = (fsize >> 16) & 0xFF;
    size_payload[2] = (fsize >>  8) & 0xFF;
    size_payload[3] =  fsize        & 0xFF;

    if (send_reliable(MSG_TAMANHO, size_payload, 4) < 0) {
        ui_show_status("Erro: cliente não aceitou tamanho");
        fclose(f);
        return -1;
    }

    // Envia conteúdo do arquivo em blocos.
    unsigned char *buf = malloc(MAX_DATA_LEN);
    size_t n;
    while ((n = fread(buf, 1, MAX_DATA_LEN, f)) > 0) {
        if (send_reliable(MSG_DADOS, buf, (unsigned char)n) < 0) {
            ui_show_status("Erro: falha ao enviar dados");
            fclose(f);
            return -1;
        }
    }
    fclose(f);

    // Sinaliza fim do arquivo
    if (send_reliable(MSG_FIM_ARQ, NULL, 0) < 0) {
        ui_show_status("Erro: falha fim de arquivo");
        return -1;
    }

    free(buf);
    return 0;  // sucesso!
}

// Atualiza visualização do mapa com os tesouros ainda não enviados
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

    static int player_x = 0, player_y = 0;

    while ((opt = getopt(argc, argv, "i:")) != -1) {
        if (opt == 'i') iface = optarg;
    }
    if (!iface) {
        fprintf(stderr, "Uso: %s -i <interface>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sock_fd = open_raw_socket(iface);
    ui_init();

    init_treasures();


    desenhar_tesouros_restantes();
    memset(sent_flags, 0, sizeof(sent_flags));

    while (1) {
        unsigned char recv_seq, len;
        msg_type_t type;
        unsigned char payload[MAX_DATA_LEN];

        int rv = protocol_recv(sock_fd, &recv_seq, &type, payload, &len);
        if (rv <= 0) {
            continue;
        }

        if (type == MSG_ERRO) {
            // cancela eventual transmissão em andamento(erros vem do cliente logo após o nome ou tamanho do arquivo serem enviados).
            protocol_send(sock_fd, recv_seq, MSG_ACK, NULL, 0); // envia ACK apenas para confirmar o recebimento do erro; não avança seq_num
            ui_show_status("Cliente reportou erro, cancelando envio.");
            continue;
        }

        if (type >= MSG_MOV_DIR && type <= MSG_MOV_LEFT) {
            
            switch (type) {
                case MSG_MOV_UP: if (player_x < GRID_SIZE - 1) player_x++; break;
                case MSG_MOV_DOWN: if (player_x > 0) player_x--; break;
                case MSG_MOV_LEFT: if (player_y > 0) player_y--; break;
                case MSG_MOV_DIR: if (player_y < GRID_SIZE - 1) player_y++; break;
                default: break;
            }

            // Confirma movimento
            protocol_send(sock_fd, recv_seq, MSG_ACK, NULL, 0);

            desenhar_tesouros_restantes();
            char status[64];
            snprintf(status, sizeof(status), "Jogador em (%d,%d)", player_x, player_y);
            ui_show_status(status);


            // Verifica se jogador achou um tesouro ainda não enviado
            for (int i = 0; i < NUM_TREASURES; i++) {
                if (!sent_flags[i] && player_x == treasure_x[i] && player_y == treasure_y[i]) {
                    if (send_file(i) == 0) {
                        sent_flags[i] = true;
                        char msg[64];
                        snprintf(msg, sizeof(msg), "Enviando tesouro %d", i + 1);
                        ui_show_status(msg);
                        desenhar_tesouros_restantes();
                    }
                }
            }
        }
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
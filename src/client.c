#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "protocol.h"
#include "net.h"
#include "ui_client.h"

#define RECV_BUF_SIZE 1500

static int sock_fd;
static uint8_t seq_num = 0;

// Envia um movimento (stop-and-wait)
void send_move(protocol_msg_t mov_type) {
    uint8_t buf[FRAME_HEADER_SIZE];
    protocol_pack_frame(buf, seq_num, mov_type, NULL, 0);
    // enviar e aguardar ACK/NACK (implementar retransmissão)
    net_send(sock_fd, buf, FRAME_HEADER_SIZE);
    // TODO: espera de ACK e retransmissão em caso de timeout/NACK
    seq_num = (seq_num + 1) & 0x1F;
}

// Thread para receber mensagens do servidor
void *receive_loop(void *arg) {
    (void)arg;
    uint8_t in_buf[RECV_BUF_SIZE];
    while (1) {
        int n = net_recv(sock_fd, in_buf, sizeof(in_buf));
        if (n <= 0) continue;

        uint8_t recv_seq, pl_len;
        protocol_msg_t type;
        uint8_t payload[MAX_PAYLOAD_SIZE];

        if (protocol_unpack_frame(in_buf, n, &recv_seq, &type, payload, &pl_len) == 0) {
            switch (type) {
                case PROTO_MSG_TEXTO:
                case PROTO_MSG_IMAGEM:
                case PROTO_MSG_VIDEO:
                    // TODO: handle incoming file (init download)
                    break;
                case PROTO_MSG_DADOS:
                    // TODO: handle file chunk
                    break;
                case PROTO_MSG_FIM_ARQ:
                    // TODO: finalize file receive and open
                    break;
                default:
                    // outros tipos: ACK/NACK/etc
                    break;
            }
        }
    }
    return NULL;
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

    // iniciar thread de recebimento
    pthread_t thr;
    pthread_create(&thr, NULL, receive_loop, NULL);

    // loop principal de leitura de comandos
    while (1) {
        char cmd = ui_read_command();
        switch (cmd) {
            case 'w': send_move(PROTO_MSG_MOV_UP);    break;
            case 'a': send_move(PROTO_MSG_MOV_LEFT);  break;
            case 's': send_move(PROTO_MSG_MOV_DOWN);  break;
            case 'd': send_move(PROTO_MSG_MOV_DIR);   break;
            default: ui_show_message("Use W/A/S/D para mover");
        }
    }

    close(sock_fd);
    return 0;
}
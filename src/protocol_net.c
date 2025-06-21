#include "protocol_net.h"
#include "socket.h"
#include <stdio.h>
#include <stdlib.h>


int protocol_send(int sockfd, unsigned char seq, msg_type_t type, const unsigned char *data, unsigned char len)
{
    unsigned char *buf = malloc(PACKET_SIZE);

    if (!buf) return -1;

    int frame_len = pack_frame(seq, type, data, len, buf);
    if (!frame_len) {
        fprintf(stderr, "[protocol_send] Frame malformado!\n");
        free(buf);
        return -1;
    }

    int len_to_send = frame_len < 60 ? 60 : frame_len;

    int sent = send(sockfd, buf, len_to_send, 0);
    free(buf);
    return sent;
}

int protocol_recv(int sockfd, unsigned char *out_seq, msg_type_t *out_type, unsigned char *out_data,
                  unsigned char *out_len)
{

    unsigned char *buf = malloc(PACKET_SIZE);

    ssize_t n = recv(sockfd, buf, PACKET_SIZE, 0);
    if (n <= 0) {
        free(buf);
        return 0;
    }

    if (buf[0] != FRAME_MARKER) {
        free(buf);
        return 0; // marcador inválido
    }

    if (unpack_frame(buf, n, out_seq, out_type, out_data, out_len) < 0) {
        free(buf);
        return 0; // erro de checksum ou formato
    }

    free(buf);
    return 1;
}
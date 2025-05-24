#include "protocol_net.h"
#include <stdio.h>

#define MAX_FRAME_SIZE (4 + MAX_DATA_LEN)

/**
 * Aplica pack_frame no buffer de aplicação e envia via net_send.
 */
ssize_t protocol_send(int sockfd,
                      uint8_t seq,
                      msg_type_t type,
                      const uint8_t *data,
                      uint8_t len)
{
    uint8_t buf[MAX_FRAME_SIZE];
    size_t frame_len = pack_frame(seq, type, data, len, buf);
    if (!frame_len) return -1;
    return net_send(sockfd, buf, frame_len);
}

/**
 * Recebe bytes via net_recv e desempacota com unpack_frame.
 * Não descarta header Ethernet porque net_recv já fornece apenas payload
 * (usa PF_PACKET+SOCK_DGRAM).
 */
int protocol_recv(int sockfd,
                  uint8_t *out_seq,
                  msg_type_t *out_type,
                  uint8_t *out_data,
                  uint8_t *out_len)
{
    // buf precisa ter espaço suficiente para cabeçalho + payload
    uint8_t buf[14 + MAX_FRAME_SIZE];  // 14 bytes do cabeçalho Ethernet
    ssize_t n = net_recv(sockfd, buf, sizeof(buf));
    // printf("[DEBUG] net_recv retornou %zd bytes\n", n);

if (n < 14 + 4) return 0; // pacote pequeno demais

// Verifica EtherType (bytes 12 e 13 do cabeçalho Ethernet)
if (!(buf[12] == 0x88 && buf[13] == 0x88)) {
    // fprintf(stderr, "[RECV] EtherType inválido: %02X %02X\n", buf[12], buf[13]);
    return 0;
}

// Verifica marcador no primeiro byte do payload
if (buf[14] != FRAME_MARKER) {
    // fprintf(stderr, "[RECV] Marcador inválido: 0x%02X\n", buf[14]);
    return 0;
}

    if (n == 0) return 0;    // timeout
    if (n < 0) return -1;    // erro

    //printf("[DEBUG] Dump do pacote recebido:\n");
    //for (int i = 0; i < n; i++) {
    //    printf("%02X ", buf[i]);
    //    if ((i+1) % 16 == 0) printf("\n");
    //}
    //printf("\n");

    // Pular o cabeçalho Ethernet
    uint8_t *payload = buf + 14;
    ssize_t payload_len = n - 14;
    // desempacota os dados recebidos diretamente
    if (unpack_frame(payload, payload_len, out_seq, out_type, out_data, out_len) < 0)
        return -1; // checksum ou formato inválido
    return 1;
}
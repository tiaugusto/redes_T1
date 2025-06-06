#include "net.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>    // ETH_P_ALL
#include <linux/if_ether.h>  // struct packet_mreq
#include <arpa/inet.h>


int global_ifindex = -1;

int net_init(const char *iface) {
    // 1) Cria raw socket
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("net_init: socket");
        exit(EXIT_FAILURE);
    }

    // 2) Descobre índice da interface
    global_ifindex = if_nametoindex(iface);
    if (global_ifindex == 0) {
        perror("net_init: if_nametoindex");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 3) Faz bind do socket à interface
    struct sockaddr_ll addr = {0};
    addr.sll_family   = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex  = global_ifindex;
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("net_init: bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 4) Ativa modo promíscuo para ler pacotes do nosso protocolo desconhecido
    struct packet_mreq mr = {0};
    mr.mr_ifindex = global_ifindex;
    mr.mr_type    = PACKET_MR_PROMISC;
    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
                   &mr, sizeof(mr)) < 0)
    {
        perror("net_init: PACKET_ADD_MEMBERSHIP");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 5) Configura timeout de recv() em 300 ms
    struct timeval tv = { .tv_sec = 0, .tv_usec = 300000 };
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &tv, sizeof(tv)) < 0)
    {
        perror("net_init: SO_RCVTIMEO");
        // não sai, pois é só timeout
    }

    return sockfd;
}

#define ETH_HDR_LEN 14
#define MIN_FRAME_LEN 60  // opcional, mas recomendado para Ethernet
#define MAX_FRAME_LEN 131

ssize_t net_send(int sockfd, const void *buf, size_t len) {
    // 1) buffer para frame completo
    uint8_t frame[ETH_HDR_LEN + MAX_FRAME_LEN]; 
    if (len > MAX_FRAME_LEN) return -1;

    memset(frame, 0, sizeof(frame));

    // 2) preenche cabeçalho Ethernet
    uint8_t *p = frame;
    memset(p, 0xFF, 6);                        // MAC destino = broadcast
    // (aqui você pode buscar o MAC real da interface e usar memcpy)
    memset(p+6, 0x11, 6);                      // MAC origem (exemplo fixo)
    uint16_t ether_type = htons(0x8888);       // EtherType custom
    memcpy(p+12, &ether_type, 2);

    // 3) copia o payload logo após o header
    memcpy(frame + ETH_HDR_LEN, buf, len);

    size_t frame_len = ETH_HDR_LEN + len;
    if (frame_len < MIN_FRAME_LEN) frame_len = MIN_FRAME_LEN;

    // 4) endereço de destino para sendto (broadcast)
    struct sockaddr_ll dest = {0};
    dest.sll_family   = AF_PACKET;
    dest.sll_protocol = htons(0x8888);
    dest.sll_ifindex  = global_ifindex;
    dest.sll_halen    = ETH_ALEN;
    memset(dest.sll_addr, 0xFF, ETH_ALEN);

    // 5) envia o quadro inteiro
    ssize_t sent = sendto(sockfd, frame, frame_len, 0,
                          (struct sockaddr *)&dest,
                          sizeof(dest));
    if (sent < 0) perror("net_send: sendto");
    return sent;
}


ssize_t net_recv(int sockfd, void *buf, size_t len) {
    // recvfrom retornará -1 em timeout (EWOULDBLOCK/EAGAIN)
    return recvfrom(sockfd, buf, len, 0, NULL, NULL);
}
#include "socket.h"
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

static int iface_index = -1;

void socket_error(const char *msg) {
    perror(msg);
    exit(1);
}

int open_raw_socket(const char *iface) {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
        socket_error("Erro ao abrir socket");

    iface_index = if_nametoindex(iface);
 
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = iface_index;

    if (bind(sockfd, (struct sockaddr*) &endereco, sizeof(endereco)) == -1)
        socket_error("Erro ao fazer bind");
 
    struct packet_mreq mr = {0};
    mr.mr_ifindex = iface_index;
    mr.mr_type = PACKET_MR_PROMISC;

    struct timeval timeout = { .tv_sec = 0, .tv_usec = TIME_OUT_MSECONDS * 1000};

    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
        socket_error("Erro ao fazer setsockopt para modo promíscuo");

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) == -1)
        socket_error("Erro ao fazer setsockopt para timeout");

    return sockfd;
}

int get_iface_index() {
    return iface_index;
}


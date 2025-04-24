#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define ETHERTYPE_CUSTOM 0x88B5  // escolha seu Ethertype (network byte order no bind)

int net_init(const char *iface) {
    int sock;
    if ((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl(SIOCGIFINDEX)");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_ll addr = {0};
    addr.sll_family   = AF_PACKET;
    addr.sll_ifindex  = ifr.ifr_ifindex;
    addr.sll_protocol = htons(ETHERTYPE_CUSTOM);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;
}

ssize_t net_send(int fd, const uint8_t *buf, size_t len) {
    ssize_t sent = send(fd, buf, len, 0);
    if (sent < 0) perror("net_send: send");
    return sent;
}

ssize_t net_recv(int fd, uint8_t *buf, size_t bufsize) {
    ssize_t rec = recv(fd, buf, bufsize, 0);
    if (rec < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        perror("net_recv: recv");
    return rec;
}

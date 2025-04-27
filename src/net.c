#include "net.h"

#include <stdio.h>          // perror
#include <unistd.h>         // close
#include <string.h>         // memset
#include <errno.h>          // errno, EAGAIN, EWOULDBLOCK
#include <sys/socket.h>     // send, recv, setsockopt
#include <sys/time.h>       // struct timeval
#include <linux/if_packet.h> // PACKET_MR_PROMISC, packet_mreq
#include <net/if.h>         // if_nametoindex

void net_init(int sockfd)
{
    // 1) Timeout de 300 ms em recv()
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 300 * 1000;  // 300 milissegundos
    if (setsockopt(sockfd,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   &tv,
                   sizeof(tv)) < 0) {
        perror("net_init: SO_RCVTIMEO");
        // Sem timeout, recv() bloqueia até chegar algo
    }

#ifdef DEBUG
    // 2)Modo promíscuo (só em dev)
    struct packet_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.mr_ifindex = if_nametoindex("eth0");  //troque se não for eth0
    mreq.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(sockfd,
                   SOL_PACKET,
                   PACKET_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0) {
        perror("net_init: PROMISC");
        //Se falhar, apenas não veremos todos os quadros
    }
#endif
}

ssize_t net_send(int sockfd, const void *buf, size_t len)
{
    ssize_t n = send(sockfd, buf, len, 0);
    if (n < 0) {
        perror("net_send: send");
        // quem chamar decide retransmitir
    }
    return n;
}

ssize_t net_recv(int sockfd, void *buf, size_t len)
{
    ssize_t n = recv(sockfd, buf, len, 0);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Timeout de 300 ms atingido
            return 0;
        }
        perror("net_recv: recv");
        return -1;
    }
    return n;
}

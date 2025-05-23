// net.h
#ifndef NET_H
#define NET_H

#include <sys/types.h>  // ssize_t

int net_init(const char *iface);
ssize_t net_send(int sockfd, const void *buf, size_t len);
ssize_t net_recv(int sockfd, void *buf, size_t len);

extern int global_ifindex;

#endif /* NET_H */

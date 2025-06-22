#ifndef PROTOCOL_NET_H
#define PROTOCOL_NET_H

#include <stdint.h>
#include <unistd.h>
#include "protocol.h"
#include "socket.h"

int protocol_send(int sockfd,
                      unsigned char seq,
                      msg_type_t type,
                      const unsigned char *data,
                      unsigned char len);

int protocol_recv(int sockfd,
                  unsigned char *out_seq,
                  msg_type_t *out_type,
                  unsigned char *out_data,
                  unsigned char *out_len);

#endif // PROTOCOL_NET_H

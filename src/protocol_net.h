#ifndef PROTOCOL_NET_H
#define PROTOCOL_NET_H

#include <stdint.h>
#include <unistd.h>
#include "protocol.h"
#include "net.h"

ssize_t protocol_send(int sockfd,
                      uint8_t seq,
                      msg_type_t type,
                      const uint8_t *data,
                      uint8_t len);

int protocol_recv(int sockfd,
                  uint8_t *out_seq,
                  msg_type_t *out_type,
                  uint8_t *out_data,
                  uint8_t *out_len);

#endif // PROTOCOL_NET_H

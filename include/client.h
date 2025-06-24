#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include "protocol.h"


void send_move(msg_type_t mov_type);
void process_packet(unsigned char seq, msg_type_t type, const unsigned char *payload, unsigned char len);
void confirm_move(msg_type_t mov_type);

#endif // CLIENT_H

#ifndef SERVER_H
#define SERVER_H

#define NUM_TREASURES 8
#define GRID_SIZE 8

#include "protocol.h"

int send_file(int id);
int send_reliable(msg_type_t type, const unsigned char *payload, unsigned char len);
void desenhar_tesouros_restantes(void);

#endif // SERVER_H

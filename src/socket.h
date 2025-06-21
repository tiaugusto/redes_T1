#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>

#define TIME_OUT_MSECONDS 300

int open_raw_socket(const char *iface);

int get_iface_index();
#endif

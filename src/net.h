#ifndef NET_H
#define NET_H

#include <stdint.h>
#include <sys/types.h>

/**
 * Inicializa um socket RAW na interface especificada.
 *
 * @param iface Nome da interface (ex: "eth0").
 * @return descritor de arquivo do socket aberto.
 * @exit em caso de erro crítico.
 */
int net_init(const char *iface);

/**
 * Envia dados brutos no socket RAW.
 *
 * @param fd    descritor de arquivo do socket (retornado por net_init)
 * @param buf   buffer de bytes a enviar
 * @param len   número de bytes a enviar
 * @return número de bytes efetivamente enviados, ou -1 em erro.
 */
ssize_t net_send(int fd, const uint8_t *buf, size_t len);

/**
 * Recebe dados brutos do socket RAW.
 *
 * @param fd       descritor de arquivo do socket
 * @param buf      buffer de recepção
 * @param bufsize  tamanho do buffer de recepção
 * @return número de bytes lidos, ou -1 em erro.
 */
ssize_t net_recv(int fd, uint8_t *buf, size_t bufsize);

#endif // NET_H
#ifndef NET_H
#define NET_H

#include <sys/types.h>   // ssize_t, size_t

/**
 * @brief Inicializa o raw socket após criação e bind:
 *   - Define SO_RCVTIMEO = 300 ms
 *   - Em DEBUG, ativa PACKET_MR_PROMISC na eth0
 *
 * @param sockfd Socket AF_PACKET/SOCK_RAW já criado e bindado.
 */
void net_init(int sockfd);

/**
 * @brief Envia len bytes pelo raw socket.
 * @param sockfd Socket bruto
 * @param buf    Dados a enviar
 * @param len    Quantidade de bytes
 * @return >0 = bytes enviados; –1 = erro (perror já chamado)
 */
ssize_t net_send(int sockfd, const void *buf, size_t len);

/**
 * @brief Recebe até len bytes do raw socket.
 * @param sockfd Socket bruto
 * @param buf    Buffer de saída
 * @param len    Capacidade de buf
 * @return >0 = bytes lidos;
 *          0 = timeout (300 ms sem dados);
 *         –1 = erro fatal (perror já chamado)
 */
ssize_t net_recv(int sockfd, void *buf, size_t len);

#endif /* NET_H */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "net.h"

void fail(const char *m) {
    perror(m);
    exit(1);
}

int main(void) {
    printf("→ Net: testes exaustivos\n");
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
        fail("socketpair");

    // 1) Aplica timeout de 300 ms em ambos os sockets
    net_init(sv[0]);
    net_init(sv[1]);

    // 2) Verifica que SO_RCVTIMEO foi realmente ajustado em sv[0]
    struct timeval tv;
    socklen_t optlen = sizeof(tv);
    assert(getsockopt(sv[0], SOL_SOCKET, SO_RCVTIMEO, &tv, &optlen) == 0);
    assert(tv.tv_sec == 0 && tv.tv_usec == 300*1000);

    // 3) Testa send/recv com vários tamanhos
    for (int len = 1; len <= 256; len *= 2) {
        uint8_t wbuf[256], rbuf[256];
        for (int i = 0; i < len; i++) wbuf[i] = (uint8_t)(i * 7);
        assert(net_send(sv[0], wbuf, len) == len);

        ssize_t n = net_recv(sv[1], rbuf, len);
        assert(n == len);
        assert(memcmp(wbuf, rbuf, len) == 0);
    }

    // 4) Agora, sem enviar nada no sv[1], net_recv deve timeout e retornar 0
    assert(net_recv(sv[1], sv, sizeof(sv)) == 0);

    // 5) Erro em socket inválido
    assert(net_send(-1, sv, sizeof(sv)) < 0);
    assert(net_recv(-1, sv, sizeof(sv)) < 0);

    close(sv[0]);
    close(sv[1]);
    printf("✅ net_extensive OK\n");
    return 0;
}

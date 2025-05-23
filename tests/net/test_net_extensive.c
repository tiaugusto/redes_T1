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
    printf("  [1] Aplicando timeout de 300ms nos sockets...\n");
    net_init(sv[0]);
    net_init(sv[1]);

    // 2) Verifica que SO_RCVTIMEO foi realmente ajustado
    struct timeval tv;
    socklen_t optlen = sizeof(tv);
    assert(getsockopt(sv[0], SOL_SOCKET, SO_RCVTIMEO, &tv, &optlen) == 0);
    assert(tv.tv_sec == 0 && tv.tv_usec == 300 * 1000);
    printf("     ✔ Timeout corretamente configurado: %ld.%06ld s\n", (long)tv.tv_sec, (long)tv.tv_usec);

    // 3) Testa send/recv com tamanhos variados
    printf("  [2] Testando envio/recebimento com tamanhos variados:\n");
    for (int len = 1; len <= 256; len *= 2) {
        uint8_t wbuf[256], rbuf[256];
        for (int i = 0; i < len; i++) wbuf[i] = (uint8_t)(i * 7);

        printf("     - Tamanho %3d: enviando...", len);
        assert(net_send(sv[0], wbuf, len) == len);

        ssize_t n = net_recv(sv[1], rbuf, len);
        assert(n == len);
        assert(memcmp(wbuf, rbuf, len) == 0);
        printf(" recebido ✔️\n");
    }

    // 4) Timeout: sem enviar nada, net_recv deve retornar 0
    printf("  [3] Testando timeout (sem enviar dados)...\n");
    uint8_t dummy[10];
    ssize_t result = net_recv(sv[1], dummy, sizeof(dummy));
    if (result == 0)
        printf("     ✔ Timeout ocorreu corretamente (net_recv retornou 0)\n");
    else
        fail("timeout falhou");

    // 5) Erros em socket inválido
    printf("  [4] Testando erro em socket inválido (-1)...\n");
    if (net_send(-1, sv, sizeof(sv)) < 0)
        printf("     ✔ net_send com fd=-1 retornou erro como esperado\n");
    else
        fail("net_send não falhou com fd inválido");

    if (net_recv(-1, sv, sizeof(sv)) < 0)
        printf("     ✔ net_recv com fd=-1 retornou erro como esperado\n");
    else
        fail("net_recv não falhou com fd inválido");

    close(sv[0]);
    close(sv[1]);

    printf("✅ net_extensive OK\n");
    return 0;
}

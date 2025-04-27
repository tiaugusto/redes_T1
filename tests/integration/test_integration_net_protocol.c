#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include "protocol.h"
#include "net.h"

static void fail(const char *m){ fprintf(stderr,"❌ %s\n",m); _exit(1); }

int main(void) {
    printf("→ Teste integração protocol+net\n");
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv)<0) fail("socketpair");
    net_init(sv[0]); net_init(sv[1]);

    struct {uint8_t seq; msg_type_t type; const uint8_t *d; uint8_t l;} cases[] = {
        {  0, MSG_ACK,               NULL,     0 },
        {  5, MSG_DADOS, (uint8_t*)"Hello",   5 },
        { 10, MSG_MOV_UP,            NULL,     0 },
        { 20, MSG_DADOS, (uint8_t*)"Lorem ipsum dolor sit amet", 26 },
        { 31, MSG_ERRO,              NULL,     1 }
    };

    uint8_t sb[4+MAX_DATA_LEN], rb[4+MAX_DATA_LEN], out[MAX_DATA_LEN];
    uint8_t s2,t2,l2; size_t fl;

    for (int i=0;i<5;i++) {
        auto c = cases[i];
        if (c.type==MSG_ERRO)
            fl = pack_error_frame(c.seq, ERR_NO_SPACE, sb);
        else
            fl = pack_frame(c.seq, c.type, c.d, c.l, sb);
        if (fl==0) fail("pack falhou");
        if (net_send(sv[0], sb, fl)<0) fail("send falhou");
        ssize_t r = net_recv(sv[1], rb, sizeof(rb));
        if (r<=0) fail("recv falhou/timeout");
        if ((size_t)r!=fl) fail("tamanho recv!=send");
        if (unpack_frame(rb, r, &s2, &t2, out, &l2)<0) fail("unpack falhou");
        if (s2!=c.seq) fail("seq mismatch");
        if (t2!=c.type) fail("type mismatch");
        if (c.type==MSG_ERRO) {
            if (l2!=1||out[0]!=ERR_NO_SPACE) fail("erro mismatch");
        } else {
            if (l2!=c.l||memcmp(out,c.d,c.l)) fail("payload mismatch");
        }
    }

    close(sv[0]); close(sv[1]);
    printf("✅ integração OK\n");
    return 0;
}

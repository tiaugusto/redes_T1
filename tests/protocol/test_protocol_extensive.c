#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

void fail(const char *msg) {
    fprintf(stderr, "❌ %s\n", msg);
    exit(1);
}

int main(void) {
    printf("→ Protocol: testes exaustivos\n");
    uint8_t buf[4 + MAX_DATA_LEN], out[MAX_DATA_LEN];
    uint8_t seq2, len2;
    msg_type_t type2;
    size_t fl;

    uint8_t seqs[]  = {0,1,15,16,31};
    uint8_t types[] = {0,1,5,8,15};
    uint8_t lens[]  = {0,1,50,127};

    // combinações de seq, type, len
    for (size_t i=0;i<sizeof(seqs);i++)
    for (size_t j=0;j<sizeof(types);j++)
    for (size_t k=0;k<sizeof(lens);k++) {
        uint8_t s=seqs[i], t=types[j], l=lens[k];
        // preenche payload de teste
        for (int x=0; x<l; x++) buf[4+x] = (uint8_t)(x*31 + s + t);
        fl = pack_frame(s, t, buf+4, l, buf);
        if (fl != (size_t)(4 + l)) fail("pack_frame tamanho errado");
        if (unpack_frame(buf, fl, &seq2, &type2, out, &len2) < 0)
            fail("unpack_frame falhou");
        if (seq2!=s || type2!=t || len2!=l ||
            (l>0 && memcmp(out, buf+4, l)!=0))
            fail("Dados divergentes");
    }

    // parâmetros inválidos
    assert(pack_frame(32,0,NULL,0,buf)==0);
    assert(pack_frame(0,16,NULL,0,buf)==0);
    assert(pack_frame(0,0,NULL,128,buf)==0);

    // unpack com erros
    buf[0]=0x00; assert(unpack_frame(buf,4,&seq2,&type2,out,&len2)==-1);
    buf[0]=FRAME_MARKER;
    assert(unpack_frame(buf,3,&seq2,&type2,out,&len2)==-1);
    fl = pack_frame(5,MSG_DADOS,(uint8_t*)"A",1,buf);
    buf[3]^=0xFF; assert(unpack_frame(buf,fl,&seq2,&type2,out,&len2)==-1);

        // Teste com mensagens reais
    printf("→ Teste com mensagens reais\n");

    struct {
        uint8_t seq;
        msg_type_t type;
        const char *msg;
    } exemplos[] = {
        { 2, MSG_DADOS,             "Olá, mundo!" },
        { 3, MSG_TEXTO_ACK_NOME,    "capitulo1.txt" },
        { 4, MSG_VIDEO_ACK_NOME,    "video_intro.mp4" },
        { 5, MSG_IMAGEM_ACK_NOME,   "foto123.jpg" },
        { 6, MSG_MOV_DIR,           "" },
        { 7, MSG_MOV_LEFT,          "" }
    };

    for (size_t i = 0; i < sizeof(exemplos)/sizeof(exemplos[0]); i++) {
        uint8_t len = (uint8_t)strlen(exemplos[i].msg);
        printf("  [%zu] seq=%u, type=%u, msg=\"%s\"\n",
               i, exemplos[i].seq, exemplos[i].type, exemplos[i].msg);

        fl = pack_frame(exemplos[i].seq, exemplos[i].type,
                        (const uint8_t *)exemplos[i].msg, len, buf);
        if (fl != (size_t)(4 + len)) fail("real: tamanho errado");

        if (unpack_frame(buf, fl, &seq2, &type2, out, &len2) < 0)
            fail("real: unpack falhou");

        if (seq2 != exemplos[i].seq) fail("real: seq mismatch");
        if (type2 != exemplos[i].type) fail("real: type mismatch");
        if (len2 != len) fail("real: len mismatch");

        if (len2 > 0 && memcmp(out, exemplos[i].msg, len2) != 0) {
            printf("   Esperado: %.*s\n", len, exemplos[i].msg);
            printf("   Recebido: %.*s\n", len2, out);
            fail("real: payload mismatch");
        }

        // printa resultado decodificado
        if (len2 > 0)
            printf("     ✔ Mensagem recebida: \"%.*s\"\n", len2, out);
        else
            printf("     ✔ Sem payload (comando)\n");
    }

    printf("✅ protocol_extensive OK\n");

    return 0;
}

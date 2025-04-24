#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// —————————— Constantes do frame ——————————
#define FRAME_START_MARKER 0x7E
#define MAX_PAYLOAD_SIZE   127

// —————————— Tipos de mensagem (4 bits) ——————————
typedef enum {
    MSG_ACK = 0,
    MSG_NACK = 1,
    MSG_OK_ACK = 2,
    MSG_TAMANHO = 4,
    MSG_DADOS = 5,
    MSG_TEXTO_ACK_NOME = 6,
    MSG_VIDEO_ACK_NOME = 7,
    MSG_IMAGEM_ACK_NOME = 8,
    MSG_FIM_ARQ = 9,
    MSG_MOV_DIR = 10,
    MSG_MOV_UP = 11,
    MSG_MOV_DOWN = 12,
    MSG_MOV_LEFT = 13,
    MSG_ERRO = 15
} msg_type_t;

// —————————— Códigos de erro (payload de erro) ——————————
typedef enum {
    ERR_NO_PERMISSION = 0,
    ERR_NO_SPACE = 1
} err_code_t;

// —————————— Estrutura de um frame ——————————
#pragma pack(push,1)
typedef struct {
    uint8_t   start;       // = FRAME_START_MARKER
    uint8_t   size_seq;    // bits [7..1]=tamanho, [0]=primeiro bit de sequência
    uint8_t   seq_type;    // bits [7..3]=seq (5 bits), [2..0]=tipo (3 bits) + 1 bit livre!
    uint8_t   checksum;
    uint8_t   payload[MAX_PAYLOAD_SIZE];
} frame_t;
#pragma pack(pop)

// —————————— Helpers de montagem/desmontagem ——————————
uint8_t build_size_seq(uint8_t size, uint8_t seq);
uint8_t build_seq_type(uint8_t seq, msg_type_t type);
uint8_t calc_checksum(const frame_t *f);

#endif // PROTOCOL_H

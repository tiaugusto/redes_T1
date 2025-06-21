#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/*
campos da msg e onde estão:
    marcador início	8 bits	Byte 0 (0x7E)
    tamanho	7 bits	Bits 7..1 do Byte 1
    MSB de seq	1 bit	Bit 0 do Byte 1
    LSBs de seq	4 bits	Bits 7..4 do Byte 2
    tipo	4 bits	Bits 3..0 do Byte 2
    checksum	8 bits	Byte 3
    payload	0–127 bytes	Bytes 4…x (conforme “tamanho”)
*/

/*— Constantes —*/
#define FRAME_MARKER 0x7E    // 0111 1110
#define MAX_DATA_LEN 127     // payload máximo
#define PACKET_SIZE 131
#define SEQ_MODULO 32      // 5 bits → 0…31
#define ERROR_NO_PERMISSION 0
#define ERR_PERMISSAO    0   /* sem permissão de acesso */
#define ERR_DISK_FULL    1   /* espaço insuficiente */

#define MAX_RETRANSMISSIONS 5

/*— Tipos de mensagem (4 bits) —*/
typedef enum {
    MSG_ACK = 0,
    MSG_NACK = 1,
    MSG_OK_ACK = 2,
    MSG_INI_ARQ = 3,
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
    /*  14 livre    */
    MSG_ERRO = 15
} msg_type_t;

typedef enum {
    ERR_NO_PERMISSION = 0,
    ERR_NO_SPACE = 1
} err_code_t;

/**
 *  Calcula o checksum do frame:
 *          checksum = ~ (len + seq + type + Σdata[i])
 * len   número de bytes de payload (0…MAX_DATA_LEN)
 * seq   número de sequência (0…SEQ_MODULO-1)
 * type  tipo de mensagem (0…15)
 * data  ponteiro para payload (ou NULL se len==0)
 * retorna checksum de 8 bits
 */
unsigned char compute_csum(unsigned char len,unsigned char seq,msg_type_t type,const unsigned char *data);


int pack_frame(unsigned char seq, msg_type_t type, const unsigned char *data, unsigned char len, unsigned char *buf);


int unpack_frame(const unsigned char *buf, int length, unsigned char *out_seq, msg_type_t *out_type, unsigned char *out_data, unsigned char *out_len);



#endif

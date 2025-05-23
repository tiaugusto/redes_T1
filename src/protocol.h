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
#define SEQ_MODULO 32      // 5 bits → 0…31

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
 * @brief   Calcula o checksum do frame:
 *          checksum = ~ (len + seq + type + Σdata[i])
 * @param   len   número de bytes de payload (0…MAX_DATA_LEN)
 * @param   seq   número de sequência (0…SEQ_MODULO-1)
 * @param   type  tipo de mensagem (0…15)
 * @param   data  ponteiro para payload (ou NULL se len==0)
 * @return  checksum de 8 bits
 */
uint8_t compute_csum(uint8_t len,
                         uint8_t seq,
                         msg_type_t type,
                         const uint8_t *data);

/**
 * @brief Empacota um frame de acordo com a mensagem descrita no início do arquivo
 *
 * @param seq     0…31 (5 bits)
 * @param type    0…15 (4 bits)
 * @param data    ponteiro (ou NULL se len==0) 
 * @param len     0…MAX_DATA_LEN (7 bits)
 * @param buf     saída, precisa ter (4+len) bytes
 * @return        total de bytes escritos (4+len), ou 0 em erro
 */
size_t pack_frame(uint8_t seq,
                  uint8_t type,
                  const uint8_t *data,
                  uint8_t len,
                  uint8_t *buf);

/**
 * @brief Desempacota buf[0..length−1], validando marker/tamanho/csum.
 * @return 0 em sucesso (preenche seq/type/data/len), –1 em erro.
 */
int unpack_frame(const uint8_t *buf,
                 size_t length,
                 uint8_t *out_seq,
                 msg_type_t *out_type,
                 uint8_t *out_data,
                 uint8_t *out_len);

#endif

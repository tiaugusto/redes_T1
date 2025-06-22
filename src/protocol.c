#include "protocol.h"
#include <stdio.h>
#include <string.h>



/* checksum = ~ (len, seq, type e data) */
unsigned char compute_csum(unsigned char len, unsigned char seq, msg_type_t type, const unsigned char *data)
{
    unsigned int sum = len + seq + type;
    for (int i = 0; i < len; i++) sum += data[i];
    return (unsigned char)(~sum);
}

/* Monta um frame no buffer. Retorna tamanho do frame gerado, ou 0 se erro. */
int pack_frame(unsigned char seq, msg_type_t type, const unsigned char *data, unsigned char len, unsigned char *buf)
{
    if (len > MAX_DATA_LEN || seq >= SEQ_MODULO || type >= 16)
        return 0;

    // Byte 0 = marcador
    buf[0] = FRAME_MARKER;

    // Byte 1 = len[6..0] <<1  | seq[4]
    buf[1] = (unsigned char)((len << 1) | ((seq >> 4) & 0x01));

    // Byte 2 = seq[3..0] <<4 | type[3..0]
    buf[2] = (unsigned char)(((seq & 0x0F) << 4) | (type & 0x0F));

    // Duplicação
    unsigned char expanded_data[2 * MAX_DATA_LEN];
    for (int i = 0; i < len; i++) {
        expanded_data[2 * i] = data[i];
        expanded_data[2 * i + 1] = 0xFF;
    }

    // Byte 3 = checksum
    buf[3] = compute_csum(len, seq, type, data);

    // Bytes 4…x = payload
    if (len)
        memcpy(buf + 4, expanded_data, 2*len);

    return 4 + 2 * len;
}

int unpack_frame(const unsigned char *buf, int length, unsigned char *out_seq, msg_type_t *out_type, unsigned char *out_data, unsigned char *out_len)
{
    if (length < 4) {
        return -1;
    }

    if (buf[0] != FRAME_MARKER) {
        return -1;
    }

    // Byte 1 e Byte 2
    unsigned char b1 = buf[1];
    unsigned char len = b1 >> 1;
    unsigned char msb_seq = b1 & 0x01;

    unsigned char b2 = buf[2];
    unsigned char seq_low = (b2 >> 4) & 0x0F;
    unsigned char type = b2 & 0x0F;

    unsigned char seq = (msb_seq << 4) | seq_low;

    if (length < 4 + 2 * len) {
        return -1;
    }

    // Remove duplicação
    unsigned char real_data[MAX_DATA_LEN];
    for (int i = 0; i < len; i++) {
        real_data[i] = buf[4 + 2 * i];  // pega só os bytes reais
    }

    unsigned char checksum = compute_csum(len, seq, type, real_data);
    if (buf[3] != checksum) {
        return -1;
    }

    // sucesso
    *out_len = len;
    *out_seq = seq;
    *out_type = type;
    if (len > 0 && out_data != NULL)
        memcpy(out_data, real_data, len);

    return 0;
}

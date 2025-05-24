#include "protocol.h"
#include <stdio.h>
#include <string.h>



/* checksum = ~ (len + seq + type + sum(bytes)) */
uint8_t compute_csum(uint8_t len,
                            uint8_t seq,
                            msg_type_t type,
                            const uint8_t *data)
{
    uint32_t sum = len + seq + type;
    for (int i = 0; i < len; i++) sum += data[i];
    return (uint8_t)(~sum);
}

size_t pack_frame(uint8_t seq,
                  uint8_t type,
                  const uint8_t *data,
                  uint8_t len,
                  uint8_t *buf)
{
    if (len > MAX_DATA_LEN || seq >= SEQ_MODULO || type >= 16)
        return 0;

    // Byte 0 = marcador
    buf[0] = FRAME_MARKER;

    // Byte 1 = len[6..0] <<1  | seq[4]
    buf[1] = (uint8_t)((len << 1) | ((seq >> 4) & 0x01));

    // Byte 2 = seq[3..0] <<4 | type[3..0]
    buf[2] = (uint8_t)(((seq & 0x0F) << 4) | (type & 0x0F));

    // Byte 3 = checksum
    buf[3] = compute_csum(len, seq, type, data);

    // Bytes 4…x = payload
    if (len)
        memcpy(buf + 4, data, len);

    return 4 + len;
}

int unpack_frame(const uint8_t *buf,
                 size_t length,
                 uint8_t *out_seq,
                 msg_type_t *out_type,
                 uint8_t *out_data,
                 uint8_t *out_len)
{
    if (length < 4) {
        fprintf(stderr, "[UNPACK] Rejeitado: pacote muito curto (%zu bytes)\n", length);
        return -1;
    }

    if (buf[0] != FRAME_MARKER) {
        fprintf(stderr, "[UNPACK] Rejeitado: marcador inválido (0x%02X)\n", buf[0]);
        return -1;
    }

    // Byte 1 e Byte 2
    uint8_t b1 = buf[1];
    uint8_t len = b1 >> 1;
    uint8_t msb_seq = b1 & 0x01;

    uint8_t b2 = buf[2];
    uint8_t seq_low = (b2 >> 4) & 0x0F;
    uint8_t type = b2 & 0x0F;

    uint8_t seq = (uint8_t)((msb_seq << 4) | seq_low);

    if (length < (size_t)(4 + len)) {
        fprintf(stderr, "[UNPACK] Rejeitado: pacote curto demais. Esperado no mínimo %u, recebido %zu\n",
                4 + len, length);
        return -1;
    }

    uint8_t checksum = compute_csum(len, seq, type, buf + 4);
    if (buf[3] != checksum) {
        fprintf(stderr,
                "[UNPACK] Rejeitado: checksum inválido. Esperado 0x%02X, recebido 0x%02X\n",
                checksum, buf[3]);
        return -1;
    }

    // sucesso
    *out_len  = len;
    *out_seq  = seq;
    *out_type = type;
    if (len > 0 && out_data != NULL)
        memcpy(out_data, buf + 4, len);

    // fprintf(stderr, "[UNPACK] OK: seq=%u, type=%u, len=%u\n", seq, type, len);
    return 0;
}

#include "protocol.h"
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
    if (length < 4 || buf[0] != FRAME_MARKER)
        return -1;

    // Byte 1
    uint8_t b1 = buf[1];
    uint8_t len = b1 >> 1;        // bits7..1
    uint8_t msb_seq = b1 & 0x01;  // bit0, 0x01 = 00000001

    // Byte 2
    uint8_t b2 = buf[2];
    uint8_t seq_low = (b2 >> 4) & 0x0F; // bits7..4, 0x0F = 00001111
    uint8_t type = b2 & 0x0F;        // bits3..0

    if (length != (size_t)(4 + len))
        return -1;

    uint8_t seq = (uint8_t)((msb_seq << 4) | seq_low);

    // Valida checksum
    if (buf[3] != compute_csum(len, seq, type, buf + 4))
        return -1;

    *out_len  = len;
    *out_seq  = seq;
    *out_type = type;
    if (len)
        memcpy(out_data, buf + 4, len);

    return 0;
}
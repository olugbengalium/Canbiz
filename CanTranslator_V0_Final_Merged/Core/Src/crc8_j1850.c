#include "crc8_j1850.h"

#define CRC8_J1850_POLY   0x1D
#define CRC8_J1850_INIT   0xFF
#define CRC8_J1850_XOROUT 0xFF

uint8_t Crc8_J1850_Compute(const uint8_t *data, size_t len)
{
    uint8_t crc = CRC8_J1850_INIT;

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ CRC8_J1850_POLY);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return (uint8_t)(crc ^ CRC8_J1850_XOROUT);
}

bool Crc8_J1850_ValidateFrame(const uint8_t frame[8])
{
    uint8_t computed = Crc8_J1850_Compute(frame, 7); /* B0..B6 */
    return (computed == frame[7]);
}

void Crc8_J1850_StampFrame(uint8_t frame[8])
{
    frame[7] = Crc8_J1850_Compute(frame, 7);
}

bool Crc8_J1850_ValidateFrame_06D(const uint8_t frame[4])
{
    uint8_t computed = Crc8_J1850_Compute(frame, 3);
    return (computed == frame[3]);
}

void Crc8_J1850_StampFrame_06D(uint8_t frame[4])
{
    frame[3] = Crc8_J1850_Compute(frame, 3);
}

/*
 * crc8_j1850.h
 *
 * Corrected integrity API from the Phase 8/9 empirical corpus.
 *
 * Standard 8-byte J1850 frames (B0..B6 -> B7):
 *   0x001, 0x073, 0x0D5, 0x0F1, 0x105, 0x14B, 0x17D, 0x1CD
 *
 * Special 0x06D:
 *   DLC=4, CRC over B0..B2 -> B3
 *
 * No CRC:
 *   0x0F3, 0x429
 */
#ifndef CRC8_J1850_H
#define CRC8_J1850_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t Crc8_J1850_Compute(const uint8_t *data, size_t len);
bool Crc8_J1850_ValidateFrame(const uint8_t frame[8]);
void Crc8_J1850_StampFrame(uint8_t frame[8]);
bool Crc8_J1850_ValidateFrame_06D(const uint8_t frame[4]);
void Crc8_J1850_StampFrame_06D(uint8_t frame[4]);

#ifdef __cplusplus
}
#endif
#endif

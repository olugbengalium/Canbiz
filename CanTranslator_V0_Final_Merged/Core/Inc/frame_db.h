/*
 * frame_db.h
 *
 * Corrected Phase 8/9 frame policy for the W204/M113 translator.
 *
 * IMPORTANT: the empirical 1.8M-frame corpus overrides stale DAT integrity
 * declarations where the real rolling counter/CRC behavior disagrees.
 *
 * Proven integrity policy used here:
 *   - Standard CRC-8 J1850 over B0..B6 -> B7:
 *       0x001, 0x073, 0x0D5, 0x0F1, 0x105, 0x14B, 0x17D, 0x1CD
 *   - 0x06D: DLC=4, CRC-8 J1850 over B0..B2 -> B3
 *   - 0x0F3: NO CRC
 *   - 0x429: NO CRC
 *
 * Empirical MC positions proven for the tested protected/counter frames:
 *   - normal 8-byte counter frames: bits 52..55 (byte 6 high nibble)
 *   - 0x06D: bits 20..23 (byte 2 high nibble)
 *
 * The DAT contains signal definitions overlapping these empirical MC nibbles
 * on some frames (notably 0x0F1/0x0F3/0x105/0x17D). Those overlapping fields
 * are NOT treated as runtime-valid until independently re-audited.
 */
#ifndef FRAME_DB_H
#define FRAME_DB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "can_bus.h"

typedef enum {
    CAN_ID_IGNITION_ECU_CTX   = 0x001,
    CAN_ID_SELECTOR_ISM       = 0x06D,
    CAN_ID_SELECTOR_P_LOCK    = 0x073,
    CAN_ID_TORQUE_INTERVENE   = 0x0D5,
    CAN_ID_TCM_REQUEST        = 0x0F1,
    CAN_ID_GEAR               = 0x0F3,
    CAN_ID_ENGINE_PUB         = 0x105,
    CAN_ID_TORQUE_PUB         = 0x14B,
    CAN_ID_ECU_PT_RESPONSE    = 0x17D,
    CAN_ID_TORQUE_ARBITRATION = 0x1CD,
    CAN_ID_NETWORK_MGMT       = 0x429,
    CAN_ID_AUX_1              = 0x30D,
    CAN_ID_AUX_2              = 0x349,
} FrameDbKnownId_t;

typedef enum {
    FRAME_INTEGRITY_NONE = 0,
    FRAME_INTEGRITY_J1850_B7,
    FRAME_INTEGRITY_J1850_B3_SHORT,
} FrameIntegrityType_t;

#define FRAME_BUS_MASK(bus) (1u << (unsigned)(bus))

typedef struct {
    uint32_t id;
    uint8_t expected_bus_mask;     /* empirically observed RX bus(s) */
    uint16_t nominal_period_ms;    /* reference cadence; not a timeout */
    uint8_t expected_dlc;
    FrameIntegrityType_t integrity;
    bool has_mc;
    uint8_t mc_bit;
    uint8_t mc_len;
    bool deferred;
} FrameDbEntry_t;

static const FrameDbEntry_t FrameDb_Table[] = {
    /* ID     bus mask                     period DLC integrity                 MC bit/len */
    {0x001, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2), 29, 8, FRAME_INTEGRITY_J1850_B7, false, 0, 0, false},
    {0x06D, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2),  6, 4, FRAME_INTEGRITY_J1850_B3_SHORT, true, 20, 4, false},
    {0x073, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2), 18, 8, FRAME_INTEGRITY_J1850_B7, true, 52, 4, false},
    {0x0D5, FRAME_BUS_MASK(CANBUS_1),                           8, 8, FRAME_INTEGRITY_J1850_B7, false, 0, 0, false},
    {0x0F1, FRAME_BUS_MASK(CANBUS_2),                          13, 8, FRAME_INTEGRITY_J1850_B7, true, 52, 4, false},
    {0x0F3, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2), 14, 8, FRAME_INTEGRITY_NONE,     true, 52, 4, false},
    {0x105, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2), 13, 8, FRAME_INTEGRITY_J1850_B7, true, 52, 4, false},
    {0x14B, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2),  7, 8, FRAME_INTEGRITY_J1850_B7, true, 52, 4, false},
    {0x17D, FRAME_BUS_MASK(CANBUS_2),                          19, 8, FRAME_INTEGRITY_J1850_B7, true, 52, 4, false},
    {0x1CD, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2), 11, 8, FRAME_INTEGRITY_J1850_B7, true, 52, 4, false},
    {0x429, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2), 17, 8, FRAME_INTEGRITY_NONE,     false, 0, 0, false},
    {0x30D, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2),  7, 8, FRAME_INTEGRITY_NONE,     false, 0, 0, true},
    {0x349, FRAME_BUS_MASK(CANBUS_1)|FRAME_BUS_MASK(CANBUS_2), 16, 8, FRAME_INTEGRITY_NONE,     false, 0, 0, true},
};

#define FRAME_DB_COUNT (sizeof(FrameDb_Table) / sizeof(FrameDb_Table[0]))

static inline const FrameDbEntry_t *FrameDb_Find(uint32_t id)
{
    for (size_t i = 0; i < FRAME_DB_COUNT; ++i) {
        if (FrameDb_Table[i].id == id) return &FrameDb_Table[i];
    }
    return NULL;
}

static inline bool FrameDb_IsExpectedBus(const FrameDbEntry_t *entry, CanBusId_t bus)
{
    return entry != NULL && (entry->expected_bus_mask & FRAME_BUS_MASK(bus)) != 0u;
}

#endif /* FRAME_DB_H */

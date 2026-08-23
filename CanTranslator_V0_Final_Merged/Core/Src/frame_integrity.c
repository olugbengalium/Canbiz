#include "frame_integrity.h"
#include "crc8_j1850.h"

static uint8_t GetBits8(const uint8_t *data, uint8_t start, uint8_t len)
{
    uint8_t value = 0;
    for (uint8_t i = 0; i < len; ++i) {
        uint8_t bit = (uint8_t)(start + i);
        value |= (uint8_t)(((data[bit / 8u] >> (bit % 8u)) & 1u) << i);
    }
    return value;
}

bool FrameIntegrity_Validate(const FrameDbEntry_t *entry, const CanFrame_t *frame)
{
    if (!entry || !frame) return false;

    switch (entry->integrity) {
    case FRAME_INTEGRITY_NONE:
        return true;
    case FRAME_INTEGRITY_J1850_B7:
        return frame->dlc == 8u && Crc8_J1850_ValidateFrame(frame->data);
    case FRAME_INTEGRITY_J1850_B3_SHORT:
        return frame->dlc == 4u && Crc8_J1850_ValidateFrame_06D(frame->data);
    default:
        return false;
    }
}

bool FrameIntegrity_GetMc(const FrameDbEntry_t *entry, const CanFrame_t *frame,
                          uint8_t *out_mc)
{
    if (!entry || !frame || !out_mc || !entry->has_mc) return false;
    if ((entry->mc_bit + entry->mc_len) > 64u || entry->mc_len == 0u) return false;
    *out_mc = GetBits8(frame->data, entry->mc_bit, entry->mc_len);
    return true;
}

bool FrameIntegrity_UpdateMc(FrameMcTracker_t *tracker, uint8_t mc)
{
    if (!tracker) return false;
    mc &= 0x0Fu;

    if (!tracker->valid) {
        tracker->last_mc = mc;
        tracker->valid = true;
        return true;
    }

    uint8_t expected = (uint8_t)((tracker->last_mc + 1u) & 0x0Fu);
    bool ok = (mc == expected);
    if (!ok) tracker->anomaly_count++;
    tracker->last_mc = mc;
    return ok;
}

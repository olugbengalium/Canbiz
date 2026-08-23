#ifndef FRAME_INTEGRITY_H
#define FRAME_INTEGRITY_H

#include <stdint.h>
#include <stdbool.h>
#include "can_bus.h"
#include "frame_db.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Validates only the integrity mechanism declared by FrameDb. */
bool FrameIntegrity_Validate(const FrameDbEntry_t *entry, const CanFrame_t *frame);

/* Extracts the empirically observed MC nibble. Returns false if this ID has
 * no empirically proven MC field in the current database. */
bool FrameIntegrity_GetMc(const FrameDbEntry_t *entry, const CanFrame_t *frame,
                          uint8_t *out_mc);

/* Runtime counter tracker. This intentionally detects sequence anomalies but
 * does not guess a universal startup seed. */
typedef struct {
    bool valid;
    uint8_t last_mc;
    uint32_t anomaly_count;
} FrameMcTracker_t;

bool FrameIntegrity_UpdateMc(FrameMcTracker_t *tracker, uint8_t mc);

#ifdef __cplusplus
}
#endif

#endif

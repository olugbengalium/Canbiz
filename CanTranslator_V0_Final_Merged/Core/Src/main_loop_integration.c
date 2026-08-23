/*
 * main_loop_integration.c
 *
 * Bench receive/integrity/semantic integration point.
 *
 * This is deliberately not the final TX scheduler. It is the corrected
 * Phase 9E observation core: receive -> per-ID integrity policy -> MC
 * telemetry -> canonical decode -> trace.
 */
#include "can_bus.h"
#include "can_trace.h"
#include "crc8_j1850.h"
#include "frame_db.h"
#include "frame_integrity.h"
#include "decoders.h"
#include "vehicle_state.h"

static VehicleState_t s_vehicle_state;
static FrameMcTracker_t s_mc_trackers[FRAME_DB_COUNT];

static int FrameDb_IndexOf(const FrameDbEntry_t *entry)
{
    if (!entry) return -1;
    return (int)(entry - FrameDb_Table);
}

static void ProcessFrame(const CanFrame_t *frame)
{
    const FrameDbEntry_t *entry = FrameDb_Find(frame->id);
    TraceRecord_t rec = {0};

    rec.timestamp_us = frame->timestamp_us;
    rec.bus = frame->bus;
    rec.id = frame->id;
    rec.dlc = frame->dlc;
    for (uint8_t i = 0; i < frame->dlc && i < 8; ++i) {
        rec.data[i] = frame->data[i];
    }

    /* Known frame: report unexpected-bus traffic but do not discard it in
     * the bench build. This keeps the evidence visible while the topology
     * is being validated. */
    if (entry && !FrameDb_IsExpectedBus(entry, frame->bus)) {
        rec.decoded = false;
        TraceLog_Push(&rec);
        return;
    }

    /* Deferred telemetry is observed only. */
    if (entry && entry->deferred) {
        rec.decoded = false;
        TraceLog_Push(&rec);
        return;
    }

    /* Per-ID integrity policy. Unknown IDs are not rejected here because
     * M113 native source IDs (0x268/0x3A0/0x3D0) are intentionally allowed
     * to enter the source decoder without a fabricated CRC rule. */
    if (entry) {
        if (entry->integrity != FRAME_INTEGRITY_NONE) {
            rec.crc_checked = true;
            rec.crc_valid = FrameIntegrity_Validate(entry, frame);
            if (!rec.crc_valid) {
                TraceLog_Push(&rec);
                return;
            }
        }

        uint8_t mc = 0;
        if (FrameIntegrity_GetMc(entry, frame, &mc)) {
            int idx = FrameDb_IndexOf(entry);
            rec.mc_checked = true;
            rec.mc_value = mc;
            if (idx >= 0) {
                rec.mc_sequence_ok = FrameIntegrity_UpdateMc(&s_mc_trackers[idx], mc);
            }
        }
    }

    rec.decoded = Decoders_Dispatch(frame, &s_vehicle_state);
    TraceLog_Push(&rec);
}

void MainLoop_Poll(void)
{
    CanFrame_t frame;

    while (CanBus_Dequeue(CANBUS_1, &frame)) {
        ProcessFrame(&frame);
    }
    while (CanBus_Dequeue(CANBUS_2, &frame)) {
        ProcessFrame(&frame);
    }

    /* TX scheduling is intentionally separate. Once CAN bring-up is proven,
     * add the native M113 -> W204 destination packers here rather than
     * coupling TX to RX callbacks. */
}

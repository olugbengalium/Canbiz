/*
 * can_trace.h
 *
 * Trace infrastructure: raw frame, decoded state, integrity and timing
 * records. Phase 9E implementation sequence step 2, and required "always
 * enabled in bench build" per Phase 9 §16 release matrix.
 *
 * This module only stores/exposes records; it does not decide UART vs
 * SD vs SWO output — wire TraceLog_Drain() to whatever transport you're
 * using for bench captures (this is deliberately transport-agnostic since
 * the evidence-gate closure work in Phase 9 §6 depends on captures, and
 * you may want different transports for cold-start vs dynamic captures).
 */

#ifndef CAN_TRACE_H
#define CAN_TRACE_H

#include "can_bus.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t   timestamp_us;
    CanBusId_t bus;
    uint32_t   id;
    uint8_t    dlc;
    uint8_t    data[8];
    bool       crc_valid;      /* only meaningful if id is CRC-protected in frame_db */
    bool       crc_checked;    /* false if not yet validated (e.g. no decoder/policy yet) */
    uint8_t    mc_value;       /* raw empirical MC nibble when available */
    bool       mc_checked;
    bool       mc_sequence_ok; /* diagnostic only; does not reject a frame */
    bool       decoded;        /* true if Decoders_Dispatch handled this ID */
} TraceRecord_t;

#define TRACE_LOG_DEPTH 256  /* power of two */

void TraceLog_Init(void);

/* Push a record. Non-blocking, drops oldest on overflow (bench visibility
 * over guaranteed retention — for guaranteed retention, drain frequently). */
void TraceLog_Push(const TraceRecord_t *rec);

/* Pop oldest record. Returns false if empty. Call from main loop / a
 * lower-priority task, not from ISR context. */
bool TraceLog_Pop(TraceRecord_t *out_rec);

uint16_t TraceLog_Depth(void);
uint32_t TraceLog_OverflowCount(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_TRACE_H */

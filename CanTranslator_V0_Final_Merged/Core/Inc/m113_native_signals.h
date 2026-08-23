/*
 * m113_native_signals.h
 *
 * Reference documentation for the M113/W164 native single-CAN-bus
 * signal positions, empirically discovered from real captures
 * (w164_log). See the findings report §9 for the full methodology and
 * evidence tables.
 *
 * CONTEXT: these are empirical M113/W164 native-bus source points used by
 * the bench decoder for raw-source telemetry. The signals remain raw until
 * their physical scaling is independently calibrated. They also serve as
 * evidence/reference for the consolidated single-CAN architecture.
 *
 * All positions below are VERIFIED against real captures (not database
 * extractions) using the method in report §9.1: correlating byte values
 * against labeled ground-truth files (pedal %, gear position, engine
 * on/off), confirming physically sensible behavior (proportional
 * scaling, correct zero states, natural jitter at steady state).
 */

#ifndef M113_NATIVE_SIGNALS_H
#define M113_NATIVE_SIGNALS_H

/* ===== 0x268 — Accelerator pedal position ===== */
/* byte 5, raw 0-255, scale ~2.55 units/% (i.e. raw/2.55 = percent).
 * Verified: 0 at idle/no-pedal, 125 @ 50.2% pedal, 227 @ 89.7% pedal —
 * within 1-3 units of ideal linear scaling at every point. */
#define M113_SIG_0x268_ACCEL_PEDAL_BYTE   5

/* ===== 0x3A0 — Gear position display character ===== */
/* byte 0, literal ASCII: '1'-'6' (0x31-0x36) for D1-D6, 'N'=0x4E,
 * 'R'=0x52, 'P'=0x50. Exact match, zero ambiguity. */
#define M113_SIG_0x3A0_GEAR_DISPLAY_BYTE  0

/* ===== 0x412 (= 0x408 byte4) — gear-correlated numeric value ===== */
/* byte 7 (0x412) / byte 4 (0x408). LOWER CONFIDENCE than the above two —
 * near-linear D1->D6 (34,36,38,40,43,45), distinct N=29/R=26/P=21, but
 * not conclusively identified. Candidate: output-shaft speed or
 * displayed-gear-ratio indicator, not a raw gear enum (progression is
 * too smooth for a simple state enum). Treat as a lead, not a closed
 * signal, until independently re-verified. */
#define M113_SIG_0x412_GEAR_CORRELATED_BYTE   7
#define M113_SIG_0x408_GEAR_CORRELATED_BYTE   4

/* ===== 0x3D0 — Engine RPM ===== */
/* byte 1, raw units. Verified: exactly 0 when engine confirmed off
 * (both a dedicated Terminal-15 baseline AND, independently, two
 * "ignition-on" pedal-test files whose names claim engine state
 * ambiguously but whose zero RPM here confirms engine was in fact off
 * during those captures). At genuine idle: tight naturally-jittering
 * band 87-104 raw, with small sensible variation between plain idle
 * (median 100), Park-idle (median 98), and brake-depressed-idle
 * (median 93) — the signature of a real filtered sensor reading, not a
 * static or coincidental byte.
 *
 * SCALE FACTOR NOT CONFIRMED. A raw-to-RPM scale near 7-8 rpm/unit
 * would place idle around 650-830 rpm (plausible for M113), but this
 * has not been verified against a reference tachometer reading. Do not
 * use this value as calibrated RPM without independent confirmation. */
#define M113_SIG_0x3D0_ENGINE_RPM_BYTE    1

/* ===== NOT FOUND: torque / torque-converter-slip signal ===== */
/* No candidate identified. No torque-labeled capture exists in the
 * evidence set to correlate against (unlike pedal%/gear/RPM, which all
 * had explicit labeled files). Closing this requires either a labeled
 * dynamic capture with known torque/load, or mapping M113's own
 * internal torque estimate (already computed by the ECU) rather than
 * searching for an equivalent CAN broadcast that may not exist in this
 * consolidated-bus form. See report §9.6. */

#endif /* M113_NATIVE_SIGNALS_H */

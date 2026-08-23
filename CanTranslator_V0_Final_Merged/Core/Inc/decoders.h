/*
 * decoders.h
 *
 * Frame -> VehicleState decoders, now using verified bit anchors from
 * signal_anchors.h (extracted from the real 204_POWERTRAIN_AEJ2010_1.dat).
 * See signal_anchors.h for the extraction method and confidence notes.
 *
 * STILL OPEN even with verified bit positions:
 *   - Scale/offset per signal (raw bits -> physical units): not extracted.
 *     Values returned here are RAW extracted integers, not physical units,
 *     unless otherwise noted. Do not treat rpm/torque fields as already
 *     scaled.
 *   - MC/CRC validation happens upstream (frame validator) — these
 *     decoders assume the frame already passed integrity checks.
 *   - Control laws (TxSlpRPM target, torque arbitration selection) remain
 *     open per Phase 9 Gates G1/G2 regardless of bit position knowledge.
 */

#ifndef DECODERS_H
#define DECODERS_H

#include "can_bus.h"
#include "vehicle_state.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Decoders_Dispatch(const CanFrame_t *frame, VehicleState_t *state);

/* Empirical M113/W164 native-bus sources (raw until scaling is calibrated). */
void Decode_M113_0x268_Pedal(const CanFrame_t *frame, VehicleState_t *state);
void Decode_M113_0x3A0_GearDisplay(const CanFrame_t *frame, VehicleState_t *state);
void Decode_M113_0x3D0_Rpm(const CanFrame_t *frame, VehicleState_t *state);

void Decode_0x001_Ignition(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x06D_SelectorStW(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x073_SelectorISM(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x0D5_TorqueIntervene(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x0F1_TcmRequest(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x0F3_Gear(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x105_Engine(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x14B_TorquePub(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x17D_EcuPtResponse(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x1CD_TorqueArbitration(const CanFrame_t *frame, VehicleState_t *state);
void Decode_0x429_NetworkMgmt(const CanFrame_t *frame, VehicleState_t *state);

#ifdef __cplusplus
}
#endif

#endif /* DECODERS_H */

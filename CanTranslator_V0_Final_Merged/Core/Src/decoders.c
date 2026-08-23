/*
 * decoders.c
 *
 * Bit numbering: Intel/LSB-first, bit N = byte (N/8), bit (N%8) from that
 * byte's LSB. This was previously an UNVERIFIED assumption; it is now
 * CONFIRMED — cross-checked against dozens of fields in the real
 * 204_POWERTRAIN_AEJ2010_1.dat, all matching Phase 8's independently
 * claimed anchors (EngRPM bit2, EngRun_Stat bit53, Term61_Actv bit40,
 * GenLoad bit42, etc.) plus the 0x0F3 gear nibble split. See
 * signal_anchors.h for the extraction method.
 *
 * All extracted values are RAW integers — no scale/offset applied, since
 * those weren't extracted with confidence. Treat torque/RPM fields as
 * raw bit-field values, not physical units, until scale/offset is
 * separately confirmed.
 */

#include "decoders.h"
#include "frame_db.h"
#include "signal_anchors.h"

static uint32_t GetBitsLE(const uint8_t *data, uint16_t start_bit, uint8_t length)
{
    uint32_t value = 0;
    for (uint8_t i = 0; i < length; i++) {
        uint16_t bit = start_bit + i;
        uint8_t byte_idx = bit / 8;
        uint8_t bit_idx  = bit % 8;
        uint8_t bit_val  = (data[byte_idx] >> bit_idx) & 0x01;
        value |= ((uint32_t)bit_val << i);
    }
    return value;
}


void Decode_0x001_Ignition(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    state->authorization.eis_state =
        (uint8_t)GetBitsLE(d, SIG_0x001_ISw_Stat_BIT, SIG_0x001_ISw_Stat_LEN);
    state->authorization.tx_park_posn_rq =
        GetBitsLE(d, SIG_0x001_TxPkPosn_Rq_BIT, SIG_0x001_TxPkPosn_Rq_LEN);
    state->authorization.tx_park_posn_emg_rq =
        GetBitsLE(d, SIG_0x001_TxPkPosn_Emg_Rq_BIT, SIG_0x001_TxPkPosn_Emg_Rq_LEN);
}

void Decode_0x06D_SelectorStW(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    state->transmission.selector_rnd_posn_stw =
        (uint8_t)GetBitsLE(d, SIG_0x06D_TSL_RND_Posn_StW_BIT, SIG_0x06D_TSL_RND_Posn_StW_LEN);
    state->transmission.selector_p_pressed_stw =
        GetBitsLE(d, SIG_0x06D_TSL_P_Psd_StW_BIT, SIG_0x06D_TSL_P_Psd_StW_LEN) != 0;
    /* NOTE: this frame's MC/CRC are at bit16/24, not the usual bit48/56 —
     * frame validator must special-case this ID's integrity field
     * positions rather than assuming the common layout. */
}

void Decode_0x073_SelectorISM(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    state->transmission.selector_posn_ism =
        (uint8_t)GetBitsLE(d, SIG_0x073_TSL_Posn_ISM_BIT, SIG_0x073_TSL_Posn_ISM_LEN);
    state->transmission.selector_posn_p_ism =
        GetBitsLE(d, SIG_0x073_TSL_Posn_P_ISM_BIT, SIG_0x073_TSL_Posn_P_ISM_LEN) != 0;
}

void Decode_0x0D5_TorqueIntervene(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    uint32_t raw = GetBitsLE(d, SIG_0x0D5_EngTrq_Rq_SBC_BIT, SIG_0x0D5_EngTrq_Rq_SBC_LEN);
    state->transmission.torque_request_sbc_raw = (uint16_t)raw;
}

void Decode_0x0F1_TcmRequest(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;

    uint32_t trq_raw = GetBitsLE(d, SIG_0x0F1_EngTrq_Rq_TCM_BIT, SIG_0x0F1_EngTrq_Rq_TCM_LEN);
    state->transmission.torque_request_raw = (uint16_t)trq_raw;

    /* RPM request — bit position closed, but the physical scale AND the
     * control law for what value to request remain Gate G1 (OPEN). Raw
     * value only. */
    state->transmission.rpm_request =
        (uint16_t)GetBitsLE(d, SIG_0x0F1_EngRPM_Rq_TCM_BIT, SIG_0x0F1_EngRPM_Rq_TCM_LEN);
    state->transmission.rpm_sync_time =
        (uint16_t)GetBitsLE(d, SIG_0x0F1_EngRPM_SyncTm_Rq_TCM_BIT, SIG_0x0F1_EngRPM_SyncTm_Rq_TCM_LEN);

    /* Do NOT decode the DAT fields at bits 52..54 as start/emergency
     * signals: empirical traffic proves bits 52..55 are the rolling MC
     * nibble on this frame. Those overlapping DAT definitions remain
     * quarantined until a start/stop capture re-audits them. */
}

void Decode_0x0F3_Gear(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    /* Confirmed against real DAT: Gr(actual)=bits4-7, Gr_Target=bits0-3.
     * Equivalent to the original nibble-split model, now verified rather
     * than assumed. */
    state->transmission.actual_gear =
        (uint8_t)GetBitsLE(d, SIG_0x0F3_Gr_BIT, SIG_0x0F3_Gr_LEN);
    state->transmission.target_gear =
        (uint8_t)GetBitsLE(d, SIG_0x0F3_Gr_Target_BIT, SIG_0x0F3_Gr_Target_LEN);
    /* No CRC was found for this frame. Empirical MC is bits 52..55. */
}

void Decode_0x105_Engine(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;

    state->engine.actual_rpm_raw =
        (uint16_t)GetBitsLE(d, SIG_0x105_EngRPM_BIT, SIG_0x105_EngRPM_LEN);
    state->engine.pedal =
        (uint8_t)GetBitsLE(d, SIG_0x105_AccelPdlPosn_BIT, SIG_0x105_AccelPdlPosn_LEN);
    /* EngRun_Stat at bits 53..55 overlaps the empirically proven MC
     * nibble on 0x105. It is therefore intentionally NOT decoded here. */
}

void Decode_0x14B_TorquePub(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    uint32_t stat = GetBitsLE(d, SIG_0x14B_EngTrqStatic_BIT, SIG_0x14B_EngTrqStatic_LEN);
    uint32_t max  = GetBitsLE(d, SIG_0x14B_EngTrqMaxETC_BIT, SIG_0x14B_EngTrqMaxETC_LEN);
    uint32_t min  = GetBitsLE(d, SIG_0x14B_EngTrqMinTTC_BIT, SIG_0x14B_EngTrqMinTTC_LEN);

    state->engine.torque_static_raw = (uint16_t)stat;
    state->engine.torque_max_raw    = (uint16_t)max;
    state->engine.torque_min_raw    = (uint16_t)min;
}

void Decode_0x17D_EcuPtResponse(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;

    state->transmission.gr_max_rq =
        (uint8_t)GetBitsLE(d, SIG_0x17D_GrMax_Rq_ECM_BIT, SIG_0x17D_GrMax_Rq_ECM_LEN);
    state->transmission.gr_min_rq =
        (uint8_t)GetBitsLE(d, SIG_0x17D_GrMin_Rq_ECM_BIT, SIG_0x17D_GrMin_Rq_ECM_LEN);
    state->transmission.creep_off_rq =
        GetBitsLE(d, SIG_0x17D_Creep_Off_Rq_BIT, SIG_0x17D_Creep_Off_Rq_LEN) != 0;
    state->transmission.gr1_rq =
        GetBitsLE(d, SIG_0x17D_Gr1_Rq_ECM_BIT, SIG_0x17D_Gr1_Rq_ECM_LEN) != 0;
    state->transmission.shift_state =
        (uint8_t)GetBitsLE(d, SIG_0x17D_ShftChrDsp_Rq_BIT, SIG_0x17D_ShftChrDsp_Rq_LEN);
    state->transmission.tcc_state =
        (uint8_t)GetBitsLE(d, SIG_0x17D_TCC_Rq_BIT, SIG_0x17D_TCC_Rq_LEN);

    /* Bit position CLOSED. Control law for what value to WRITE here
     * remains Gate G1 (OPEN) — this decoder only reads what's on the bus
     * (useful for passive bench observation / F1); it does not compute a
     * value for our own TX side. */
    state->transmission.tx_slip_rpm_rq =
        (uint16_t)GetBitsLE(d, SIG_0x17D_TxSlpRPM_Rq_ECM_BIT, SIG_0x17D_TxSlpRPM_Rq_ECM_LEN);
}

void Decode_0x1CD_TorqueArbitration(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    uint32_t d_raw   = GetBitsLE(d, SIG_0x1CD_EngTrqSel_D_TTC_BIT, SIG_0x1CD_EngTrqSel_D_TTC_LEN);
    uint32_t as_raw  = GetBitsLE(d, SIG_0x1CD_EngTrqSel_AS_TTC_BIT, SIG_0x1CD_EngTrqSel_AS_TTC_LEN);
    uint32_t sbc_raw = GetBitsLE(d, SIG_0x1CD_EngTrqSel_SBC_TTC_BIT, SIG_0x1CD_EngTrqSel_SBC_TTC_LEN);

    state->transmission.trq_sel_d_ttc_raw   = (uint16_t)d_raw;
    state->transmission.trq_sel_as_ttc_raw  = (uint16_t)as_raw;
    state->transmission.trq_sel_sbc_ttc_raw = (uint16_t)sbc_raw;
    /* Which of these three (or some function of them) is the "selected"
     * arbitrated torque remains Gate G2 (OPEN) — bit positions are known,
     * the arbitration logic is not. */
}

void Decode_0x429_NetworkMgmt(const CanFrame_t *frame, VehicleState_t *state)
{
    const uint8_t *d = frame->data;
    state->network.nm_mode =
        (uint8_t)GetBitsLE(d, SIG_0x429_NM_Mode_BIT, SIG_0x429_NM_Mode_LEN);
    state->network.nm_successor =
        (uint8_t)GetBitsLE(d, SIG_0x429_NM_Successor_BIT, SIG_0x429_NM_Successor_LEN);
    state->network.nw_id =
        (uint8_t)GetBitsLE(d, SIG_0x429_Nw_Id_BIT, SIG_0x429_Nw_Id_LEN);
    state->network.awake_powertrain =
        GetBitsLE(d, SIG_0x429_Awake_POWERTRAIN_BIT, SIG_0x429_Awake_POWERTRAIN_LEN) != 0;
    state->network.awake_chassis =
        GetBitsLE(d, SIG_0x429_Awake_CHASSIS_BIT, SIG_0x429_Awake_CHASSIS_LEN) != 0;
    state->network.awake_ignition_on =
        GetBitsLE(d, SIG_0x429_Awake_Ignition_On_BIT, SIG_0x429_Awake_Ignition_On_LEN) != 0;
    state->network.powertrain_alive = true; /* receiving this frame at all implies alive */
    /* NOTE: no CRC field found for this frame — NM frames commonly skip
     * CRC by design (short, low-rate). Do not enforce CRC on 0x429. */
}

static uint8_t M113_GearAsciiToNormalized(uint8_t c)
{
    switch (c) {
    case 'P': return 0x0D;
    case 'R': return 0x0B;
    case 'N': return 0x00;
    case '1': return 0x01;
    case '2': return 0x02;
    case '3': return 0x03;
    case '4': return 0x04;
    case '5': return 0x05;
    case '6': return 0x06;
    default:  return 0xFF;
    }
}

void Decode_M113_0x268_Pedal(const CanFrame_t *frame, VehicleState_t *state)
{
    state->m113_native.pedal_raw = frame->data[5];
    state->m113_native.pedal_valid = true;
}

void Decode_M113_0x3A0_GearDisplay(const CanFrame_t *frame, VehicleState_t *state)
{
    uint8_t c = frame->data[0];
    uint8_t gear = M113_GearAsciiToNormalized(c);
    state->m113_native.gear_ascii = c;
    state->m113_native.normalized_gear = gear;
    state->m113_native.gear_valid = (gear != 0xFF);
}

void Decode_M113_0x3D0_Rpm(const CanFrame_t *frame, VehicleState_t *state)
{
    state->m113_native.rpm_raw = frame->data[1];
    state->m113_native.rpm_valid = true;
}

bool Decoders_Dispatch(const CanFrame_t *frame, VehicleState_t *state)
{
    switch (frame->id) {
        case 0x268: Decode_M113_0x268_Pedal(frame, state); return true;
        case 0x3A0: Decode_M113_0x3A0_GearDisplay(frame, state); return true;
        case 0x3D0: Decode_M113_0x3D0_Rpm(frame, state); return true;
        case CAN_ID_IGNITION_ECU_CTX: Decode_0x001_Ignition(frame, state); return true;
        case CAN_ID_SELECTOR_ISM:     Decode_0x06D_SelectorStW(frame, state); return true;
        case CAN_ID_SELECTOR_P_LOCK:  Decode_0x073_SelectorISM(frame, state); return true;
        case CAN_ID_TORQUE_INTERVENE: Decode_0x0D5_TorqueIntervene(frame, state); return true;
        case CAN_ID_TCM_REQUEST:      Decode_0x0F1_TcmRequest(frame, state); return true;
        case CAN_ID_GEAR:             Decode_0x0F3_Gear(frame, state); return true;
        case CAN_ID_ENGINE_PUB:       Decode_0x105_Engine(frame, state); return true;
        case CAN_ID_TORQUE_PUB:       Decode_0x14B_TorquePub(frame, state); return true;
        case CAN_ID_ECU_PT_RESPONSE:  Decode_0x17D_EcuPtResponse(frame, state); return true;
        case CAN_ID_TORQUE_ARBITRATION: Decode_0x1CD_TorqueArbitration(frame, state); return true;
        case CAN_ID_NETWORK_MGMT:     Decode_0x429_NetworkMgmt(frame, state); return true;
        default:
            /* 0x30D/0x349: deferred per Phase 9 §15, not decoded even
             * though their bit anchors are now known — route to raw
             * trace logging instead. */
            return false;
    }
}

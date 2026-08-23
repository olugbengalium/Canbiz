/*
 * vehicle_state.h
 *
 * Canonical semantic state, per Phase 9 §10 baseline. This is the single
 * decision layer the translator reads/writes — raw CAN IDs are never used
 * as application state (Phase 9 §10, Phase 8 §7 ownership rules).
 *
 * Field-level "open" markers below point back to the evidence gates in
 * Phase 9 §4/§6/§16 — do not treat those fields as validated numeric
 * truth, only as the correct place to store whatever value is decoded
 * or computed until the gate closes.
 */

#ifndef VEHICLE_STATE_H
#define VEHICLE_STATE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t actual_rpm_raw;
    uint8_t  pedal;
    bool     engine_running;
    uint16_t torque_actual_raw;
    uint16_t torque_static_raw;
    uint16_t torque_max_raw;
    uint16_t torque_min_raw;
} EngineState_t;

typedef struct {
    uint8_t  actual_gear;      /* Phase 8 Impl Spec §4 normalized enum: 0x0..0xF */
    uint8_t  target_gear;      /* kept independent of actual_gear at all times */
    uint8_t  selector_state;   /* from 0x06D/0x073, input/context only */
    uint16_t torque_request_raw;   /* 0x0F1 EngTrq_Rq_TCM */
    uint16_t rpm_request;      /* 0x0F1 EngRPM_Rq_TCM — numerical law OPEN (Phase 9 G1) */
    uint16_t rpm_sync_time;    /* 0x0F1 EngRPM_SyncTm_Rq_TCM */
    uint16_t turbine_rpm;      /* candidate input to slip control law, see 0x17D boundary */
    uint8_t  tcc_state;        /* 0x17D TCC_Rq */
    uint8_t  shift_state;      /* 0x17D ShftChrDsp_Rq */
    uint16_t tx_slip_rpm_rq;   /* 0x17D TxSlpRPM_Rq_ECM — CONTROL DOMAIN CLOSED, law OPEN (G1) */
    uint8_t  gr_max_rq;        /* 0x17D GrMax_Rq_ECM */
    uint8_t  gr_min_rq;        /* 0x17D GrMin_Rq_ECM */
    bool     creep_off_rq;     /* 0x17D Creep_Off_Rq */
    bool     gr1_rq;           /* 0x17D Gr1_Rq_ECM */
    /* 0x1CD torque arbitration — real signal names have _TTC suffix
     * ("including trailing throttle component"), raw units */
    uint16_t trq_sel_d_ttc_raw;    /* EngTrqSel_D_TTC — driver-selected torque */
    uint16_t trq_sel_as_ttc_raw;   /* EngTrqSel_AS_TTC — assistance-system-selected torque */
    uint16_t trq_sel_sbc_ttc_raw;  /* EngTrqSel_SBC_TTC — SBC-selected torque */
    /* 0x06D/0x073 selector inputs, kept separate per Phase 8 ownership rule
     * (not synthesized into a single Park-only flag) */
    uint8_t  selector_rnd_posn_stw; /* 0x06D TSL_RND_Posn_StW */
    bool     selector_p_pressed_stw; /* 0x06D TSL_P_Psd_StW (raw 2-bit field) */
    uint8_t  selector_posn_ism; /* 0x073 TSL_Posn_ISM — actual position */
    bool     selector_posn_p_ism; /* 0x073 TSL_Posn_P_ISM — dedicated P flag */
    /* 0x0D5 torque intervention (SBC/ESP side, source-side input) */
    uint16_t torque_request_sbc_raw; /* 0x0D5 EngTrq_Rq_SBC */
} TransmissionState_t;


typedef struct {
    uint8_t  pedal_raw;              /* 0x268 B5; empirical native M113 source */
    uint8_t  gear_ascii;             /* 0x3A0 B0; literal P/R/N/1..6 */
    uint8_t  normalized_gear;        /* derived from gear_ascii when valid */
    uint8_t  rpm_raw;                /* 0x3D0 B1; RPM scale NOT calibrated */
    bool     pedal_valid;
    bool     gear_valid;
    bool     rpm_valid;
} M113NativeState_t;

typedef struct {
    bool da_personalized;
    bool da_valid_runtime;
    bool start_enable;         /* 0x0F1 EngSt_Enbl_Rq_TCM */
    bool emergency_off_rq;     /* 0x0F1 EngEmgOff_Rq */
    uint8_t eis_state;         /* 0x001 ISw_Stat (raw enum, 3 bits) */
    bool tx_park_posn_rq;      /* 0x001 TxPkPosn_Rq */
    bool tx_park_posn_emg_rq;  /* 0x001 TxPkPosn_Emg_Rq */
    bool drivetrain_ready;
} AuthorizationState_t;

typedef struct {
    bool     chassis_alive;
    bool     powertrain_alive;
    bool     mbus_alive;
    uint8_t  nm_state_chassis;
    uint8_t  nm_mode;          /* 0x429 NM_Mode, raw 8-bit */
    uint8_t  nm_successor;     /* 0x429 NM_Successor, raw 8-bit */
    uint8_t  nw_id;            /* 0x429 Nw_Id, raw 8-bit */
    bool     awake_powertrain; /* 0x429 Awake_POWERTRAIN */
    bool     awake_chassis;    /* 0x429 Awake_CHASSIS */
    bool     awake_ignition_on; /* 0x429 Awake_Ignition_On */
} NetworkState_t;

/* Per-bus/per-ID integrity bookkeeping — MC seed/placement is an OPEN
 * validation gate (Phase 9 G3); never hardcode a universal reset value. */
typedef struct {
    uint8_t  mc_last_value;    /* last observed 4-bit MC nibble */
    bool     mc_synced;        /* true once first valid progression observed */
    bool     last_crc_ok;
    uint32_t last_rx_timestamp_us;
    bool     fresh;            /* freshness/timeout validation result */
} FrameIntegrityState_t;

typedef struct {
    EngineState_t         engine;
    TransmissionState_t   transmission;
    AuthorizationState_t  authorization;
    NetworkState_t        network;
    M113NativeState_t     m113_native;
} VehicleState_t;

#ifdef __cplusplus
}
#endif

#endif /* VEHICLE_STATE_H */

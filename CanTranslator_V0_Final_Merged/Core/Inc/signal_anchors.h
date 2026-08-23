/*
 * signal_anchors.h
 *
 * Ground-truth bit anchors extracted from 204_POWERTRAIN_AEJ2010_1.dat
 * (the real evidence-hierarchy source Phase 8 cited). Extraction method:
 * binary scan for [1-byte name_len][name][1-byte start_bit][1-byte bit_len]
 * records, bit numbering confirmed Intel/LSB-first. See frame_db.h header
 * comment for the confidence/verification notes.
 *
 * ============================================================
 * CORRECTION (closes Phase 9 Gate G3): MC bit position
 * ============================================================
 * The DAT file declares MC_<framename> at bit 48 (0x06D: bit 16), but
 * cross-referencing against ~1.86M real captured frames (w204_readings)
 * shows the field that actually behaves as a wrapping 4-bit counter sits
 * ONE NIBBLE HIGHER: bit 52 for all byte6-based frames (0x0F1, 0x0F3,
 * 0x105, 0x14B, 0x17D, 0x1CD, 0x073), and bit 20 for 0x06D's 4-byte
 * frame. This was confirmed independently for all 7 IDs tested, each
 * with a systematic nibble-position scan across 100+ consecutive real
 * frames — not a one-off anomaly. The bit48/bit16 positions the DAT
 * declared are STATIC in real traffic, not counters. MC_BIT values below
 * already reflect this correction (real, verified position).
 *
 * SIDE EFFECT: whatever the DAT file declared as occupying bits 52-55
 * for 0x105 (AccelPdlPosnSens_Flt at bit52 len1, EngRun_Stat at bit53
 * len3) is now of UNCERTAIN accuracy, since that's the same nibble the
 * real MC counter occupies. Do not trust those two fields' bit positions
 * without separate re-verification; they are left in this file as
 * originally extracted but should be treated as OPEN, not closed.
 *
 * This header intentionally does NOT include scale/offset/signedness —
 * those weren't extracted with confidence (see frame_db.h). Values here
 * are BIT POSITIONS ONLY. Do not assume a raw extracted integer is a
 * physical unit without separately confirming scale/offset from the DAT
 * file's float fields or from bench correlation.
 */

#ifndef SIGNAL_ANCHORS_H
#define SIGNAL_ANCHORS_H

/* ===== 0x001 — EIS_A1 (Ignition/ECU context) ===== */
#define SIG_0x001_ISw_Stat_BIT           5   /* len 3 — ignition switch state enum */
#define SIG_0x001_ISw_Stat_LEN           3
#define SIG_0x001_Ign_On_StProc_Inact_BIT 4
#define SIG_0x001_Ign_On_StProc_Inact_LEN 1
#define SIG_0x001_TxPkPosn_Rq_BIT        3
#define SIG_0x001_TxPkPosn_Rq_LEN        1
#define SIG_0x001_TxPkPosn_Emg_Rq_BIT    2
#define SIG_0x001_TxPkPosn_Emg_Rq_LEN    1
#define SIG_0x001_TxPkPosnAuto_Enbl_BIT  1
#define SIG_0x001_TxPkPosnAuto_Enbl_LEN  1
#define SIG_0x001_TxPkPosn_Rq_SBC_Enbl_BIT 0
#define SIG_0x001_TxPkPosn_Rq_SBC_Enbl_LEN 1
#define SIG_0x001_KG_IgnCtrl_Actv_BIT    15
#define SIG_0x001_KG_IgnCtrl_Actv_LEN    1
#define SIG_0x001_EngSt_Enbl_Rq_KG_BIT   14
#define SIG_0x001_EngSt_Enbl_Rq_KG_LEN   1
#define SIG_0x001_KG_StSw_Psd_BIT        13
#define SIG_0x001_KG_StSw_Psd_LEN        1

/* ===== 0x06D — SBW_RQ_SCCM (Selector, steering-wheel side) ===== */
#define SIG_0x06D_TSL_RND_Posn_StW_BIT   12  /* len 4 — RND selector position from steering wheel */
#define SIG_0x06D_TSL_RND_Posn_StW_LEN   4
#define SIG_0x06D_TSL_P_Psd_StW_BIT      10
#define SIG_0x06D_TSL_P_Psd_StW_LEN      2
#define SIG_0x06D_TSL_Sgnl_Id_StW_BIT    8
#define SIG_0x06D_TSL_Sgnl_Id_StW_LEN    2
#define SIG_0x06D_MC_BIT                 20
#define SIG_0x06D_MC_LEN                 4
#define SIG_0x06D_CRC_BIT                24
#define SIG_0x06D_CRC_LEN                8
#define SIG_0x06D_VehLoad_Stat_BIT       6
#define SIG_0x06D_VehLoad_Stat_LEN       2
/* NOTE: MC/CRC on this frame are at bit16/24, NOT the usual bit48/56 —
 * confirmed different frame layout than the powertrain-side frames below.
 * Do not reuse the bit48/56 assumption for this ID. */

/* ===== 0x073 — SBW_RS_ISM (Selector/P/lock, shifter module side) ===== */
#define SIG_0x073_TSL_Posn_ISM_BIT       4   /* len 4 — actual selector position */
#define SIG_0x073_TSL_Posn_ISM_LEN       4
#define SIG_0x073_TSL_MtnLk_Actv_BIT     3
#define SIG_0x073_TSL_MtnLk_Actv_LEN     1
#define SIG_0x073_TSL_Posn_P_ISM_BIT     2   /* dedicated P-position flag */
#define SIG_0x073_TSL_Posn_P_ISM_LEN     1
#define SIG_0x073_TxDrvProgSw_Psd_V3_BIT 1
#define SIG_0x073_TxDrvProgSw_Psd_V3_LEN 1
#define SIG_0x073_MC_BIT                 52
#define SIG_0x073_MC_LEN                 4
#define SIG_0x073_CRC_BIT                56
#define SIG_0x073_CRC_LEN                8

/* ===== 0x0D5 — ENG_RQ_SBC (Torque intervention, SBC/ESP side) ===== */
#define SIG_0x0D5_EngTrqMax_Rq_SBC_BIT   1
#define SIG_0x0D5_EngTrqMax_Rq_SBC_LEN   1
#define SIG_0x0D5_EngTrqMin_Rq_SBC_BIT   0
#define SIG_0x0D5_EngTrqMin_Rq_SBC_LEN   1
#define SIG_0x0D5_EngTrq_Rq_SBC_BIT      3
#define SIG_0x0D5_EngTrq_Rq_SBC_LEN      13
#define SIG_0x0D5_SSA_DsablPrmnt_Rq_SBC_BIT 19
#define SIG_0x0D5_SSA_DsablPrmnt_Rq_SBC_LEN 1
#define SIG_0x0D5_SSA_Enbl_Rq_SBC_BIT    18
#define SIG_0x0D5_SSA_Enbl_Rq_SBC_LEN    1
#define SIG_0x0D5_IntrvntnMd_SBC_BIT     16
#define SIG_0x0D5_IntrvntnMd_SBC_LEN     2
#define SIG_0x0D5_WhlSlpRecup_BIT        24
#define SIG_0x0D5_WhlSlpRecup_LEN        1
#define SIG_0x0D5_RecupEnbl_RBS_BIT      34
#define SIG_0x0D5_RecupEnbl_RBS_LEN      1

/* ===== 0x0F1 — ENG_RQ1_TCM (TCM request) ===== */
#define SIG_0x0F1_EngTrq_Rq_TCM_BIT      3
#define SIG_0x0F1_EngTrq_Rq_TCM_LEN      13
#define SIG_0x0F1_EngTrqMax_Rq_TCM_BIT   1
#define SIG_0x0F1_EngTrqMax_Rq_TCM_LEN   1
#define SIG_0x0F1_EngTrqMin_Rq_TCM_BIT   0
#define SIG_0x0F1_EngTrqMin_Rq_TCM_LEN   1
#define SIG_0x0F1_IntrvntnMd_TCM_BIT     16
#define SIG_0x0F1_IntrvntnMd_TCM_LEN     2
#define SIG_0x0F1_SSA_Enbl_Rq_TCM_BIT    32
#define SIG_0x0F1_SSA_Enbl_Rq_TCM_LEN    1
#define SIG_0x0F1_VehCreep_Actv_BIT      33
#define SIG_0x0F1_VehCreep_Actv_LEN      1
#define SIG_0x0F1_SSA_Clutch_Stat_BIT    34
#define SIG_0x0F1_SSA_Clutch_Stat_LEN    2
#define SIG_0x0F1_EngSt_Enbl_Rq_TCM_BIT  52 /* QUARANTINED: overlaps empirical MC nibble */
#define SIG_0x0F1_EngSt_Enbl_Rq_TCM_LEN  1
#define SIG_0x0F1_EngEmgOff_Rq_BIT       53 /* QUARANTINED: overlaps empirical MC nibble */
#define SIG_0x0F1_EngEmgOff_Rq_LEN       1
#define SIG_0x0F1_JmpSt_Actv_BIT         54
#define SIG_0x0F1_JmpSt_Actv_LEN         1
#define SIG_0x0F1_MC_BIT                 52
#define SIG_0x0F1_MC_LEN                 4
#define SIG_0x0F1_CRC_BIT                56
#define SIG_0x0F1_CRC_LEN                8
/* EngRPM_Rq_TCM / EngRPM_SyncTm_Rq_TCM appeared in a second document
 * section (offset ~75954+) rather than the primary block used above.
 * Values agree with Phase 8's own claimed anchors (36/12), giving two
 * independent confirmations, but flagging the discrepancy in document
 * structure since it wasn't explained: */
#define SIG_0x0F1_EngRPM_Rq_TCM_BIT      36
#define SIG_0x0F1_EngRPM_Rq_TCM_LEN      12  /* control law itself still OPEN, Gate G1 */
#define SIG_0x0F1_EngRPM_SyncTm_Rq_TCM_BIT 24
#define SIG_0x0F1_EngRPM_SyncTm_Rq_TCM_LEN 8

/* ===== 0x0F3 — ENG_RQ2_TCM (Gear actual/target) ===== */
#define SIG_0x0F3_Gr_BIT                 4   /* actual gear — B0 high nibble, confirms Phase 8 model */
#define SIG_0x0F3_Gr_LEN                 4
#define SIG_0x0F3_Gr_Target_BIT          0   /* target gear — B0 low nibble */
#define SIG_0x0F3_Gr_Target_LEN          4
#define SIG_0x0F3_Clutch_Actv_Stat_BIT   16
#define SIG_0x0F3_Clutch_Actv_Stat_LEN   2
#define SIG_0x0F3_EngWhlTrqRatio_TCM_BIT 18
#define SIG_0x0F3_EngWhlTrqRatio_TCM_LEN 14
#define SIG_0x0F3_TxTrqLoss_BIT          32  /* = byte 4, matches "B4 includes TxTrqLoss" */
#define SIG_0x0F3_TxTrqLoss_LEN          8
#define SIG_0x0F3_VehDrvStyle_BIT        40
#define SIG_0x0F3_VehDrvStyle_LEN        2
#define SIG_0x0F3_TxStyle_BIT            42
#define SIG_0x0F3_TxStyle_LEN            2
#define SIG_0x0F3_TxMechStyle_BIT        44
#define SIG_0x0F3_TxMechStyle_LEN        2
#define SIG_0x0F3_TxShiftStyle_BIT       46
#define SIG_0x0F3_TxShiftStyle_LEN       2
#define SIG_0x0F3_TCM_EngIdleRPM_Lvl_Rq_BIT 52 /* QUARANTINED: overlaps empirical MC nibble */
#define SIG_0x0F3_TCM_EngIdleRPM_Lvl_Rq_LEN 2
#define SIG_0x0F3_MC_BIT                 52
#define SIG_0x0F3_MC_LEN                 4
/* No CRC_ENG_RQ2_TCM signal was found in this frame's window — unlike
 * every other CRC-protected frame here. Treat 0x0F3 as MC-only until
 * independently confirmed against a real capture; do not assume CRC
 * validation applies to it. */

/* ===== 0x105 — ENG_RS3_PT (Engine publication) ===== */
#define SIG_0x105_KickDnSw_Psd_BIT       0
#define SIG_0x105_KickDnSw_Psd_LEN       1
#define SIG_0x105_PreHt_Stat_BIT         1
#define SIG_0x105_PreHt_Stat_LEN         1
#define SIG_0x105_EngRPM_BIT             2
#define SIG_0x105_EngRPM_LEN             14
#define SIG_0x105_EngTrqMaxCorrFctr_BIT  16
#define SIG_0x105_EngTrqMaxCorrFctr_LEN  8
#define SIG_0x105_AccelPdlPosn_BIT       24
#define SIG_0x105_AccelPdlPosn_LEN       8
#define SIG_0x105_AccelPdlPosn_Raw_BIT   32
#define SIG_0x105_AccelPdlPosn_Raw_LEN   8
#define SIG_0x105_Term61_Actv_BIT        40
#define SIG_0x105_Term61_Actv_LEN        1
#define SIG_0x105_AddPwrCnsmr_On_Rq_BIT  41
#define SIG_0x105_AddPwrCnsmr_On_Rq_LEN  1
#define SIG_0x105_GenLoad_BIT            42
#define SIG_0x105_GenLoad_LEN            6
#define SIG_0x105_AccelPdlPosnSens_Flt_BIT 52 /* QUARANTINED: overlaps empirical MC nibble */
#define SIG_0x105_AccelPdlPosnSens_Flt_LEN 1
#define SIG_0x105_EngRun_Stat_BIT        53 /* QUARANTINED: overlaps empirical MC nibble */
#define SIG_0x105_EngRun_Stat_LEN        3
#define SIG_0x105_MC_BIT                 52
#define SIG_0x105_MC_LEN                 4
#define SIG_0x105_CRC_BIT                56
#define SIG_0x105_CRC_LEN                8

/* ===== 0x14B — ENG_RS2_PT (Torque publication) ===== */
#define SIG_0x14B_EngTrqStatic_BIT       3
#define SIG_0x14B_EngTrqStatic_LEN       13
#define SIG_0x14B_EngTrqMaxETC_BIT       19
#define SIG_0x14B_EngTrqMaxETC_LEN       13
#define SIG_0x14B_FullOFC_Actv_BIT       32
#define SIG_0x14B_FullOFC_Actv_LEN       1
#define SIG_0x14B_PartOFC_Actv_BIT       33
#define SIG_0x14B_PartOFC_Actv_LEN       1
#define SIG_0x14B_EngTrqMinTTC_BIT       35
#define SIG_0x14B_EngTrqMinTTC_LEN       13
#define SIG_0x14B_MC_BIT                 52
#define SIG_0x14B_MC_LEN                 4
#define SIG_0x14B_CRC_BIT                56
#define SIG_0x14B_CRC_LEN                8

/* ===== 0x17D — TX_RQ_ECM (ECU/PT response) ===== */
#define SIG_0x17D_PwrFreeD_Dsabl_Rq_BIT  0
#define SIG_0x17D_PwrFreeD_Dsabl_Rq_LEN  1
#define SIG_0x17D_ECM_TxShftPoint_Inc_Rq_BIT 1
#define SIG_0x17D_ECM_TxShftPoint_Inc_Rq_LEN 1
#define SIG_0x17D_GrMax_Rq_ECM_BIT       2
#define SIG_0x17D_GrMax_Rq_ECM_LEN       3
#define SIG_0x17D_GrMin_Rq_ECM_BIT       5
#define SIG_0x17D_GrMin_Rq_ECM_LEN       3
#define SIG_0x17D_SSA_EngSp_BIT          8
#define SIG_0x17D_SSA_EngSp_LEN          1
#define SIG_0x17D_SSA_Sp_Warn_BIT        9
#define SIG_0x17D_SSA_Sp_Warn_LEN        1
#define SIG_0x17D_Creep_Off_Rq_BIT       10
#define SIG_0x17D_Creep_Off_Rq_LEN       1
#define SIG_0x17D_Gr1_Rq_ECM_BIT         11
#define SIG_0x17D_Gr1_Rq_ECM_LEN         1
#define SIG_0x17D_ShftChrDsp_Rq_BIT      12
#define SIG_0x17D_ShftChrDsp_Rq_LEN      4
#define SIG_0x17D_SSA_ECO_Sw_LED_On_Rq_BIT 16
#define SIG_0x17D_SSA_ECO_Sw_LED_On_Rq_LEN 2
#define SIG_0x17D_SSA_Enbl_Rq_AS_BIT     18
#define SIG_0x17D_SSA_Enbl_Rq_AS_LEN     1
#define SIG_0x17D_TCC_Rq_BIT             32
#define SIG_0x17D_TCC_Rq_LEN             2
#define SIG_0x17D_TxSlpRPM_Rq_ECM_BIT    34  /* bit position CLOSED; control law OPEN (G1) */
#define SIG_0x17D_TxSlpRPM_Rq_ECM_LEN    14
#define SIG_0x17D_ECM_LHOM_BIT           52
#define SIG_0x17D_ECM_LHOM_LEN           1
#define SIG_0x17D_ECM_LHOM2_BIT          53
#define SIG_0x17D_ECM_LHOM2_LEN          1
#define SIG_0x17D_EngExhstAfterTreat_Actv_BIT 54 /* QUARANTINED: overlaps empirical MC nibble */
#define SIG_0x17D_EngExhstAfterTreat_Actv_LEN 1
#define SIG_0x17D_EngRPM_Sens_LHOM_BIT   55 /* QUARANTINED: overlaps empirical MC nibble */
#define SIG_0x17D_EngRPM_Sens_LHOM_LEN   1
#define SIG_0x17D_MC_BIT                 52
#define SIG_0x17D_MC_LEN                 4
#define SIG_0x17D_CRC_BIT                56
#define SIG_0x17D_CRC_LEN                8
/* NOTE: no B1=0x20/0x30 "mode" field was found under this exact name in
 * the DAT file — Phase 8's B1 selection-condition question may refer to
 * a packed representation of GrMax/GrMin/ShftChrDsp/Gr1_Rq/Creep_Off_Rq
 * within byte 1 (bits 8-15) rather than a single named enum. Treat the
 * B1=0x20/0x30 question as still open; this extraction doesn't resolve
 * it, only the surrounding field positions. */

/* ===== 0x1CD — ENG_RS1_PT (Torque arbitration) ===== */
#define SIG_0x1CD_CC_Encode_ECM_BIT      1
#define SIG_0x1CD_CC_Encode_ECM_LEN      1
#define SIG_0x1CD_EngTrq_Enbl_Rq_AS_BIT  2
#define SIG_0x1CD_EngTrq_Enbl_Rq_AS_LEN  1
#define SIG_0x1CD_EngTrqSel_D_TTC_BIT    3   /* real name has _TTC suffix, not plain "EngTrqSel_D" */
#define SIG_0x1CD_EngTrqSel_D_TTC_LEN    13
#define SIG_0x1CD_EngTrqAdjFast_Enbl_BIT 17
#define SIG_0x1CD_EngTrqAdjFast_Enbl_LEN 1
#define SIG_0x1CD_EngTrq_Enbl_Rq_SBC_BIT 18
#define SIG_0x1CD_EngTrq_Enbl_Rq_SBC_LEN 1
#define SIG_0x1CD_EngTrqSel_AS_TTC_BIT   19
#define SIG_0x1CD_EngTrqSel_AS_TTC_LEN   13
#define SIG_0x1CD_EngTrq_Ack_ECM_BIT     33
#define SIG_0x1CD_EngTrq_Ack_ECM_LEN     1
#define SIG_0x1CD_EngTrq_Enbl_Rq_TCM_BIT 34
#define SIG_0x1CD_EngTrq_Enbl_Rq_TCM_LEN 1
#define SIG_0x1CD_EngTrqSel_SBC_TTC_BIT  35
#define SIG_0x1CD_EngTrqSel_SBC_TTC_LEN  13
#define SIG_0x1CD_MC_BIT                 52
#define SIG_0x1CD_MC_LEN                 4
#define SIG_0x1CD_CRC_BIT                56
#define SIG_0x1CD_CRC_LEN                8

/* ===== 0x429 — NM_ECM (Network management) ===== */
#define SIG_0x429_NM_Mode_BIT            0
#define SIG_0x429_NM_Mode_LEN            8
#define SIG_0x429_NM_Successor_BIT       8
#define SIG_0x429_NM_Successor_LEN       8
#define SIG_0x429_NM_Sleep_Ind_BIT       16
#define SIG_0x429_NM_Sleep_Ind_LEN       1
#define SIG_0x429_NM_Sleep_Ack_BIT       17
#define SIG_0x429_NM_Sleep_Ack_LEN       1
#define SIG_0x429_NM_Ud_Launch_BIT       18
#define SIG_0x429_NM_Ud_Launch_LEN       6
#define SIG_0x429_NM_Ud_Srv_BIT          24
#define SIG_0x429_NM_Ud_Srv_LEN          8
#define SIG_0x429_Awake_EngRestart_BIT   32
#define SIG_0x429_Awake_EngRestart_LEN   1
#define SIG_0x429_Awake_EngFan_Actv_BIT  33
#define SIG_0x429_Awake_EngFan_Actv_LEN  1
#define SIG_0x429_Awake_Afterrun_Actv_BIT 34
#define SIG_0x429_Awake_Afterrun_Actv_LEN 1
#define SIG_0x429_Awake_Ignition_On_BIT  35
#define SIG_0x429_Awake_Ignition_On_LEN  1
#define SIG_0x429_Awake_Diag_Actv_BIT    38
#define SIG_0x429_Awake_Diag_Actv_LEN    1
#define SIG_0x429_Awake_NwSt_BIT         39
#define SIG_0x429_Awake_NwSt_LEN         1
#define SIG_0x429_Awake_HYBRID_BIT       44
#define SIG_0x429_Awake_HYBRID_LEN       1
#define SIG_0x429_Awake_CCP_Actv_BIT     45
#define SIG_0x429_Awake_CCP_Actv_LEN     1
#define SIG_0x429_Awake_POWERTRAIN_BIT   46
#define SIG_0x429_Awake_POWERTRAIN_LEN   1
#define SIG_0x429_Awake_CHASSIS_BIT      47
#define SIG_0x429_Awake_CHASSIS_LEN      1
#define SIG_0x429_Nw_Id_BIT              56
#define SIG_0x429_Nw_Id_LEN              8
/* No CRC field found for NM_ECM — NM frames in AUTOSAR/OSEK-style network
 * management commonly skip CRC by design (short, low-rate control frames).
 * Do not enforce CRC validation on 0x429. */

/* ===== 0x30D — ECM_A1 (deferred — telemetry, not drivetrain-critical) ===== */
#define SIG_0x30D_EngCoolTemp_BIT        0
#define SIG_0x30D_EngCoolTemp_LEN        8
#define SIG_0x30D_IntkAirTemp_BIT        8
#define SIG_0x30D_IntkAirTemp_LEN        8
#define SIG_0x30D_EngOilTemp_BIT         16
#define SIG_0x30D_EngOilTemp_LEN         8
#define SIG_0x30D_EngOilLvl_BIT          24
#define SIG_0x30D_EngOilLvl_LEN          8
#define SIG_0x30D_EngOilQual_BIT         32
#define SIG_0x30D_EngOilQual_LEN         8
#define SIG_0x30D_FuelCons_BIT           40
#define SIG_0x30D_FuelCons_LEN           16
#define SIG_0x30D_AirPress_Outsd_BIT     56
#define SIG_0x30D_AirPress_Outsd_LEN     8

/* ===== 0x349 — ECM_A2 (deferred — telemetry, not drivetrain-critical) ===== */
#define SIG_0x349_AddWtrPmp_On_Rq_ECM_BIT 0
#define SIG_0x349_AddWtrPmp_On_Rq_ECM_LEN 1
#define SIG_0x349_OBD_Clr_Rq_BIT          1
#define SIG_0x349_OBD_Clr_Rq_LEN          1
#define SIG_0x349_RevGr_Engg_MTX_ECM_BIT  2  /* manual-transmission only — not relevant to auto */
#define SIG_0x349_RevGr_Engg_MTX_ECM_LEN  2
#define SIG_0x349_EM2_Run_Stat_BIT        4
#define SIG_0x349_EM2_Run_Stat_LEN        2
#define SIG_0x349_HtPwr_Stat_BIT          6
#define SIG_0x349_HtPwr_Stat_LEN          2
#define SIG_0x349_HVAC_CompTrq_Max_BIT    8
#define SIG_0x349_HVAC_CompTrq_Max_LEN    8
#define SIG_0x349_Clutch_Disengg_BIT      24
#define SIG_0x349_Clutch_Disengg_LEN      1
#define SIG_0x349_FuelPmp_On_Rq_BIT       25
#define SIG_0x349_FuelPmp_On_Rq_LEN       1
#define SIG_0x349_EngIdleRPM_Dsr_BIT      26
#define SIG_0x349_EngIdleRPM_Dsr_LEN      14

#endif /* SIGNAL_ANCHORS_H */

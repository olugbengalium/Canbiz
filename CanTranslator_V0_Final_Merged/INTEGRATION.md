# CanTranslator_V0 — merged Core integration

## What this package is

This is the corrected merge of the newly uploaded Phase 9E files into a CubeMX-friendly Core layout. `main.c` is retained as the application entry point.
It preserves the evidence-gated architecture and fixes the protocol/configuration errors found while reconciling the uploaded code with the latest Phase 8/9 corrections.

## Important limitation

The live GitHub Core tree could not be fetched from this runtime, so this package is a **corrected merge of the uploaded Core files**, not a byte-for-byte merge against the current remote repository. The uploaded files are the source of truth used here.

## Major corrections made

1. **CAN timing corrected to the uploaded IOC clock tree**
   - HCLK/SYSCLK = 50 MHz
   - APB1 = 25 MHz
   - CAN1/CAN2 = 500 kbit/s
   - Prescaler = 5
   - BS1 = 8TQ
   - BS2 = 1TQ
   - TIM2 timestamp = 1 MHz using prescaler 49

2. **CAN mode corrected for a translator**
   - IOC changed from SILENT to NORMAL.
   - NART changed to DISABLE so HAL auto-retransmission remains enabled.
   - This is not a passive sniffer configuration.

3. **CubeMX ownership respected**
   - `MX_GPIO_Init()`, `MX_CAN1_Init()` and `MX_CAN2_Init()` remain CubeMX-owned.
   - `can_bus.c` no longer duplicates peripheral/GPIO initialization.
   - `CanBus_Init()` is called only after the CubeMX init functions.

4. **Per-ID integrity policy corrected**
   - Standard J1850 B0..B6 -> B7: 0x001, 0x073, 0x0D5, 0x0F1, 0x105, 0x14B, 0x17D, 0x1CD.
   - 0x06D: DLC=4, B0..B2 -> B3.
   - 0x0F3: no CRC.
   - 0x429: no CRC.

5. **Empirical MC positions**
   - Normal tested counter frames: bits 52..55.
   - 0x06D: bits 20..23.
   - MC sequence is tracked diagnostically; startup seed is not guessed.

6. **Overlapping DAT fields quarantined**
   - DAT-defined fields overlapping the empirically proven MC nibble are not decoded as runtime state.
   - This affects, among others, 0x0F1 start/emergency fields, 0x0F3 fields in the MC nibble, and 0x105 EngRun_Stat.

7. **M113 native source signals added as raw source telemetry**
   - 0x268 B5 = accelerator pedal raw.
   - 0x3A0 B0 = ASCII gear display and normalized gear.
   - 0x3D0 B1 = engine-RPM raw byte; scale intentionally not assumed.

8. **Bus assignment is represented explicitly**
   - 0x0F1 and 0x17D are CAN2/PT.
   - IDs observed on both buses are marked accordingly.
   - Unexpected-bus traffic is logged and not silently discarded in the bench build.

9. **Nonexistent HAL error-reset call removed**
   - Bus-off is recorded; recovery is deferred to main-context bus-state management.

## Current scope

This package is the **receive/integrity/semantic foundation**. It is not yet the final vehicle translator because the following remain intentionally open:

- exact M113 torque source / scaling
- 0x17D TxSlpRPM numerical law
- 0x1CD arbitration law
- 0x17D B1 mode question
- 0x03F crank/start sequence
- physical scaling/offset/signedness where not independently established

## CubeMX main integration

After CubeMX-generated initialization:

```c
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_CAN1_Init();
MX_CAN2_Init();

if (!Translator_AppInit()) {
    Error_Handler();
}

while (1) {
    Translator_AppPoll();
}
```

The CubeMX-generated interrupt file must contain the CAN IRQ handlers and call `HAL_CAN_IRQHandler()` for CAN1 and CAN2. The uploaded IOC already enables CAN1_RX0_IRQn and CAN2_RX0_IRQn.

## Next implementation step

Once this foundation builds and receives real frames, the next module should be the **TX scheduler/packers** for the proven W204 outputs. Do not add guessed TxSlpRPM, torque-arbitration equations, or unverified 0x105/0x0F1 fields just to make the transmitter appear complete.

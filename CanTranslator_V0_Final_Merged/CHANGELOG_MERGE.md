# Merge / correction changelog

## Sources merged

- can_bus.[ch]
- can_trace.[ch]
- crc8_j1850.[ch]
- decoders.[ch]
- frame_db.h
- signal_anchors.h
- vehicle_state.h
- m113_native_signals.h
- main_loop_integration.c
- CanTranslator_V0.ioc

## Added

- frame_integrity.[ch]
- translator_app.[ch]
- INTEGRATION.md

## Corrections made from the latest Phase 8/9 evidence

### CAN timing / CubeMX

- 50 MHz SYSCLK/HCLK and 25 MHz APB1 from the supplied IOC are authoritative.
- 500 kbit/s uses Prescaler 5, BS1 8TQ, BS2 1TQ.
- TIM2 timestamp changed to prescaler 49 for a 1 MHz tick.
- CAN1/CAN2 changed from SILENT to NORMAL because this is a translator, not a passive sniffer.
- NART changed to DISABLE so auto retransmission remains enabled.
- CubeMX remains owner of GPIO and CAN peripheral initialization.

### Integrity

- Standard J1850 B0..B6 -> B7: 0x001, 0x073, 0x0D5, 0x0F1, 0x105, 0x14B, 0x17D, 0x1CD.
- 0x06D: DLC 4, CRC B0..B2 -> B3.
- 0x0F3: no CRC.
- 0x429: no CRC.
- MC: normal tested frames bits 52..55; 0x06D bits 20..23.
- MC is observed/diagnostic only; startup seed is not guessed.

### Signal quarantine

DAT-defined signals overlapping the empirical MC nibble are retained as documentary anchors but are not decoded into runtime state. This includes the affected 0x0F1, 0x0F3, 0x105 and 0x17D fields in the MC nibble.

### M113 native source evidence

Added raw source decoding for:

- 0x268 B5 accelerator pedal
- 0x3A0 B0 ASCII gear display
- 0x3D0 B1 engine RPM raw byte

No unverified RPM scaling or torque signal was invented.

## Verification

The merged C sources were syntax-checked with GCC against a local HAL interface stub. This verifies C-level integration and symbol consistency but is not a substitute for an actual STM32CubeIDE build with STM32CubeF4 HAL V1.28.3.

The exact live GitHub Core tree could not be fetched from this runtime, so the merge is against the newly uploaded source set and the supplied CubeMX IOC. The package is intended as a corrected drop-in Core baseline.

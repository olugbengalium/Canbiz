# CanTranslator V0 — final merge/reconciliation report

Date: 2026-08-23

## Result

The uploaded translator modules were re-integrated into a CubeMX-friendly STM32F407 baseline. `main.c` is explicitly retained as the application entry point.

## Source basis

- Uploaded C/H module set supplied in this conversation.
- Supplied `CanTranslator_V0.ioc`.
- The previously generated merged Core package was used as an intermediate baseline.
- The live GitHub Core tree could not be fetched from this runtime because external GitHub access was unavailable. Therefore this is not a byte-for-byte merge against the current remote repository.

## Corrected integration

### Application entry point

Added a complete `Core/Src/main.c` with:

- HAL startup
- 50 MHz system clock
- CAN1/CAN2 CubeMX-style initialization
- `Translator_AppInit()` after peripheral initialization
- `Translator_AppPoll()` inside the main loop
- `Error_Handler()`

### CAN configuration

- CAN1/CAN2 = 500 kbit/s
- Prescaler 5
- BS1 8TQ
- BS2 1TQ
- SJW 1TQ
- NORMAL mode
- automatic retransmission enabled
- RX FIFO0 interrupts enabled
- SCE error interrupts enabled

### Interrupt/MSP support

Added:

- `stm32f4xx_hal_msp.c`
- `stm32f4xx_it.c`

These provide the CAN GPIO/clock configuration and CAN RX/error IRQ handlers required by the uploaded application layer.

### Timestamping

Removed the hidden TIM2 dependency from `can_bus.c`. Timestamping now uses the Cortex-M4 DWT cycle counter. This avoids adding a peripheral that was absent from the supplied IOC. At 50 MHz HCLK it provides a microsecond-equivalent counter.

### Integrity corrections

The implementation now follows the latest evidence matrix:

- J1850 B0..B6 → B7: 0x001, 0x073, 0x0D5, 0x0F1, 0x105, 0x14B, 0x17D, 0x1CD
- 0x06D: DLC4, J1850 B0..B2 → B3
- 0x0F3: no CRC
- 0x429: no CRC
- normal tested MC: bits 52..55
- 0x06D MC: bits 20..23

### Evidence-boundary correction

Raw torque fields are no longer sign-extended in the semantic state because signedness/scaling was explicitly left open by the evidence. They are stored as raw unsigned values until calibration is independently proven.

### M113 native source integration

The bench decoder recognizes:

- 0x268 B5 accelerator pedal raw
- 0x3A0 B0 ASCII gear display
- 0x3D0 B1 engine-RPM raw byte

No unverified physical RPM scale or M113 torque signal is invented.

## Intentionally not implemented

The following remain evidence-gated:

- 0x17D TxSlpRPM numerical law
- 0x1CD torque arbitration law
- 0x17D B1 operating-mode question
- 0x03F crank/start sequence
- M113 native torque CAN source/scaling
- physical scaling/offset/signedness not independently established

## Verification performed

The C sources in the merged baseline were syntax-checked with GCC against a local HAL interface stub during the merge process. This verified C-level symbol/prototype integration. It is not a replacement for the real STM32CubeIDE build with STM32CubeF4 V1.28.3.

## Immediate build sequence

1. Open `CanTranslator_V0.ioc` in STM32CubeIDE.
2. Generate/update CubeMX code.
3. Preserve the translator calls in `main.c` if CubeMX regenerates the file.
4. Build with STM32CubeF4 V1.28.3.
5. Program the STM32F407 bench board without connecting the vehicle.
6. Verify CAN RX, trace records, CRC validation, MC telemetry and CAN error handling.
7. Only after this baseline passes should TX packers/schedulers be added.

# CanTranslator V0 — corrected STM32F407 baseline

This is the corrected Core baseline for the M113 single-CAN → W204 dual-CAN translator.

## MCU / CAN baseline

- MCU: STM32F407VGTx
- HCLK/SYSCLK: 50 MHz
- APB1 / bxCAN clock: 25 MHz
- CAN1: 500 kbit/s, NORMAL, PB8/PB9 AF9
- CAN2: 500 kbit/s, NORMAL, PB12/PB13 AF9
- RX FIFO0 interrupts enabled
- SCE error interrupts enabled
- CAN auto-retransmission enabled

## Application entry point

`Core/Src/main.c` remains the CubeMX application entry point.

The runtime sequence is:

```c
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_CAN1_Init();
MX_CAN2_Init();
Translator_AppInit();
while (1) {
    Translator_AppPoll();
}
```

Do not move translator logic into `main.c`. Keep it in the application/modules.

## Current implementation scope

Implemented:

- CAN RX/TX infrastructure
- raw timestamping via Cortex-M4 DWT (no hidden TIM2 peripheral)
- per-ID integrity policy
- CRC-8 J1850
- special 0x06D CRC
- empirical MC extraction
- canonical vehicle state
- W204 receive decoders
- M113 raw pedal/gear/RPM source telemetry
- trace logging

Intentionally open:

- M113 torque source/scaling
- physical scaling/offset/signedness not independently established
- 0x17D TxSlpRPM control law
- 0x1CD torque arbitration law
- 0x17D B1 operating-mode question
- 0x03F crank/start sequence

## Integrity policy

- J1850 B0..B6 → B7: 0x001, 0x073, 0x0D5, 0x0F1, 0x105, 0x14B, 0x17D, 0x1CD
- 0x06D: DLC4, J1850 B0..B2 → B3
- 0x0F3: no CRC
- 0x429: no CRC
- normal empirical MC: bits 52..55
- 0x06D MC: bits 20..23

## Important

This is a bench-ready foundation, not a vehicle-release firmware. The next coding step is the TX scheduler and packers for only the proven outputs. Unknown control laws must remain configurable and instrumented.

## CubeMX workflow

1. Open `CanTranslator_V0.ioc` in STM32CubeIDE/CubeMX.
2. Generate code for STM32CubeF4 V1.28.3.
3. Keep the generated `main.c`, `stm32f4xx_hal_msp.c`, and `stm32f4xx_it.c` if CubeMX regenerates them, then preserve the translator calls in `main.c`.
4. Copy/retain the translator modules in `Core/Inc` and `Core/Src`.
5. Build before connecting a vehicle.
6. First bench test should verify CAN RX, trace, CRC/MC telemetry and error handling only.

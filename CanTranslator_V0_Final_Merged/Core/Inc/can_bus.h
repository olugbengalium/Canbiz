/*
 * can_bus.h
 *
 * Dual-CAN bring-up for CanTranslator_V0 (STM32F407VG)
 *   CAN1 (M113 source bus)      : PB8 (RX), PB9 (TX)   500 kbps
 *   CAN2 (W204 CAN2/PT dest bus): PB12(RX), PB13(TX)   500 kbps
 *
 * Phase 9E implementation sequence, step 1: hardware/CAN bring-up —
 * stable RX/TX, timestamps, bus-error handling. No semantic decoding
 * happens here; this layer only moves raw frames + timestamps into queues.
 *
 * NOTE: On STM32F407, CAN2 is a "slave" peripheral that shares the filter
 * bank and cannot operate unless CAN1's peripheral clock is enabled
 * (CAN1 does not need to be actively used on the bus for this to be true).
 */

#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CANBUS_1 = 0,   /* M113 ECU-side source bus */
    CANBUS_2 = 1,   /* W204 CAN2 / blue Powertrain destination bus */
    CANBUS_COUNT
} CanBusId_t;

typedef struct {
    uint32_t   id;
    bool       is_extended;
    bool       is_rtr;
    uint8_t    dlc;
    uint8_t    data[8];
    uint32_t   timestamp_us;
    CanBusId_t bus;
} CanFrame_t;

#define CAN_RX_QUEUE_DEPTH   64   /* power of two */

typedef struct {
    CanFrame_t buf[CAN_RX_QUEUE_DEPTH];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint32_t overflow_count;
} CanRxQueue_t;

typedef struct {
    volatile uint32_t rx_count;
    volatile uint32_t tx_count;
    volatile uint32_t tx_fail_count;
    volatile uint32_t error_count;
    volatile uint32_t last_esr;
    volatile uint32_t bus_off_count;
    volatile uint32_t queue_overflow_count;
} CanBusStats_t;

bool CanBus_Init(void);
bool CanBus_Transmit(CanBusId_t bus, uint32_t id, bool extended,
                      const uint8_t *data, uint8_t dlc);
bool CanBus_Dequeue(CanBusId_t bus, CanFrame_t *out_frame);
uint16_t CanBus_QueueDepth(CanBusId_t bus);
const CanBusStats_t *CanBus_GetStats(CanBusId_t bus);
uint32_t CanBus_GetTimestampUs(void);

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

#ifdef __cplusplus
}
#endif

#endif /* CAN_BUS_H */

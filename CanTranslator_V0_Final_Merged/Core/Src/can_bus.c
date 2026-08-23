/*
 * can_bus.c
 *
 * CubeMX-owned peripheral initialization is intentionally kept in main.c /
 * stm32f4xx_hal_msp.c. This module only owns runtime CAN filter/start,
 * RX queues, timestamps, TX and error telemetry.
 *
 * Corrected clock/timing baseline from CanTranslator_V0.ioc:
 *   HCLK/SYSCLK = 50 MHz
 *   APB1        = 25 MHz
 *   bxCAN clock = 25 MHz
 *   Prescaler=5, BS1=8TQ, BS2=1TQ -> 500 kbit/s
 *   TIM2 timer clock = 50 MHz; Prescaler=49 -> 1 MHz timestamp tick
 */
#include "can_bus.h"
#include <string.h>

static CanRxQueue_t  s_rxq[CANBUS_COUNT];
static CanBusStats_t s_stats[CANBUS_COUNT];

static uint32_t s_timestamp_cycles_per_us;

static bool CanBus_TimestampInit(void)
{
    /* Use the Cortex-M4 DWT cycle counter instead of adding a hidden TIM2
     * peripheral that is absent from the supplied CubeMX IOC. At the frozen
     * 50 MHz HCLK this gives a 1 MHz-equivalent microsecond timestamp. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    s_timestamp_cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000u;
    return s_timestamp_cycles_per_us != 0u;
}

uint32_t CanBus_GetTimestampUs(void)
{
    return DWT->CYCCNT / s_timestamp_cycles_per_us;
}

static bool CanBus_FilterInit(void)
{
    CAN_FilterTypeDef filt = {0};

    /* Accept-all during bench bring-up. Semantic/integrity policy is applied
     * in software so the raw evidence stream is not hidden by hardware filters. */
    filt.FilterMode = CAN_FILTERMODE_IDMASK;
    filt.FilterScale = CAN_FILTERSCALE_32BIT;
    filt.FilterIdHigh = 0;
    filt.FilterIdLow = 0;
    filt.FilterMaskIdHigh = 0;
    filt.FilterMaskIdLow = 0;
    filt.FilterFIFOAssignment = CAN_RX_FIFO0;
    filt.FilterActivation = ENABLE;
    filt.SlaveStartFilterBank = 14;

    filt.FilterBank = 0;
    if (HAL_CAN_ConfigFilter(&hcan1, &filt) != HAL_OK) return false;

    filt.FilterBank = 14;
    if (HAL_CAN_ConfigFilter(&hcan2, &filt) != HAL_OK) return false;

    return true;
}

static bool CanBus_Start(void)
{
    if (HAL_CAN_Start(&hcan1) != HAL_OK) return false;
    if (HAL_CAN_Start(&hcan2) != HAL_OK) return false;

    const uint32_t its = CAN_IT_RX_FIFO0_MSG_PENDING |
                         CAN_IT_ERROR_WARNING |
                         CAN_IT_ERROR_PASSIVE |
                         CAN_IT_BUSOFF |
                         CAN_IT_ERROR;

    if (HAL_CAN_ActivateNotification(&hcan1, its) != HAL_OK) return false;
    if (HAL_CAN_ActivateNotification(&hcan2, its) != HAL_OK) return false;
    return true;
}

bool CanBus_Init(void)
{
    /* IMPORTANT: call this only after CubeMX has executed MX_CAN1_Init(),
     * MX_CAN2_Init() and MX_GPIO_Init(). The .ioc is configured for NORMAL
     * mode and 500 kbit/s on both buses. */
    memset(s_rxq, 0, sizeof(s_rxq));
    memset(s_stats, 0, sizeof(s_stats));

    if (!CanBus_TimestampInit()) return false;
    if (!CanBus_FilterInit()) return false;
    if (!CanBus_Start()) return false;
    return true;
}

static void CanBus_EnqueueRx(CanBusId_t bus, const CanFrame_t *frame)
{
    CanRxQueue_t *q = &s_rxq[bus];
    uint16_t next_head = (uint16_t)((q->head + 1u) & (CAN_RX_QUEUE_DEPTH - 1u));

    if (next_head == q->tail) {
        q->overflow_count++;
        s_stats[bus].queue_overflow_count++;
        return;
    }

    q->buf[q->head] = *frame;
    q->head = next_head;
}

bool CanBus_Dequeue(CanBusId_t bus, CanFrame_t *out_frame)
{
    CanRxQueue_t *q = &s_rxq[bus];
    if (q->tail == q->head) return false;

    __disable_irq();
    *out_frame = q->buf[q->tail];
    q->tail = (uint16_t)((q->tail + 1u) & (CAN_RX_QUEUE_DEPTH - 1u));
    __enable_irq();
    return true;
}

uint16_t CanBus_QueueDepth(CanBusId_t bus)
{
    const CanRxQueue_t *q = &s_rxq[bus];
    return (uint16_t)((q->head - q->tail) & (CAN_RX_QUEUE_DEPTH - 1u));
}

const CanBusStats_t *CanBus_GetStats(CanBusId_t bus)
{
    return &s_stats[bus];
}

bool CanBus_Transmit(CanBusId_t bus, uint32_t id, bool extended,
                     const uint8_t *data, uint8_t dlc)
{
    CAN_HandleTypeDef *hcan = (bus == CANBUS_1) ? &hcan1 : &hcan2;
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t tx_mailbox;

    if (!data || dlc > 8u) return false;

    tx_header.IDE = extended ? CAN_ID_EXT : CAN_ID_STD;
    if (extended) tx_header.ExtId = id;
    else          tx_header.StdId = id;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = dlc;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0u) {
        s_stats[bus].tx_fail_count++;
        return false;
    }

    if (HAL_CAN_AddTxMessage(hcan, &tx_header, (uint8_t *)data, &tx_mailbox) != HAL_OK) {
        s_stats[bus].tx_fail_count++;
        return false;
    }

    s_stats[bus].tx_count++;
    return true;
}

static void CanBus_HandleRxFifo0(CAN_HandleTypeDef *hcan, CanBusId_t bus)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0u) {
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) break;

        CanFrame_t frame = {0};
        frame.bus = bus;
        frame.is_extended = (rx_header.IDE == CAN_ID_EXT);
        frame.id = frame.is_extended ? rx_header.ExtId : rx_header.StdId;
        frame.is_rtr = (rx_header.RTR == CAN_RTR_REMOTE);
        frame.dlc = (uint8_t)rx_header.DLC;
        frame.timestamp_us = CanBus_GetTimestampUs();
        memcpy(frame.data, rx_data, sizeof(frame.data));

        CanBus_EnqueueRx(bus, &frame);
        s_stats[bus].rx_count++;
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1) CanBus_HandleRxFifo0(hcan, CANBUS_1);
    else if (hcan->Instance == CAN2) CanBus_HandleRxFifo0(hcan, CANBUS_2);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    CanBusId_t bus = (hcan->Instance == CAN1) ? CANBUS_1 : CANBUS_2;
    uint32_t err = HAL_CAN_GetError(hcan);

    s_stats[bus].error_count++;
    s_stats[bus].last_esr = hcan->Instance->ESR;

    if (err & HAL_CAN_ERROR_BOF) {
        s_stats[bus].bus_off_count++;
        /* Recovery is intentionally not performed inside the ISR. A future
         * bus-state manager can stop/start the peripheral from main context. */
    }
}

/**
 ******************************************************************************
 * @file        co_can_nrf24l01.c
 * @author      Zoltan Dolensky
 * @brief       
 *
 ******************************************************************************
 * @attention
 *
 * MIT License
 * -----------
 * Copyright (c) 2022 Technical university of Liberec (https://tul.cz)
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */

#include "co_can_nrf24l01.h"

#define NRFCAN_DLC_EXT_ID           (1 << 7)

static void     DrvCanInit(void);
static void     DrvCanEnable(uint32_t baudrate);
static int16_t  DrvCanSend(CO_IF_FRM *frm);
static int16_t  DrvCanRead(CO_IF_FRM *frm);
static void     DrvCanReset(void);
static void     DrvCanClose(void);

static int      nrf24l01_service_send(nrf24l01_service_t *svc, nrf24l01_message_t *message);
static int      nrf24l01_service_recv(nrf24l01_service_t *svc, nrf24l01_message_t *message);
static void     nrf24l01_service_on_event(void *context);

static nrf24l01_service_t service;

const CO_IF_CAN_DRV co_can_nrf24l01 = {
    &DrvCanInit,
    &DrvCanEnable,
    &DrvCanRead,
    &DrvCanSend,
    &DrvCanReset,
    &DrvCanClose
};

static void DrvCanInit(void) {
    nrf24l01_config_t config = { .address = 0xcecececece, .channel = 110, .retr_count = 3, .retr_delay = 250 };

    nrf24l01_hal_attach(&service.device, &nrf24l01_hal_stm32l4xx);
    nrf24l01_initialize(&service.device);
    nrf24l01_configure(&service.device, &config);

    if (nrf24l01_probe(&service.device) < 0) {
        HAL_NVIC_SystemReset();
    }

    service.txq = xQueueCreateStatic(16,
                                     sizeof(nrf24l01_message_t),
                                     (uint8_t* ) &service.txbuff[0],
                                     &service.txc);
    service.rxq = xQueueCreateStatic(16,
                                     sizeof(nrf24l01_message_t),
                                     (uint8_t* ) &service.rxbuff[0],
                                     &service.rxc);
}

static void DrvCanEnable(uint32_t baudrate) {
    (void) baudrate;
    nrf24l01_notify(&service.device, &nrf24l01_service_on_event, &service);
    nrf24l01_open(&service.device);
    nrf24l01_listen(&service.device);
}

static int16_t DrvCanSend(CO_IF_FRM *frm) {
    nrf24l01_message_t message;
    uint8_t index;

    index = 0;
    message.data[index] = frm->DLC;

    if (frm->Identifier > 0x7ff) {
        message.data[index++] |= NRFCAN_DLC_EXT_ID;
        message.data[index++] = ((frm->Identifier >> 24) & 0xff);
        message.data[index++] = ((frm->Identifier >> 16) & 0xff);
        message.data[index++] = ((frm->Identifier >> 8 ) & 0xff);
        message.data[index++] = ( frm->Identifier        & 0xff);
    } else {
        message.data[index++] &= ~NRFCAN_DLC_EXT_ID;
        message.data[index++] = ((frm->Identifier >> 8)  & 0xff);
        message.data[index++] = ( frm->Identifier        & 0xff);
    }

    for (uint8_t i = 0; i < frm->DLC; i++) {
        message.data[index++] = frm->Data[i];
    }

    message.size = index;

    if (nrf24l01_service_send(&service, &message) < 0) {
        return (-1);
    }

    return sizeof(CO_IF_FRM);
}

static int16_t DrvCanRead(CO_IF_FRM *frm) {
    nrf24l01_message_t message;
    uint8_t index;

    if (nrf24l01_service_recv(&service, &message) < 0) {
        return (-1);
    }
    if (message.size < 3) {
        return (-1);
    }

    if (message.data[0] & NRFCAN_DLC_EXT_ID) {
        index = 5;
        frm->DLC = message.data[0] & ~(NRFCAN_DLC_EXT_ID);
        frm->Identifier = (message.data[1] << 24) |
                          (message.data[2] << 16) |
                          (message.data[3] << 8 ) |
                          (message.data[4]      ) ;
    } else {
        index = 3;
        frm->DLC = message.data[0];
        frm->Identifier = (message.data[1] << 8 ) |
                          (message.data[2]      ) ;
    }

    for (uint8_t i = 0; i < frm->DLC; i++) {
        frm->Data[i] = message.data[index++];
    }

    return (sizeof(CO_IF_FRM));
}

static void DrvCanReset(void) {
    DrvCanClose();
    DrvCanEnable(0);
}

static void DrvCanClose(void) {
    nrf24l01_close(&service.device);
}

static int nrf24l01_service_send(struct nrf24l01_service *svc, nrf24l01_message_t *message) {

    if (xQueueSend(svc->txq, message, 0) == pdTRUE) {
        nrf24l01_trigger_irq(&svc->device);
        return (0);
    }
    return (-1);
}

static int nrf24l01_service_recv(struct nrf24l01_service *svc, nrf24l01_message_t *message) {

    if (xQueueReceive(svc->rxq, message, portMAX_DELAY) == pdTRUE) {
        return (0);
    }
    return (-1);
}

static void nrf24l01_service_on_event(void *context) {
    BaseType_t          xHigherPriorityTaskWoken = pdFALSE;
    nrf24l01_service_t *svc = (nrf24l01_service_t*) (context);
    nrf24l01_message_t  message;
    uint8_t             status;

    /* Set device to standby mode to disable its clock */
    nrf24l01_standby(&svc->device);
    /* Read and clear status register */
    status = nrf24l01_clear_status(&svc->device);
    /* Start listening in order to acquire channel activity measurement */
    nrf24l01_listen(&svc->device);

    if (status & NRF24L01_STATUS_RX_DR) {
        /* Read all pending messages */
        while (nrf24l01_rx_pending(&svc->device) > 0) {
            /* Fetch message from device */
            nrf24l01_read(&svc->device, &message.data[0], &message.size);
            if (xQueueSendFromISR(svc->rxq, &message, &xHigherPriorityTaskWoken) != pdFALSE) {
                /* Reception complete */
                svc->stats.rx_complete++;
            } else {
                /* Queue is full, message is lost */
                svc->stats.rx_lost++;
            }
        }
    }
    if (status & NRF24L01_STATUS_MAX_RT) {
        /* Flush devices tx fifo in order to release failed transmission */
        nrf24l01_flush_tx(&svc->device);
        svc->stats.tx_lost++;
    }
    if (status & NRF24L01_STATUS_TX_DS) {
        /* Transmission complete */
        svc->stats.tx_complete++;
    }
    if (uxQueueMessagesWaitingFromISR(svc->txq) > 0) {
        /* Outgoing messages available, check channel availability */
        if (nrf24l01_channel_available(&svc->device)) {
            /* Fetch outgoing message from queue and transmit */
            xQueueReceiveFromISR(svc->txq, &message, &xHigherPriorityTaskWoken);
            nrf24l01_write(&svc->device, &message.data[0], message.size);
        } else {
            /* Channel is not available, postpone transmission */
            svc->stats.tx_postponed++;
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 ******************************************************************************
 * @file        co_can_nrf24l01.h
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

#ifndef INC_CO_CAN_NRF24L01_H_
#define INC_CO_CAN_NRF24L01_H_

#ifdef __cpluplus 
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"

#include "nrf24l01.h"
#include "nrf24l01_hal_stm32l4xx.h"

#include "co_if.h"

typedef struct {
    uint8_t             size;
    uint8_t             data[NRF24L01_MAX_PAYLOAD_SIZE];
}nrf24l01_message_t;

typedef struct {
    uint32_t            tx_complete;
    uint32_t            tx_lost;
    uint32_t            tx_postponed;

    uint32_t            rx_complete;
    uint32_t            rx_lost;
} nrf24l01_stats_t;

typedef struct nrf24l01_service {
    nrf24l01_t          device;
    nrf24l01_stats_t    stats;

    nrf24l01_message_t  txbuff[16];
    QueueHandle_t       txq;
    StaticQueue_t       txc;

    nrf24l01_message_t  rxbuff[16];
    QueueHandle_t       rxq;
    StaticQueue_t       rxc;
} nrf24l01_service_t;


extern const CO_IF_CAN_DRV co_can_nrf24l01;

#ifdef __cpluplus 
}
#endif

#endif /* INC_CO_CAN_NRF24L01_H_ */

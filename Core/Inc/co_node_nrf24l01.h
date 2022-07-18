/**
 ******************************************************************************
 * @file        co_node_nrf24l01.h
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

#ifndef INC_CO_NODE_NRF24L01_H_
#define INC_CO_NODE_NRF24L01_H_

#ifdef __cpluplus 
extern "C" {
#endif

#include "co_core.h"

#define CO_NODE_ID                  (2u)
#define CO_NODE_BAUDRATE            (500000)
#define CO_NODE_TMR_N               (16u)
#define CO_NODE_TICKS_PER_S         (1000u)

#define CO_OD_SIZE                  (32u)

enum CO_EMCY_CODES {
    CO_ERR_ID_SOMETHING = 0,
    CO_ERR_ID_HOT,
    CO_ERR_ID_NUM,
};

extern CO_NODE co_node_nrf24l01;

extern CO_NODE_SPEC co_node_nrf24l01_spec;

extern CO_OBJ co_od_nrf24l01[CO_OD_SIZE];

extern CO_NODE* co_node_initialize(void);

#ifdef __cpluplus 
}
#endif

#endif /* INC_CO_NODE_NRF24L01_H_ */

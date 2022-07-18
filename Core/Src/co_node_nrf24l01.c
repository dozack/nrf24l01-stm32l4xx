/**
 ******************************************************************************
 * @file        co_node_nrf24l01.c
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

#include "co_node_nrf24l01.h"
#include "co_can_nrf24l01.h"
#include "co_nvm_dummy.h"
#include "co_timer_swcycle.h"

#define CO_TIMER_TASK_PRIO    (3u)
#define CO_CAN_TASK_PRIO      (3u)


static void co_timer_task_handler(void *context);

static void co_can_task_handler(void *context);


static CO_TMR_MEM CoTimerMemory[CO_NODE_TMR_N];

static uint8_t CoSdoSrvMemory[CO_SSDO_N * CO_SDO_BUF_BYTE];

static struct CO_IF_DRV_T CoDriver = {
    &co_can_nrf24l01,
    &SwCycleTimerDriver,
    &DummyNvmDriver,
};

static CO_EMCY_TBL CoEmcyTable[CO_ERR_ID_NUM] = {
    { CO_EMCY_REG_GENERAL, CO_EMCY_CODE_GEN_ERR          }, /* CO_ERR_ID_SOMETHING */
    { CO_EMCY_REG_TEMP   , CO_EMCY_CODE_TEMP_AMBIENT_ERR }  /* CO_ERR_ID_HOT   */
};

CO_NODE co_node_nrf24l01 = (CO_NODE){0};

CO_NODE_SPEC co_node_nrf24l01_spec = {
    CO_NODE_ID,
    CO_NODE_BAUDRATE,
    co_od_nrf24l01,
    CO_OD_SIZE,
    &CoEmcyTable[0],
    &CoTimerMemory[0],
    CO_NODE_TMR_N,
    CO_NODE_TICKS_PER_S,
    &CoDriver,
    &CoSdoSrvMemory[0],
};

static uint8_t      Obj1001_00_08 = 0;
static CO_HBCONS    Obj1016_01_xx = { .NodeId = 0x2, .Time = 3000 };

CO_OBJ co_od_nrf24l01[CO_OD_SIZE] = {

    {CO_KEY(0x1000, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0},
    {CO_KEY(0x1001, 0, CO_UNSIGNED8 |CO_OBJ____R_), 0,              (uintptr_t)&Obj1001_00_08},
#if 0
    {CO_KEY(0x1005, 0, CO_UNSIGNED32|CO_OBJ_D__RW), CO_TSYNCID,     (uintptr_t)0x40000080},
    {CO_KEY(0x1006, 0, CO_UNSIGNED32|CO_OBJ_D__RW), CO_TSYNCCYCLE,  (uintptr_t)20000},
#else
    {CO_KEY(0x1005, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0x80},
#endif

    {CO_KEY(0x1016, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)1},
    {CO_KEY(0x1016, 1, CO_UNSIGNED32|CO_OBJ____R_), CO_THB_CONS,    (uintptr_t)&Obj1016_01_xx},

    {CO_KEY(0x1017, 0, CO_UNSIGNED16|CO_OBJ_D__RW), CO_THB_PROD,    (uintptr_t)500},

    {CO_KEY(0x1018, 0, CO_UNSIGNED8 |CO_OBJ_D__R_), 0,              (uintptr_t)4},
    {CO_KEY(0x1018, 1, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0},
    {CO_KEY(0x1018, 2, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0},
    {CO_KEY(0x1018, 3, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0},
    {CO_KEY(0x1018, 4, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0},

    {CO_KEY(0x1200, 0, CO_UNSIGNED8 |CO_OBJ_D__R_), 0,              (uintptr_t)2},
    {CO_KEY(0x1200, 1, CO_UNSIGNED32|CO_OBJ_DN_R_), 0,              CO_COBID_SDO_REQUEST()},
    {CO_KEY(0x1200, 2, CO_UNSIGNED32|CO_OBJ_DN_R_), 0,              CO_COBID_SDO_RESPONSE()},

#if 0
    {CO_KEY(0x1280, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)2},
    {CO_KEY(0x1280, 1, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0x412},
    {CO_KEY(0x1280, 2, CO_UNSIGNED32|CO_OBJ_D__R_), 0,              (uintptr_t)0x512},
#endif

    CO_OBJ_DIR_ENDMARK
};

CO_NODE* co_node_initialize(void) {
    static StackType_t  tstack[configMINIMAL_STACK_SIZE];
    static StaticTask_t tcntrl;
    static StackType_t  cstack[configMINIMAL_STACK_SIZE];
    static StaticTask_t ccntrl;

    CONodeInit(&co_node_nrf24l01, &co_node_nrf24l01_spec);

    xTaskCreateStatic(&co_timer_task_handler,
                      "CO_TMR",
                      configMINIMAL_STACK_SIZE,
                      &co_node_nrf24l01,
                      CO_TIMER_TASK_PRIO,
                      &tstack[0],
                      &tcntrl);

    xTaskCreateStatic(&co_can_task_handler,
                      "CO_CAN",
                      configMINIMAL_STACK_SIZE,
                      &co_node_nrf24l01,
                      CO_CAN_TASK_PRIO,
                      &cstack[0],
                      &ccntrl);

    CONodeStart(&co_node_nrf24l01);

    return &co_node_nrf24l01;
}


static void co_timer_task_handler(void *context) {
    CO_NODE    *node = (CO_NODE*) context;
    int16_t     num;

    /* Wait for node to boot up properly */
    while (CONmtGetMode(&(node->Nmt)) == CO_INIT) {
        taskYIELD();
    }

    while (1) {
        /* Periodically service timer events */
        num = COTmrService(&(node->Tmr));
        while (num > 0) {
            COTmrProcess(&(node->Tmr));
            num--;
        }
        vTaskDelay((1 / portTICK_RATE_MS));
    }
}

static void co_can_task_handler(void *context) {
    CO_NODE    *node = (CO_NODE*) context;

    /* Wait for node to boot up properly */
    while (CONmtGetMode(&(node->Nmt)) == CO_INIT) {
        taskYIELD();
    }

    while (1) {
        /* Blocking is ensured by driver */
        CONodeProcess(node);
    }
}


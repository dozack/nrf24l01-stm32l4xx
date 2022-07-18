/**
 ******************************************************************************
 * @file        co_callbacks.c
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


#include "co_core.h"
#include "stm32l4xx.h"
#include "FreeRTOS.h"
#include "task.h"

/******************************************************************************
 * MANDATORY CALLBACK FUNCTIONS
 ******************************************************************************/

void CONodeFatalError(void) {
    volatile uint8_t debugExit = 0u;

    /* Place here your fatal error handling.
     * There is most likely a programming error.
     * !! Please don't ignore this errors. !!
     */
    for (; debugExit == 0u;);
}

void COTmrLock(void) {
    /* This function helps to guarantee the consistancy
     * of the internal timer management while interrupted
     * by the used timer interrupt. Most likely you need
     * at this point on of the following mechanisms:
     * - disable the used hardware timer interrupt
     * - get a 'timer-mutex' from your RTOS (ensure to
     *   call COTmrService() in a timer triggered task)
     */
    taskENTER_CRITICAL();
}

void COTmrUnlock(void) {
    /* This function helps to guarantee the consistancy
     * of the internal timer management while interrupted
     * by the used timer interrupt. Most likely you need
     * at this point on of the following mechanisms:
     * - (re)enable the used hardware timer interrupt
     * - release the 'timer-mutex' from your RTOS (ensure
     *   to call COTmrService() in a timer triggered task)
     */
    taskEXIT_CRITICAL();
}

/******************************************************************************
 * OPTIONAL CALLBACK FUNCTIONS
 ******************************************************************************/

void CONmtModeChange(CO_NMT *nmt, CO_MODE mode) {
    (void) nmt;
    (void) mode;

    /* Optional: place here some code, which is called
     * when a NMT mode change is initiated.
     */
}

void CONmtResetRequest(CO_NMT *nmt, CO_NMT_RESET reset) {
    (void) nmt;
    (void) reset;

    /* Optional: place here some code, which is called
     * when a NMT reset is requested by the network.
     */
    __NVIC_SystemReset();
}

void CONmtHbConsEvent(CO_NMT *nmt, uint8_t nodeId) {
    (void) nmt;
    (void) nodeId;

    /* Optional: place here some code, which is called
     * called when heartbeat consumer is in use and
     * detects an error on monitored node(s).
     */
}

void CONmtHbConsChange(CO_NMT *nmt, uint8_t nodeId, CO_MODE mode) {
    (void) nmt;
    (void) nodeId;
    (void) mode;

    /* Optional: place here some code, which is called
     * when heartbeat consumer is in use and detects a
     * NMT state change on monitored node(s).
     */
}

#if 0
void CoCSdoDownloadComplete(CO_CSDO *csdo, uint32_t abort) {
    (void) csdo;
    (void) abort;
}

void CoCSdoUploadComplete(CO_CSDO *csdo, uint32_t abort) {
    (void) csdo;
    (void) abort;
}
#endif

int16_t COLssLoad(uint32_t *baudrate, uint8_t *nodeId) {
    (void) baudrate;
    (void) nodeId;

    /* Optional: place here some code, which is called
     * when LSS client is in use and the CANopen node
     * is initialized.
     */
    return (0u);
}

int16_t COLssStore(uint32_t baudrate, uint8_t nodeId) {
    (void) baudrate;
    (void) nodeId;

    /* Optional: place here some code, which is called
     * when LSS client is in use and the CANopen node
     * needs to store updated values.
     */
    return (0u);
}

void COIfCanReceive(CO_IF_FRM *frm) {
    (void) frm;

    /* Optional: place here some code, which is called
     * when you need to handle CAN messages, which are
     * not part of the CANopen protocol.
     */
}

void COPdoTransmit(CO_IF_FRM *frm) {
    (void) frm;

    /* Optional: place here some code, which is called
     * just before a PDO is transmitted. You may adjust
     * the given CAN frame which is send afterwards.
     */
}

int16_t COPdoReceive(CO_IF_FRM *frm) {
    (void) frm;

    /* Optional: place here some code, which is called
     * right after receiving a PDO. You may adjust
     * the given CAN frame which is written into the
     * object dictionary afterwards or suppress the
     * write operation.
     */
    return (0u);
}

void COPdoSyncUpdate(CO_RPDO *pdo) {
    (void) pdo;

    /* Optional: place here some code, which is called
     * right after the object dictionary update due to
     * a synchronized PDO.
     */
}

#if (USE_PARAMS == 1)
int16_t COParaDefault(CO_PARA *pg) {
    (void) pg;

    /* Optional: place here some code, which is called
     * when a parameter group is restored to factory
     * settings.
     */
    return (0u);
}
#endif


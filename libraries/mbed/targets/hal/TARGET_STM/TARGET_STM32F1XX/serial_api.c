/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// math.h required for floating point operations for baud rate calculation
#include <string.h>

#include "serial_api.h"
#include "cmsis.h"
#include "pinmap.h"
#include "error.h"

/******************************************************************************
 * INITIALIZATION
 ******************************************************************************/
#define UART_NUM    3

static const PinMap PinMap_UART_TX[] = {
    {PA_9 , UART_1, STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {PA_2 , UART_2, STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {NC   , NC    , 0x00}
};

static const PinMap PinMap_UART_RX[] = {
    {PA_10, UART_1, STM_PIN_DATA(STM_PIN_MODE_INPUT, STM_PIN_CONF_INPUT_FLOAT)},
    {PA_3,  UART_2, STM_PIN_DATA(STM_PIN_MODE_INPUT, STM_PIN_CONF_INPUT_FLOAT)},
    {NC   , NC    , 0x00}
};

static uint32_t serial_irq_ids[UART_NUM] = {0};
static uart_irq_handler irq_handler;

int stdio_uart_inited = 0;
serial_t stdio_uart;

void serial_init(serial_t *obj, PinName tx, PinName rx) {
    int is_stdio_uart = 0;

    // determine the UART to use
    UARTName uart_tx = (UARTName)pinmap_peripheral(tx, PinMap_UART_TX);
    UARTName uart_rx = (UARTName)pinmap_peripheral(rx, PinMap_UART_RX);
    UARTName uart = (UARTName)pinmap_merge(uart_tx, uart_rx);
    if ((int)uart == NC) {
        error("Serial pinout mapping failed");
    }

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN ;
    switch (uart) {
    case UART_1: RCC->APB2ENR |= RCC_APB2ENR_USART1EN; break;
    case UART_2: RCC->APB1ENR |= RCC_APB1ENR_USART2EN; break;
    }
    obj->uart = (USART_TypeDef *)uart;

    // set default baud rate and format
    serial_baud  (obj, 9600);
    serial_format(obj, 8, ParityNone, 1);

    // pinout the chosen uart
    pinmap_pinout(tx, PinMap_UART_TX);
    pinmap_pinout(rx, PinMap_UART_RX);

    // set rx/tx pins in PullUp mode
    pin_mode(tx, PullUp);
    pin_mode(rx, PullUp);

    obj->uart->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    switch (uart) {
        case UART_1: obj->index = 0; break;
    }

    is_stdio_uart = (uart == STDIO_UART) ? (1) : (0);

    if (is_stdio_uart) {
        stdio_uart_inited = 1;
        memcpy(&stdio_uart, obj, sizeof(serial_t));
    }
}

void serial_free(serial_t *obj) {
    serial_irq_ids[obj->index] = 0;
}

// serial_baud
// set the baud rate, taking in to account the current SystemFrequency
void serial_baud(serial_t *obj, int baudrate) {
    obj->uart->BRR = SystemCoreClock / baudrate / (obj->uart == UART_1 ? 1 : 2);
}

void serial_format(serial_t *obj, int data_bits, SerialParity parity, int stop_bits) {
    // 0: 1 stop bits, 1: 2 stop bits
    if (stop_bits != 1 && stop_bits != 2) {
        error("Invalid stop bits specified");
    }
    obj->uart->CR2 &= ~USART_CR2_STOP;
    if (stop_bits == 1) obj->uart->CR2 |= USART_CR2_STOP_1;

    if (data_bits < 8 || data_bits > 9) {
        error("Invalid number of bits (%d) in serial format, should be 8..9", data_bits);
    }

    obj->uart->CR1 &= ~(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS);
    if (data_bits == 9) obj->uart->CR1 |= USART_CR1_M;

    int parity_enable, parity_select;
    switch (parity) {
        case ParityNone: break;
        case ParityOdd : obj->uart->CR1 |= USART_CR1_PCE | USART_CR1_PS; break;
        case ParityEven: obj->uart->CR1 |= USART_CR1_PCE ; break;
        default:
            error("Invalid serial parity setting");
            return;
    }
}

/******************************************************************************
 * INTERRUPTS HANDLING
 ******************************************************************************/
#if 0
static inline void uart_irq(uint32_t iir, uint32_t index) {
    // [Chapter 14] LPC17xx UART0/2/3: UARTn Interrupt Handling
    SerialIrq irq_type;
    switch (iir) {
        case 1: irq_type = TxIrq; break;
        case 2: irq_type = RxIrq; break;
        default: return;
    }

    if (serial_irq_ids[index] != 0)
        irq_handler(serial_irq_ids[index], irq_type);
}

void uart0_irq() {uart_irq((LPC_UART->IIR >> 1) & 0x7, 0);}

#endif
void serial_irq_handler(serial_t *obj, uart_irq_handler handler, uint32_t id) {
    irq_handler = handler;
    serial_irq_ids[obj->index] = id;
}

void serial_irq_set(serial_t *obj, SerialIrq irq, uint32_t enable) {
#if 0
    IRQn_Type irq_n = (IRQn_Type)0;
    uint32_t vector = 0;
    switch ((int)obj->uart) {
        case UART_0:
            irq_n=UART_IRQn;
            vector = (uint32_t)&uart0_irq;
            break;
        default:
            return;
    }

    if (enable) {
        obj->uart->IER |= 1 << irq;
        NVIC_SetVector(irq_n, vector);
        NVIC_EnableIRQ(irq_n);
    } else { // disable
        int all_disabled = 0;
        SerialIrq other_irq = (irq == RxIrq) ? (TxIrq) : (RxIrq);

        obj->uart->IER &= ~(1 << irq);
        all_disabled = (obj->uart->IER & (1 << other_irq)) == 0;

        if (all_disabled)
            NVIC_DisableIRQ(irq_n);
    }
#endif
}

/******************************************************************************
 * READ/WRITE
 ******************************************************************************/
int serial_getc(serial_t *obj) {
    while (!serial_readable(obj));
    return obj->uart->DR;
}

void serial_putc(serial_t *obj, int c) {
    while (!serial_writable(obj));
    obj->uart->DR = c;
}

int serial_readable(serial_t *obj) {
    return obj->uart->SR & USART_SR_RXNE;
}

int serial_writable(serial_t *obj) {
    return obj->uart->SR & USART_SR_TXE;
}

void serial_clear(serial_t *obj) {
}

void serial_pinout_tx(PinName tx) {
    pinmap_pinout(tx, PinMap_UART_TX);
}

void serial_break_clear(serial_t *obj) {
    obj->uart->CR1 &= USART_CR1_SBK;
}

void serial_break_set(serial_t *obj) {
    obj->uart->CR1 |= USART_CR1_SBK;
}

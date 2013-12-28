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
#ifndef MBED_PERIPHERALNAMES_H
#define MBED_PERIPHERALNAMES_H

#include "cmsis.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UART_1 = (int)USART1_BASE,
} UARTName;

typedef enum {
    ADC0_0 = 0,
    ADC0_1,
    ADC0_2,
    ADC0_3,
    ADC0_4,
    ADC0_5,
    ADC0_6,
    ADC0_7,
    ADC0_8,
    ADC0_9,
    ADC0_10,
    ADC0_11,
    ADC0_12,
    ADC0_13,
    ADC0_14,
    ADC0_15
} ADCName;

typedef enum {
    SPI_1 = (int)SPI1_BASE,
    SPI_2 = (int)SPI2_BASE,
} SPIName;

typedef enum {
    I2C_1 = (int)I2C1_BASE,
    I2C_2 = (int)I2C2_BASE,
} I2CName;

typedef enum {
    TIM_1 =    (int)TIM1_BASE,
    TIM_1_1 =  (int)TIM1_BASE + 1,
    TIM_1_2 =  (int)TIM1_BASE + 2,
    TIM_1_3 =  (int)TIM1_BASE + 4,
    TIM_1_4 =  (int)TIM1_BASE + 4,
    TIM_3 =    (int)TIM3_BASE,
    TIM_3_1 =  (int)TIM3_BASE + 1,
    TIM_3_2 =  (int)TIM3_BASE + 2,
    TIM_3_3 =  (int)TIM3_BASE + 3,
    TIM_3_4 =  (int)TIM3_BASE + 4,
    TIM_14 =   (int)TIM14_BASE,
    TIM_14_1 = (int)TIM14_BASE + 1,
    TIM_14_2 = (int)TIM14_BASE + 2,
    TIM_14_3 = (int)TIM14_BASE + 3,
    TIM_14_4 = (int)TIM14_BASE + 4,
    TIM_16 =   (int)TIM16_BASE,
    TIM_16_1 = (int)TIM16_BASE + 1,
    TIM_16_2 = (int)TIM16_BASE + 2,
    TIM_16_3 = (int)TIM16_BASE + 3,
    TIM_16_4 = (int)TIM16_BASE + 4,
    TIM_17 =   (int)TIM17_BASE,
    TIM_17_1 = (int)TIM17_BASE + 1,
    TIM_17_2 = (int)TIM17_BASE + 2,
    TIM_17_3 = (int)TIM17_BASE + 3,
    TIM_17_4 = (int)TIM17_BASE + 4,
} TimerName;

#define STDIO_UART_TX     PA_9
#define STDIO_UART_RX     PA_10
#define STDIO_UART        UART_1

#ifdef __cplusplus
}
#endif

#endif

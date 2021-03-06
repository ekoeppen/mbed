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
#ifndef MBED_OBJECTS_H
#define MBED_OBJECTS_H

#include "cmsis.h"
#include "PortNames.h"
#include "PeripheralNames.h"
#include "PinNames.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gpio_irq_s {
    uint32_t port;
    uint32_t pin;
    uint32_t ch;
};

struct port_s {
    __IO uint32_t *reg_mode;
    __IO uint32_t *reg_in;
    __IO uint32_t *reg_out;
    __IO uint16_t *reg_set;
    __IO uint16_t *reg_clr;
    PortName port;
    uint32_t mask;
    PinDirection direction;
};

struct pwmout_s {
    TIM_TypeDef *timer;
    int channel;
};

struct serial_s {
    USART_TypeDef *uart;
    int index;
};

struct analogin_s {
    ADCName adc;
};

struct i2c_s {
    I2C_TypeDef *i2c;
};

struct spi_s {
    SPI_TypeDef *spi;
    int data_size;
};

#include "gpio_object.h"

#ifdef __cplusplus
}
#endif

#endif

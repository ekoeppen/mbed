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
#include "pinmap.h"
#include "error.h"

/**
 * Set the pin into input, output, alternate function or analog mode
 */
void pin_function(PinName pin, int data) {
    if (pin == (int32_t)NC) return;

    int mode = STM_PIN_MODE(data);
    int conf = STM_PIN_CONF(data);

    uint32_t pin_number = (uint32_t)pin;
    int port_index = pin_number >> 4;
    int pin_index = (pin_number & 0xF);
    GPIO_TypeDef * gpio = ((GPIO_TypeDef *) (GPIOA_BASE + (port_index << 10)));
    __IO uint32_t *CR;

    // MODE, SPEED and ALTFUNC
    int offset = pin_index * 4;
    if (pin_index < 8) {
        CR = &gpio->CRL;
    } else {
        offset -= 32;
        CR = &gpio->CRH;
    }

    *CR &= ~(0xf << offset);
    *CR |= (mode << offset) | (conf << (offset + 2));
}

void pin_mode(PinName pin, PinMode mode) {
    if (pin == (int32_t)NC) { return; }

    uint32_t pin_number = (uint32_t)pin;
    int port_index = pin_number >> 4;
    int pin_index = (pin_number & 0xF);
    int offset = pin_index * 4;
    __IO uint32_t *CR;
    uint8_t conf = 0, orig_conf;

    GPIO_TypeDef * gpio = ((GPIO_TypeDef *) (GPIOA_BASE + (port_index << 10)));

    if (pin_index < 8) {
        CR = &gpio->CRL;
    } else {
        offset -= 32;
        CR = &gpio->CRH;
    }

    orig_conf = (*CR >> (offset + 2)) & 0x03;
    if ((*CR & (0x03 << offset)) == STM_PIN_MODE_INPUT) {
        switch (mode) {
            case PullNone:
                conf = STM_PIN_CONF_INPUT_FLOAT;
                break;
            case PullUp:
                conf = STM_PIN_CONF_INPUT_PUPD;
                gpio->ODR |= 1 << pin_index;
                break;
            case PullDown:
                conf = STM_PIN_CONF_INPUT_PUPD;
                gpio->ODR &= ~(1 << pin_index);
                break;
            default:
                conf = orig_conf;
        }
    } else {
        int is_alt = *CR & (0x02 << (offset + 2));
        switch (mode) {
            case OpenDrain:
                conf = is_alt ? STM_PIN_CONF_OUTPUT_ALT_OPENDRAIN : STM_PIN_CONF_OUTPUT_OPENDRAIN;
                break;
            case PullUp:
            case PullDown:
                conf = is_alt ? STM_PIN_CONF_OUTPUT_ALT_PUSHPULL : STM_PIN_CONF_OUTPUT_PUSHPULL;
                break;
            default:
                conf = orig_conf;
                break;
        }
    }
    *CR &= ~(0x03 << (offset + 2));
    *CR |= conf << (offset + 2);
}


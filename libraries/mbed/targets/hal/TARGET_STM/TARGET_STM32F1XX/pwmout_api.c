/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
{
}

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
#include "pwmout_api.h"
#include "pinmap.h"

static const PinMap PinMap_PWM_Out[] = {
    {PB_6, PWM_1,   STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {PB_7, PWM_2,   STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {PB_8, PWM_3,   STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {PB_9, PWM_4,   STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {NC,   NC,      0}
};

void pwmout_init         (pwmout_t* obj, PinName pin)
{
    PWMName pwm = (PWMName)pinmap_peripheral(pin, PinMap_PWM_Out);
    if ((int)pwm == NC) {
        error("PWM pinout mapping failed");
    }
    obj->pwm = pwm;
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    if (pin == PB_6 || pin == PB_7) {
        AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;
    }
    pinmap_pinout(pin, PinMap_PWM_Out);
    switch (pwm) {
    case PWM_1: TIM4->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE; break;
    case PWM_2: TIM4->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE; break;
    case PWM_3: TIM4->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE; break;
    case PWM_4: TIM4->CCMR2 = TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE; break;
    }
    TIM4->CCER |= 1 << ((pwm - 1) * 4);
    TIM4->ARR = 240;
    TIM4->PSC = 2;
    TIM4->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
}

void pwmout_free         (pwmout_t* obj)
{
}


void  pwmout_write       (pwmout_t* obj, float percent)
{
    __IO uint32_t *CCR = &((uint32_t *) &TIM4->CCR1)[obj->pwm - 1];
    *CCR = (uint32_t) ((float) (TIM4->ARR) * percent);
}

float pwmout_read        (pwmout_t* obj)
{
    return 0;
}


void pwmout_period       (pwmout_t* obj, float seconds)
{
    TIM4->ARR = (uint32_t) (24000000.0 * seconds);
}

void pwmout_period_ms    (pwmout_t* obj, int ms)
{
    TIM4->ARR = 24000 * ms;
}

void pwmout_period_us    (pwmout_t* obj, int us)
{
    TIM4->ARR = 24 * us;
}


void pwmout_pulsewidth   (pwmout_t* obj, float seconds)
{
    __IO uint32_t *CCR = &((uint32_t *) &TIM4->CCR1)[obj->pwm - 1];
    *CCR = (uint32_t) (24000000.0 * seconds);
}

void pwmout_pulsewidth_ms(pwmout_t* obj, int ms)
{
    __IO uint32_t *CCR = &((uint32_t *) &TIM4->CCR1)[obj->pwm - 1];
    *CCR = 24000 * ms;
}

void pwmout_pulsewidth_us(pwmout_t* obj, int us)
{
    __IO uint32_t *CCR = &((uint32_t *) &TIM4->CCR1)[obj->pwm - 1];
    *CCR = 24 * us;
}


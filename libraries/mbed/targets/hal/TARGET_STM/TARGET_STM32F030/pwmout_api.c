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
#include "error.h"

static const PinMap PinMap_PWM_Out[] = {
    {PA_4,  TIM_14_1, STM_PIN_DATA(2, 4)},
    {PA_6,  TIM_16_1, STM_PIN_DATA(2, 5)},
    {PA_7,  TIM_17_1, STM_PIN_DATA(2, 5)},
    // {PA_6,  TIM_3_1,  STM_PIN_DATA(2, 1)},
    // {PA_7,  TIM_3_2,  STM_PIN_DATA(2, 1)},
    {PA_8,  TIM_1_1,  STM_PIN_DATA(2, 2)},
    {PA_9,  TIM_1_2,  STM_PIN_DATA(2, 2)},
    {PA_10, TIM_1_3,  STM_PIN_DATA(2, 2)},
    {PA_11, TIM_1_4,  STM_PIN_DATA(2, 2)},
    {PB_1,  TIM_3_4,  STM_PIN_DATA(2, 1)},
    {NC,    NC,     0}
};

void pwmout_init(pwmout_t* obj, PinName pin)
{
    __IO uint32_t *CCR;
    TimerName timer = (TimerName)pinmap_peripheral(pin, PinMap_PWM_Out);
    if ((int)timer == NC) {
        error("PWM pinout mapping failed");
    }
    obj->timer = (TIM_TypeDef *)(timer & ~0xf);
    obj->channel = timer & 0xf;
    switch ((int)obj->timer) {
        case TIM_1: RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; break;
        case TIM_3: RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; break;
        case TIM_14: RCC->APB1ENR |= RCC_APB1ENR_TIM14EN; break;
        case TIM_16: RCC->APB2ENR |= RCC_APB2ENR_TIM16EN; break;
        case TIM_17: RCC->APB2ENR |= RCC_APB2ENR_TIM17EN; break;
    }
    RCC->AHBENR |= 1 << (((uint32_t) pin >> 4) + 17);
    pinmap_pinout(pin, PinMap_PWM_Out);

    CCR = &((uint32_t *) &obj->timer->CCR1)[obj->channel - 1];
    obj->timer->PSC = SystemCoreClock / 1000000;
    obj->timer->ARR = 20000;
    obj->timer->CNT = 0;
    switch (obj->channel) {
        case 1:
            obj->timer->CCMR1 |= TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE;
            break;
        case 2:
            obj->timer->CCMR1 |= TIM_CCMR1_OC2M_0 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE;
            break;
        case 3:
            obj->timer->CCMR2 |= TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;
            break;
        case 4:
            obj->timer->CCMR2 |= TIM_CCMR2_OC4M_0 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE;
            break;
    }
    *CCR = 0;
    obj->timer->CCER |= (TIM_CCER_CC1E << ((obj->channel - 1) * 4));
    obj->timer->BDTR = TIM_BDTR_MOE;
    obj->timer->CR1 = TIM_CR1_CEN;
    obj->timer->EGR = TIM_EGR_UG | (TIM_EGR_CC1G << (obj->channel - 1));
}

void pwmout_free(pwmout_t* obj)
{
}


void pwmout_write(pwmout_t* obj, float percent)
{
    __IO uint32_t *CCR = &((uint32_t *) &obj->timer->CCR1)[obj->channel - 1];
    *CCR = (uint32_t) ((float) (obj->timer->ARR) * percent);
}

float pwmout_read(pwmout_t* obj)
{
    return 0;
}


void pwmout_period(pwmout_t* obj, float seconds)
{
    obj->timer->ARR = (uint32_t) (24000000.0 * seconds);
}

void pwmout_period_ms(pwmout_t* obj, int ms)
{
    obj->timer->ARR = 24000 * ms;
}

void pwmout_period_us(pwmout_t* obj, int us)
{
    obj->timer->ARR = 24 * us;
}


void pwmout_pulsewidth(pwmout_t* obj, float seconds)
{
    __IO uint32_t *CCR = &((uint32_t *) &obj->timer->CCR1)[obj->channel - 1];
    *CCR = (uint32_t) (24000000.0 * seconds);
}

void pwmout_pulsewidth_ms(pwmout_t* obj, int ms)
{
    __IO uint32_t *CCR = &((uint32_t *) &obj->timer->CCR1)[obj->channel - 1];
    *CCR = 24000 * ms;
}

void pwmout_pulsewidth_us(pwmout_t* obj, int us)
{
    __IO uint32_t *CCR = &((uint32_t *) &obj->timer->CCR1)[obj->channel - 1];
    *CCR = 24 * us;
}

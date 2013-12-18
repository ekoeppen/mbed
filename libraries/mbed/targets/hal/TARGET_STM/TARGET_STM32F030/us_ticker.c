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
#include <stddef.h>
#include "us_ticker_api.h"
#include "PeripheralNames.h"

int us_ticker_inited = 0;

/* Set up TIM14 as prescaler for TIM3, and hook up µs IRQ to TIM3 */

void us_ticker_init(void) {
    if (us_ticker_inited) return;
    us_ticker_inited = 1;

    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN | RCC_APB1ENR_TIM3EN;

    uint32_t prescale = SystemCoreClock / 1000000;
    TIM14->CNT = 0;
    TIM14->PSC = prescale - 1;
    TIM14->CCMR1 = TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
    TIM14->CCER = TIM_CCER_CC1E;
    TIM14->CCR1 = 0xffff;
    TIM14->EGR = TIM_EGR_CC1G | TIM_EGR_UG;

    TIM3->CNT = 0;
    TIM3->SMCR = TIM_SMCR_TS_0 | TIM_SMCR_TS_1;
    TIM3->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2;

    TIM14->CR1 |= TIM_CR1_CEN;
    TIM3->CR1 |= TIM_CR1_CEN;

    NVIC_SetVector(TIM3_IRQn, (uint32_t)us_ticker_irq_handler);
    NVIC_EnableIRQ(TIM3_IRQn);
}

uint32_t us_ticker_read() {
    if (!us_ticker_inited)
        us_ticker_init();

    uint32_t lsb_0 = TIM14->CNT;
    uint32_t lsb_1 = TIM14->CNT;
    return lsb_0 + (TIM3->CNT << 16) + (lsb_1 < lsb_0 ? 65536 : 0);
}

void us_ticker_set_interrupt(unsigned int timestamp) {
#if 0
    // set match value
    US_TICKER_TIMER->CCR1 = timestamp;
    // enable compare interrupt
    US_TICKER_TIMER->DIER |= TIM_DIER_CC1IE;
#endif
}

void us_ticker_disable_interrupt(void) {
    TIM14->DIER &= ~TIM_DIER_CC1IE;
}

void us_ticker_clear_interrupt(void) {
    TIM14->SR &= ~TIM_SR_CC1IF;
}

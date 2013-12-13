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
#include "spi_api.h"

#if DEVICE_SPI
#include <math.h>

#include "cmsis.h"
#include "pinmap.h"
#include "error.h"

static const PinMap PinMap_SPI_SCLK[] = {
    {PA_5,  SPI_1, STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {PB_13, SPI_2, STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {NC,    NC,    0}
};

static const PinMap PinMap_SPI_MOSI[] = {
    {PA_7,  SPI_1, STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {PB_15, SPI_2, STM_PIN_DATA(STM_PIN_MODE_OUTPUT_50MHZ, STM_PIN_CONF_OUTPUT_ALT_PUSHPULL)},
    {NC,    NC,    0}
};

static const PinMap PinMap_SPI_MISO[] = {
    {PA_6,  SPI_1, STM_PIN_DATA(STM_PIN_MODE_INPUT, STM_PIN_CONF_INPUT_FLOAT)},
    {PB_14, SPI_2, STM_PIN_DATA(STM_PIN_MODE_INPUT, STM_PIN_CONF_INPUT_FLOAT)},
    {NC,    NC,    0}
};

static inline int ssp_disable(spi_t *obj);
static inline int ssp_enable(spi_t *obj);

/* TODO: Implement slave functionality */
void spi_init(spi_t *obj, PinName mosi, PinName miso, PinName sclk, PinName ssel) {
    // determine the SPI to use
    SPIName spi_mosi = (SPIName)pinmap_peripheral(mosi, PinMap_SPI_MOSI);
    SPIName spi_miso = (SPIName)pinmap_peripheral(miso, PinMap_SPI_MISO);
    SPIName spi_sclk = (SPIName)pinmap_peripheral(sclk, PinMap_SPI_SCLK);
    SPIName spi_data = (SPIName)pinmap_merge(spi_mosi, spi_miso);
    obj->spi = (SPI_TypeDef*)pinmap_merge(spi_data, spi_sclk);
    if ((int)obj->spi == NC) {
        while(1);
        error("SPI pinout mapping failed");
    }

    // enable power and clocking
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    switch ((int)obj->spi) {
        case SPI_1:
            RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_SPI1EN;
            break;
        case SPI_2:
            RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
            RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
            break;
    }


    // set default format and frequency
    spi_format(obj, 8, 0, 0);  // 8 bits, mode 0, master
    spi_frequency(obj, 1000000);

    // enable the ssp channel
    ssp_enable(obj);

    // pin out the spi pins
    pinmap_pinout(mosi, PinMap_SPI_MOSI);
    pinmap_pinout(miso, PinMap_SPI_MISO);
    pinmap_pinout(sclk, PinMap_SPI_SCLK);
    obj->spi->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
}

void spi_free(spi_t *obj) {}

void spi_format(spi_t *obj, int bits, int mode, int slave) {
    ssp_disable(obj);

    if (!(bits == 8 || bits == 16) || !(mode >= 0 && mode <= 3)) {
        error("SPI format error");
    }

    int polarity = (mode & 0x2) ? 1 : 0;
    int phase = (mode & 0x1) ? 1 : 0;

    obj->spi->CR1 &= ~0x807;
    obj->spi->CR1 |= ((phase) ? 1 : 0) << 0 |
                     ((polarity) ? 1 : 0) << 1 |
                     ((slave) ? 0: 1) << 2 |
                     ((bits == 16) ? 1 : 0) << 11;

    if (obj->spi->SR & SPI_SR_MODF) {
        obj->spi->CR1 = obj->spi->CR1;
    }

    ssp_enable(obj);
}

void spi_frequency(spi_t *obj, int hz) {
    uint32_t baud_rate;

    ssp_disable(obj);

    uint32_t PCLK = SystemCoreClock;
    if ((int)obj->spi == SPI_2) PCLK = PCLK >> 1;

    baud_rate == 0;
    while (PCLK / (1 << (baud_rate + 2)) > hz && baud_rate < 7) {
        baud_rate++;
    }

    obj->spi->CR1 &= ~(0x7 << 3);
    obj->spi->CR1 |= baud_rate << 3;

    ssp_enable(obj);
}

static inline int ssp_disable(spi_t *obj) {
    // TODO: Follow the instructions in 25.3.8 for safely disabling the SPI
    return obj->spi->CR1 &= ~SPI_CR1_SPE;
}

static inline int ssp_enable(spi_t *obj) {
    return obj->spi->CR1 |= SPI_CR1_SPE;
}

static inline int ssp_readable(spi_t *obj) {
    return obj->spi->SR & SPI_SR_RXNE;
}

static inline int ssp_writeable(spi_t *obj) {
    return obj->spi->SR & SPI_SR_TXE;
}

static inline void ssp_write(spi_t *obj, int value) {
    while (!ssp_writeable(obj));
    obj->spi->DR = value;
}

static inline int ssp_read(spi_t *obj) {
    while (!ssp_readable(obj));
    return obj->spi->DR;
}

static inline int ssp_busy(spi_t *obj) {
    return (obj->spi->SR & SPI_SR_BSY) ? (1) : (0);
}

int spi_master_write(spi_t *obj, int value) {
    ssp_write(obj, value);
    return ssp_read(obj);
}

int spi_slave_receive(spi_t *obj) {
    return (ssp_readable(obj) && !ssp_busy(obj)) ? (1) : (0);
};

int spi_slave_read(spi_t *obj) {
    return obj->spi->DR;
}

void spi_slave_write(spi_t *obj, int value) {
    while (ssp_writeable(obj) == 0) ;
    obj->spi->DR = value;
}

int spi_busy(spi_t *obj) {
    return ssp_busy(obj);
}

#endif

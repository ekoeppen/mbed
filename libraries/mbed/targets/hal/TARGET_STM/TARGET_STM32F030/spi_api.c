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
    {PA_5,  SPI_1, STM_PIN_DATA(2, 0)},
    {PB_3,  SPI_1, STM_PIN_DATA(2, 0)},
    {PB_13, SPI_2, STM_PIN_DATA(2, 0)},
    {PB_13, SPI_1, STM_PIN_DATA(2, 0)},
    {NC,    NC,    0}
};

static const PinMap PinMap_SPI_MOSI[] = {
    {PA_7,  SPI_1, STM_PIN_DATA(2, 0)},
    {PB_5,  SPI_1, STM_PIN_DATA(2, 0)},
    {PB_15, SPI_2, STM_PIN_DATA(2, 0)},
    {PB_15, SPI_1, STM_PIN_DATA(2, 0)},
    {NC,    NC,    0}
};

static const PinMap PinMap_SPI_MISO[] = {
    {PA_6,  SPI_1, STM_PIN_DATA(2, 0)},
    {PB_4,  SPI_1, STM_PIN_DATA(2, 0)},
    {PB_14, SPI_2, STM_PIN_DATA(2, 0)},
    {PB_14, SPI_1, STM_PIN_DATA(2, 0)},
    {NC,    NC,    0}
};

static const PinMap PinMap_SPI_SSEL[] = {
    {PA_4,  SPI_1, STM_PIN_DATA(2, 0)},
    {PA_15, SPI_1, STM_PIN_DATA(2, 0)},
    {PB_12, SPI_2, STM_PIN_DATA(2, 0)},
    {PB_12, SPI_1, STM_PIN_DATA(2, 0)},
    {NC,    NC,    0}
};


static inline int spi_disable(spi_t *obj);
static inline int spi_enable(spi_t *obj);

void spi_init(spi_t *obj, PinName mosi, PinName miso, PinName sclk, PinName ssel) {
    // determine the SPI to use
    SPIName spi_mosi = (SPIName)pinmap_peripheral(mosi, PinMap_SPI_MOSI);
    SPIName spi_miso = (SPIName)pinmap_peripheral(miso, PinMap_SPI_MISO);
    SPIName spi_sclk = (SPIName)pinmap_peripheral(sclk, PinMap_SPI_SCLK);
    SPIName spi_ssel = (SPIName)pinmap_peripheral(ssel, PinMap_SPI_SSEL);
    SPIName spi_data = (SPIName)pinmap_merge(spi_mosi, spi_miso);
    SPIName spi_cntl = (SPIName)pinmap_merge(spi_sclk, spi_ssel);
    obj->spi = (SPI_TypeDef*)pinmap_merge(spi_data, spi_cntl);
    if ((int)obj->spi == NC) {
        error("SPI pinout mapping failed");
    }

    // enable power and clocking
    switch ((int)obj->spi) {
        case SPI_1:
            RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
            RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
            break;
        case SPI_2:
            RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
            break;
    }
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;


    // set default format and frequency
    if (ssel == NC) {
        spi_format(obj, 8, 0, 0);  // 8 bits, mode 0, master
    } else {
        spi_format(obj, 8, 0, 1);  // 8 bits, mode 0, slave
    }
    spi_frequency(obj, 1000000);

    // enable the spi channel
    spi_enable(obj);

    // pin out the spi pins
    pinmap_pinout(mosi, PinMap_SPI_MOSI);
    pinmap_pinout(miso, PinMap_SPI_MISO);
    pinmap_pinout(sclk, PinMap_SPI_SCLK);
    if (ssel != NC) {
        pinmap_pinout(ssel, PinMap_SPI_SSEL);
    }
    else {
        // Use software slave management
        obj->spi->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
    }
}

void spi_free(spi_t *obj) {}

void spi_format(spi_t *obj, int bits, int mode, int slave) {
    spi_disable(obj);

    if (!(bits >= 4 || bits <= 16) || !(mode >= 0 && mode <= 3)) {
        error("SPI format error");
    }
    obj->data_size = bits;
    if (bits <= 8) obj->spi->CR2 |= SPI_CR2_FRXTH;

    int polarity = (mode & 0x2) ? 1 : 0;
    int phase = (mode & 0x1) ? 1 : 0;

    obj->spi->CR1 &= ~(SPI_CR1_MSTR | SPI_CR1_CPHA | SPI_CR1_CPOL);
    obj->spi->CR1 |= ((phase) ? SPI_CR1_CPHA : 0) |
                     ((polarity) ? SPI_CR1_CPOL : 0) |
                     ((slave) ? 0 : SPI_CR1_MSTR);

    obj->spi->CR2 &= ~(SPI_CR2_DS);
    obj->spi->CR2 |= ((bits - 1) << 8);

    if (obj->spi->SR & SPI_SR_MODF) {
        obj->spi->CR1 = obj->spi->CR1;
    }

    spi_enable(obj);
}

void spi_frequency(spi_t *obj, int hz) {
    spi_disable(obj);

    // SPI1 runs from PCLK2, which runs at SystemCoreClock / 2.  SPI2 and SPI3
    // run from PCLK1, which runs at SystemCoreClock / 4.
    uint32_t PCLK = SystemCoreClock;
    switch ((int)obj->spi) {
        case SPI_1: PCLK = PCLK >> 1; break;
        case SPI_2: PCLK = PCLK >> 2; break;
    }

    // Choose the baud rate divisor (between 2 and 256)
    uint32_t divisor = PCLK / hz;

    // Find the nearest power-of-2
    divisor = divisor > 0 ? divisor-1 : 0;
    divisor |= divisor >> 1;
    divisor |= divisor >> 2;
    divisor |= divisor >> 4;
    divisor |= divisor >> 8;
    divisor |= divisor >> 16;
    divisor++;

    uint32_t baud_rate = __builtin_ffs(divisor) - 1;
    baud_rate = baud_rate > 0x7 ? 0x7 : baud_rate;

    obj->spi->CR1 &= ~(0x7 << 3);
    obj->spi->CR1 |= baud_rate << 3;

    spi_enable(obj);
}

static inline int spi_disable(spi_t *obj) {
    // TODO: Follow the instructions in 25.3.8 for safely disabling the SPI
    return obj->spi->CR1 &= ~SPI_CR1_SPE;
}

static inline int spi_enable(spi_t *obj) {
    return obj->spi->CR1 |= SPI_CR1_SPE;
}

static inline int spi_readable(spi_t *obj) {
    return obj->spi->SR & SPI_SR_RXNE;
}

static inline int spi_writeable(spi_t *obj) {
    return obj->spi->SR & SPI_SR_TXE;
}

static inline void spi_write(spi_t *obj, int value) {
    while (!spi_writeable(obj));
    if (obj->data_size > 8) {
        obj->spi->DR = (uint16_t) value;
    } else {
        *(uint8_t *) &obj->spi->DR = (uint8_t) value;
    }
}

static inline int spi_read(spi_t *obj) {
    while (!spi_readable(obj));
    return obj->spi->DR;
}

int spi_master_write(spi_t *obj, int value) {
    spi_write(obj, value);
    return spi_read(obj);
}

int spi_slave_receive(spi_t *obj) {
    return (spi_readable(obj) && !spi_busy(obj)) ? (1) : (0);
};

int spi_slave_read(spi_t *obj) {
    return obj->spi->DR;
}

void spi_slave_write(spi_t *obj, int value) {
    while (spi_writeable(obj) == 0) ;
    obj->spi->DR = value;
}

int spi_busy(spi_t *obj) {
    return (obj->spi->SR & SPI_SR_BSY) ? (1) : (0);
}

#endif

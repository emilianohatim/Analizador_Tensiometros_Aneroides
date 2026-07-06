/*********************************************************************************************************************
Copyright 2026, Proyecto de Graduación
Facultad de Ciencias Exactas y Tecnologia
Universidad Nacional de Tucuman - UNT

Copyright 2026, Emiliano Hatim <emilianohatim01@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

/** @file hal_gpio.c
 ** @brief implementacion de la biblioteca para gestion de entradas y salidas digitales
 **/

/* === Headers files inclusions ================================================================ */

#include "hal_gpio.h"
#include "driver/gpio.h"

/* === Macros definitions ====================================================================== */

/* === Private data type definitions ========================================================== */

/* === Private function definitions =========================================================== */

void hal_gpio_init(hal_gpio_t * gpio){
    if (gpio == NULL){
        return;
    }
    gpio_config_t input_output_conf = {
        .pin_bit_mask = (1ULL << gpio->pin),
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    if (gpio->direction == HAL_GPIO_DIR_OUTPUT){
        input_output_conf.mode = GPIO_MODE_OUTPUT;
        input_output_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    } else {
        input_output_conf.mode = GPIO_MODE_INPUT;
        input_output_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    }
    gpio_config(&input_output_conf);
}

void hal_gpio_write(hal_gpio_t * gpio, hal_gpio_state_t state) {
    if (gpio != NULL && gpio->direction == HAL_GPIO_DIR_OUTPUT) {
        gpio_set_level((gpio_num_t)gpio->pin, (uint32_t)state);
    }
}

hal_gpio_state_t hal_gpio_read(hal_gpio_t * gpio) {
    if (gpio != NULL && gpio->direction == HAL_GPIO_DIR_INPUT) {
        return (gpio_get_level((gpio_num_t)gpio->pin) == 1) ? HAL_GPIO_STATE_HIGH : HAL_GPIO_STATE_LOW;
    }
    return HAL_GPIO_STATE_LOW;
}

/* === Private variable definitions ============================================================ */

/* === Public data type definitions =============================================================*/

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Public function implementation ========================================================== */

/* === End of documentation ==================================================================== */
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

#ifndef HAL_GPIO_H_
#define HAL_GPIO_H_

/** @file GPIO.h
 ** @author Emiliano Hatim (emilianohatim01@gmail.com)
 ** @brief Declaraciones de la biblioteca para gestion de entradas y salidas digitales
 ** @version 1.0
 * @date 2026-07
 * @copyright Copyright (c) 2026
 **/

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/* === Public data type declarations =============================================================================== */

typedef enum {
    HAL_GPIO_DIR_INPUT,
    HAL_GPIO_DIR_OUTPUT
} hal_gpio_dir_t;

typedef enum {
    HAL_GPIO_STATE_LOW = 0,
    HAL_GPIO_STATE_HIGH = 1
} hal_gpio_state_t;

typedef struct {
    uint8_t pin;              /**< Número físico del pin*/
    hal_gpio_dir_t direction; /**< Entrada o Salida digital */
} hal_gpio_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

void hal_gpio_init(hal_gpio_t * gpio);

void hal_gpio_write(hal_gpio_t * gpio, hal_gpio_state_t state);

hal_gpio_state_t hal_gpio_read(hal_gpio_t * gpio);

void hal_gpio_toggle(hal_gpio_t * gpio);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H_ */
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

/** @file 
 * @brief 
 * 
 */

/* === Headers files inclusions ================================================================ */

#include <stdio.h>
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* === Macros definitions ====================================================================== */

/* === Private data type definitions ========================================================== */

hal_gpio_t boton_up = { .pin = 17, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_down = { .pin = 5, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_ok = { .pin = 18, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_back = { .pin = 23, .direction = HAL_GPIO_DIR_INPUT};

hal_gpio_t oled_reset = { .pin = 16, .direction = HAL_GPIO_DIR_OUTPUT};

hal_gpio_t led1 = { .pin = 19, .direction = HAL_GPIO_DIR_OUTPUT};

hal_adc_t sensor_presion = { .unit = 1, .channel = 0};
hal_adc_t tension_ref = { .unit = 1, .channel = 1};

/* === Private function definitions =========================================================== */

/* === Private variable definitions ============================================================ */

/* === Public data type definitions =============================================================*/

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Public function implementation ========================================================== */

void app_main(void){
    hal_gpio_init(&boton_up);
    hal_gpio_init(&boton_down);
    hal_gpio_init(&boton_ok);
    hal_gpio_init(&boton_back);

    hal_gpio_init(&oled_reset);

    hal_gpio_write(&oled_reset, HAL_GPIO_STATE_LOW);
    vTaskDelay(pdMS_TO_TICKS(50));
    hal_gpio_write(&oled_reset, HAL_GPIO_STATE_HIGH);
    vTaskDelay(pdMS_TO_TICKS(50));

    hal_adc_init(&sensor_presion);
    hal_adc_init(&tension_ref);

    hal_gpio_init(&led1);

    hal_i2c_init(4,15);
    uint8_t cmd_on[] = {0x00,
                        0x8D,
                        0x14,
                        0xAF};
    hal_i2c_write(cmd_on, sizeof(cmd_on));

    while(1){
        int presion_mv = hal_adc_read_mv(&sensor_presion);
        int ref_mv = hal_adc_read_mv(&tension_ref);

        printf("Sensor MPX: %d mV | referencia: %d mV\n", presion_mv, ref_mv);

        hal_gpio_state_t estado = hal_gpio_read(&boton_back);
        if(estado == HAL_GPIO_STATE_LOW){
            hal_gpio_write(&led1, HAL_GPIO_STATE_HIGH);
        }
        else {
            hal_gpio_write(&led1, HAL_GPIO_STATE_LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* === End of documentation ==================================================================== */

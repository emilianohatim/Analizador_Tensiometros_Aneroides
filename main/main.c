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
#include "hal_ssd1306.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* === Macros definitions ====================================================================== */

/* === Private data type definitions ========================================================== */

hal_gpio_t boton_up = { .pin = 17, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_down = { .pin = 5, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_ok = { .pin = 18, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_back = { .pin = 23, .direction = HAL_GPIO_DIR_INPUT};

hal_gpio_t oled_reset = { .pin = 16, .direction = HAL_GPIO_DIR_OUTPUT};

hal_adc_t sensor_presion = { .unit = 1, .channel = 0};
hal_adc_t tension_ref = { .unit = 1, .channel = 1};

/* === Private function definitions =========================================================== */

void inicializar_hardware(void){
    //Inicialización del reset del OLED
    hal_gpio_init(&oled_reset);

    hal_gpio_write(&oled_reset, HAL_GPIO_STATE_LOW);
    vTaskDelay(pdMS_TO_TICKS(50));
    hal_gpio_write(&oled_reset, HAL_GPIO_STATE_HIGH);
    vTaskDelay(pdMS_TO_TICKS(50));

    //Inicialización de los botones 
    hal_gpio_init(&boton_up);
    hal_gpio_init(&boton_down);
    hal_gpio_init(&boton_ok);
    hal_gpio_init(&boton_back);

    //Inicialización del ADC
    hal_adc_init(&sensor_presion);
    hal_adc_init(&tension_ref);

    //Inicialización de la comunicación y pantalla 
    hal_i2c_init(4,15);
    hal_ssd1306_init();
}

int Calibrar(hal_adc_t * presion_mv){
    int offset = 0;
    for (int i = 0; i < 100; i++){
        offset = offset + hal_adc_read_mv(presion_mv);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    int offsetprom = offset/100;
    return offsetprom;
}

/* === Private variable definitions ============================================================ */

int conv_kPa_mmhg = 7.5006375;

/* === Public data type definitions =============================================================*/

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Public function implementation ========================================================== */

void app_main(void){    

    inicializar_hardware();
    int offsetprom = Calibrar(&sensor_presion);
    
    while(1){
        //lectura inicial de ambos canales, la lectura del sensor ya esta calibrada 
        int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion) - offsetprom;
        int ref_mv = hal_adc_read_mv(&tension_ref);

        // Pasamos los valores de mV a V 
        float tension_regulador = (ref_mv * 2.0) / 1000.0;
        float tension_sensor_cal_v = (tension_sensor_cal_mv * 2.0) / 1000.0;

        // Formula del fabricante: Vout = Vs * (0.018 * P) [V]
        // Despejando P = (Vout / Vs) / 0.018 [kPa]
        float presion_kPa = (tension_sensor_cal_v / tension_regulador) / 0.018;
        float presion_mmHg = presion_kPa * conv_kPa_mmhg;
        if (presion_mmHg < 0.0){
            presion_mmHg = 0.0; 
        }
        printf("Sensor MPX: %d mV | referencia: %d mV\n", tension_sensor_cal_mv, ref_mv);
        //printf("Sensor MPX: %.0f mmHg\n", presion_mmHg);
        
        //Se muestra por pantalla el verdadero valor de la medicion sin el ruido interno del uC
        char texto_oled[32];

        snprintf(texto_oled, sizeof(texto_oled), "Presion: %.0f mmHg\n", presion_mmHg);
        hal_ssd1306_clear();
        hal_ssd1306_draw_string(0, 2, texto_oled);
        hal_ssd1306_update();

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* === End of documentation ==================================================================== */

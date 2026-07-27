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
#include "hal_bateria.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

/* === Macros definitions ====================================================================== */

/* === Private data type definitions ========================================================== */

hal_gpio_t boton_up = { .pin = 17, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_down = { .pin = 5, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_ok = { .pin = 18, .direction = HAL_GPIO_DIR_INPUT};
hal_gpio_t boton_back = { .pin = 23, .direction = HAL_GPIO_DIR_INPUT};

hal_gpio_t oled_reset = { .pin = 16, .direction = HAL_GPIO_DIR_OUTPUT};

hal_adc_t sensor_presion = { .unit = 1, .channel = 0};
hal_adc_t tension_ref = { .unit = 1, .channel = 1};
hal_adc_t nivel_bat = { .unit = 2, .channel = 5};

typedef enum{
    PANTALLA_MENU,
    PANTALLA_MEDICION,
    PANTALLA_VELOCIDAD,
    PANTALLA_fUGAS
} estado_equipo_t;

estado_equipo_t estado_actual = PANTALLA_MENU;
int opcion_cursor = 0;                          /**< Selección de opciones */
const int MAX_OPCIONES = 2;                     /**< Limite del cursor */ 

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
    hal_adc_init(&nivel_bat);

    //Inicialización de la comunicación y pantalla 
    hal_i2c_init(4,15);
    hal_ssd1306_init();
}

/* === Private variable definitions ============================================================ */

float conv_kPa_mmhg = 7.5006375f;

/* === Public data type definitions =============================================================*/

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Public function implementation ========================================================== */

void app_main(void){    

    inicializar_hardware();
    
    while(1){

        if (hal_gpio_read(&boton_down) == HAL_GPIO_STATE_LOW){
            if (estado_actual == PANTALLA_MENU){
                opcion_cursor++;
                if (opcion_cursor > MAX_OPCIONES) opcion_cursor = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        if (hal_gpio_read(&boton_up) == HAL_GPIO_STATE_LOW) {
            if (estado_actual == PANTALLA_MENU) {
                opcion_cursor--;
                if (opcion_cursor < 0) opcion_cursor = MAX_OPCIONES;
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        if (hal_gpio_read(&boton_ok) == HAL_GPIO_STATE_LOW){
            if(estado_actual == PANTALLA_MENU){
                if (opcion_cursor == 0){
                    estado_actual = PANTALLA_MEDICION;
                } else if (opcion_cursor == 1){
                    estado_actual = PANTALLA_VELOCIDAD;
                } else if (opcion_cursor == 2){
                    estado_actual = PANTALLA_fUGAS;
                }
                hal_ssd1306_clear();
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        if (hal_gpio_read(&boton_back) == HAL_GPIO_STATE_LOW){
            if (estado_actual == PANTALLA_MEDICION){
                estado_actual = PANTALLA_MENU;
                hal_ssd1306_clear();
            }
            if (estado_actual == PANTALLA_VELOCIDAD){
                estado_actual = PANTALLA_MENU;
                hal_ssd1306_clear();
            }
            if (estado_actual == PANTALLA_fUGAS) {
                estado_actual = PANTALLA_MENU;
                hal_ssd1306_clear();;
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        switch (estado_actual){

            case PANTALLA_MENU: {
                static int contador_vueltas = 0;
                contador_vueltas++;
                int mostrar_cursor = (contador_vueltas / 10) % 2;

                uint8_t nivel_bateria = hal_bateria_obtener_porcentaje(&nivel_bat);
                int cargador_enchufado = hal_bateria_esta_cargando();
                char texto_bat[16];
                if (cargador_enchufado == 1){
                    snprintf(texto_bat, sizeof(texto_bat), "USB Bat:%d%%", nivel_bateria);                
                } else {
                    snprintf(texto_bat, sizeof(texto_bat), "    Bat:%d%%", nivel_bateria);
                }

                hal_ssd1306_clear();
                hal_ssd1306_draw_string(0, 0, "MENU");
                hal_ssd1306_draw_string(55, 0, texto_bat);
                const char* textos_opciones[3] = {"1. Medir Presion", 
                                                  "2. Medir Velocidad",
                                                  "3. medir fugas"
                                                  };
                uint8_t posiciones_y[3] = {2, 4, 6}; 

                for (int i = 0; i < 3; i++) {
                    char texto_a_dibujar[32];
                    if (opcion_cursor == i && mostrar_cursor == 1) {
                        snprintf(texto_a_dibujar, sizeof(texto_a_dibujar), "> %s", textos_opciones[i]);
                    } else {
                        snprintf(texto_a_dibujar, sizeof(texto_a_dibujar), "  %s", textos_opciones[i]);
                    }
                    hal_ssd1306_draw_string(0, posiciones_y[i], texto_a_dibujar);
                }
                hal_ssd1306_update();
                break;
            }

            case PANTALLA_MEDICION: {
                //lectura inicial de ambos canales, la lectura del sensor ya esta calibrada 
                int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion);
                int ref_mv = hal_adc_read_mv(&tension_ref);

                // Pasamos los valores de mV a V 
                float tension_regulador = (ref_mv * 2.0) / 1000.0;                    // los 2.0 esta dado por el factor proveniente del divisor de tensión
                float tension_sensor_cal_v = (tension_sensor_cal_mv * 2.0) / 1000.0;

                // Formula del fabricante: Vout = Vs * (0.018 * P + 0.04) [V]
                // Despejando P = ((Vout / Vs) - 0.04) / 0.018 [kPa]
                float presion_kPa = ((tension_sensor_cal_v / tension_regulador) - 0.056) / 0.018;
                float presion_mmHg = presion_kPa * conv_kPa_mmhg;
                printf("el valor de presion es: %f\n", presion_mmHg);
                if (presion_mmHg < 0.0){
                    presion_mmHg = 0.0; 
                }

                //Se muestra por pantalla el verdadero valor de la medicion sin el ruido interno del uC
                char texto_oled[32];

                snprintf(texto_oled, sizeof(texto_oled), "Presion: %.0f mmHg\n", presion_mmHg);     
                hal_ssd1306_draw_string(0, 0, "MODO MEDICION");
                hal_ssd1306_draw_string(0, 2, texto_oled);
                hal_ssd1306_draw_string(0, 7, "[BACK] -> salir");
                hal_ssd1306_update();
                break;
            }

            case PANTALLA_VELOCIDAD: {
                static int64_t tiempo_anterior = 0;
                static float presion_mmHg_anterior = 0.0;
                static float velocidad_mmHg_s = 0.0;

                int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion);
                int ref_mv = hal_adc_read_mv(&tension_ref);
                float tension_regulador = (ref_mv * 2.0) / 1000.0;
                float tension_sensor_cal_v = (tension_sensor_cal_mv * 2.0) / 1000.0;
                float presion_kPa = ((tension_sensor_cal_v / tension_regulador) - 0.04) / 0.018;
                float presion_mmHg_actual = presion_kPa * conv_kPa_mmhg;
                if (presion_mmHg_actual < 0.0){
                    presion_mmHg_actual = 0.0; 
                }
                
                int64_t tiempo_actual = esp_timer_get_time();
                if ((tiempo_actual - tiempo_anterior) >= 1000000){
                    float delta_p = presion_mmHg_actual - presion_mmHg_anterior;
                    velocidad_mmHg_s = delta_p;
                    tiempo_anterior = tiempo_actual;
                    presion_mmHg_anterior = presion_mmHg_actual;
                }

                char texto_vel[32];
                snprintf(texto_vel, sizeof(texto_vel), "V: %.1f mmHg/s", velocidad_mmHg_s);

                hal_ssd1306_draw_string(0, 0, "MODO VELOCIDAD");
                hal_ssd1306_draw_string(0, 3, texto_vel);
                hal_ssd1306_draw_string(0, 7, "[BACK] p/Salir");
                
                hal_ssd1306_update();
                break;
            }

            case PANTALLA_fUGAS: {
                static int fase_prueba = 0;
                static int64_t tiempo_inicio = 0.0;
                static float presion_inicial = 0.0;
                static float fuga_total = 0.0;
                static int tiempo_restante = 60;

                int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion);
                int ref_mv = hal_adc_read_mv(&tension_ref);
                float tension_regulador = (ref_mv * 2.0) / 1000.0;
                float tension_sensor_cal_v = (tension_sensor_cal_mv * 2.0) / 1000.0;
                float presion_kPa = ((tension_sensor_cal_v / tension_regulador) - 0.04) / 0.018;
                float presion_mmHg_actual = presion_kPa * conv_kPa_mmhg;
                if (presion_mmHg_actual < 0.0){
                    presion_mmHg_actual = 0.0; 
                }

                hal_ssd1306_clear();
                hal_ssd1306_draw_string(0, 0, "MODO FUGAS");

                if (fase_prueba == 0){
                    char txt_pres[32];
                    snprintf(txt_pres, sizeof(txt_pres), "P_actual: %.0f mmHg", presion_mmHg_actual);
                    hal_ssd1306_draw_string(0, 2, "Inflar brazalete...");
                    hal_ssd1306_draw_string(0, 4, txt_pres);
                    hal_ssd1306_draw_string(0, 6, "[UP] -> Iniciar");

                    if (hal_gpio_read(&boton_up) == HAL_GPIO_STATE_LOW){
                        presion_inicial = presion_mmHg_actual;
                        tiempo_inicio = esp_timer_get_time();
                        fase_prueba = 1;
                        vTaskDelay(pdMS_TO_TICKS(150));
                    }
                }
                else if (fase_prueba == 1){
                    int64_t tiempo_actual = esp_timer_get_time();
                    int segundos_pasados = (tiempo_actual - tiempo_inicio) / 1000000;
                    tiempo_restante = 60 - segundos_pasados;
                    
                    char txt_tiempo[32];
                    char txt_pinicio[32];
                    snprintf(txt_tiempo, sizeof(txt_tiempo), "Tiempo: %d s", tiempo_restante);
                    snprintf(txt_pinicio, sizeof(txt_pinicio), "P_inicial: %.0f mmHg", presion_inicial);

                    hal_ssd1306_draw_string(0, 2, txt_tiempo);
                    hal_ssd1306_draw_string(0, 4, txt_pinicio);
                    hal_ssd1306_draw_string(0, 6, "Midiendo....");

                    if (tiempo_restante <= 0){
                        fuga_total = presion_inicial - presion_mmHg_actual;
                        fase_prueba = 2;
                    }
                }
                else if (fase_prueba == 2){
                    char txt_res[32];
                    snprintf(txt_res, sizeof(txt_res), "Perdidas: %.1f mmHg", fuga_total);

                    hal_ssd1306_draw_string(0, 2, "Prueba finalizada");
                    hal_ssd1306_draw_string(0, 4, txt_res);
                    hal_ssd1306_draw_string(0, 6, "[UP] -> Reiniciar");

                    if (hal_gpio_read(&boton_up) == HAL_GPIO_STATE_LOW){
                        fase_prueba = 0;
                        vTaskDelay(pdMS_TO_TICKS(150));
                    }
                }
                if (hal_gpio_read(&boton_back) == HAL_GPIO_STATE_LOW){
                    estado_actual = PANTALLA_MENU;
                    fase_prueba = 0;
                    hal_ssd1306_clear();
                    vTaskDelay(pdMS_TO_TICKS(150));
                }
                hal_ssd1306_update();
                break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* === End of documentation ==================================================================== */

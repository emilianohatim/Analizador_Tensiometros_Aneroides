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

/** @file main.c
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
    PANTALLA_FUGAS,
    PANTALLA_DESINFLADO, 
    PANTALLA_LIBERACION
} estado_equipo_t;

estado_equipo_t estado_actual = PANTALLA_MENU;
int opcion_cursor = 0;                          /**< Selección de opciones */
const int MAX_OPCIONES = 3;                     /**< Limite del cursor */ 
const uint8_t CANTIDAD_ENSAYOS = 4;

int presion_ajustada = 300;

int fase_liberacion = 0;
int64_t tiempo_inicio_liberacion = 0;
int64_t tiempo_final_liberacion_s = 0;

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

float conv_kPa_mmhg = 7.50062;

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
            if (estado_actual == PANTALLA_LIBERACION){
                presion_ajustada--;
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        if (hal_gpio_read(&boton_up) == HAL_GPIO_STATE_LOW) {
            if (estado_actual == PANTALLA_MENU) {
                opcion_cursor--;
                if (opcion_cursor < 0) opcion_cursor = MAX_OPCIONES;
            }
            if (estado_actual == PANTALLA_LIBERACION){
                if (fase_liberacion == 0) {
                    presion_ajustada++;
                } else if (fase_liberacion == 2){
                    fase_liberacion = 0;
                    tiempo_inicio_liberacion = 0;
                    tiempo_final_liberacion_s = 0;
                } 
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        if (hal_gpio_read(&boton_ok) == HAL_GPIO_STATE_LOW){
            if(estado_actual == PANTALLA_MENU){
                if (opcion_cursor == 0){
                    estado_actual = PANTALLA_MEDICION;
                } else if (opcion_cursor == 1){
                    estado_actual = PANTALLA_FUGAS;
                } else if (opcion_cursor == 2){
                    estado_actual = PANTALLA_DESINFLADO;
                } else if (opcion_cursor == 3){
                    estado_actual = PANTALLA_LIBERACION;
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
            if (estado_actual == PANTALLA_DESINFLADO){
                estado_actual = PANTALLA_MENU;
                hal_ssd1306_clear();
            }
            if (estado_actual == PANTALLA_FUGAS) {
                estado_actual = PANTALLA_MENU;
                hal_ssd1306_clear();;
            }
            if (estado_actual == PANTALLA_LIBERACION) {
                estado_actual = PANTALLA_MENU;
                hal_ssd1306_clear();
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        switch (estado_actual){

            case PANTALLA_MENU: {
                static int contador_vueltas = 0;
                contador_vueltas++;
                int mostrar_cursor = (contador_vueltas / 10) % 2;
                hal_ssd1306_clear();

                uint8_t nivel_bateria = hal_bateria_obtener_porcentaje(&nivel_bat);
                int cargador_enchufado = hal_bateria_esta_cargando();
                char texto_bat[16];
                if (cargador_enchufado == 1){
                    snprintf(texto_bat, sizeof(texto_bat), "USB Bat:%d%%", nivel_bateria);                
                } else {
                    snprintf(texto_bat, sizeof(texto_bat), "    Bat:%d%%", nivel_bateria);
                }

                char titulo_menu[32];
                int pagina_actual = (opcion_cursor / 3) + 1;
                int paginas_totales = (CANTIDAD_ENSAYOS + 2) / 3;
                snprintf(titulo_menu, sizeof(titulo_menu), "MENU %d/%d", pagina_actual, paginas_totales);
                hal_ssd1306_draw_string(0, 0, titulo_menu);
                hal_ssd1306_draw_string(55, 0, texto_bat);
                const char* textos_opciones[4] = {"1. Presion", 
                                                  "2. Fugas",
                                                  "3. Tasa desinflado",
                                                  "4. Liberacion"
                                                  };
                int indice_inicio = (opcion_cursor / 3) * 3;
                uint8_t posiciones_y[3] = {2, 4, 6}; 

                for (int i = 0; i < 3; i++) {
                    int indice_real = indice_inicio + i;
                    if (indice_real < CANTIDAD_ENSAYOS){
                        char texto_a_dibujar[32];
                        if (opcion_cursor == indice_real && mostrar_cursor == 1) {
                            snprintf(texto_a_dibujar, sizeof(texto_a_dibujar), "> %s", textos_opciones[indice_real]);
                        } else {
                            snprintf(texto_a_dibujar, sizeof(texto_a_dibujar), "  %s", textos_opciones[indice_real]);
                        }
                        hal_ssd1306_draw_string(0, posiciones_y[i], texto_a_dibujar);
                    }
                }
                hal_ssd1306_update();
                break;
            }

            case PANTALLA_MEDICION: {
                hal_ssd1306_clear();
                int32_t promedio_presion = 0;
                for (int i = 0; i < 16; i++) {
                    int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion);
                    int ref_mv = hal_adc_read_mv(&tension_ref);
                    int32_t numerador = (7000 * tension_sensor_cal_mv) - (392 * ref_mv);
                    int32_t denominador = 18 * ref_mv;
                    int32_t muestra = numerador / denominador;
                    if (muestra < 0) muestra = 0;
                    promedio_presion += muestra;
                }
                int32_t presion_mmHg_promediada = (promedio_presion / 16) + 7;
                int32_t presion_calibrada = (int32_t)(((presion_mmHg_promediada - 6.46f) / 0.898f) + 0.5f); 
                if (presion_calibrada <= 198) {
                    presion_calibrada = (int32_t)((presion_calibrada * 1.0052f) + 1.89f);
                    if (presion_calibrada < 0) presion_calibrada = 0;
                }
                char texto_oled[32];
                snprintf(texto_oled, sizeof(texto_oled), "Presion: %d mmHg", (int)presion_calibrada);    
                hal_ssd1306_draw_string(0, 0, "MODO MEDICION");
                hal_ssd1306_draw_string(0, 2, texto_oled);
                hal_ssd1306_draw_string(0, 7, "[BACK] -> salir");
                hal_ssd1306_update();
                break;
            }

            case PANTALLA_DESINFLADO: {
                hal_ssd1306_clear();
                static int64_t tiempo_anterior = 0;
                static int64_t presion_mmHg_anterior = 0;
                static int64_t velocidad_mmHg_s = 0;

                int32_t promedio_presion = 0;
                for (int i = 0; i < 16; i++) {
                    int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion);
                    int ref_mv = hal_adc_read_mv(&tension_ref);
                    int32_t numerador = (7000 * tension_sensor_cal_mv) - (392 * ref_mv);
                    int32_t denominador = 18 * ref_mv;
                    int32_t muestra = numerador / denominador;
                    if (muestra < 0) muestra = 0;
                    promedio_presion += muestra;
                }
                int32_t presion_mmHg_promediada = (promedio_presion / 16) + 7;
                int32_t presion_calibrada = (int32_t)(((presion_mmHg_promediada - 6.46f) / 0.898f) + 0.5f); 
                if (presion_calibrada <= 198) {
                    presion_calibrada = (int32_t)((presion_calibrada * 1.0052f) + 1.89f);
                    if (presion_calibrada < 0) presion_calibrada = 0;
                }
                int64_t tiempo_actual = esp_timer_get_time();
                int64_t dt_us = tiempo_actual - tiempo_anterior;
                if (dt_us >= 50000){
                    int64_t delta_p = presion_calibrada - presion_mmHg_anterior;
                    presion_mmHg_anterior = presion_calibrada;
                    tiempo_anterior = tiempo_actual;
                    int64_t velocidad_cruda = (delta_p * 1000000) / dt_us;

                    velocidad_mmHg_s = velocidad_cruda;
                    if (velocidad_mmHg_s < 0){
                        velocidad_mmHg_s = -velocidad_mmHg_s;
                    }
                }

                char texto_pres[32];
                snprintf(texto_pres, sizeof(texto_pres), "Presion: %d mmHg", (int)presion_calibrada);

                char texto_vel[32];
                snprintf(texto_vel, sizeof(texto_vel), "Vel: %d mmHg/s", (int)velocidad_mmHg_s);

                hal_ssd1306_draw_string(0, 0, "MODO DESINFLADO");
                hal_ssd1306_draw_string(0, 2, texto_pres);
                hal_ssd1306_draw_string(0, 4, texto_vel);
                hal_ssd1306_draw_string(0, 7, "[BACK] -> Salir");
                
                hal_ssd1306_update();
                break;
            }

            case PANTALLA_FUGAS: {
                static int fase_prueba = 0;
                static int64_t tiempo_inicio = 0;
                static int64_t presion_inicial = 0;
                static int64_t fuga_total = 0;
                static int64_t tiempo_restante = 30;

                int32_t promedio_presion = 0;
                for (int i = 0; i < 16; i++) {
                    int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion);
                    int ref_mv = hal_adc_read_mv(&tension_ref);
                    int32_t numerador = (7000 * tension_sensor_cal_mv) - (392 * ref_mv);
                    int32_t denominador = 18 * ref_mv;
                    int32_t muestra = numerador / denominador;
                    if (muestra < 0) muestra = 0;
                    promedio_presion += muestra;
                }
                int32_t presion_mmHg_promediada = (promedio_presion / 16) + 7;
                int32_t presion_calibrada = (int32_t)(((presion_mmHg_promediada - 6.46f) / 0.898f) + 0.5f); 
                if (presion_calibrada <= 198) {
                    presion_calibrada = (int32_t)((presion_calibrada * 1.0052f) + 1.89f);
                    if (presion_calibrada < 0) presion_calibrada = 0;
                }
                hal_ssd1306_clear();
                hal_ssd1306_draw_string(0, 0, "MODO FUGAS");

                if (fase_prueba == 0){
                    char txt_pres[32];
                    snprintf(txt_pres, sizeof(txt_pres), "P_actual: %d mmHg", (int)presion_calibrada);
                    hal_ssd1306_draw_string(0, 2, "Inflar brazalete...");
                    hal_ssd1306_draw_string(0, 4, txt_pres);
                    hal_ssd1306_draw_string(0, 6, "[UP] -> Iniciar");

                    if (hal_gpio_read(&boton_up) == HAL_GPIO_STATE_LOW){
                        presion_inicial = presion_calibrada;
                        tiempo_inicio = esp_timer_get_time();
                        fase_prueba = 1;
                        vTaskDelay(pdMS_TO_TICKS(150));
                    }
                }
                else if (fase_prueba == 1){
                    int64_t tiempo_actual = esp_timer_get_time();
                    int segundos_pasados = (tiempo_actual - tiempo_inicio) / 1000000;
                    tiempo_restante = 30 - segundos_pasados;
                    
                    char txt_tiempo[32];
                    char txt_pinicio[32];
                    snprintf(txt_tiempo, sizeof(txt_tiempo), "Tiempo: %d s", (int)tiempo_restante);
                    snprintf(txt_pinicio, sizeof(txt_pinicio), "P_inicial: %d mmHg", (int)presion_inicial);

                    hal_ssd1306_draw_string(0, 2, txt_tiempo);
                    hal_ssd1306_draw_string(0, 4, txt_pinicio);
                    hal_ssd1306_draw_string(0, 6, "Midiendo....");

                    if (tiempo_restante <= 0){
                        fuga_total = presion_inicial - presion_calibrada;
                        fase_prueba = 2;
                    }
                }
                else if (fase_prueba == 2){
                    char txt_res[32];
                    snprintf(txt_res, sizeof(txt_res), "Perdidas: %d mmHg", (int)fuga_total);

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

            case PANTALLA_LIBERACION: {
                hal_ssd1306_clear();
                int32_t promedio_presion = 0;
                for (int i = 0; i < 16; i++) {
                    int tension_sensor_cal_mv = hal_adc_read_mv(&sensor_presion);
                    int ref_mv = hal_adc_read_mv(&tension_ref);
                    int32_t numerador = (7000 * tension_sensor_cal_mv) - (392 * ref_mv);
                    int32_t denominador = 18 * ref_mv;
                    int32_t muestra = numerador / denominador;
                    if (muestra < 0) muestra = 0;
                    promedio_presion += muestra;
                }
                int32_t presion_mmHg_promediada = (promedio_presion / 16) + 7;
                int32_t presion_calibrada = (int32_t)(((presion_mmHg_promediada - 6.46f) / 0.898f) + 0.5f);
                if (presion_calibrada <= 198) {
                    presion_calibrada = (int32_t)((presion_calibrada * 1.0052f) + 1.89f);
                    if (presion_calibrada < 0) presion_calibrada = 0;
                }
                char ajuste_presion[32];
                snprintf(ajuste_presion, sizeof(ajuste_presion), "P_ajus: %d mmHg", (int)presion_ajustada);

                char texto_presion[32];
                snprintf(texto_presion, sizeof(texto_presion), "P_actual: %d mmHg", (int)presion_calibrada);

                hal_ssd1306_draw_string(0, 0, "MODO LIBERACION");
                hal_ssd1306_draw_string(0, 2, ajuste_presion);
                hal_ssd1306_draw_string(0, 3, texto_presion);

                if (fase_liberacion == 0){
                    hal_ssd1306_draw_string(0, 5, "Insuflar");
                    if (presion_calibrada > presion_ajustada){
                        tiempo_inicio_liberacion = esp_timer_get_time();
                        fase_liberacion = 1;
                    }
                } else if (fase_liberacion == 1){
                    hal_ssd1306_draw_string(0, 5, "Girar valvula");
                    if (presion_calibrada <= 15) {
                        int64_t tiempo_actual = esp_timer_get_time();
                        tiempo_final_liberacion_s = (tiempo_actual - tiempo_inicio_liberacion) / 1000000;
                        fase_liberacion = 2;
                    }
                } else if (fase_liberacion == 2){
                    char texto_tiempo[32];
                    snprintf(texto_tiempo, sizeof(texto_tiempo), "Tiempo: %d s", (int)tiempo_final_liberacion_s);
                    hal_ssd1306_draw_string(0, 5, texto_tiempo);
                    hal_ssd1306_draw_string(0, 6, "[UP] -> Reiniciar");
                }

                hal_ssd1306_draw_string(0, 7, "[BACK] -> Salir");
                hal_ssd1306_update();

                if (hal_gpio_read(&boton_back) == HAL_GPIO_STATE_LOW) {
                    fase_liberacion = 0;
                }

                break;
            }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
}

/* === End of documentation ==================================================================== */

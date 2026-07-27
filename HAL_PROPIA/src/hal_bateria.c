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

/** @file hal_adc.c
 ** @brief implementacion de la biblioteca de gestión para el conversor analógico digital
 **/

/* === Headers files inclusions ================================================================ */

#include "hal_bateria.h"

/* === Macros definitions ====================================================================== */

#define VOLTAJE_MAX_BAT 4.2f
#define VOLTAJE_MIN_BAT 3.2f
#define FACTOR_DIVISOR  2.0f

/* === Private data type definitions ========================================================== */

/* === Private function definitions =========================================================== */

/* === Private variable definitions ============================================================ */

static int porcentaje_historico = -1;

static int estado_cargando = 0;

/* === Public data type definitions =============================================================*/

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Public function implementation ========================================================== */

uint8_t hal_bateria_obtener_porcentaje(hal_adc_t * nivel_bat) {
    uint32_t suma_adc = 0;
    for(int i = 0; i < 10; i++) {
        suma_adc += hal_adc_read_mv(nivel_bat); 
    }
    float voltaje_adc = (suma_adc / 10.0f) / 1000.0f;
    float voltaje_bat = voltaje_adc * FACTOR_DIVISOR;
    float porcentaje_float = ((voltaje_bat - VOLTAJE_MIN_BAT) / (VOLTAJE_MAX_BAT - VOLTAJE_MIN_BAT)) * 100.0f;
    int porcentaje_crudo = (int)porcentaje_float;
    if(porcentaje_crudo > 100) porcentaje_crudo = 100;
    if(porcentaje_crudo < 0) porcentaje_crudo = 0;

    if (porcentaje_historico == -1){
        porcentaje_historico = porcentaje_crudo;
    }

    if (porcentaje_crudo > (porcentaje_historico + 10)){
        estado_cargando = 1;
        return (uint8_t) porcentaje_historico;
    } else {
        estado_cargando = 0;
        porcentaje_historico = porcentaje_crudo;
        return (uint8_t)porcentaje_historico;
    }
}

int hal_bateria_esta_cargando(void) {
    return estado_cargando;
}

/* === End of documentation ==================================================================== */
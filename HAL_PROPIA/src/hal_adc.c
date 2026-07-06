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

#include "hal_adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

/* === Macros definitions ====================================================================== */

/* === Private data type definitions ========================================================== */

/* === Private function definitions =========================================================== */

/* === Private variable definitions ============================================================ */

static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_oneshot_unit_handle_t adc2_handle = NULL;

/* === Public data type definitions =============================================================*/

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Public function implementation ========================================================== */

void hal_adc_init(hal_adc_t * adc_config){
    adc_oneshot_unit_handle_t * current_handle = NULL;

    if (adc_config->unit == 1){
        current_handle = &adc1_handle;
    } else {
        current_handle = &adc2_handle;
    }

    if (*current_handle == NULL){
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = (adc_config->unit == 1) ? ADC_UNIT_1 : ADC_UNIT_2,
        };
        adc_oneshot_new_unit(&init_config, current_handle);
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(*current_handle, adc_config->channel,&config);

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = (adc_config->unit == 1) ? ADC_UNIT_1 : ADC_UNIT_2,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_cali_handle_t cali_handle = NULL;
    adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);
    adc_config->call_handle = (void*)cali_handle;
}

int hal_adc_read_mv(hal_adc_t * adc_config){
    if (adc_config == NULL || adc_config->call_handle == NULL) return -1;

    adc_oneshot_unit_handle_t handle = (adc_config->unit == 1) ? adc1_handle : adc2_handle;
    int raw_value = 0;
    int voltage_mv = 0;

    adc_oneshot_read(handle, adc_config->channel, &raw_value);

    adc_cali_raw_to_voltage((adc_cali_handle_t)adc_config->call_handle, raw_value, &voltage_mv);

    return voltage_mv;
}

/* === End of documentation ==================================================================== */
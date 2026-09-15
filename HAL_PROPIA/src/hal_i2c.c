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

/** @file hal_i2c.c
 ** @brief implementacion de la biblioteca para comunicación i2c
 **/

/* === Headers files inclusions ================================================================ */

#include "hal_i2c.h"
#include "driver/i2c_master.h"

/* === Macros definitions ====================================================================== */

/* === Private data type definitions ========================================================== */

/* === Private function definitions =========================================================== */

/* === Private variable definitions ============================================================ */

static i2c_master_bus_handle_t bus_handle_oled = NULL;
static i2c_master_bus_handle_t bus_handle_dac = NULL;

/* === Public data type definitions =============================================================*/

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

void hal_i2c_init_oled(uint8_t sda_pin, uint8_t scl_pin){
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = scl_pin,
        .sda_io_num = sda_pin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_new_master_bus(&i2c_bus_config, &bus_handle_oled);
}

void hal_i2c_init_dac(uint8_t sda_pin, uint8_t scl_pin){
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_1, 
        .scl_io_num = scl_pin,
        .sda_io_num = sda_pin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_new_master_bus(&i2c_bus_config, &bus_handle_dac);
}

void hal_i2c_add_device_oled(uint8_t dev_addr, i2c_master_dev_handle_t * dev_handle){
    if (bus_handle_oled == NULL){
        return;
    }
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = 400000,
    };
    i2c_master_bus_add_device(bus_handle_oled, &dev_config, dev_handle);
}

void hal_i2c_add_device_dac(uint8_t dev_addr, i2c_master_dev_handle_t * dev_handle){
    if (bus_handle_dac == NULL) {
        return;
    }
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = 10000, 
    };
    i2c_master_bus_add_device(bus_handle_dac, &dev_config, dev_handle);
}

void hal_i2c_write(i2c_master_dev_handle_t dev_hanlde, uint8_t * data, size_t length){
    if (dev_hanlde != NULL){
        i2c_master_transmit(dev_hanlde, data, length, -1);
    }
}

/* === Public function implementation ========================================================== */

/* === End of documentation ==================================================================== */
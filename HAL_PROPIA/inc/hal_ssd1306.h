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

#ifndef HAL_SSD1306_H_
#define HAL_SSD1306_H_

/** @file hal_ssd1306.h
 ** @author Emiliano Hatim (emilianohatim01@gmail.com)
 ** @brief Declaraciones de la biblioteca para graficar en la pantalla OLED
 ** @version 1.0
 * @date 2026-07
 * @copyright Copyright (c) 2026
 **/

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/* === Public data type declarations =============================================================================== */

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

void hal_ssd1306_init(void);

void hal_ssd1306_clear(void);

void hal_ssd1306_draw_string(uint8_t x, uint8_t page_y, const char * str);

/** @brief Enciende un pixel individual. x: 0-127, y: 0-63 (coordenada real, no pagina) */
void hal_ssd1306_draw_pixel(uint8_t x, uint8_t y);

/** @brief Dibuja el contorno (marco vacio) de un rectangulo */
void hal_ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/** @brief Rectangulo solido (relleno) */
void hal_ssd1306_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

void hal_ssd1306_update(void);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* HAL_SSD1306_H_ */
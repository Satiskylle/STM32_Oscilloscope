/*
*	==========================================================================
*   lcd_fonts.h	
*   (c) 2019, Michal Berdzik, Damian Chorazy
*		Based on Petr Machala and Tilen MAJERLE  <https://stm32f4-discovery.net> codes
*		Modified to work with HAL libraries
*
*   Description:
*   Fonts library used in LCD
*   Optimized for 32F429IDISCOVERY board.
*
*	==========================================================================
*/

#ifndef LCD_FONTS_H
#define LCD_FONTS_H
 
// Includes
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Font struct
typedef struct {
	uint8_t FontWidth;
	uint8_t FontHeight;
	const uint16_t *data;
} LCD_FontDef_t;

extern LCD_FontDef_t LCD_Font_7x10;
extern LCD_FontDef_t LCD_Font_11x18;
extern LCD_FontDef_t LCD_Font_16x26;
#endif

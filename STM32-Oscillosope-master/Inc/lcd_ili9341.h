/*
*	==========================================================================
*   lcd_ili9341.h	
*   (c) 2019, Michal Berdzik, Damian Chorazy
*		Based on Petr Machala and Tilen MAJERLE  <https://stm32f4-discovery.net> codes
*		Modified to work with HAL libraries
*
*   Description:
*   ILI9341 control file for STM32F4xx with SPI communication only.
*   Optimized for 32F429IDISCOVERY board.
*
*	==========================================================================
*/
#ifndef _LCD_ILI9341_H_
#define _LCD_ILI9341_H_

// Includes
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "lcd_spi.h"
#include "stm32f4xx_hal.h"
#include "lcd_fonts.h"

// LCD settings
#define ILI9341_WIDTH 				240
#define ILI9341_HEIGHT				320
#define ILI9341_PIXEL				76800
 
// Colors
#define ILI9341_COLOR_WHITE			0xFFFF
#define ILI9341_COLOR_BLACK			0x0000
#define ILI9341_COLOR_RED				0xF800
#define ILI9341_COLOR_GREEN			0x07E0
#define ILI9341_COLOR_GREEN2		0xB723
#define ILI9341_COLOR_BLUE			0x001F
#define ILI9341_COLOR_BLUE2			0x051D
#define ILI9341_COLOR_YELLOW		0xFFE0
#define ILI9341_COLOR_ORANGE		0xFBE4
#define ILI9341_COLOR_CYAN			0x07FF
#define ILI9341_COLOR_MAGENTA		0xA254
#define ILI9341_COLOR_GRAY			0x7BEF
#define ILI9341_COLOR_BROWN			0xBBCA

// Commands
#define ILI9341_RESET					0x01
#define ILI9341_SLEEP_OUT			0x11
#define ILI9341_GAMMA					0x26
#define ILI9341_DISPLAY_OFF		0x28
#define ILI9341_DISPLAY_ON		0x29
#define ILI9341_COLUMN_ADDR		0x2A
#define ILI9341_PAGE_ADDR			0x2B
#define ILI9341_GRAM					0x2C
#define ILI9341_MAC						0x36
#define ILI9341_PIXEL_FORMAT	0x3A
#define ILI9341_WDB						0x51
#define ILI9341_WCD						0x53
#define ILI9341_RGB_INTERFACE	0xB0
#define ILI9341_FRC						0xB1
#define ILI9341_BPC						0xB5
#define ILI9341_DFC						0xB6
#define ILI9341_POWER1				0xC0
#define ILI9341_POWER2				0xC1
#define ILI9341_VCOM1					0xC5
#define ILI9341_VCOM2					0xC7
#define ILI9341_POWERA				0xCB
#define ILI9341_POWERB				0xCF
#define ILI9341_PGAMMA				0xE0
#define ILI9341_NGAMMA				0xE1
#define ILI9341_DTCA					0xE8
#define ILI9341_DTCB					0xEA
#define ILI9341_POWER_SEQ			0xED
#define ILI9341_3GAMMA_EN			0xF2
#define ILI9341_INTERFACE			0xF6
#define ILI9341_PRC						0xF7

// Select orientation for LCD
typedef enum {
	LCD_Orientation_Portrait_1,
	LCD_Orientation_Portrait_2,
	LCD_Orientation_Landscape_1,
	LCD_Orientation_Landscape_2
} LCD_Orientation_t;

// Orientation, Used private
typedef enum {
	LCD_Landscape,
	LCD_Portrait
} LCD_Orientation;

// LCD options, Used private
typedef struct {
	uint16_t width;
	uint16_t height;
	LCD_Orientation orientation; // 1 = portrait; 0 = landscape
} LCD_Options_t;

/*
*		- Initialization of ILI9341 LCD
*/
extern void LCD_Init(void);

/*
*		- Send data to LCD via SPI
*
* Parameters:
* 	- uint8_t data: data to be sent
*		
*	Returns:
*		- none
*/
extern void LCD_SendData(uint8_t data);

/*
*		- Send command to LCD via SPI
*
* Parameters:
* 	- uint8_t data: data to be sent
*		
*	Returns:
*		- none
*/
extern void LCD_SendCmd(uint8_t data);

/*
*		- Simple delay
*
* Parameters:
* 	- volatile uint32_t delay: clock cycles
*		
*	Returns:
*		- none
*/
extern void LCD_Delay(volatile uint32_t delay);

/*
*		- Set cursor position
*
* Parameters:
* 	- uint16_t x1,x2: min and max position of X
* 	- uint16_t y1,y2: min and max position of Y
*		
*	Returns:
*		- none
*/
extern void LCD_SetCursorPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/*
*		- Fill screen with given color
*
* Parameters:
* 	- uint16_t color: which color should be used to fill screen
*		
*	Returns:
*		- none
*/
extern void LCD_Fill(uint16_t color);

/*
*		- Display image from camera
*
* Parameters:
* 	- uint16_t image[ILI9341_PIXEL]: table with image pixels
*		
*	Returns:
*		- none
*/
extern void LCD_DisplayImage(uint16_t image[ILI9341_PIXEL]);

/*
*		- Rotate LCD orientation
*
* Parameters:
* 	- LCD_Orientation_t orientation: landscape/portrait orientation
*		
*	Returns:
*		- none
*/
extern void LCD_Rotate(LCD_Orientation_t orientation);

/*
*		- Draw color pixel at (x,y) coords
*
* Parameters:
* 	- uint16_t x,y: coordinates
* 	- uint16_t color: color of pixel
*		
*	Returns:
*		- none
*/
extern void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
	
/*
*		- Display string at (x,y) coords
*
* Parameters:
* 	- uint16_t x,y: coordinates
* 	- uint16_t foreground: color of font
*   - char *str: string to display
*		- LCD_FontDef_t *font: which type of font to use
*		
*	Returns:
*		- none
*/
extern void LCD_Puts(uint16_t x, uint16_t y, char *str, LCD_FontDef_t *font, uint16_t foreground);

extern void LCD_GetStringSize(char *str, LCD_FontDef_t *font, uint16_t *width, uint16_t *height);

/*
*		- Display character at (x,y) coords
*
* Parameters:
* 	- uint16_t x,y: coordinates
* 	- uint16_t foreground: color of font
*   - char c: character to display
*		- LCD_FontDef_t *font: which type of font to use
*		
*	Returns:
*		- none
*/
extern void LCD_Putc(uint16_t x, uint16_t y, char c, LCD_FontDef_t *font, uint16_t foreground);

/*
*		- Draw line at screen
*
* Parameters:
* 	- uint16_t x0,y0: start coordinates
* 	- uint16_t x1,y1: end coordinates
* 	- uint16_t color: color of line
*		
*	Returns:
*		- none
*/
extern void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/*
*		- Draw empty rectangle at screen
*
* Parameters:
* 	- uint16_t x0,y0: start coordinates
* 	- uint16_t x1,y1: end coordinates
* 	- uint16_t color: color of line
*		
*	Returns:
*		- none
*/
extern void LCD_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/*
*		- Draw filled rectangle at screen
*
* Parameters:
* 	- uint16_t x0,y0: start coordinates
* 	- uint16_t x1,y1: end coordinates
* 	- uint16_t color: color of rectangle
*		
*	Returns:
*		- none
*/
extern void LCD_DrawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/*
*		- Draw empty circle at screen
*
* Parameters:
* 	- uint16_t x0,y0: center coordinates
* 	- uint16_t r: radius of circle
* 	- uint16_t color: color of line
*		
*	Returns:
*		- none
*/
extern void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/*
*		- Draw filled circle at screen
*
* Parameters:
* 	- uint16_t x0,y0: center coordinates
* 	- uint16_t r: radius of circle
* 	- uint16_t color: color of line
*		
*	Returns:
*		- none
*/
extern void LCD_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
#endif

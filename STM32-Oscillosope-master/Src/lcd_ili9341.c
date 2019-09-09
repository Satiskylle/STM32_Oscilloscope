/*
*	==========================================================================
*   lcd_ili9341.c	
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
#include "lcd_ili9341.h"

#define LCD_CS_SET					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET)
#define LCD_CS_RESET				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET)
#define LCD_WRX_SET					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET)
#define LCD_WRX_RESET				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET)
#define LCD_RST_SET					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET)
#define LCD_RST_RESET				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET)

uint16_t ILI9341_x;
uint16_t ILI9341_y;
LCD_Options_t ILI9341_Opts;
uint8_t ILI9341_INT_CalledFromPuts = 0;

void LCD_Init(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//Data/Cmd pin WRX -> PD13
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	__GPIOD_CLK_ENABLE();		//Init of clock
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	//NSS/CS pin -> PC2
	__GPIOC_CLK_ENABLE();		//Init of clock
	
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	//RST pin -> PD12
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	LCD_CS_SET;
	
	LCD_SPI_Init();
	
	LCD_RST_RESET;
	LCD_Delay(20000);
	LCD_RST_SET;
	LCD_Delay(20000);
	
	LCD_SendCmd(ILI9341_RESET);
	LCD_Delay(50000);
	
	LCD_SendCmd(ILI9341_POWERA);
	LCD_SendData(0x39);
	LCD_SendData(0x2C);
	LCD_SendData(0x00);
	LCD_SendData(0x34);
	LCD_SendData(0x02);
	LCD_SendCmd(ILI9341_POWERB);
	LCD_SendData(0x00);
	LCD_SendData(0xC1);
	LCD_SendData(0x30);
	LCD_SendCmd(ILI9341_DTCA);
	LCD_SendData(0x85);
	LCD_SendData(0x00);
	LCD_SendData(0x78);
	LCD_SendCmd(ILI9341_DTCB);
	LCD_SendData(0x00);
	LCD_SendData(0x00);
	LCD_SendCmd(ILI9341_POWER_SEQ);
	LCD_SendData(0x64);
	LCD_SendData(0x03);
	LCD_SendData(0x12);
	LCD_SendData(0x81);
	LCD_SendCmd(ILI9341_PRC);
	LCD_SendData(0x20);
	LCD_SendCmd(ILI9341_POWER1);
	LCD_SendData(0x23);
	LCD_SendCmd(ILI9341_POWER2);
	LCD_SendData(0x10);
	LCD_SendCmd(ILI9341_VCOM1);
	LCD_SendData(0x3E);
	LCD_SendData(0x28);
	LCD_SendCmd(ILI9341_VCOM2);
	LCD_SendData(0x86);
	LCD_SendCmd(ILI9341_MAC);
	LCD_SendData(0x48);
	LCD_SendCmd(ILI9341_PIXEL_FORMAT);
	LCD_SendData(0x55);
	LCD_SendCmd(ILI9341_FRC);
	LCD_SendData(0x00);
	LCD_SendData(0x18);
	LCD_SendCmd(ILI9341_DFC);
	LCD_SendData(0x08);
	LCD_SendData(0x82);
	LCD_SendData(0x27);
	LCD_SendCmd(ILI9341_3GAMMA_EN);
	LCD_SendData(0x00);
	LCD_SendCmd(ILI9341_COLUMN_ADDR);
	LCD_SendData(0x00);
	LCD_SendData(0x00);
	LCD_SendData(0x00);
	LCD_SendData(0xEF);
	LCD_SendCmd(ILI9341_PAGE_ADDR);
	LCD_SendData(0x00);
	LCD_SendData(0x00);
	LCD_SendData(0x01);
	LCD_SendData(0x3F);
	LCD_SendCmd(ILI9341_GAMMA);
	LCD_SendData(0x01);
	LCD_SendCmd(ILI9341_PGAMMA);
	LCD_SendData(0x0F);
	LCD_SendData(0x31);
	LCD_SendData(0x2B);
	LCD_SendData(0x0C);
	LCD_SendData(0x0E);
	LCD_SendData(0x08);
	LCD_SendData(0x4E);
	LCD_SendData(0xF1);
	LCD_SendData(0x37);
	LCD_SendData(0x07);
	LCD_SendData(0x10);
	LCD_SendData(0x03);
	LCD_SendData(0x0E);
	LCD_SendData(0x09);
	LCD_SendData(0x00);
	LCD_SendCmd(ILI9341_NGAMMA);
	LCD_SendData(0x00);
	LCD_SendData(0x0E);
	LCD_SendData(0x14);
	LCD_SendData(0x03);
	LCD_SendData(0x11);
	LCD_SendData(0x07);
	LCD_SendData(0x31);
	LCD_SendData(0xC1);
	LCD_SendData(0x48);
	LCD_SendData(0x08);
	LCD_SendData(0x0F);
	LCD_SendData(0x0C);
	LCD_SendData(0x31);
	LCD_SendData(0x36);
	LCD_SendData(0x0F);
	LCD_SendCmd(ILI9341_SLEEP_OUT);

	LCD_Delay(1000000);

	LCD_SendCmd(ILI9341_DISPLAY_ON);
	LCD_SendCmd(ILI9341_GRAM);
	
	ILI9341_x = ILI9341_y = 0;
	
	ILI9341_Opts.width = ILI9341_WIDTH;
	ILI9341_Opts.height = ILI9341_HEIGHT;
	ILI9341_Opts.orientation = LCD_Portrait;
	
	LCD_Fill(ILI9341_COLOR_WHITE);
}

void LCD_SendData(uint8_t data){
	LCD_WRX_SET;
	LCD_CS_RESET;
	LCD_SPI_Send(data);
	LCD_CS_SET;
}

void LCD_SendCmd(uint8_t data){
	LCD_WRX_RESET;
	LCD_CS_RESET;
	LCD_SPI_Send(data);
	LCD_CS_SET;
}

void LCD_Delay(volatile uint32_t delay){
	for (; delay != 0; delay--);
}

void LCD_SetCursorPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	LCD_SendCmd(ILI9341_COLUMN_ADDR);
	LCD_SendData(x1 >> 8);
	LCD_SendData(x1 & 0xFF);
	LCD_SendData(x2 >> 8);
	LCD_SendData(x2 & 0xFF);

	LCD_SendCmd(ILI9341_PAGE_ADDR);
	LCD_SendData(y1 >> 8);
	LCD_SendData(y1 & 0xFF);
	LCD_SendData(y2 >> 8);
	LCD_SendData(y2 & 0xFF);
}

void LCD_Fill(uint16_t color) {
	unsigned int n, i, j;
	i = color >> 8;
	j = color & 0xFF;
	LCD_SetCursorPosition(0, 0, ILI9341_Opts.width - 1, ILI9341_Opts.height - 1);

	LCD_SendCmd(ILI9341_GRAM);

	LCD_CS_RESET;
	LCD_WRX_SET;
	
	for (n = 0; n < ILI9341_PIXEL; n++) {
		LCD_SPI_Send(i);
		LCD_SPI_Send(j);
	}
	
	LCD_CS_SET;
}

void LCD_DisplayImage(uint16_t image[ILI9341_PIXEL]) {
	uint32_t n, i, j;
	
	LCD_SetCursorPosition(0, 0, ILI9341_Opts.width - 1, ILI9341_Opts.height - 1);

	LCD_SendCmd(ILI9341_GRAM);

	LCD_WRX_SET;
	LCD_CS_RESET;
	
	for (n = 0; n < ILI9341_PIXEL; n++) {
		i = image[n] >> 8;
		j = image[n] & 0xFF;
				
		LCD_SPI_Send(i);
		LCD_SPI_Send(j);
	}
	
	LCD_CS_SET;
}

void LCD_Rotate(LCD_Orientation_t orientation) {
	LCD_SendCmd(ILI9341_MAC);
	if (orientation == LCD_Orientation_Portrait_1) {
		LCD_SendData(0x58);
	} else if (orientation == LCD_Orientation_Portrait_2) {
		LCD_SendData(0x88);
	} else if (orientation == LCD_Orientation_Landscape_1) {
		LCD_SendData(0x28);
	} else if (orientation == LCD_Orientation_Landscape_2) {
		LCD_SendData(0xE8);
	}
	
	if (orientation == LCD_Orientation_Portrait_1 || orientation == LCD_Orientation_Portrait_2) {
		ILI9341_Opts.width = ILI9341_WIDTH;
		ILI9341_Opts.height = ILI9341_HEIGHT;
		ILI9341_Opts.orientation = LCD_Portrait;
	} else {
		ILI9341_Opts.width = ILI9341_HEIGHT;
		ILI9341_Opts.height = ILI9341_WIDTH;
		ILI9341_Opts.orientation = LCD_Landscape;
	}
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	LCD_SetCursorPosition(x, y, x, y);

	LCD_SendCmd(ILI9341_GRAM);
	LCD_SendData(color >> 8);
	LCD_SendData(color & 0xFF);
}

void LCD_Puts(uint16_t x, uint16_t y, char *str, LCD_FontDef_t *font, uint16_t foreground) {
	uint16_t startX = x;
	
	/* Set X and Y coordinates */
	ILI9341_x = x;
	ILI9341_y = y;
	
	while (*str) {
		//New line
		if (*str == '\n') {
			ILI9341_y += font->FontHeight + 1;
			//if after \n is also \r, than go to the left of the screen
			if (*(str + 1) == '\r') {
				ILI9341_x = 0;
				str++;
			} else {
				ILI9341_x = startX;
			}
			str++;
			continue;
		} else if (*str == '\r') {
			str++;
			continue;
		}
		
		LCD_Putc(ILI9341_x, ILI9341_y, *str++, font, foreground);
	}
}

void LCD_GetStringSize(char *str, LCD_FontDef_t *font, uint16_t *width, uint16_t *height) {
	uint16_t w = 0;
	*height = font->FontHeight;
	while (*str++) {
		w += font->FontWidth;
	}
	*width = w;
}

void LCD_Putc(uint16_t x, uint16_t y, char c, LCD_FontDef_t *font, uint16_t foreground) {
	uint32_t i, b, j;
	/* Set coordinates */
	ILI9341_x = x;
	ILI9341_y = y;
	if ((ILI9341_x + font->FontWidth) > ILI9341_Opts.width) {
		//If at the end of a line of display, go to new line and set x to 0 position
		ILI9341_y += font->FontHeight;
		ILI9341_x = 0;
	}
	
	/* Draw font data */
	for (i = 0; i < font->FontHeight; i++) {
		b = font->data[(c - 32) * font->FontHeight + i];
		for (j = 0; j < font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				LCD_DrawPixel(ILI9341_x + j, (ILI9341_y + i), foreground);
			}
		}
	}
	ILI9341_x += font->FontWidth;
}


void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	/* Code by dewoller: https://github.com/dewoller */
	
	int16_t dx, dy, sx, sy, err, e2; 
	
	/* Check for overflow */
	if (x0 >= ILI9341_Opts.width) {
		x0 = ILI9341_Opts.width - 1;
	}
	if (x1 >= ILI9341_Opts.width) {
		x1 = ILI9341_Opts.width - 1;
	}
	if (y0 >= ILI9341_Opts.height) {
		y0 = ILI9341_Opts.height - 1;
	}
	if (y1 >= ILI9341_Opts.height) {
		y1 = ILI9341_Opts.height - 1;
	}
	
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
	sx = (x0 < x1) ? 1 : -1; 
	sy = (y0 < y1) ? 1 : -1; 
	err = ((dx > dy) ? dx : -dy) / 2; 

	while (1) {
		LCD_DrawPixel(x0, y0, color); 
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err; 
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		} 
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		} 
	}
}

void LCD_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	LCD_DrawLine(x0, y0, x1, y0, color); //Top
	LCD_DrawLine(x0, y0, x0, y1, color);	//Left
	LCD_DrawLine(x1, y0, x1, y1, color);	//Right
	LCD_DrawLine(x0, y1, x1, y1, color);	//Bottom
}

void LCD_DrawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	for (; y0 < y1; y0++) {
		LCD_DrawLine(x0, y0, x1, y0, color);
	}
}


void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    LCD_DrawPixel(x0, y0 + r, color);
    LCD_DrawPixel(x0, y0 - r, color);
    LCD_DrawPixel(x0 + r, y0, color);
    LCD_DrawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        LCD_DrawPixel(x0 + x, y0 + y, color);
        LCD_DrawPixel(x0 - x, y0 + y, color);
        LCD_DrawPixel(x0 + x, y0 - y, color);
        LCD_DrawPixel(x0 - x, y0 - y, color);

        LCD_DrawPixel(x0 + y, y0 + x, color);
        LCD_DrawPixel(x0 - y, y0 + x, color);
        LCD_DrawPixel(x0 + y, y0 - x, color);
        LCD_DrawPixel(x0 - y, y0 - x, color);
    }
}

void LCD_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    LCD_DrawPixel(x0, y0 + r, color);
    LCD_DrawPixel(x0, y0 - r, color);
    LCD_DrawPixel(x0 + r, y0, color);
    LCD_DrawPixel(x0 - r, y0, color);
    LCD_DrawLine(x0 - r, y0, x0 + r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        LCD_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
        LCD_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, color);

        LCD_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, color);
        LCD_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, color);
    }
}  


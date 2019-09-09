/*
*	==========================================================================
*   lcd_spi.h
*   (c) 2019, Michal Berdzik, Damian Chorazy
*
*   Description:
*   SPI communication for LCD
*
*	==========================================================================
*/
#ifndef _LCD_SPI_H_
#define _LCD_SPI_H_

// Includes
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stm32f4xx_hal.h"

/*
*		- Initialization of SPI5 for controlling LCD
*
* Used pinpack 1:
* 	- PF9 -> MOSI
*		- PF8 -> MISO
*		- PF7 -> SCK
*/
extern void LCD_SPI_Init(void);

/*
*		- Send and recive data over SPI5
*
* Parameters:
* 	- uint8_t data: data to be sent via SPI
*		
*	Returns:
*		- uint8_t: data recived from slave
*/
extern uint8_t LCD_SPI_Send(uint8_t data);

/*
*		- Increase baudrate of SPI
*
* Parameters:
* 	- none
*		
*	Returns:
*		- none
*/
extern void LCD_SPI_IncBaudrate(void);

#endif

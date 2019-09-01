/**
*	==========================================================================
*   @file 	lcd_spi.h
*   @note	(c) 2019, Michal Berdzik, Damian Chorazy
*   @note	Description:
*   @note	SPI communication for LCD
*	==========================================================================
*/

//----------------------------------------------------------------------------

#ifndef _LCD_SPI_H_
#define _LCD_SPI_H_

//----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stm32f4xx_hal.h"

/**
 * @brief	Initialization of SPI5 for controlling LCD
 * @note	PF9 -> MOSI
 * @note	PF8 -> MISO
 * @note	PF7 -> SCK
 */
extern void LCD_SPI_Init(void);

/**
 * @brief	Send and recive data over SPI5
 * @note	Parameters:
 * @param	data - data to be sent via SPI
 * @return  uint8_t - data recived from slave
 */
extern uint8_t LCD_SPI_Send(uint8_t data);

/**
 * @brief	Increase baudrate of SPI
 * @param	none
 * @return	none
 */
extern void LCD_SPI_IncBaudrate(void);

#endif

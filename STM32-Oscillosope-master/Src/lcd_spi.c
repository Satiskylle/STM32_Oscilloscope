/*
*	==========================================================================
*   lcd_spi.c	
*   (c) 2019, Michal Berdzik, Damian Chorazy
*
*   Description:
*   SPI communication for LCD
*
*	==========================================================================
*/
#include "lcd_spi.h"

static 	SPI_HandleTypeDef SPIhandle;

void LCD_SPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//Init SPI
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
	
	__GPIOF_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	//Init SPI
	__SPI5_CLK_ENABLE();
	
	SPIhandle.Instance = SPI5;
	
	SPIhandle.Init.DataSize = SPI_DATASIZE_8BIT;
	SPIhandle.Init.Direction = SPI_DIRECTION_2LINES;
	SPIhandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
	SPIhandle.Init.Mode= SPI_MODE_MASTER;
	SPIhandle.Init.CLKPolarity = SPI_POLARITY_LOW;
	SPIhandle.Init.CLKPhase = SPI_PHASE_1EDGE;
	SPIhandle.Init.NSS = SPI_NSS_SOFT;
	
	HAL_SPI_Init(&SPIhandle);	
	__HAL_SPI_ENABLE(&SPIhandle);
}



uint8_t LCD_SPI_Send(uint8_t data)
{
	SPI5->DR = data;
	
	while(!(SPI5->SR & SPI_SR_TXE))
	{
		;
	}
	while(!(SPI5->SR & SPI_SR_RXNE))
	{
		;
	}
	while((SPI5->SR & SPI_SR_BSY))
	{
		;
	}
		
	return SPI5->DR;
}

void LCD_SPI_IncBaudrate(void)
{
	__HAL_SPI_DISABLE(&SPIhandle);
	
	SPIhandle.Instance = SPI5;
	
	SPIhandle.Init.DataSize = SPI_DATASIZE_8BIT;
	SPIhandle.Init.Direction = SPI_DIRECTION_2LINES;
	SPIhandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
	SPIhandle.Init.Mode= SPI_MODE_MASTER;
	SPIhandle.Init.CLKPolarity = SPI_POLARITY_LOW;
	SPIhandle.Init.CLKPhase = SPI_PHASE_1EDGE;
	SPIhandle.Init.NSS = SPI_NSS_SOFT;
	
	HAL_SPI_Init(&SPIhandle);
	__HAL_SPI_ENABLE(&SPIhandle);
}

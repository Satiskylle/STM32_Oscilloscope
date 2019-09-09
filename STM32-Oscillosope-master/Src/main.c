/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

//------------------------------------------------------------------------------

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"

#include "lcd_ili9341.h"
#include "lcd_fonts.h"
#include "lcd_spi.h"
#include "FullBuffers.h"

#include <math.h>

//------------------------------------------------------------------------------

#define xGridOffset 			(30)
#define yGridOffset 			(25)
#define ANALOG_MAX_VALUE 		(4096)
#define TRIGGER_POSITION_STEP 	(ANALOG_MAX_VALUE/64)
#define maxSamples 				(1024*20)

//------------------------------------------------------------------------------

enum
{
	_1uV,
	_2uV,
	_10uV,
	_20uV,
	_100uV,
	_200uV,
	_1mV,
	_2mV,
	_10mV,
	_20mV,
	_100mV,
	_200mV,
	_1V,
	_2V,
} VolPerDiv = _1V;

enum
{
	_1us,
	_2us,
	_10us,
	_20us,
	_100us,
	_200us,
	_1ms,
	_2ms,
	_10ms,
	_20ms,
	_100ms,
	_200ms,
	_1s,
	_2s,
} TimePerDiv = _1s;

enum
{
	TRIGGER, TRIGGER_TYPE, TIME_BASE, VOL_DIV
} CurrentMode = TRIGGER;

//------------------------------------------------------------------------------

float VolPerDivVal[14] = { 0.001, 0.002, 0.01, 0.02, 0.1, 0.2, 1, 2, 10, 20, 100, 200, 1000, 2000 };
float TimePerDivVal[14] = { 0.000001, 0.000002, 0.00001, 0.00002, 0.0001, 0.0002, 0.001, 0.002, 0.01, 0.02, 0.1, 0.2, 1, 2 };

uint32_t samplingFreq = 2000000;

uint16_t ADCvalue = 0;
uint16_t MidYPos = (ILI9341_WIDTH - 2) / 2; //Middle of LCD
uint16_t MidXPos = (ILI9341_HEIGHT - 2) / 2; //Middle of LCD
uint16_t xPos;
uint16_t yPos;

float samplingTime = 0;
float displayTime = 0;
int16_t yPosition = 0;

/* Zoom variables -> used for V/div scaling */
int16_t voltageValue = 0;
int16_t xZoomFactor = 1;
int16_t yZoomFactor = 100;

/* Trigger */
int32_t triggerValue = 2048;
bool triggerHeld = 0;
bool notTriggered;

int16_t retriggerDelay = 0;
int8_t triggerType = 2; //0-both 1-negative 2-positive
uint16_t triggerPoints[2]; //Array for trigger points

/* Timebase */
unsigned long timeBase = 50;  //Timebase in microseconds
unsigned long sweepDelayFactor = 1;

/* ADC Data */
uint32_t dataPoints32[maxSamples]; // Array for the ADC data
uint16_t *dataPoints = (uint16_t *) &dataPoints32;

/* Array for computed data (speedup) */
uint16_t dataPlot[320]; //max (width, height) for this display

/* Variables for the beam position */
uint32_t signalX;
uint32_t signalY;
uint32_t signalY1;
uint32_t startSample = 0; //10
uint32_t endSample = maxSamples;
bool dma_complete;

FullDoubleBuffer Buffer;
int8_t showData = 0;

//------------------------------------------------------------------------------

void SystemClock_Config(void);

void decreaseTimebase()
{
  if (timeBase > 100)
	  timeBase -= 100;

  showTrace();
}

void increaseTimebase()
{
  if (timeBase < 10000)
  {
	  timeBase = timeBase * 2;
  }
  else
  {
	  timeBase = 50;
  }
}

void increaseZoomFactor()
{
  if ( yZoomFactor < 100000)
  {
	  yZoomFactor += 100;
  }
  else
  {
	  yZoomFactor = 1;
  }
}

void decreaseZoomFactor()
{
  if (xZoomFactor > 1) {
    xZoomFactor -= 1;
  }
}

void clearTrace()
{
  TFTSamplesClear();
  drawGUI();
  DrawInfo();
}

void showTrace()
{
  TFTSamples();
}

void increaseTriggerPosition()
{
  if (triggerValue < ANALOG_MAX_VALUE )
  {
    triggerValue += TRIGGER_POSITION_STEP;
  }
  else
  {
	  triggerValue = 0;
  }
}

void decreaseTriggerPosition()
{
  if (triggerValue > 0 )
  {
    triggerValue -= TRIGGER_POSITION_STEP;
  }
}

void changeMode() {
	switch(CurrentMode)
	{
	case TRIGGER:
		CurrentMode = TRIGGER_TYPE;
		break;
	case TRIGGER_TYPE:
		CurrentMode = TIME_BASE;
		break;
	case TIME_BASE:
		CurrentMode = VOL_DIV;
		break;
	case VOL_DIV:
		CurrentMode = TRIGGER;
		break;
	default:
		CurrentMode = TRIGGER;
		break;
	}
	clearTrace();
	showTrace();
}

void changeValue()
{
	switch(CurrentMode)
	{
	case TRIGGER:
		increaseTriggerPosition();
		break;
	case TRIGGER_TYPE:
		incEdgeType();
		break;
	case TIME_BASE:
		increaseTimebase();
		break;
	case VOL_DIV:
		increaseZoomFactor();
		break;
	default:
		break;
	}
	clearTrace();
	showTrace();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == Mode_Pin)
	{
		if (HAL_GPIO_ReadPin(Mode_GPIO_Port, Mode_Pin) == GPIO_PIN_RESET)
		{
			for (int i = 0; i < 100000; i++)
				asm("NOP");
			if (HAL_GPIO_ReadPin(Mode_GPIO_Port, Mode_Pin) == GPIO_PIN_RESET)
				changeMode();
		}
	}
	else if (GPIO_Pin == Value_Pin)
	{
		if (HAL_GPIO_ReadPin(Value_GPIO_Port, Value_Pin) == GPIO_PIN_RESET)
		{
			for (int i = 0; i < 100000; i++)
				asm("NOP");
			if (HAL_GPIO_ReadPin(Value_GPIO_Port, Value_Pin) == GPIO_PIN_RESET)
			{
				changeValue();
			}
		}
	}
}

void triggerBoth()
{
	uint16_t posX;
	uint16_t posTMin, posTMax;

	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	{
		triggerPoints[0] = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Start(&hadc1);
	}

	while (notTriggered)
	{
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
		{
			triggerPoints[1] = HAL_ADC_GetValue(&hadc1);
			HAL_ADC_Start(&hadc1);
		}

		if ( ((triggerPoints[1] < triggerValue) && (triggerPoints[0] > triggerValue)) || ((triggerPoints[1] > triggerValue) && (triggerPoints[0] < triggerValue)) )
		{
			/* Tasiemiec... Do zmiany */
			posX =  (((ILI9341_WIDTH / 4.9) * triggerPoints[1]) / ANALOG_MAX_VALUE) * yZoomFactor / 100 + (120 - ((yZoomFactor * (ILI9341_WIDTH / 4.9) - yZoomFactor) / 200)) + yPosition;

			posTMax = ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 1;
			posTMin = ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 1;

			if(posX <= posTMin && posX >= posTMax)
				notTriggered = false;
		}

		triggerPoints[0] = triggerPoints[1];
	}
}

void triggerPositive()
{
	uint16_t posX;
	uint16_t posTMin, posTMax;

	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	{
		triggerPoints[0] = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Start(&hadc1);
	}

	while (notTriggered)
	{
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
		{
			triggerPoints[1] = HAL_ADC_GetValue(&hadc1);
			HAL_ADC_Start(&hadc1);
		}

		if ((triggerPoints[1] > triggerValue) && (triggerPoints[0] < triggerValue) )
		{
			posX =  (((ILI9341_WIDTH / 4.9) * triggerPoints[1]) / ANALOG_MAX_VALUE) * yZoomFactor / 100 + (120 - ((yZoomFactor * (ILI9341_WIDTH / 4.9) - yZoomFactor) / 200)) + yPosition;

			posTMax = ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 1;
			posTMin = ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 1;

			if(posX <= posTMin && posX >= posTMax)
				notTriggered = false;
		}

    triggerPoints[0] = triggerPoints[1];
	}
}

void triggerNegative()
{
	uint16_t posX;
	uint16_t posTMin, posTMax;

	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	{
		triggerPoints[0] = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Start(&hadc1);
	}

	while(notTriggered)
	{
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
		{
			triggerPoints[1] = HAL_ADC_GetValue(&hadc1);
			HAL_ADC_Start(&hadc1);
		}

		if ((triggerPoints[1] < triggerValue) && (triggerPoints[0] > triggerValue) )
		{
			posX =  (((ILI9341_WIDTH / 4.9) * triggerPoints[1]) / ANALOG_MAX_VALUE) * yZoomFactor / 100 + (120 - ((yZoomFactor * (ILI9341_WIDTH / 4.9) - yZoomFactor) / 200)) + yPosition;
			posTMax = ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 1;
			posTMin = ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 1;
			if(posX <= posTMin && posX >= posTMax)
			notTriggered = false;
		}

		triggerPoints[0] = triggerPoints[1];
	}
}

/* Crude triggering on positive or negative or either change from previous to current sample. */
void trigger()
{
	notTriggered = true;

	switch (triggerType)
	{
	case 1:
		triggerNegative();
		break;
	case 2:
		triggerPositive();
		break;
	default:
		triggerBoth();
		break;
	}
}

void incEdgeType()
{
	triggerType += 1;

	if (triggerType > 2)
		triggerType = 0;
}

void DrawInfo()
{
	char buffer[30];

	switch(CurrentMode)
	{

	case TRIGGER:

		//Time base
		itoa(10000/timeBase,buffer,10);
		LCD_Puts(71, 2,buffer , &LCD_Font_7x10, ILI9341_COLOR_WHITE);

		//Trigger
		itoa(100000 / yZoomFactor,buffer,10);
		LCD_Puts(71, 12,buffer , &LCD_Font_7x10, ILI9341_COLOR_WHITE);
		LCD_DrawFilledRectangle(0, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 2, 4, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 2, ILI9341_COLOR_GREEN);

		switch (triggerType)
		{
		case 1:
			LCD_Puts(260, 2, "N", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		case 2:
			LCD_Puts(260, 2, "P", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		default:
			LCD_Puts(260, 2, "B", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		}
		break;


	case TRIGGER_TYPE:

		//Time base
		itoa(10000 / timeBase, buffer, 10);
		LCD_Puts(71, 2, buffer, &LCD_Font_7x10, ILI9341_COLOR_WHITE);

		//Trigger
		itoa(100000 / yZoomFactor, buffer, 10);
		LCD_Puts(71, 12, buffer, &LCD_Font_7x10, ILI9341_COLOR_WHITE);
		LCD_DrawFilledRectangle(0,
		ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 2, 4,
		ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 2,
		ILI9341_COLOR_YELLOW);

		switch (triggerType)
		{
		case 1:
			LCD_Puts(260, 2, "N", &LCD_Font_7x10, ILI9341_COLOR_GREEN);
			break;
		case 2:
			LCD_Puts(260, 2, "P", &LCD_Font_7x10, ILI9341_COLOR_GREEN);
			break;
		default:
			LCD_Puts(260, 2, "B", &LCD_Font_7x10, ILI9341_COLOR_GREEN);
			break;
		}
			break;


	case TIME_BASE:

	  	//Time base
		itoa(10000 / timeBase, buffer, 10);
		LCD_Puts(71, 2, buffer, &LCD_Font_7x10, ILI9341_COLOR_GREEN);

		//Trigger
		itoa(100000 / yZoomFactor, buffer, 10);
		LCD_Puts(71, 12, buffer, &LCD_Font_7x10, ILI9341_COLOR_WHITE);
		LCD_DrawFilledRectangle(0, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 2, 4, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 2, ILI9341_COLOR_YELLOW);

		switch (triggerType)
		{
		case 1:
			LCD_Puts(260, 2, "N", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		case 2:
			LCD_Puts(260, 2, "P", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		default:
			LCD_Puts(260, 2, "B", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		}
	  		break;


	case VOL_DIV:

		//Time base
		itoa(10000 / timeBase, buffer, 10);
		LCD_Puts(71, 2, buffer, &LCD_Font_7x10, ILI9341_COLOR_WHITE);

		//Trigger
		itoa(100000 / yZoomFactor, buffer, 10);
		LCD_Puts(71, 12, buffer, &LCD_Font_7x10, ILI9341_COLOR_GREEN);

		LCD_DrawFilledRectangle(0, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 2, 4, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 2, ILI9341_COLOR_YELLOW);

		switch (triggerType)
		{
		case 1:
			LCD_Puts(260, 2, "N", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		case 2:
			LCD_Puts(260, 2, "P", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		default:
			LCD_Puts(260, 2, "B", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		}
		break;


	default:

		//Time base
		itoa(10000 / timeBase, buffer, 10);
		LCD_Puts(71, 2, buffer, &LCD_Font_7x10, ILI9341_COLOR_WHITE);

		//Trigger
		itoa(100000 / yZoomFactor, buffer, 10);

		LCD_Puts(71, 12, buffer, &LCD_Font_7x10, ILI9341_COLOR_WHITE);
		LCD_DrawFilledRectangle(0, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE - 2, 4, ILI9341_WIDTH * (triggerValue) / ANALOG_MAX_VALUE + 2, ILI9341_COLOR_YELLOW);

		switch (triggerType)
		{
		case 1:
			LCD_Puts(260, 2, "N", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		case 2:
			LCD_Puts(260, 2, "P", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		default:
			LCD_Puts(260, 2, "B", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
			break;
		}
		break;
	}
}

void TFTSamples ()
{
	//calculate first sample
	signalY =  (((ILI9341_WIDTH / 4.9) * dataPoints32[0]) / ANALOG_MAX_VALUE) * yZoomFactor / 100 + (120 - ((yZoomFactor * (ILI9341_WIDTH / 4.9) - yZoomFactor) / 200)) + yPosition;
	dataPlot[0]=signalY * 99 / 100 + 1;

	for (signalX=1 ; signalX < ILI9341_HEIGHT - 2; signalX++)
	{
		signalY1 = (((ILI9341_WIDTH / 4.9) * dataPoints32[(signalX)*(50 * (endSample - startSample)/(ILI9341_HEIGHT * timeBase) + 1)]) / ANALOG_MAX_VALUE) * yZoomFactor / 100 + (120 - ((yZoomFactor * (ILI9341_WIDTH / 4.9) - yZoomFactor) / 200)) + yPosition ;
		dataPlot[signalX] = signalY1 * 99 / 100 + 1;
		LCD_DrawLine(signalX, dataPlot[signalX-1], signalX + 1, dataPlot[signalX], ILI9341_COLOR_RED);
		signalY = signalY1;
	}

	DrawInfo();
}

void TFTSamplesClear ()
{
	for (signalX=1 ; signalX < ILI9341_HEIGHT - 2; signalX++)
	{
    //use saved data to improve speed
		LCD_DrawLine(signalX, dataPlot[signalX-1], signalX + 1, dataPlot[signalX], ILI9341_COLOR_BLACK);
	}
}

void takeSamples ()
{
// This loop uses dual interleaved mode to get the best performance out of the ADCs
//
//  dma_init(DMA1);
//  dma_attach_interrupt(DMA1, DMA_CH1, DMA1_CH1_Event);
//
//  adc_dma_enable(ADC1);
//  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS,
//                     dataPoints32, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
//  dma_set_num_transfers(DMA1, DMA_CH1, maxSamples / 2);
//  dma1_ch1_Active = 1;
//  //  regs->CR2 |= ADC_CR2_SWSTART; //moved to setADC
//  dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
//  //adc_calibrate(ADC1);
//  //adc_calibrate(ADC2);
//  samplingTime = micros();
//  while (dma1_ch1_Active);
//  samplingTime = (micros() - samplingTime);
//
//  dma_disable(DMA1, DMA_CH1); //End of trasfer, disable DMA and Continuous mode.
//  // regs->CR2 &= ~ADC_CR2_CONT;

	for(int i = 0; i < maxSamples; i++)
	{
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
		{
			dataPoints32[i] = HAL_ADC_GetValue(&hadc1);
			HAL_ADC_Start(&hadc1);
		}
//		dataPoints32[i] = 2048;
	}
}

void drawGUI(void)
{
	LCD_Fill(ILI9341_COLOR_BLACK);

	//Draw rectangle around screen
	LCD_DrawRectangle(0, 0, ILI9341_HEIGHT, ILI9341_WIDTH, ILI9341_COLOR_BLUE);

	// Dot grid - ten distinct divisions (9 dots) in both X and Y axis.
	for (uint16_t TicksX = 1; TicksX < 10; TicksX++)
	{
		for (uint16_t TicksY = 1; TicksY < 10; TicksY++)
	      LCD_DrawPixel(  TicksX * (ILI9341_HEIGHT / 10), TicksY * (ILI9341_WIDTH / 10), ILI9341_COLOR_BLUE);
	}

	// Horizontal and Vertical centre lines 5 ticks per grid square with a longer tick in line with our dots
	for (uint16_t TicksX = 0; TicksX < ILI9341_WIDTH; TicksX += (ILI9341_HEIGHT / 50))
	{
		if (TicksX % (ILI9341_WIDTH / 10) > 0 )
	    	LCD_DrawLine((ILI9341_HEIGHT / 2) - 1 , TicksX, (ILI9341_HEIGHT / 2) - 1 + 3, TicksX, ILI9341_COLOR_BLUE);
	    else
	    	LCD_DrawLine((ILI9341_HEIGHT / 2) - 3 , TicksX, (ILI9341_HEIGHT / 2) - 3 + 7, TicksX, ILI9341_COLOR_BLUE);
	}

	for (uint16_t TicksY = 0; TicksY < ILI9341_HEIGHT; TicksY += (ILI9341_HEIGHT / 50) )
	{
		if (TicksY % (ILI9341_HEIGHT / 10) > 0 )
	    	LCD_DrawLine( TicksY,  (ILI9341_WIDTH / 2) - 1 , TicksY,  (ILI9341_WIDTH / 2) - 1 + 3, ILI9341_COLOR_BLUE);
	    else
	    	LCD_DrawLine( TicksY,  (ILI9341_WIDTH / 2) - 3 , TicksY,  (ILI9341_WIDTH / 2) - 3 + 7, ILI9341_COLOR_BLUE);
	}

	LCD_Puts(1, 2, "ms/div", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
	LCD_Puts(1, 12, "mV/div:", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
	LCD_Puts(180, 2, "Trig. type:", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
}

//-------------------------------------------------------------------------------------------------------------

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc == ADC1)
		dma_complete = 0;
}

//-------------------------------------------------------------------------------------------------------------

void drawADC()
{
	static uint32_t posBuff = 0;
	static MonoBuffer *temp;

	temp = FullDoubleBuffer_TakeMicBuff(&Buffer);

	for(int i = 1; i < 319; i++)
	{
		yPos = 119 + temp->buffer_M[i + posBuff] / VolPerDivVal[VolPerDiv] * 5 * 15;
		if(yPos > ILI9341_WIDTH) yPos = ILI9341_WIDTH;
		if(yPos < 0) yPos = 0;

//	xPos = (ILI9341_HEIGHT - 2) / 2 + time / TimePerDiv * 5 * 15;
//	if(xPos > ILI9341_HEIGHT) yPos = ILI9341_HEIGHT;
//	if(xPos < 0) yPos = 0;

		LCD_DrawPixel(i, yPos, ILI9341_COLOR_WHITE);
		posBuff += floor(samplingFreq / 319 * TimePerDivVal[TimePerDiv]);
	}
}
//-------------------------------------------------------------------------------------------------------------

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_SPI5_Init();
	/* USER CODE BEGIN 2 */
	LCD_SPI_IncBaudrate();
	LCD_Init();
	LCD_Rotate(3);
	drawGUI();
	DrawInfo();
	FullDoubleBuffer_Init(&Buffer);

	HAL_ADC_Start(&hadc1);

	while (1)
	{
		if (!triggerHeld )
		{
			trigger();
			if ( !notTriggered )
			{
				// Take our samples
				takeSamples();

				//Blank  out previous plot
				TFTSamplesClear();

				// Show the showGraticule
				drawGUI();

				//Display the samples
				TFTSamples();

				// displayTime = (micros() - displayTime);
				// Display the Labels ( uS/Div, Volts/Div etc).
				// showLabels();
				// displayTime = micros();

			}
			else
			{
	    	drawGUI();
			}
		}
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the CPU, AHB and APB busses clocks */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		Error_Handler();

	/** Activate the Over-Drive mode */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
		Error_Handler();

	/** Initializes the CPU, AHB and APB busses clocks */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
		Error_Handler();
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

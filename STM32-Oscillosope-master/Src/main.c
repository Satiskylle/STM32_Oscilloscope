/* USER CODE BEGIN Header */
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "lcd_ili9341.h"
#include "lcd_fonts.h"
#include "lcd_spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define xGridOffset 30
#define yGridOffset 25
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t samplingFreq = 2000000;

enum {
//	_1uV = 0.001,
//	_2uV = 0.002,
//	_10uV = 0.01,
//	_20uV = 0.02,
//	_100uV = 0.1,
//	_200uV = 0.2,
	_1mV = 1,
	_2mV = 2,
	_10mV = 10,
	_20mV = 20,
	_100mV = 100,
	_200mV = 200,
	_1V = 1000,
	_2V = 2000,
}VolPerDiv = _1V;

enum {
	_1s = 1,
	_2s = 2,
}TimePerDiv = _1s;

uint16_t ADCvalue = 0;
int16_t voltageValue = 0;
uint16_t MidYPos = (ILI9341_WIDTH - 2) / 2; 	//Middle of LCD
uint16_t MidXPos = (ILI9341_HEIGHT - 2) / 2; 	//Middle of LCD
uint16_t xPos, yPos;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void drawGUI(void) {
	//Draw rectangle around screen

	LCD_Fill(ILI9341_COLOR_BLACK);
	LCD_DrawRectangle(0, 0, ILI9341_HEIGHT, ILI9341_WIDTH, ILI9341_COLOR_BLUE);
	//Draw dots on LCD
	//The differences need to be at least 15!!!!
	for(int i = (ILI9341_HEIGHT - 2) / 2, j = (ILI9341_HEIGHT - 2) / 2; i < ILI9341_HEIGHT || j > 0; i+=xGridOffset,j-=xGridOffset) {
		if(i<ILI9341_HEIGHT) {
			//DRAW DOT ON LCD - i
			LCD_DrawPixel(i, (ILI9341_WIDTH - 2) / 2, ILI9341_COLOR_BLUE);
			if(i%(5*xGridOffset)) {
				//DRAW THICKER LINE ON LCD
				LCD_DrawPixel(i, (ILI9341_WIDTH - 2) / 2 - 1, ILI9341_COLOR_BLUE);
				LCD_DrawPixel(i, (ILI9341_WIDTH - 2) / 2 + 1, ILI9341_COLOR_BLUE);
			}
		}
		if(j>0) {
			//DRAW DOT ON LCD - j
			LCD_DrawPixel(j, (ILI9341_WIDTH - 2) / 2, ILI9341_COLOR_BLUE);
			if(j%(10*xGridOffset)) {
				//DRAW THICKER LINE ON LCD
				LCD_DrawPixel(j, (ILI9341_WIDTH - 2) / 2 - 1, ILI9341_COLOR_BLUE);
				LCD_DrawPixel(j, (ILI9341_WIDTH - 2) / 2 + 1, ILI9341_COLOR_BLUE);
			}
		}
	}
	for(int i = (ILI9341_WIDTH - 2) / 2, j = (ILI9341_WIDTH - 2) / 2; i < ILI9341_WIDTH || j > 0; i+=yGridOffset,j-=yGridOffset) {
			if(i<ILI9341_WIDTH) {
				//DRAW DOT ON LCD - i
				LCD_DrawPixel((ILI9341_HEIGHT - 2) / 2, i, ILI9341_COLOR_BLUE);
				if(i%(10*yGridOffset)) {
					//DRAW THICKER LINE ON LCD
					LCD_DrawPixel((ILI9341_HEIGHT - 2) / 2 - 1, i, ILI9341_COLOR_BLUE);
					LCD_DrawPixel((ILI9341_HEIGHT - 2) / 2 + 1, i, ILI9341_COLOR_BLUE);
				}
			}
			if(j>0) {
				//DRAW DOT ON LCD - j
				LCD_DrawPixel(159, j, ILI9341_COLOR_BLUE);
				if(j%(10*yGridOffset)) {
					//DRAW THICKER LINE ON LCD
					LCD_DrawPixel((ILI9341_HEIGHT - 2) / 2 - 1, j, ILI9341_COLOR_BLUE);
					LCD_DrawPixel((ILI9341_HEIGHT - 2) / 2 + 1, j, ILI9341_COLOR_BLUE);
				}
			}
		}
	LCD_Puts(1, 2, "Time base:", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
	LCD_Puts(1, 12, "V/div:", &LCD_Font_7x10, ILI9341_COLOR_WHITE);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if(hadc == ADC1) {
		//Convert ADC value to signed value representing input voltage
		voltageValue = ADCvalue * 3.3 / 4096 - 3.3 / 2; //bipolar voltage that comes to INPUT
	}
}

void drawADC() {
static uint32_t posBuff = 0;
	for(int i = 1; i < 319; i++) {
//		yPos = 119 + voltageValue[i + posBuff] / VolPerDiv * 5 * 15;
		if(yPos > ILI9341_WIDTH) yPos = ILI9341_WIDTH;
		if(yPos < 0) yPos = 0;

//	xPos = (ILI9341_HEIGHT - 2) / 2 + time / TimePerDiv * 5 * 15;
//	if(xPos > ILI9341_HEIGHT) yPos = ILI9341_HEIGHT;
//	if(xPos < 0) yPos = 0;

		LCD_DrawPixel(i, yPos, ILI9341_COLOR_WHITE);

		posBuff += floor(samplingFreq / 319 * TimePerDiv);
	}
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_SPI5_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1,ADCvalue,1);
  LCD_SPI_IncBaudrate();
  LCD_Init();
  LCD_Rotate(3);
  drawGUI();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
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
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
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

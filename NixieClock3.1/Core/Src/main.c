/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ESP8266_HAL.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define BITVALUE(X,N) (((X) >> (N)) & 0x1)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
char  SerialData[4]={0};
int setflag=0;
int stopflag=0;
int setH=0;
int setM=0;
double ledload=100;//mennyi ideig villog a led
double RTCload=1000;//mennyi időnként frissítjük az órát

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */
uint8_t int_to_bcd(uint8_t value)
{
    return ((value / 10) << 4) | (value % 10);
}
int bcd_to_int(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void SetRTC(RTC_TimeTypeDef TimeSet,RTC_DateTypeDef DateSet)
{
	HAL_Delay(10);
	  TimeSet.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  TimeSet.StoreOperation = RTC_STOREOPERATION_RESET;
	  TimeSet.TimeFormat = RTC_HOURFORMAT_24  ;
    HAL_RTC_SetTime(&hrtc, &TimeSet, RTC_FORMAT_BIN);//binary setting !!!
    HAL_RTC_SetDate(&hrtc, &DateSet, RTC_FORMAT_BIN);

}

void ReadRTC(void)
{
	char buffer[30];
	  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD); //ez csak azért kell, mert enélkül megáll az rtc
	  sprintf(buffer,"Time:%d:%d:%d\r\n",bcd_to_int(sTime.Hours),bcd_to_int(sTime.Minutes),bcd_to_int(sTime.Seconds));
	  HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
}
void Display(int showtime)
{
  	  HAL_Delay(showtime);
  	  //4-7 bitek a 10-es bcd helyiértékek
	  HAL_GPIO_WritePin(GPIOA, A_Pin, BITVALUE(sTime.Hours,4));
	  HAL_GPIO_WritePin(GPIOA, B_Pin, BITVALUE(sTime.Hours,5));
	  HAL_GPIO_WritePin(GPIOA, C_Pin, BITVALUE(sTime.Hours,6));
	  HAL_GPIO_WritePin(GPIOA, D_Pin, BITVALUE(sTime.Hours,7));
	  if (showtime!=0)HAL_GPIO_WritePin(GPIOA, M1_Pin, 1);
	  HAL_Delay(showtime);
	  HAL_GPIO_WritePin(GPIOA, M1_Pin, 0);
	  //0-3 bitek az 1-es bcd helyiértékek
	  HAL_GPIO_WritePin(GPIOA, A_Pin, BITVALUE(sTime.Hours,0));
	  HAL_GPIO_WritePin(GPIOA, B_Pin, BITVALUE(sTime.Hours,1));
	  HAL_GPIO_WritePin(GPIOA, C_Pin, BITVALUE(sTime.Hours,2));
	  HAL_GPIO_WritePin(GPIOA, D_Pin, BITVALUE(sTime.Hours,3));
	  if (showtime!=0)HAL_GPIO_WritePin(GPIOA, M2_Pin, 1);
	  HAL_Delay(showtime);
	  HAL_GPIO_WritePin(GPIOA, M2_Pin, 0);
	  //4-7 bitek a 10-es bcd helyiértékek
	  HAL_GPIO_WritePin(GPIOA, A_Pin, BITVALUE(sTime.Minutes,4));
	  HAL_GPIO_WritePin(GPIOA, B_Pin, BITVALUE(sTime.Minutes,5));
	  HAL_GPIO_WritePin(GPIOA, C_Pin, BITVALUE(sTime.Minutes,6));
	  HAL_GPIO_WritePin(GPIOA, D_Pin, BITVALUE(sTime.Minutes,7));
	  if (showtime!=0)HAL_GPIO_WritePin(GPIOA, M4_Pin, 1);
	  HAL_Delay(showtime);
	  HAL_GPIO_WritePin(GPIOA, M4_Pin, 0);
	  //0-3 bitek az 1-es bcd helyiértékek
	  HAL_GPIO_WritePin(GPIOA, A_Pin, BITVALUE(sTime.Minutes,0));
	  HAL_GPIO_WritePin(GPIOA, B_Pin, BITVALUE(sTime.Minutes,1));
	  HAL_GPIO_WritePin(GPIOA, C_Pin, BITVALUE(sTime.Minutes,2));
	  HAL_GPIO_WritePin(GPIOA, D_Pin, BITVALUE(sTime.Minutes,3));
	  if (showtime!=0)HAL_GPIO_WritePin(GPIOA, M3_Pin, 1);
	  HAL_Delay(showtime);
	  HAL_GPIO_WritePin(GPIOA, M3_Pin, 0);
}

void InitProc(void)
{
	  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD); //ez csak azért kell, mert enélkül megáll az rtc

	  while(ESP_Init("Viva la Bajot","negyvenketto42")!=1)
	  {
	 	HAL_Delay(1000);//led villogjon minden próbálkozásnál
	 	HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
	  }
	  HAL_Delay(1000);
	  SetRTC(AskTime(),sDate);
	  HAL_Delay(1000);
	  ReadRTC();
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
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
 char buffer[100]; // Buffer to hold time/date string
  int ledblink=0;
  int readRTC=0;
  int displaytime=2;
  int PH=0;
  int H=0;
  sDate.WeekDay = 1;
  sDate.Month = 12;
  sDate.Date = 29;
  sDate.Year = 25;
  InitProc();
  PH=bcd_to_int(sTime.Hours);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)

  {

	  readRTC++;

	  if (readRTC>RTCload){
		  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
		  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD); //ez csak azért kell, mert enélkül megáll az rtc
		  sprintf(buffer,"Time:%d:%d:%d\r\n",bcd_to_int(sTime.Hours),bcd_to_int(sTime.Minutes),bcd_to_int(sTime.Seconds));
		  HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		  H=bcd_to_int(sTime.Hours);
		  if (bcd_to_int(sTime.Minutes)==13){sTime.Minutes=int_to_bcd(12);}//13talanítás
		  if (bcd_to_int(sTime.Hours)>24){while(1);}//ha 24 óránál többet mutat watchdog reset
		  if (H-PH!=0){ 						//ha aktuális óra-előző óra =0 tehát megegyeznek akkor ne csináljon semmit
			  if(H-PH==1){PH=H;}				//ha aktuális óra- előző óra =1 akkor normál üzem van, lementjük
			  if(H-PH<0){while(1);}				 //ha aktuális óra- előző óra =negatív szám, akkor watchdog reset
		  }


		 // uint8_t hour = bcd_to_int(sTime.Hours & 0x3F);//chat gpt megoldás,kijelző kikapcsolás este 10 után vagyról módosítva
		 // if (hour > 21 || hour < 6) {
		 //     displaytime = 0;
		 // } else {				nem működik, nem kapcsol vissza valamiért!
		 //     displaytime = 2;
		 // }

		  if (bcd_to_int(sTime.Hours)>22 || bcd_to_int(sTime.Hours)<5){displaytime=0;}//kijelző kikapcsolás este 11 után vagyról módosítva
		  else{displaytime=2;}
		  readRTC=0;
	  }
	  ledblink++;

	  if (ledblink>ledload){
		  HAL_GPIO_WritePin(GPIOB, LD3_Pin,1);
		  HAL_GPIO_WritePin(GPIOA, SEC_LED_Pin,1);
		  if (ledblink>110){HAL_GPIO_WritePin(GPIOB, LD3_Pin,0);HAL_GPIO_WritePin(GPIOA, SEC_LED_Pin,0);ledblink=0;}
	  }

	  HAL_IWDG_Refresh(&hiwdg);
	  Display(displaytime);



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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x5;
  sTime.Minutes = 0x59;
  sTime.Seconds = 0x1;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_DECEMBER;
  sDate.Date = 0x1;
  sDate.Year = 0x25;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, A_Pin|B_Pin|C_Pin|D_Pin
                          |M1_Pin|M2_Pin|M3_Pin|M4_Pin
                          |SEC_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : A_Pin B_Pin C_Pin D_Pin
                           M1_Pin M2_Pin M3_Pin M4_Pin
                           SEC_LED_Pin */
  GPIO_InitStruct.Pin = A_Pin|B_Pin|C_Pin|D_Pin
                          |M1_Pin|M2_Pin|M3_Pin|M4_Pin
                          |SEC_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD3_Pin */
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//{
//	HAL_UARTEx_ReceiveToIdle_IT(&huart2,(uint8_t*)SerialData, 4);
//	setflag=1;

//}
//void HAL_PWR_PVDCallback()
//{
//	stopflag=0;
//	  PWR_PVDTypeDef sConfigPVD = {0};
//if (stopflag==0){//stopmode bekapcs
//	stopflag=1;
//	  sConfigPVD.PVDLevel = PWR_PVDLEVEL_6;
//	  sConfigPVD.Mode = PWR_PVD_MODE_IT_FALLING;
//	  HAL_PWR_ConfigPVD(&sConfigPVD);
//	HAL_GPIO_WritePin(GPIOB, LD3_Pin,0);
//	HAL_SuspendTick();
	//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_STOPENTRY_WFI);
//	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
//}
//else{//stopmode kikapcs
//	HAL_ResumeTick();
//	  sConfigPVD.PVDLevel = PWR_PVDLEVEL_6;
//	  sConfigPVD.Mode = PWR_PVD_MODE_IT_RISING;
//	  HAL_PWR_ConfigPVD(&sConfigPVD);
//	HAL_GPIO_WritePin(GPIOB, LD3_Pin,1);
//	stopflag=0;
//}
//}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	if(GPIO_Pin == GPIO_PIN_4){

		//  if(stopflag == 1)
		 // {

			  SystemClock_Config ();
			  HAL_ResumeTick();
			  HAL_Delay(1000);

//			  char *str = "WAKEUP FROM EXTII\r\n";
//			  HAL_UART_Transmit(&huart2, (uint8_t *) str, strlen (str), HAL_MAX_DELAY);

		  //}
		  //else
		  //{


		  //}
	//	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	//	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
	//	sTime.Minutes = sTime.Minutes+1;
	//	HAL_Delay(100);
	//   HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	//    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
//	}
/*	if(GPIO_Pin == GPIO_PIN_6){
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
		sTime.Hours= sTime.Hours+1;
		HAL_Delay(100);
	    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
	}
*/
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

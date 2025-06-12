/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 Living Labs (THUAS).
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
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim8;

/* USER CODE BEGIN PV */
uint32_t light_timer;
uint32_t opt3002_timer;

/*CAN network variables-------------------------------------------------------*/
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;

// Receiving data buffer
uint8_t RxData[1];

// Different messages used on the bus
uint8_t dark_out_msg = 0x01;
uint8_t light_out_msg = 0x03;
uint8_t timer_rst_msg = 0x07;
uint8_t light_off_msg = 0x0F;
uint8_t detection_msg = 0xFF;
/*END CAN network variables---------------------------------------------------*/

/*Callback Flags--------------------------------------------------------------*/

/*CAN message flags*/
uint8_t dark_out_flag;
uint8_t timer_rst_flag;
uint8_t light_off_flag;

/*Internal flags*/
uint8_t light_on;
uint8_t dark_outside;
uint8_t dark_counter;
uint8_t vandalised;

/*Shared flags*/
uint8_t detection_flag;
/*END Callback Flags----------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM8_Init(void);
/* USER CODE BEGIN PFP */
/* Callback functions prototypes */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan);

/* LED module driving functions prototypes */
void leds_on(void);
void leds_off(void);

/* OPT3002 light sensor driving functions prototypes */
void opt3002_set_conf(void);
uint8_t opt3002_result(void);

/* CAN address readout function prototype */
uint16_t read_address(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration------------------------------------------------------*/

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
	MX_CAN1_Init();
	MX_I2C1_Init();
	MX_TIM1_Init();
	MX_TIM8_Init();
	/* USER CODE BEGIN 2 */
	/*Turn on PWM channels for driving the LED's------------------------------*/
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); //test leds
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	/*End of PWM channels initialization--------------------------------------*/

	/*CAN Filter Configuration--------------------------------------------------
	 * @brief Filter configuration to make sure Fifo0 only receives messages from neighbouring addresses, Fifo1 receives all messages with global addresses
	 */
	CAN_FilterTypeDef canfilterconfig;

	uint16_t address = read_address();
	canfilterconfig.FilterBank = 0;
	canfilterconfig.FilterMode = CAN_FILTERMODE_IDLIST;
	canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
	// Add directly neighbouring addresses to the filter
	canfilterconfig.FilterIdLow = ((address - 1) << 5);
	canfilterconfig.FilterIdHigh = ((address + 1) << 5);
	canfilterconfig.FilterMaskIdLow = 0;
	canfilterconfig.FilterMaskIdHigh = 0;
	// Receive these messages on Fifo0 for use of separate interrupts
	canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	canfilterconfig.FilterActivation = ENABLE;
	canfilterconfig.SlaveStartFilterBank = 14;

	HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig);

	canfilterconfig.FilterBank = 1;
	canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
	canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
	// Add all globaal addresses to the filter, global 11-bit addresses are defined as 0b111 + address (8 bits from dips)
	canfilterconfig.FilterIdHigh = (0x700 << 5);
	canfilterconfig.FilterIdLow = 0x0000;
	canfilterconfig.FilterMaskIdHigh = (0x700 << 5);
	canfilterconfig.FilterMaskIdLow = 0x0000;
	// Receive these global messages on Fifo1 for use of separate interrupts
	canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO1;
	canfilterconfig.FilterActivation = ENABLE;

	HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig);

	HAL_CAN_Start(&hcan1);

	HAL_CAN_ActivateNotification(&hcan1,
	CAN_IT_RX_FIFO0_MSG_PENDING |
	CAN_IT_RX_FIFO1_MSG_PENDING);
	/*End CAN Filter Configuration--------------------------------------------*/
	/*CAN transmission variables----------------------------------------------*/
	TxHeader.DLC = 1; 				// Send 1 byte of data
	TxHeader.IDE = CAN_ID_STD;		// Standard 11-bit ID
	TxHeader.RTR = CAN_RTR_DATA;	// Send data, none is requested

	uint32_t mb1;					// Use mailbox 1
	/*END CAN transmission variables------------------------------------------*/

	/*  */
	opt3002_set_conf();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		/*Only run when dark outside*/
		if (dark_outside == 1) {

			/** Detection handling
			 * @brief When detection occurs, through PIR or CAN, turn light on when light is off and signal neighbouring shrooms to turn on, else reset timer and signal whole array to reset timer.
			 * */
			if (detection_flag == 1 && light_on == 0) {

				light_on = 1;
				detection_flag = 0;

				leds_on();
				light_timer = uwTick;

				TxHeader.StdId = read_address();
				if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &detection_msg,
						&mb1) != HAL_OK) {
					Error_Handler();
				}
			} else if (detection_flag == 1 && light_on == 1) {
				detection_flag = 0;

				light_timer = uwTick;

				// (0x700 | read_address()) gives this message the "global" ID of the shroom
				TxHeader.StdId = (0x700 | read_address());
				if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &timer_rst_msg,
						&mb1) != HAL_OK) {
					Error_Handler();
				}
			}
			/* END Detection handling */

			/** timer_rst_flag handling
			 * @brief Reset light timer when timer_rst_msg received, ignored if light is on.
			 * */
			if (timer_rst_flag == 1 && light_on == 1) {
				timer_rst_flag = 0;

				light_timer = uwTick;
			} else if (timer_rst_flag == 1 && light_on == 0) {
				timer_rst_flag = 0;
			}
			/* END timer_st_flag handling*/

			/** leds_off handling
			 * @brief Turns light off when timer exceeds 1 minute or when light_off_msg received.
			 * */
			if ((uwTick - light_timer) > 60000 && light_on == 1) {
				light_on = 0;

				// When mushroom is the first in it's array to turn off, message other shrooms to turn off for nice effect.
				TxHeader.StdId = (0x700 | read_address());
				if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &light_off_msg,
						&mb1) != HAL_OK) {
					Error_Handler();
				}

				leds_off();
			} else if (light_off_flag == 1 && light_on == 1) {
				light_off_flag = 0;
				light_on = 0;

				leds_off();
			}
			/* END leds_off handling */

			/** Handling of darkness flags when dark outside
			 * @brief Read OPT3002 result register every 2 minutes, with five negatives switch dark_outside and send light_out_msg. Also switches when receiving light_out_msg.
			 */
			if (uwTick - opt3002_timer > 120000) {
				if (opt3002_result() == 0) {
					dark_counter++;

					if (dark_counter >= 5) {
						dark_counter = 0;
						dark_outside = 0;

						TxHeader.StdId = (0x700 | read_address());
						if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader,
								&light_out_msg, &mb1) != HAL_OK) {
							Error_Handler();
						}
					}

					opt3002_set_conf();
					opt3002_timer = uwTick;
				}
			} else if (dark_out_flag == 3) {
				dark_out_flag = 0;
				dark_outside = 0;
			} else if (dark_out_flag == 1) {
				dark_out_flag = 0;
			}
			/* END Handling of darkness flags */
		}

		/** Handling of darkness flags when light outside
		 * @brief Read OPT3002 result register every 2 minutes, with five negatives switch dark_outside and send light_out_msg. Also switches when receiving light_out_msg.
		 */
		if (dark_outside == 0 && (uwTick - opt3002_timer > 120000)) {
			if (opt3002_result() == 1) {
				dark_counter++;

				if (dark_counter >= 5) {
					dark_counter = 0;
					dark_outside = 1;

					TxHeader.StdId = (0x700 | read_address());
					if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &dark_out_msg,
							&mb1) != HAL_OK) {
						Error_Handler();
					}
				}

				opt3002_set_conf();
				opt3002_timer = uwTick;
			}
		} else if (dark_outside == 0 && dark_out_flag == 1) {
			dark_counter = 0;
			dark_outside = 1;
			dark_out_flag = 0;
		}
		/* END Handling of darkness flags */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 200;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief CAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN1_Init(void) {

	/* USER CODE BEGIN CAN1_Init 0 */

	/* USER CODE END CAN1_Init 0 */

	/* USER CODE BEGIN CAN1_Init 1 */

	/* USER CODE END CAN1_Init 1 */
	hcan1.Instance = CAN1;
	hcan1.Init.Prescaler = 10;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_5TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.AutoRetransmission = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CAN1_Init 2 */

	/* USER CODE END CAN1_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 4;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 39999;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief TIM8 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM8_Init(void) {

	/* USER CODE BEGIN TIM8_Init 0 */

	/* USER CODE END TIM8_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM8_Init 1 */

	/* USER CODE END TIM8_Init 1 */
	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 4;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 39999;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.RepetitionCounter = 0;
	htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim8) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM8_Init 2 */

	/* USER CODE END TIM8_Init 2 */
	HAL_TIM_MspPostInit(&htim8);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(PWR_DIPS_GPIO_Port, PWR_DIPS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : PWR_DIPS_Pin */
	GPIO_InitStruct.Pin = PWR_DIPS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PWR_DIPS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : KICK_Pin DETECT_1_Pin */
	GPIO_InitStruct.Pin = KICK_Pin | DETECT_1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : DIP_0_Pin DIP_1_Pin DIP_2_Pin */
	GPIO_InitStruct.Pin = DIP_0_Pin | DIP_1_Pin | DIP_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : DIP_3_Pin DIP_4_Pin */
	GPIO_InitStruct.Pin = DIP_3_Pin | DIP_4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : DIP_5_Pin DIP_6_Pin DIP_7_Pin */
	GPIO_InitStruct.Pin = DIP_5_Pin | DIP_6_Pin | DIP_7_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : DETECT_0_Pin */
	GPIO_InitStruct.Pin = DETECT_0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(DETECT_0_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : AUX_BUTTON_Pin */
	GPIO_InitStruct.Pin = AUX_BUTTON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(AUX_BUTTON_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/** read_address()--------------------------------------------------------------
 * @brief Power the DIP switches and iterate through each to determine address
 * @retval 16 bit integer containing the CAN address
 */
uint16_t read_address(void) {
	uint16_t address = 0x0000;

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
	address |= !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) << 7;
	address |= !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) << 6;
	address |= !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) << 5;
	address |= !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4) << 4;
	address |= !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) << 3;
	address |= !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) << 2;
	address |= !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) << 1;
	address |= !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

	return address;
}
/*END read_address()----------------------------------------------------------*/

/** opt3002_set_conf()----------------------------------------------------------
 * @brief Writes configuration to OPT3002 configuration register
 */
void opt3002_set_conf(void) {
	uint8_t buf[3];
	uint8_t opt3002_addr = 0x44 << 1;

	// OPT3002 configuration register address
	buf[0] = 0x01;

	// Write 0xBA14 to configuration register, setting max. FSR, masking EXP and setting single-shot mode.
	buf[1] = 0xBA;
	buf[2] = 0x14;

	HAL_I2C_Master_Transmit(&hi2c1, opt3002_addr, buf, 3, HAL_MAX_DELAY);
}
/*END opt3002_set_conf()------------------------------------------------------*/

/*opt3002_result()--------------------------------------------------------------
 * @brief Read OPT3002 result register and process data
 * @retval boolean: true when dark, false when light outside
 */
uint8_t opt3002_result(void) {
	uint8_t buf[2];
	uint8_t opt3002_addr = 0x44 << 1;

	// Set buffer to result register address and writing it to the OPT3002.
	buf[0] = 0x00;
	if (HAL_I2C_Master_Transmit(&hi2c1, opt3002_addr, buf, 1, HAL_MAX_DELAY)
			== HAL_OK) {

		// Request 2 bytes from result register.
		if (HAL_I2C_Master_Receive(&hi2c1, opt3002_addr, buf, 2, HAL_MAX_DELAY)
				== HAL_OK) {

			// Compare result to darkness threshold, returns 1 when true.
			return (buf[0] == 0 && buf[1] < 0xFF);
		}
	}

	return -1;
}
/*END opt3002_result()--------------------------------------------------------*/

/*LED control functions-------------------------------------------------------*/
/* @brief Increases duty cycle on LED_CH_0 in nice fashion
 */
void leds_on(void) {
	for (int i = 0; i < 200; i++) {
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, i * i);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, i * i);
		HAL_Delay(3);
	}
}

/* @brief Decreases duty cycle on LED_CH_0 in nice fashion
 */
void leds_off(void) {
	for (int i = 200; i > 0; i--) {
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, i * i);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, i * i);
		HAL_Delay(30);
	}
}
/*END LED control functions---------------------------------------------------*/

/*HAL_GPIO_EXTI_Callback()------------------------------------------------------
 * @brief Handles GPIO interrupts, sets detection flags for PIR interrupt and vandalised flag for kick sensor interrupt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_12 || GPIO_Pin == GPIO_PIN_11
			|| GPIO_Pin == GPIO_PIN_15) {
		detection_flag = 1;
	} else if (GPIO_Pin == GPIO_PIN_4) {
		vandalised = 1;
	}
}
/*END HAL_GPIO_EXTI_Callback()------------------------------------------------*/

/* CAN callback configurations------------------------------------------------*/
/* @brief Sets detection flag when message is received on Fifo0, resets RxData
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
		detection_flag = 1;
		RxData[0] = 0;
	}
}

/* @brief Sets different flags depending on received message on Fifo1, resets RxData
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData) == HAL_OK) {

		switch (RxData[0]) {
		case 0x01:
			dark_out_flag = 1;
			break;

		case 0x03:
			dark_out_flag = 3;
			break;

		case 0x07:
			timer_rst_flag = 1;
			break;

		case 0x0F:
			light_off_flag = 1;
			break;

		default:
		}

		RxData[0] = 0;
	}
}

/* END CAN Callbacks----------------------------------------------------------*/

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence, starts main().
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	main();
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

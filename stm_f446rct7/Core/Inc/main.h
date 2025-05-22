/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PWR_DIPS_Pin GPIO_PIN_0
#define PWR_DIPS_GPIO_Port GPIOC
#define KICK_Pin GPIO_PIN_4
#define KICK_GPIO_Port GPIOA
#define KICK_EXTI_IRQn EXTI4_IRQn
#define DIP_0_Pin GPIO_PIN_5
#define DIP_0_GPIO_Port GPIOA
#define DIP_1_Pin GPIO_PIN_6
#define DIP_1_GPIO_Port GPIOA
#define DIP_2_Pin GPIO_PIN_7
#define DIP_2_GPIO_Port GPIOA
#define DIP_3_Pin GPIO_PIN_4
#define DIP_3_GPIO_Port GPIOC
#define DIP_4_Pin GPIO_PIN_5
#define DIP_4_GPIO_Port GPIOC
#define DIP_5_Pin GPIO_PIN_0
#define DIP_5_GPIO_Port GPIOB
#define DIP_6_Pin GPIO_PIN_1
#define DIP_6_GPIO_Port GPIOB
#define DIP_7_Pin GPIO_PIN_2
#define DIP_7_GPIO_Port GPIOB
#define DETECT_0_Pin GPIO_PIN_15
#define DETECT_0_GPIO_Port GPIOB
#define DETECT_0_EXTI_IRQn EXTI15_10_IRQn
#define LED_0_Pin GPIO_PIN_6
#define LED_0_GPIO_Port GPIOC
#define LED_1_Pin GPIO_PIN_9
#define LED_1_GPIO_Port GPIOC
#define DETECT_1_Pin GPIO_PIN_11
#define DETECT_1_GPIO_Port GPIOA
#define DETECT_1_EXTI_IRQn EXTI15_10_IRQn
#define AUX_BUTTON_Pin GPIO_PIN_12
#define AUX_BUTTON_GPIO_Port GPIOA
#define Light_Sense_I2C_SCL_Pin GPIO_PIN_6
#define Light_Sense_I2C_SCL_GPIO_Port GPIOB
#define Light_sense_I2C_SDA_Pin GPIO_PIN_7
#define Light_sense_I2C_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

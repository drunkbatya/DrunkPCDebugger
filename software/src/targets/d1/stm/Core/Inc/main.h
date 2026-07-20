/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_crs.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_exti.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_hsem.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define D2_Pin LL_GPIO_PIN_2
#define D2_GPIO_Port GPIOE
#define D3_Pin LL_GPIO_PIN_3
#define D3_GPIO_Port GPIOE
#define D4_Pin LL_GPIO_PIN_4
#define D4_GPIO_Port GPIOE
#define D5_Pin LL_GPIO_PIN_5
#define D5_GPIO_Port GPIOE
#define D6_Pin LL_GPIO_PIN_6
#define D6_GPIO_Port GPIOE
#define LED_Pin LL_GPIO_PIN_0
#define LED_GPIO_Port GPIOA
#define BUS_PWR_Pin LL_GPIO_PIN_1
#define BUS_PWR_GPIO_Port GPIOA
#define _WR_Pin LL_GPIO_PIN_0
#define _WR_GPIO_Port GPIOB
#define _RD_Pin LL_GPIO_PIN_1
#define _RD_GPIO_Port GPIOB
#define _IORQ_Pin LL_GPIO_PIN_2
#define _IORQ_GPIO_Port GPIOB
#define D7_Pin LL_GPIO_PIN_7
#define D7_GPIO_Port GPIOE
#define _M1_Pin LL_GPIO_PIN_10
#define _M1_GPIO_Port GPIOB
#define _CPU_CLK_Pin LL_GPIO_PIN_11
#define _CPU_CLK_GPIO_Port GPIOB
#define BUS_DIR_Pin LL_GPIO_PIN_12
#define BUS_DIR_GPIO_Port GPIOB
#define BUS_OE_Pin LL_GPIO_PIN_13
#define BUS_OE_GPIO_Port GPIOB
#define A8_Pin LL_GPIO_PIN_8
#define A8_GPIO_Port GPIOD
#define A9_Pin LL_GPIO_PIN_9
#define A9_GPIO_Port GPIOD
#define A10_Pin LL_GPIO_PIN_10
#define A10_GPIO_Port GPIOD
#define A11_Pin LL_GPIO_PIN_11
#define A11_GPIO_Port GPIOD
#define A12_Pin LL_GPIO_PIN_12
#define A12_GPIO_Port GPIOD
#define A13_Pin LL_GPIO_PIN_13
#define A13_GPIO_Port GPIOD
#define A14_Pin LL_GPIO_PIN_14
#define A14_GPIO_Port GPIOD
#define A15_Pin LL_GPIO_PIN_15
#define A15_GPIO_Port GPIOD
#define A0_Pin LL_GPIO_PIN_0
#define A0_GPIO_Port GPIOD
#define A1_Pin LL_GPIO_PIN_1
#define A1_GPIO_Port GPIOD
#define A2_Pin LL_GPIO_PIN_2
#define A2_GPIO_Port GPIOD
#define A3_Pin LL_GPIO_PIN_3
#define A3_GPIO_Port GPIOD
#define A4_Pin LL_GPIO_PIN_4
#define A4_GPIO_Port GPIOD
#define A5_Pin LL_GPIO_PIN_5
#define A5_GPIO_Port GPIOD
#define A6_Pin LL_GPIO_PIN_6
#define A6_GPIO_Port GPIOD
#define A7_Pin LL_GPIO_PIN_7
#define A7_GPIO_Port GPIOD
#define _MEMRQ_Pin LL_GPIO_PIN_3
#define _MEMRQ_GPIO_Port GPIOB
#define _BUSREQ_Pin LL_GPIO_PIN_4
#define _BUSREQ_GPIO_Port GPIOB
#define _BUSACK_Pin LL_GPIO_PIN_5
#define _BUSACK_GPIO_Port GPIOB
#define _CPURST_Pin LL_GPIO_PIN_6
#define _CPURST_GPIO_Port GPIOB
#define _WAIT_Pin LL_GPIO_PIN_7
#define _WAIT_GPIO_Port GPIOB
#define _NMI_Pin LL_GPIO_PIN_8
#define _NMI_GPIO_Port GPIOB
#define _INT_Pin LL_GPIO_PIN_9
#define _INT_GPIO_Port GPIOB
#define D0_Pin LL_GPIO_PIN_0
#define D0_GPIO_Port GPIOE
#define D1_Pin LL_GPIO_PIN_1
#define D1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

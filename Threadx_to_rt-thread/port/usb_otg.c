/**
  ******************************************************************************
  * @file    usb_otg.c
  * @brief   This file provides code for the configuration
  *          of the USB_OTG instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_otg.h"

/* USER CODE BEGIN 0 */
#include <string.h>
#include <drv_common.h>
/* Set hpcd_USB_OTG_HS at the end of non-cacheable
   Memory address 0x24027000. */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = 0x24027000
#elif defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".UsbHpcdSection")))
#endif
/* USER CODE END 0 */

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USB_OTG_HS init function */

void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 9;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */
}


// void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
// {

//   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
//   if(pcdHandle->Instance==USB_OTG_HS)
//   {
//   /* USER CODE BEGIN USB_OTG_HS_MspInit 0 */

//   /* USER CODE END USB_OTG_HS_MspInit 0 */
//   /** Initializes the peripherals clock
//   */
//     PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
//     PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
//     if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
//     {
//       Error_Handler();
//     }
//   /** Enable USB Voltage detector
//   */
//     HAL_PWREx_EnableUSBVoltageDetector();

//     /* USB_OTG_HS clock enable */
//     __HAL_RCC_USB_OTG_HS_CLK_ENABLE();

//     /* USB_OTG_HS interrupt Init */
//     HAL_NVIC_SetPriority(OTG_HS_IRQn, 6, 0);
//     HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
//   /* USER CODE BEGIN USB_OTG_HS_MspInit 1 */

//   /* USER CODE END USB_OTG_HS_MspInit 1 */
//   }
// }

// void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
// {

//   if(pcdHandle->Instance==USB_OTG_HS)
//   {
//   /* USER CODE BEGIN USB_OTG_HS_MspDeInit 0 */

//   /* USER CODE END USB_OTG_HS_MspDeInit 0 */
//     /* Peripheral clock disable */
//     __HAL_RCC_USB_OTG_HS_CLK_DISABLE();

//     /* USB_OTG_HS interrupt Deinit */
//     HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
//   /* USER CODE BEGIN USB_OTG_HS_MspDeInit 1 */

//   /* USER CODE END USB_OTG_HS_MspDeInit 1 */
//   }
// }

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

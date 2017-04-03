/**
  ******************************************************************************
  * @file           : usbd_cdc_eem_if.c
  * @brief          :
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_eem_if.h"
#include "usbd_buffers.h"
/* USER CODE BEGIN INCLUDE */
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_CDC_EEM 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_CDC_EEM_Private_TypesDefinitions
  * @{
  */ 
/* USER CODE BEGIN PRIVATE_TYPES */
/* USER CODE END PRIVATE_TYPES */ 
/**
  * @}
  */ 

/** @defgroup USBD_CDC_EEM_Private_Defines
  * @{
  */ 
/* USER CODE BEGIN PRIVATE_DEFINES */
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
/* USER CODE END PRIVATE_DEFINES */
/**
  * @}
  */ 

/** @defgroup USBD_CDC_EEM_Private_Macros
  * @{
  */ 
/* USER CODE BEGIN PRIVATE_MACRO */
/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */ 
  
/** @defgroup USBD_CDC_EEM_Private_Variables
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
//extern uint8_t UserRxBufferFS[EEM_RX_DATA_SIZE];

/* Send Data over USB CDC are stored in this buffer       */
//extern uint8_t UserTxBufferFS[EEM_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */ 
  
/** @defgroup USBD_CDC_EEM_IF_Exported_Variables
  * @{
  */ 
  extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE BEGIN EXPORTED_VARIABLES */
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */ 
  
/** @defgroup USBD_CDC_EEM_Private_FunctionPrototypes
  * @{
  */
static int8_t CDC_EEM_Init_FS     (void);
static int8_t CDC_EEM_DeInit_FS   (void);
static int8_t CDC_EEM_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
/*static int8_t CDC_EEM_Receive_FS  (uint8_t* pbuf, uint32_t *Len);*/

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */ 
  
USBD_CDC_EEM_ItfTypeDef USBD_Interface_fops_FS = 
{
  CDC_EEM_Init_FS,
  CDC_EEM_DeInit_FS,
  CDC_EEM_Control_FS,  
  NULL /*CDC_EEM_Receive_FS*/ /* excessive method */
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  CDC_EEM_Init_FS
  *         Initializes the CDC media low layer over the FS USB IP
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_EEM_Init_FS(void)
{ 
  /* USER CODE BEGIN 3 */ 
  /* Set Application Buffers */
/*  USBD_CDC_EEM_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, EEM_TX_DATA_SIZE, 0);
  USBD_CDC_EEM_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS, EEM_RX_DATA_SIZE);
*/
  init_eem_pkt_bpool(EEM_RX_BUFFER);
  init_eem_pkt_bpool(EEM_TX_BUFFER);

  return (USBD_OK);
  /* USER CODE END 3 */ 
}

/**
  * @brief  CDC_EEM_DeInit_FS
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_EEM_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */ 
  return (USBD_OK);
  /* USER CODE END 4 */ 
}

/**
  * @brief  CDC_EEM_Control_FS
  *         Manage the CDC class requests
  * @param  cmd: Command code            
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_EEM_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
  /* USER CODE BEGIN 5 */
#if 0
  switch (cmd)
  {
  case CDC_EEM_SEND_ENCAPSULATED_COMMAND:
 
    break;

  case CDC_EEM_GET_ENCAPSULATED_RESPONSE:
 
    break;

  case CDC_EEM_SET_COMM_FEATURE:
 
    break;

  case CDC_EEM_GET_COMM_FEATURE:

    break;

  case CDC_EEM_CLEAR_COMM_FEATURE:

    break;

  case CDC_EEM_SET_LINE_CODING:   
	
    break;

  case CDC_EEM_GET_LINE_CODING:     

    break;

  case CDC_EEM_SET_CONTROL_LINE_STATE:

    break;

  case CDC_EEM_SEND_BREAK:
 
    break;    
    
  default:
    break;
  }
#endif

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  CDC_EEM_Receive_FS
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
#if 0
static int8_t CDC_EEM_Receive_FS (uint8_t*buf, uint32_t *size)
{
  /* USER CODE BEGIN 6 */
  USBD_CDC_EEM_ReceivePacket(&hUsbDeviceFS, buf, size);
  return (USBD_OK);
  /* USER CODE END 6 */ 
}
#endif

#if 0
/**
  * @brief  CDC_EEM_Transmit_FS
  *         Data send over USB IN endpoint are sent over CDC interface 
  *         through this function.           
  *         @note
  *         
  *                 
  * @param  Buf: Buffer of data to be send
  * @param  Len: Number of data to be send (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_EEM_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */ 
  USBD_CDC_EEM_HandleTypeDef *hcdc = (USBD_CDC_EEM_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->tx_state != 0){
    return USBD_BUSY;
  }
  
  if (Buf == NULL){ /* send zero packet */
    Len = 2;
    Buf = UserTxBufferFS;
    Buf[0] = 0;
    Buf[1] = 0;
  }
  USBD_CDC_EEM_SetTxBuffer(&hUsbDeviceFS, Buf, EEM_TX_DATA_SIZE, Len);
  result = USBD_CDC_EEM_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */ 
  return result;
}
#endif

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


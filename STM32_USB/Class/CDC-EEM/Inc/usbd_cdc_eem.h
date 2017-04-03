/**
  ******************************************************************************
  * @file    usbd_cdc_eem.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   header file for the usbd_cdc.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_EEM_H
#define __USB_CDC_EEM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_cdc.c
  * @{
  */ 


/** @defgroup usbd_cdc_Exported_Defines
  * @{
  */ 
#define USB_CLASS_CDC_DATA                              0x0A //???

#define USB_DESCTYPE_CS_INTERFACE                       0x24
#define USB_DESCTYPE_CS_ENDPOINT                        0x25 

#define CDC_EEM_IN_EP                                   0x81  /* EP1 for data IN */
#define CDC_EEM_OUT_EP                                  0x01  /* EP1 for data OUT */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_EEM_DATA_HS_MAX_PACKET_SIZE                 512 /* Endpoint IN & OUT Packet size */
#define CDC_EEM_DATA_FS_MAX_PACKET_SIZE                 64  /* Endpoint IN & OUT Packet size */
#define CDC_EEM_CMD_PACKET_SIZE                         64  /* Control Endpoint Packet size */ // ??? 8

#define USB_CDC_EEM_CONFIG_DESC_SIZ                     0x32
#define CDC_EEM_DATA_HS_IN_PACKET_SIZE                  CDC_EEM_DATA_HS_MAX_PACKET_SIZE
#define CDC_EEM_DATA_HS_OUT_PACKET_SIZE                 CDC_EEM_DATA_HS_MAX_PACKET_SIZE

#define CDC_EEM_DATA_FS_IN_PACKET_SIZE                  CDC_EEM_DATA_FS_MAX_PACKET_SIZE
#define CDC_EEM_DATA_FS_OUT_PACKET_SIZE                 CDC_EEM_DATA_FS_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/
#if 0
#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK                              0x23
#endif

/* eem buffer size */
#define EEM_RX_DATA_SIZE  1524
#define EEM_TX_DATA_SIZE  1524

#define EEM_RX_BUF_CNT 3
#define EEM_TX_BUF_CNT 1

/*
extern uint8_t UserRxBufferFS[EEM_RX_DATA_SIZE];
extern uint8_t UserTxBufferFS[EEM_TX_DATA_SIZE];
*/


/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */ 
/*
typedef struct
{
  uint32_t bitrate;
  uint8_t  format;
  uint8_t  paritytype;
  uint8_t  datatype;
}USBD_CDC_EEM_LineCodingTypeDef;
*/

typedef struct _USBD_CDC_EEM_Itf
{
  int8_t (* Init)          (void);
  int8_t (* DeInit)        (void);
  int8_t (* Control)       (uint8_t, uint8_t * , uint16_t);   
  int8_t (* Receive)       (uint8_t *, uint32_t *);  

}USBD_CDC_EEM_ItfTypeDef;


typedef struct
{
  uint32_t data[CDC_EEM_CMD_PACKET_SIZE/4]; /* control transfers */     /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint8_t  CmdLength;    

  uint8_t  *tx_buffer;
  uint8_t  *rx_buffer;

  uint32_t tx_buf_size;
  uint32_t tx_buf_rd_pos;
  uint32_t tx_length;

  uint32_t rx_buf_size;
  uint32_t rx_buf_wr_pos;

  __IO uint32_t tx_state;
  __IO uint32_t rx_state;

  __IO uint64_t rx_cnt_received;
  __IO uint64_t rx_cnt_processed;
  __IO uint64_t rx_cnt_sending;
  __IO uint64_t rx_cnt_sent;
}
USBD_CDC_EEM_HandleTypeDef; 


typedef enum _eem_receiver_state_enum {
  EEM_RECEIVER_INITIAL = 0,
  EEM_RECEIVER_CMD_IN_PROGRESS = 1,
  EEM_RECEIVER_CMD_DONE = 2,
  EEM_RECEIVER_DATA_IN_PROGRESS = 3,
  EEM_RECEIVER_DATA_DONE = 4,

  EEM_RECEIVER_FAULT = 0xFF
}eem_receiver_state_enum;


typedef enum _eem_transmitter_state_enum {
  EEM_TRANSMITTER_INITIAL = 0,
  EEM_TRANSMITTER_SEND_DATA = 1,
  EEM_TRANSMITTER_SEND_ZPKT= 2,
}eem_transmitter_state_enum;

typedef enum _eem_packet_type_enum {
  EEM_DATA_PACKET = 0,
  EEM_CMD_PACKET =1
}eem_packet_type_enum;


/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 
  
/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_CDC_EEM;
#define USBD_CDC_EEM_CLASS    &USBD_CDC_EEM
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_CDC_EEM_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_CDC_EEM_ItfTypeDef *fops);

#if 0
uint8_t  USBD_CDC_EEM_SetTxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff,
                                      uint16_t buf_size,
                                      uint16_t length
                                         );

uint8_t  USBD_CDC_EEM_SetRxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff,
                                      uint16_t buf_size);
#endif

#if 0
uint8_t  USBD_CDC_EEM_ReceivePacket      (USBD_HandleTypeDef *pdev,
                                          uint8_t*pbuf);
#endif

uint8_t  USBD_CDC_EEM_TransmitPacket     (USBD_HandleTypeDef *pdev,
                                          eem_transmitter_state_enum tx_state);
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_CDC_EEM_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

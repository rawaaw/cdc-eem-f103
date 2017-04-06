/**
  ******************************************************************************
  * @file    usbd_cdc_eem.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                CDC EEM Class Driver Description
  *          =================================================================== 
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus 
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *      
  *  @endverbatim
  *                                  
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_eem.h"
#include "usbd_buffers.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC_EEM 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_CDC_EEM_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_CDC_EEM_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 
/** @defgroup USBD_CDC_EEM_Private_Macros
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_CDC_EEM_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_CDC_EEM_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_CDC_EEM_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_CDC_EEM_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  USBD_CDC_EEM_DataIn (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_CDC_EEM_DataOut (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_CDC_EEM_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  *USBD_CDC_EEM_GetFSCfgDesc (uint16_t *length);

static uint8_t  *USBD_CDC_EEM_GetHSCfgDesc (uint16_t *length);

static uint8_t  *USBD_CDC_EEM_GetOtherSpeedCfgDesc (uint16_t *length);

static uint8_t  *USBD_CDC_EEM_GetOtherSpeedCfgDesc (uint16_t *length);

uint8_t  *USBD_CDC_EEM_GetDeviceQualifierDescriptor (uint16_t *length);

/* USB Standard Device Descriptor */
/* https://msdn.microsoft.com/en-us/library/windows/hardware/ff539288%28v=vs.85%29.aspx */
/* https://www.keil.com/pack/doc/mw/USB/html/_u_s_b__device__qualifier__descriptor.html */
__ALIGN_BEGIN static uint8_t USBD_CDC_EEM_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,      // 0x0A
  USB_DESC_TYPE_DEVICE_QUALIFIER,  // 0x06
  0x00,   // BCD USB 2._0_
  0x02,   // BCD USB _2_.0 
  0x02,   //bDeviceClass
  0x0c,   //bDeviceSubClass
  0x07,   //bDeviceProtocol
  0x40,   //bMaxPacketSize0
  0x01,   //bNumConfigurations
  0x00,   //bReserved
};

/**
  * @}
  */ 

/** @defgroup USBD_CDC_EEM_Private_Variables
  * @{
  */ 


/* CDC interface class callbacks structure */
USBD_ClassTypeDef  USBD_CDC_EEM = 
{
  USBD_CDC_EEM_Init,
  USBD_CDC_EEM_DeInit,
  USBD_CDC_EEM_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_CDC_EEM_EP0_RxReady,
  USBD_CDC_EEM_DataIn,
  USBD_CDC_EEM_DataOut,
  NULL,
  NULL,
  NULL,     
  USBD_CDC_EEM_GetHSCfgDesc,  
  USBD_CDC_EEM_GetFSCfgDesc,    
  USBD_CDC_EEM_GetOtherSpeedCfgDesc, 
  USBD_CDC_EEM_GetDeviceQualifierDescriptor,
};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_CDC_EEM_CfgHSDesc[USB_CDC_EEM_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* 0 bLength */
  USB_DESC_TYPE_CONFIGURATION,/* 1 bDescriptortype - Configuration*/
  0x32,00,                  /* 2 wTotalLength */ /*USB_CDC_ECM_CONFIG_DESC_SIZ*/
  0x01,                     /* 4 bNumInterfaces */
  0x01,                     /* 5 bConfigurationValue */
  0x04,                     /* 6 iConfiguration - index of string */
  0x80,                     /* 7 bmAttributes - Bus powered */
  0xC8,                     /* 8 bMaxPower - 400mA */

  /* CDC Communication interface */
  0x09,                     /* 0 bLength */
  USB_DESC_TYPE_INTERFACE,  /* 1 bDescriptorType - Interface */  //0x04
  0x00,                     /* 2 bInterfaceNumber - Interface 0 */
  0x00,                     /* 3 bAlternateSetting */
  0x02,                     /* 4 bNumEndpoints */
  0x02,                     /* 5 bInterfaceClass */ /*USB_CLASS_COMMUNICATIONS*/
  0x0c,                     /* 6 bInterfaceSubClass - Ethernet Control Model */ /*USB_CDC_ECM_SUBCLASS*/
  0x07,                     /* 7 bInterfaceProtocol - No specific protocol */
  0x00,                     /* 8 iInterface - No string descriptor */

  /* Header Functional descriptor */
  0x05,                     /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,/* 1 bDescriptortype, CS_INTERFACE */ //0x24
  0x00,                     /* 2 bDescriptorsubtype, HEADER */
  0x10, 0x01,               /* 3 bcdCDC */

  /* Ethernet Networking Functional descriptor */
  0x0D,                     /* 0 bLength - 13 bytes */
  USB_DESCTYPE_CS_INTERFACE,/* 1 bDescriptortype, CS_INTERFACE */
  0x0F,                     /* 2 bDescriptorsubtype, ETHERNET NETWORKING */
  // !!! MAC need check //index 6 in USBD_GetDescriptor() (usb/Core/usbd_ctlreq.c) ??????
  0x06,                     /* 3 iMACAddress, Index of MAC address string */ 

  0x00,0x00,0x00,0x00,      /* 4 bmEthernetStatistics - Handles None */
  0xEA,05,                  /* 8 wMaxSegmentSize - 1514 bytes */
  0x00,0x00,                /* 10 wNumberMCFilters - No multicast filters */
  0x00,                     /* 12 bNumberPowerFilters - No wake-up feature */

  /* Data IN Endpoint descriptor */
  0x07,                     /* 0 bLength */
  USB_DESC_TYPE_ENDPOINT,   /* 1 bDescriptorType */
  CDC_EEM_IN_EP,            /* 2 bEndpointAddress - IN endpoint*/ // 0x81
  0x02,                     /* 3 bmAttributes - Bulk type */
  LOBYTE(CDC_EEM_DATA_HS_MAX_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_EEM_DATA_HS_MAX_PACKET_SIZE),
  0x01,                     /* 6 bInterval */

  /* Data OUT Endpoint descriptor */
  0x07,                     /* 0 bLength */
  USB_DESC_TYPE_ENDPOINT,   /* 1 bDescriptorType */
  CDC_EEM_OUT_EP,           /* 2 bEndpointAddress - OUT endpoint */ // 0x01
  0x02,                     /* 3 bmAttributes - Bulk type */
  LOBYTE(CDC_EEM_DATA_HS_MAX_PACKET_SIZE), /* 4 wMaxPacketSize - Low byte */
  HIBYTE(CDC_EEM_DATA_HS_MAX_PACKET_SIZE), /* 5 wMaxPacketSize - High byte */
  0x01                     /* 6 bInterval */
} ;


/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_CDC_EEM_CfgFSDesc[USB_CDC_EEM_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* 0 bLength */
  USB_DESC_TYPE_CONFIGURATION,/* 1 bDescriptortype - Configuration*/
  0x32,00,                  /* 2 wTotalLength */ /*USB_CDC_ECM_CONFIG_DESC_SIZ*/
  0x01,                     /* 4 bNumInterfaces */
  0x01,                     /* 5 bConfigurationValue */
  0x04,                     /* 6 iConfiguration - index of string */
  0x80,                     /* 7 bmAttributes - Bus powered */
  0xC8,                     /* 8 bMaxPower - 400mA */

  /* CDC Communication interface */
  0x09,                     /* 0 bLength */
  USB_DESC_TYPE_INTERFACE,  /* 1 bDescriptorType - Interface */  //0x04
  0x00,                     /* 2 bInterfaceNumber - Interface 0 */
  0x00,                     /* 3 bAlternateSetting */
  0x02,                     /* 4 bNumEndpoints */
  0x02,                     /* 5 bInterfaceClass */ /*USB_CLASS_COMMUNICATIONS*/
  0x0c,                     /* 6 bInterfaceSubClass - Ethernet Control Model */ /*USB_CDC_ECM_SUBCLASS*/
  0x07,                     /* 7 bInterfaceProtocol - No specific protocol */
  0x00,                     /* 8 iInterface - No string descriptor */

  /* Header Functional descriptor */
  0x05,                     /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,/* 1 bDescriptortype, CS_INTERFACE */ //0x24
  0x00,                     /* 2 bDescriptorsubtype, HEADER */
  0x10, 0x01,               /* 3 bcdCDC */

  /* Ethernet Networking Functional descriptor */
  0x0D,                     /* 0 bLength - 13 bytes */
  USB_DESCTYPE_CS_INTERFACE,/* 1 bDescriptortype, CS_INTERFACE */
  0x0F,                     /* 2 bDescriptorsubtype, ETHERNET NETWORKING */
  // !!! MAC need check //index 6 in USBD_GetDescriptor() (usb/Core/usbd_ctlreq.c) ??????
  0x06,                     /* 3 iMACAddress, Index of MAC address string */ 

  0x00,0x00,0x00,0x00,      /* 4 bmEthernetStatistics - Handles None */
  0xEA,05,                  /* 8 wMaxSegmentSize - 1514 bytes */
  0x00,0x00,                /* 10 wNumberMCFilters - No multicast filters */
  0x00,                     /* 12 bNumberPowerFilters - No wake-up feature */

  /* Data IN Endpoint descriptor */
  0x07,                     /* 0 bLength */
  USB_DESC_TYPE_ENDPOINT,   /* 1 bDescriptorType */
  CDC_EEM_IN_EP,            /* 2 bEndpointAddress - IN endpoint*/ // 0x81
  0x02,                     /* 3 bmAttributes - Bulk type */
  LOBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE),
  0x01,                     /* 6 bInterval */

  /* Data OUT Endpoint descriptor */
  0x07,                     /* 0 bLength */
  USB_DESC_TYPE_ENDPOINT,   /* 1 bDescriptorType */
  CDC_EEM_OUT_EP,           /* 2 bEndpointAddress - OUT endpoint */ // 0x01
  0x02,                     /* 3 bmAttributes - Bulk type */
  LOBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE), /* 4 wMaxPacketSize - Low byte */
  HIBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE), /* 5 wMaxPacketSize - High byte */
  0x01                     /* 6 bInterval */
} ;

__ALIGN_BEGIN uint8_t USBD_CDC_EEM_OtherSpeedCfgDesc[USB_CDC_EEM_CONFIG_DESC_SIZ] __ALIGN_END =
{ 
  0x09, /* 0 bLength */
  USB_DESC_TYPE_CONFIGURATION,/* 1 bDescriptortype - Configuration*/
  0x32,00,                  /* 2 wTotalLength */ /*USB_CDC_ECM_CONFIG_DESC_SIZ*/
  0x01,                     /* 4 bNumInterfaces */
  0x01,                     /* 5 bConfigurationValue */
  0x04,                     /* 6 iConfiguration - index of string */
  0x80,                     /* 7 bmAttributes - Bus powered */
  0xC8,                     /* 8 bMaxPower - 400mA */

  /* CDC Communication interface */
  0x09,                     /* 0 bLength */
  USB_DESC_TYPE_INTERFACE,  /* 1 bDescriptorType - Interface */  //0x04
  0x00,                     /* 2 bInterfaceNumber - Interface 0 */
  0x00,                     /* 3 bAlternateSetting */
  0x02,                     /* 4 bNumEndpoints */
  0x02,                     /* 5 bInterfaceClass */ /*USB_CLASS_COMMUNICATIONS*/
  0x0c,                     /* 6 bInterfaceSubClass - Ethernet Control Model */ /*USB_CDC_ECM_SUBCLASS*/
  0x07,                     /* 7 bInterfaceProtocol - No specific protocol */
  0x00,                     /* 8 iInterface - No string descriptor */

  /* Header Functional descriptor */
  0x05,                     /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,/* 1 bDescriptortype, CS_INTERFACE */ //0x24
  0x00,                     /* 2 bDescriptorsubtype, HEADER */
  0x10, 0x01,               /* 3 bcdCDC */

  /* Ethernet Networking Functional descriptor */
  0x0D,                     /* 0 bLength - 13 bytes */
  USB_DESCTYPE_CS_INTERFACE,/* 1 bDescriptortype, CS_INTERFACE */
  0x0F,                     /* 2 bDescriptorsubtype, ETHERNET NETWORKING */
  // !!! MAC need check //index 6 in USBD_GetDescriptor() (usb/Core/usbd_ctlreq.c) ??????
  0x06,                     /* 3 iMACAddress, Index of MAC address string */ 

  0x00,0x00,0x00,0x00,      /* 4 bmEthernetStatistics - Handles None */
  0xEA,05,                  /* 8 wMaxSegmentSize - 1514 bytes */
  0x00,0x00,                /* 10 wNumberMCFilters - No multicast filters */
  0x00,                     /* 12 bNumberPowerFilters - No wake-up feature */

  /* Data IN Endpoint descriptor */
  0x07,                     /* 0 bLength */
  USB_DESC_TYPE_ENDPOINT,   /* 1 bDescriptorType */
  CDC_EEM_IN_EP,            /* 2 bEndpointAddress - IN endpoint*/ // 0x81
  0x02,                     /* 3 bmAttributes - Bulk type */
  LOBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE),
  0x01,                     /* 6 bInterval */

  /* Data OUT Endpoint descriptor */
  0x07,                     /* 0 bLength */
  USB_DESC_TYPE_ENDPOINT,   /* 1 bDescriptorType */
  CDC_EEM_OUT_EP,           /* 2 bEndpointAddress - OUT endpoint */ // 0x01
  0x02,                     /* 3 bmAttributes - Bulk type */
  LOBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE), /* 4 wMaxPacketSize - Low byte */
  HIBYTE(CDC_EEM_DATA_FS_MAX_PACKET_SIZE), /* 5 wMaxPacketSize - High byte */
  0x01                     /* 6 bInterval */
};

/**
  * @}
  */ 

/** @defgroup USBD_CDC_EEM_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_CDC_EEM_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_EEM_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  uint8_t ret = 0;
  uint32_t buf_size;
  uint32_t bpool_full;
  USBD_CDC_EEM_HandleTypeDef   *hcdc;
  
  if(pdev->dev_speed == USBD_SPEED_HIGH  ) {  
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev,
                   CDC_EEM_OUT_EP,
                   USBD_EP_TYPE_BULK,
                   CDC_EEM_DATA_HS_OUT_PACKET_SIZE);
    /* Open IN EP */
    USBD_LL_OpenEP(pdev,
                   CDC_EEM_IN_EP,
                   USBD_EP_TYPE_BULK,
                   CDC_EEM_DATA_HS_IN_PACKET_SIZE);
    
  }
  else {
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev,
                   CDC_EEM_OUT_EP,
                   USBD_EP_TYPE_BULK,
                   CDC_EEM_DATA_FS_OUT_PACKET_SIZE);
    /* Open IN EP */
    USBD_LL_OpenEP(pdev,
                   CDC_EEM_IN_EP,
                   //USBD_EP_TYPE_INTR, 
                   USBD_EP_TYPE_BULK,
                   CDC_EEM_DATA_FS_IN_PACKET_SIZE);
  }
  
    
  pdev->pClassData = USBD_malloc(sizeof (USBD_CDC_EEM_HandleTypeDef));
  
  if(pdev->pClassData == NULL) {
    ret = 1; 
  }
  else {
    hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
    
    /* Init  physical Interface components */
    ((USBD_CDC_EEM_ItfTypeDef *)pdev->pUserData)->Init();
    
    /* Init Xfer states */
    hcdc->tx_state = EEM_TRANSMITTER_INITIAL;

    hcdc->rx_state = EEM_RECEIVER_INITIAL;
    hcdc->rx_buf_wr_pos = 0;
    hcdc->rx_cnt_received = 0;
    hcdc->rx_cnt_processed = 0;
    hcdc->rx_cnt_sending = 0;
    hcdc->rx_cnt_sent = 0;
    hcdc->rx_buffer = alloc_eem_pkt_buffer(EEM_RX_BUFFER, hcdc->rx_cnt_received, hcdc->rx_cnt_sent, &buf_size, NULL, &bpool_full);
    hcdc->rx_buf_size = buf_size;

    if(pdev->dev_speed == USBD_SPEED_HIGH  ) {      
      /* Prepare Out endpoint to receive next packet */
      USBD_LL_PrepareReceive(pdev,
                             CDC_EEM_OUT_EP,
                             hcdc->rx_buffer,
                             CDC_EEM_DATA_HS_OUT_PACKET_SIZE);
    }
    else {
      /* Prepare Out endpoint to receive next packet */
      USBD_LL_PrepareReceive(pdev,
                             CDC_EEM_OUT_EP,
                             hcdc->rx_buffer,
                             CDC_EEM_DATA_FS_OUT_PACKET_SIZE);
    }
  }
  return ret;
}

/**
  * @brief  USBD_CDC_EEM_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_EEM_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  uint8_t ret = 0;
  
  /* Close EP OUT */
  USBD_LL_CloseEP(pdev,
              CDC_EEM_OUT_EP);
  
  /* Close IN EP */
  USBD_LL_CloseEP(pdev,
              CDC_EEM_IN_EP);
  
  
  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL) {
    ((USBD_CDC_EEM_ItfTypeDef *)pdev->pUserData)->DeInit();
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  
  return ret;
}

/**
  * @brief  USBD_CDC_EEM_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_CDC_EEM_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  USBD_CDC_EEM_HandleTypeDef   *hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
  static uint8_t ifalt = 0;
    
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    if (req->wLength)
    {
      if (req->bmRequest & 0x80)
      {
        ((USBD_CDC_EEM_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                          (uint8_t *)hcdc->data,
                                                          req->wLength);
          USBD_CtlSendData (pdev, 
                            (uint8_t *)hcdc->data,
                            req->wLength);
      }
      else
      {
        hcdc->CmdOpCode = req->bRequest;
        hcdc->CmdLength = req->wLength;
        
        USBD_CtlPrepareRx (pdev, 
                           (uint8_t *)hcdc->data,
                           req->wLength);
      }
      
    }
    else
    {
      ((USBD_CDC_EEM_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                        (uint8_t*)req,
                                                        0);
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        &ifalt,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      break;
    }
 
  default: 
    break;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CDC_EEM_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_EEM_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum) {

  uint32_t tx_len;
  uint8_t *tx_buf;
  USBD_CDC_EEM_HandleTypeDef *hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
  USBD_StatusTypeDef ret_code;
  
  if(pdev->pClassData != NULL) {

    if (hcdc->tx_state == EEM_TRANSMITTER_SEND_DATA){
      tx_len = (hcdc->tx_length >= CDC_EEM_DATA_FS_IN_PACKET_SIZE) ? CDC_EEM_DATA_FS_IN_PACKET_SIZE:hcdc->tx_length;
      if (tx_len != 0){
#if 0 /* code added for experiment */
        if (tx_len < CDC_EEM_DATA_FS_IN_PACKET_SIZE){
          hcdc->tx_state = EEM_TRANSMITTER_SENDING_ZPKT;
        }
#endif
        tx_buf = hcdc->tx_buffer;

        hcdc->tx_buffer += tx_len;
        hcdc->tx_length -= tx_len;
        while ((ret_code = USBD_LL_Transmit(pdev,
                                            CDC_EEM_IN_EP,
                                            tx_buf,
                                            tx_len)) == USBD_BUSY)
          ;
        if (ret_code != USBD_OK) {
          hcdc->tx_state = EEM_TRANSMITTER_FAULT;
          return ret_code;
        }
      }else
      {
        hcdc->tx_state = EEM_TRANSMITTER_INITIAL;
        hcdc->rx_cnt_sent ++;
      }
#if 0 /* code added for experiment */
    }else if (hcdc->tx_state == EEM_TRANSMITTER_SENDING_ZPKT){
      hcdc->tx_state = EEM_TRANSMITTER_SENT_ZPKT;
      while ((ret_code = USBD_LL_Transmit(pdev,
                                          CDC_EEM_IN_EP,
                                          hcdc->tx_buffer,
                                          0)) == USBD_BUSY)
        ;
      if (ret_code != USBD_OK) {
        hcdc->tx_state = EEM_TRANSMITTER_FAULT;
        return ret_code;
      }
    }
    else if (hcdc->tx_state == EEM_TRANSMITTER_SEND_EEM_ZPKT ||
             hcdc->tx_state == EEM_TRANSMITTER_SENT_ZPKT){
#else
    }else if (hcdc->tx_state == EEM_TRANSMITTER_SEND_EEM_ZPKT){
#endif
      /* sent 2 bytes */
      hcdc->tx_state = EEM_TRANSMITTER_INITIAL;
    }else{
    }
    return USBD_OK;
  }
  else {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_CDC_EEM_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */

static uint32_t eem_packet_type, eem_packet_size, eem_cmd;

static uint8_t  USBD_CDC_EEM_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum) {      

  USBD_CDC_EEM_HandleTypeDef *hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
  uint32_t rx_data_size;
  
  if(pdev->pClassData != NULL) {
    rx_data_size  = USBD_LL_GetRxDataSize (pdev, epnum);

    switch (hcdc->rx_state){
      case EEM_RECEIVER_INITIAL: // initial state

        eem_packet_type = (hcdc->rx_buffer[1] & 0x80) ? EEM_CMD_PACKET : EEM_DATA_PACKET;
        /* get eem header */
        if (eem_packet_type == EEM_CMD_PACKET){
          eem_cmd = ((((uint32_t)hcdc->rx_buffer[1]) & 0x03) >> 3);
          eem_packet_size = (uint32_t)hcdc->rx_buffer[0] | ((((uint32_t)hcdc->rx_buffer[1]) & 0x03) << 8);
          if (eem_packet_size > hcdc->rx_buf_size){
            hcdc->rx_state = EEM_RECEIVER_FAULT;
            return USBD_FAIL;
          }

          if (rx_data_size >= eem_packet_size + 2){/* usb packet contains whole eem packet */
            hcdc->rx_state = EEM_RECEIVER_CMD_DONE;
            hcdc->rx_buf_wr_pos = 0;
          }else{
            hcdc->rx_state = EEM_RECEIVER_CMD_IN_PROGRESS;
            hcdc->rx_buf_wr_pos += rx_data_size;
          }

        }else{
          eem_packet_size = (uint32_t)hcdc->rx_buffer[0] | ((((uint32_t)hcdc->rx_buffer[1]) & 0x3F) << 8);
          if (eem_packet_size > hcdc->rx_buf_size){
            hcdc->rx_state = EEM_RECEIVER_FAULT;
            return USBD_FAIL;
          }

          if (rx_data_size >= eem_packet_size + 2){/* usb packet contains whole eem packet */
            uint32_t buf_size;
            uint32_t bpool_full;

            hcdc->rx_state = EEM_RECEIVER_INITIAL;
            hcdc->rx_buffer = alloc_eem_pkt_buffer(EEM_RX_BUFFER, hcdc->rx_cnt_received + 1, hcdc->rx_cnt_sent, &buf_size, hcdc->rx_buffer, &bpool_full);
            hcdc->rx_buf_wr_pos = 0;

            if (!bpool_full)
              hcdc->rx_cnt_received ++;
          }else{
            hcdc->rx_state = EEM_RECEIVER_DATA_IN_PROGRESS;
            hcdc->rx_buf_wr_pos += rx_data_size;
          }
        }
      break;

      case EEM_RECEIVER_CMD_IN_PROGRESS: /* command reception in progress */
        /* now we ignore command reception: after all cmd received we are ready receive next packet  */
        if ((hcdc->rx_buf_wr_pos + rx_data_size) >= (eem_packet_size + 2)){

          hcdc->rx_state = EEM_RECEIVER_INITIAL;
          hcdc->rx_buf_wr_pos = 0;
        }else{
          hcdc->rx_buf_wr_pos += rx_data_size;
        }  
      break;

      case EEM_RECEIVER_DATA_IN_PROGRESS: /* data reception in progress */

        if ((hcdc->rx_buf_wr_pos + rx_data_size)  >= (eem_packet_size + 2)){
          uint32_t buf_size;
          uint32_t bpool_full;

          hcdc->rx_state = EEM_RECEIVER_INITIAL;
          hcdc->rx_buffer = alloc_eem_pkt_buffer(EEM_RX_BUFFER, hcdc->rx_cnt_received + 1, hcdc->rx_cnt_sent, &buf_size, hcdc->rx_buffer, &bpool_full);
          hcdc->rx_buf_size = buf_size;
          hcdc->rx_buf_wr_pos = 0;

          if (!bpool_full)
            hcdc->rx_cnt_received ++;
        }else{
          hcdc->rx_buf_wr_pos += rx_data_size;
        }

      break;

      default:
      break;
    }

    if (USBD_LL_PrepareReceive(pdev, 
                               CDC_EEM_OUT_EP, 
                               &hcdc->rx_buffer[hcdc->rx_buf_wr_pos], 
                               CDC_EEM_DATA_FS_OUT_PACKET_SIZE) == USBD_FAIL){
      hcdc->rx_state = EEM_RECEIVER_FAULT;
      return USBD_FAIL;

    }

    return USBD_OK;


  }
  else {
    return USBD_FAIL;
  }
}



/**
  * @brief  USBD_CDC_EEM_EP0_RxReady
  *         Data received on control endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_EEM_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
  USBD_CDC_EEM_HandleTypeDef   *hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
  
  if((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFF))
  {
    ((USBD_CDC_EEM_ItfTypeDef *)pdev->pUserData)->Control(hcdc->CmdOpCode,
                                                      (uint8_t *)hcdc->data,
                                                      hcdc->CmdLength);
      hcdc->CmdOpCode = 0xFF; 
      
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CDC_EEM_GetFSCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_EEM_GetFSCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_CDC_EEM_CfgFSDesc);
  return USBD_CDC_EEM_CfgFSDesc;
}

/**
  * @brief  USBD_CDC_EEM_GetHSCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_EEM_GetHSCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_CDC_EEM_CfgHSDesc);
  return USBD_CDC_EEM_CfgHSDesc;
}

/**
  * @brief  USBD_CDC_EEM_GetCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_EEM_GetOtherSpeedCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_CDC_EEM_OtherSpeedCfgDesc);
  return USBD_CDC_EEM_OtherSpeedCfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_CDC_EEM_GetDeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_CDC_EEM_DeviceQualifierDesc);
  return USBD_CDC_EEM_DeviceQualifierDesc;
}

/**
* @brief  USBD_CDC_EEM_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t  USBD_CDC_EEM_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_CDC_EEM_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;
  
  if(fops != NULL)
  {
    pdev->pUserData= fops;
    ret = USBD_OK;    
  }
  
  return ret;
}

#if 0
/**
  * @brief  USBD_CDC_EEM_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t  USBD_CDC_EEM_SetTxBuffer  (USBD_HandleTypeDef   *pdev,
                                uint8_t  *pbuff,
                                uint16_t buf_size,
                                uint16_t length)
{
  USBD_CDC_EEM_HandleTypeDef   *hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
  
  hcdc->TxBuffer = pbuff;
  hcdc->TxBufferSize = buf_size;
  hcdc->TxBufRdPos = 0;
  hcdc->TxLength = length;  
  
  return USBD_OK;  
}
#endif

#if 0
/**
  * @brief  USBD_CDC_EEM_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t  USBD_CDC_EEM_SetRxBuffer  (USBD_HandleTypeDef   *pdev,
                                   uint8_t  *pbuff,
                                   uint16_t buf_size)
{
  USBD_CDC_EEM_HandleTypeDef   *hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
  
  hcdc->RxBuffer = pbuff;
  hcdc->RxBufferSize = buf_size;
  hcdc->RxBufWrPos = 0;
  hcdc->RxLength = 0;

  
  return USBD_OK;
}
#endif

/**
  * @brief  USBD_CDC_EEM_TransmitPacket
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  USBD_CDC_EEM_TransmitPacket(USBD_HandleTypeDef *pdev, eem_transmitter_state_enum tx_state)
{      
  uint32_t tx_len;
  uint8_t *tx_buf;
  USBD_CDC_EEM_HandleTypeDef   *hcdc = (USBD_CDC_EEM_HandleTypeDef*) pdev->pClassData;
  
  if(pdev->pClassData != NULL) {
    if (hcdc->tx_state == EEM_TRANSMITTER_INITIAL) {
      /* Tx Transfer in progress */
      if (hcdc->tx_length == 80){
        hcdc->tx_state = tx_state;

      }

      hcdc->tx_state = tx_state;
      
      tx_len = (hcdc->tx_length >= CDC_EEM_DATA_FS_IN_PACKET_SIZE) ? CDC_EEM_DATA_FS_IN_PACKET_SIZE:hcdc->tx_length;
      tx_buf = hcdc->tx_buffer;

      hcdc->tx_length -= tx_len;
      hcdc->tx_buffer += tx_len;
      
      /* Transmit next packet */
      USBD_LL_Transmit(pdev,
                       CDC_EEM_IN_EP,
                       tx_buf,
                       tx_len);
      return USBD_OK;
    }
    else {
      return USBD_BUSY;
    }
  }
  else {
    return USBD_FAIL;
  }
}


/**
  * @brief  USBD_CDC_EEM_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
#if 0
uint8_t  USBD_CDC_EEM_ReceivePacket(USBD_HandleTypeDef *pdev,
                                    uint8_t*pbuf)
{      
  /* Suspend or Resume USB Out process */
  if(pdev->pClassData != NULL)
  {
    if(pdev->dev_speed == USBD_SPEED_HIGH  ) 
    {      
      /* Prepare Out endpoint to receive next packet */
      USBD_LL_PrepareReceive(pdev,
                             CDC_EEM_OUT_EP,
                             pbuf,
                             CDC_EEM_DATA_HS_OUT_PACKET_SIZE);
    }
    else
    {
      /* Prepare Out endpoint to receive next packet */
      USBD_LL_PrepareReceive(pdev,
                             CDC_EEM_OUT_EP,
                             pbuf,
                             CDC_EEM_DATA_FS_OUT_PACKET_SIZE);
    }
    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}
#endif
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

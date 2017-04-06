/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
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
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
//!!!
#include "usbd_cdc_eem.h"
#include "usbd_cdc_eem_if.h"
#include "usbd_buffers.h"
#include "timer.h"
#include "uip.h"
#include "uip_arp.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
uint8_t *uip_buf;
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static uint32_t eem_add_crc(uint8_t*buf, uint32_t ip_len){
  buf[ip_len++] = 0xDE;
  buf[ip_len++] = 0xAD;
  buf[ip_len++] = 0xBE;
  buf[ip_len++] = 0xEF;
  return ip_len;
}

static uint32_t process_incoming_packet(USBD_CDC_EEM_HandleTypeDef*hcdc){
  uip_buf = hcdc->tx_buffer + 2;
  uip_len = get_eem_pkt_size(hcdc->tx_buffer);

  if (uip_len > 0) {
    if (BUF->type == htons(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_input();
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
      if(uip_len > 0) {
        uip_arp_out();
        uip_len = eem_add_crc(uip_buf, uip_len /*DEADBEEF*/);
        set_eem_pkt_size(hcdc->tx_buffer, uip_len);
        uip_len += 2;
        /* send data */
      }else{
        uip_len = 0;
      }
    } else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
      if (uip_len > 0) {
        uip_len = eem_add_crc(uip_buf, uip_len /*DEADBEEF*/);
        set_eem_pkt_size(hcdc->tx_buffer, uip_len);
        uip_len += 2;
        /* send data */
      }else{
        uip_len = 0;
      }
    }else{ /* ipv6 and other */
      uip_len = 0;
    }
  }
  return uip_len;
}


static uint32_t process_ip_timers(USBD_CDC_EEM_HandleTypeDef*hcdc,
                                  struct timer *periodic_timer, 
                                  struct timer *arp_timer){

  uint32_t i;

  uip_buf = hcdc->tx_buffer + 2;
  uip_len = 0;

  if (timer_expired(periodic_timer)) {
    timer_reset(periodic_timer);
    for(i = 0; i < UIP_CONNS; i++) {
      uip_periodic(i);
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
      if(uip_len > 0) {
        uip_arp_out();
        uip_len = eem_add_crc(uip_buf, uip_len /*DEADBEEF*/);
        set_eem_pkt_size(hcdc->tx_buffer, uip_len);
        uip_len += 2;
        hcdc->tx_length = uip_len;
        while( USBD_CDC_EEM_TransmitPacket(&hUsbDeviceFS, EEM_TRANSMITTER_SEND_DATA) == USBD_BUSY)
          ;
      }
    }

#if UIP_UDP
    for(i = 0; i < UIP_UDP_CONNS; i++) {
      uip_udp_periodic(i);
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
      if(uip_len > 0) {
        uip_arp_out();
        uip_len = eem_add_crc(uip_buf, uip_len /*DEADBEEF*/);
        set_eem_pkt_size(hcdc->tx_buffer, ip_len);
        uip_len += 2;
        hcdc->tx_length = ip_len;
        while( USBD_CDC_EEM_TransmitPacket(&hUsbDeviceFS, EEM_TRANSMITTER_SEND_DATA) == USBD_BUSY)
          ;
      }
    }
#endif /* UIP_UDP */
    /* Call the ARP timer function every 10 seconds. */
    if (timer_expired(arp_timer)) {
      timer_reset(arp_timer);
      uip_arp_timer();
    }
  }else{
    /* send zero eem packet */
#if 0 /* code added for experiment */
    hcdc->tx_buffer[0] = 0;
    hcdc->tx_buffer[1] = 0;
    hcdc->tx_length = 2;
    USBD_CDC_EEM_TransmitPacket(&hUsbDeviceFS, EEM_TRANSMITTER_SEND_EEM_ZPKT);
#endif
  }
  return uip_len;
}
/* USER CODE END 0 */

int main(void) {

  /* USER CODE BEGIN 1 */
  uint32_t tx_len;
  uint8_t *buf;
  struct uip_eth_addr brd_eth_addr = {0x11,0x12,0x13,0x13,0x14,0x15};

  USBD_CDC_EEM_HandleTypeDef   *hcdc;

  uip_ipaddr_t ipaddr;
  struct timer periodic_timer, arp_timer;
  
  timer_set(&periodic_timer, CLOCK_SECOND / 2);
  timer_set(&arp_timer, CLOCK_SECOND * 10);

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

  uip_init();

  uip_setethaddr(brd_eth_addr);
  uip_ipaddr(ipaddr, 10,11,12,2);
  uip_sethostaddr(ipaddr);
  uip_ipaddr(ipaddr, 255,255,255,0);
  uip_setnetmask(ipaddr);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1){
    hcdc = (USBD_CDC_EEM_HandleTypeDef*) hUsbDeviceFS.pClassData;
    if (hcdc){

      if (hcdc->rx_cnt_received > hcdc->rx_cnt_sent){
        if (hcdc->rx_cnt_received > hcdc->rx_cnt_processed){
          while (hcdc->tx_state != EEM_TRANSMITTER_INITIAL)
            ;
          buf = get_eem_pkt_buffer(EEM_RX_BUFFER, hcdc->rx_cnt_processed, NULL);
          hcdc->tx_buffer = buf;
          hcdc->tx_buf_size = EEM_TX_BUFFER;
          tx_len = process_incoming_packet(hcdc);
          hcdc->rx_cnt_processed ++;

          if (!tx_len){
            /* nothing to send */
            hcdc->rx_cnt_sent ++;
          }else{
          }
          hcdc->rx_cnt_sending ++;
        }
        if (hcdc->rx_cnt_sending > hcdc->rx_cnt_sent){ /* send packet to host */
           /* rx_cnt_sent increased after packet sent in USBD_CDC_EEM_DataIn() */
           while (hcdc->tx_state != EEM_TRANSMITTER_INITIAL)
             ;
           if (hcdc->rx_cnt_sending > hcdc->rx_cnt_sent) {
             buf = get_eem_pkt_buffer(EEM_RX_BUFFER, hcdc->rx_cnt_sent, NULL);
             hcdc->tx_buffer = buf;
             hcdc->tx_length = get_eem_pkt_size(buf) + 2;
             while (USBD_CDC_EEM_TransmitPacket(&hUsbDeviceFS, EEM_TRANSMITTER_SEND_DATA) == USBD_BUSY)
               ;
           }
        }

      }else{
        while (hcdc->tx_state != EEM_TRANSMITTER_INITIAL)
          ;
        buf = get_eem_pkt_buffer(EEM_TX_BUFFER, 0, NULL);
        hcdc->tx_buffer = buf;
        hcdc->tx_buf_size = EEM_TX_BUFFER;
        process_ip_timers(hcdc,
                          &periodic_timer, 
                          &arp_timer);
      }
    }
  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  return 0;
  /* USER CODE END 3 */
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

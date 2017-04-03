/*
* EEM packet buffers
*/
//#include "ARMCM3.h"
#include <string.h>

#include "usbd_cdc_eem.h"
#include "usbd_buffers.h"

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
__ALIGN_BEGIN static uint8_t eem_rx_buffer_pool_fs[EEM_RX_DATA_SIZE * EEM_RX_BUF_CNT]; __ALIGN_END;
__ALIGN_BEGIN static uint8_t eem_tx_buffer_pool_fs[EEM_TX_DATA_SIZE * EEM_TX_BUF_CNT]; __ALIGN_END;

static uint8_t* get_bpool(t_eem_bpool_idx_enum bidx, uint32_t *bpool_size){
  if (bidx == EEM_RX_BUFFER){
    *bpool_size = EEM_RX_BUF_CNT;
    return eem_rx_buffer_pool_fs;
  }else if (bidx == EEM_TX_BUFFER){
    *bpool_size = EEM_TX_BUF_CNT;
    return eem_tx_buffer_pool_fs;
  }else{
    *bpool_size = 0;
    return NULL;
  }
}

int init_eem_pkt_bpool(t_eem_bpool_idx_enum bidx){
  uint32_t bpool_size;
  uint8_t *pbpool = get_bpool(bidx, &bpool_size);
  memset(pbpool, 0, bpool_size * EEM_RX_DATA_SIZE);
  return 0;
}

int deinit_eem_pkt_bpool(t_eem_bpool_idx_enum bidx){
  return 0;
}

uint8_t* alloc_eem_pkt_buffer(t_eem_bpool_idx_enum bidx, 
                              uint32_t n_pkt_received,
                              uint32_t n_pkt_sent,
                              uint32_t *buf_size, 
                              uint8_t  *prev_buf,
                              uint32_t *bpool_full){
  uint32_t bpool_size;
  uint8_t *pbpool = get_bpool(bidx, &bpool_size);
  if (bidx == EEM_RX_BUFFER){
    *buf_size = EEM_RX_DATA_SIZE;
    if (n_pkt_received - n_pkt_sent >= EEM_RX_BUF_CNT){
      *bpool_full = 1;
      return prev_buf;
    }
    *bpool_full = 0;
    return &pbpool[EEM_RX_DATA_SIZE * (n_pkt_received % EEM_RX_BUF_CNT)];
  }else if (bidx == EEM_TX_BUFFER){
    *buf_size = EEM_TX_DATA_SIZE;
    *bpool_full = 1;
    return prev_buf;
  }
  return NULL;
}


uint8_t* get_eem_pkt_buffer(t_eem_bpool_idx_enum bidx, 
                            uint32_t n_pkt,
                            uint32_t *buf_size){
  uint32_t bpool_size;
  uint8_t *pbpool = get_bpool(bidx, &bpool_size);

  if (bidx == EEM_RX_BUFFER){
    if (buf_size)
      *buf_size = EEM_RX_DATA_SIZE;
    return &pbpool[EEM_RX_DATA_SIZE * (n_pkt % EEM_RX_BUF_CNT)];
  }else if (bidx == EEM_TX_BUFFER){
    if (buf_size)
      *buf_size = EEM_TX_DATA_SIZE;
    return &pbpool[EEM_TX_DATA_SIZE * (n_pkt % EEM_TX_BUF_CNT)];
  }
  return NULL;
}

uint32_t get_eem_pkt_size(uint8_t *buffer){
  return (uint32_t)buffer[0] | ((((uint32_t)buffer[1]) & 0x3F) << 8);
}


void set_eem_pkt_size(uint8_t *buffer, uint32_t pkt_size){
  buffer[0] = (uint8_t)(pkt_size & 0xFF);
  buffer[1] = (uint8_t)((pkt_size >> 8) & 0x3F);
  return;
}


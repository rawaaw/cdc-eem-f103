#if !defined __USBD_BUFFERS_H__
#define __USBD_BUFFERS_H__

#define BPOOL_FLAG_BPOOL_FULL 0x00000001

typedef enum _eem_bpool_idx_enum {
  EEM_RX_BUFFER = 0,
  EEM_TX_BUFFER = 1,
  EEM_RX_DUMMY_BUFFER = 2
}t_eem_bpool_idx_enum;


int init_eem_pkt_bpool(t_eem_bpool_idx_enum bidx);
int deinit_eem_pkt_bpool(t_eem_bpool_idx_enum bidx);

uint8_t* alloc_eem_pkt_buffer(t_eem_bpool_idx_enum bidx, 
                              uint32_t n_pkt_received,
                              uint32_t n_pkt_sent,
                              uint32_t*buf_size, 
                              uint8_t *prev_buf,
                              uint32_t*bpool_full);

uint8_t* get_eem_pkt_buffer(t_eem_bpool_idx_enum bidx, 
                            uint32_t n_pkt,
                            uint32_t *buf_size);

uint32_t get_eem_pkt_size(uint8_t *buffer);

void set_eem_pkt_size(uint8_t *buffer, uint32_t pkt_size);

#endif /*__USBD_BUFFERS_H__*/


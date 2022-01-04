#include <stddef.h>
#include <string.h>

#include "sdk_config.h"
#include "nrf_drv_spis.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_spis.h"
#include "stm_spis.h"
#include "sensor_service.h"
#include "control_service.h"
#ifdef STM_SPIS_RINGBUFF
#include "nrf_ringbuf.h"

#define QUEUE_SAMPLE_SIZE 256
#define SENSOR_SAMPLE_NUM 4
#define SENSOR_QUEUE_SIZE (QUEUE_SAMPLE_SIZE*SENSOR_SAMPLE_NUM)

#define SPIS_RX_BUFF_LENGTH 240
#define SPIS_HEADER_LENGTH 4
#define SPIS_TX_BUFF_LENGTH (SPIS_HEADER_LENGTH)
#define SPIS_INSTANCE 1 /**< SPIS instance index. */


NRF_RINGBUF_DEF(ss_data_queue, SENSOR_QUEUE_SIZE);
#endif

static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);/**< SPIS instance. */

static uint8_t m_sensor_data_head[SPIS_HEADER_LENGTH]={0xAA,0xBB,0xCC,0xDD};
static uint8_t m_raw_impdance_data_head[SPIS_HEADER_LENGTH]={0x11,0x22,0x33,0x44};
static uint8_t m_impdance_data_head[SPIS_HEADER_LENGTH]={0x12,0x34,0x56,0x78};
static uint8_t m_tx_buf[SPIS_TX_BUFF_LENGTH]={0xAA,0xBB,0xCC,0xDD};/**< TX buffer. */
static uint8_t m_rx_buf[SPIS_RX_BUFF_LENGTH];    /**< RX buffer. */

//static stm_spis_events_handler_t m_stm_spis_callback = NULL;

static stm_spis_evt_type_t stm_spi_rx_event = STM_SPIS_EVT_IDLE;

stm_spis_evt_type_t stm_spis_get_event(void)
{
    return stm_spi_rx_event;
}

void stm_spis_reset_event(void)
{
    stm_spi_rx_event = STM_SPIS_EVT_IDLE;
}

bool stm_spis_check_ringbuf(void)
{
    if (ss_data_queue.p_cb->wr_idx != ss_data_queue.p_cb->rd_idx)
    {
        return true;
    }
    return false;
}

void stm_spis_adjust_ring_rd(size_t len)
{
    ss_data_queue.p_cb->rd_idx += (QUEUE_SAMPLE_SIZE - len);
}

static void spis_event_handler(nrf_drv_spis_event_t event)
{
    if (event.evt_type == NRF_DRV_SPIS_XFER_DONE)
    {
#ifdef STM_SPIS_RINGBUFF
        ret_code_t err_code;
#endif
        //NRF_LOG_INFO(" Slave Transfer completed. len : %d Received: [%d]", event.rx_amount, m_rx_buf[4]);
                 //NRF_LOG_HEXDUMP_INFO(m_tx_buf, strlen((const char *)m_tx_buf));
                 //NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));

        if(memcmp(m_rx_buf, m_sensor_data_head, SPIS_HEADER_LENGTH) == 0 )
        {
            size_t len = EEG_ATTR_VALUE_LENGTH;
//          NRF_LOG_ERROR("wr : %d, rd : %d, wr-rd : %d, avail : %d ", ss_data_queue.p_cb->wr_idx, ss_data_queue.p_cb->rd_idx, (ss_data_queue.p_cb->wr_idx -  ss_data_queue.p_cb->rd_idx), SENSOR_QUEUE_SIZE - (ss_data_queue.p_cb->wr_idx -  ss_data_queue.p_cb->rd_idx));

            err_code= nrf_ringbuf_cpy_put(&ss_data_queue,&m_rx_buf[4], &len);

            if(len!=EEG_ATTR_VALUE_LENGTH)
            {
                NRF_LOG_ERROR(" sensor data put err len:%d", len);
            }
            else
            {
                ss_data_queue.p_cb->wr_idx += (QUEUE_SAMPLE_SIZE - len);
            }

  //          if((m_stm_spis_callback!=NULL) && (err_code==NRF_SUCCESS))
            if(err_code==NRF_SUCCESS)
            {
                stm_spi_rx_event = STM_SPIS_EVT_SEND_SENSOR_DATA;
            }
            else
            {
                NRF_LOG_INFO(" Slave Transfer fail. buffer put error");
            }
        }
        else if(memcmp(m_rx_buf, m_impdance_data_head, SPIS_HEADER_LENGTH) == 0)
        {
            size_t len = IV_ATTR_VALUE_LENGTH;

            err_code = nrf_ringbuf_cpy_put(&ss_data_queue, &m_rx_buf[4], &len);

            if(len!=IV_ATTR_VALUE_LENGTH)
            {
                NRF_LOG_ERROR("impedance data put err len : %d",len);
            }
            else
            {
                ss_data_queue.p_cb->wr_idx += (QUEUE_SAMPLE_SIZE - len);
            }

  //          if((m_stm_spis_callback!=NULL) && (err_code==NRF_SUCCESS))
            if(err_code==NRF_SUCCESS)
            {
                stm_spi_rx_event = STM_SPIS_EVT_SEND_IMPEDANCE_DATA;
            }
            else
            {
                NRF_LOG_INFO(" Slave Transfer fail. buffer put error");
            }
        }
        else if(memcmp(m_rx_buf, m_raw_impdance_data_head, SPIS_HEADER_LENGTH) == 0)
        {
            size_t len = (IV_ATTR_VALUE_LENGTH*4);

            err_code = nrf_ringbuf_cpy_put(&ss_data_queue, &m_rx_buf[4], &len);

            if(len!=(IV_ATTR_VALUE_LENGTH*4))
            {
                NRF_LOG_ERROR("raw impedance data put err len : %d",len);
            }
            else
            {
                ss_data_queue.p_cb->wr_idx += (QUEUE_SAMPLE_SIZE - len);
            }

  //          if((m_stm_spis_callback!=NULL) && (err_code==NRF_SUCCESS))
            if(err_code==NRF_SUCCESS)
            {
                stm_spi_rx_event = STM_SPIS_EVT_SEND_RAW_IMPEDANCE_DATA;
            }
            else
            {
                NRF_LOG_INFO(" Slave Transfer fail. buffer put error");
            }
        }

        memset(m_rx_buf, 0, SPIS_RX_BUFF_LENGTH);
        APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_tx_buf, SPIS_TX_BUFF_LENGTH, m_rx_buf, SPIS_RX_BUFF_LENGTH));
        
    }

}

#ifdef STM_SPIS_RINGBUFF
uint32_t sensor_data_get(uint8_t * p_ss_data,size_t * p_size)
{
  ret_code_t err_code;
  size_t len = EEG_ATTR_VALUE_LENGTH;

  err_code = nrf_ringbuf_cpy_get(&ss_data_queue,p_ss_data, &len);
  (*p_size) = len;
  return err_code;

}

void sensor_data_backup(size_t len)
{

    if(((ss_data_queue.p_cb->wr_idx-ss_data_queue.p_cb->rd_idx-len) <SENSOR_QUEUE_SIZE) || (ss_data_queue.p_cb->wr_idx==ss_data_queue.p_cb->rd_idx))
    {
      ss_data_queue.p_cb->rd_idx -=len;
    }else{
        NRF_LOG_INFO("sensor data rollback err wr_idx:0x%x rd_idx:0x%x",ss_data_queue.p_cb->wr_idx,ss_data_queue.p_cb->rd_idx);
    }
}

void sensor_buff_reset(void)
{
    nrf_ringbuf_init(&ss_data_queue);
}
#endif
uint32_t stm_spis_init(void)
//uint32_t stm_spis_init(stm_spis_events_handler_t events_handler)
{

    uint32_t              err_code;
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG;
    spis_config.mode                  = NRF_SPIS_MODE_1;
    spis_config.csn_pin               = STM_SPIS_CS_PIN;
    spis_config.miso_pin              = STM_SPIS_MISO_PIN;
    spis_config.mosi_pin              = STM_SPIS_MOSI_PIN;
    spis_config.sck_pin               = STM_SPIS_SCK_PIN;

    //if (events_handler == NULL)
    //{
    //    return NRF_ERROR_NULL;
    //}

#ifdef STM_SPIS_RINGBUFF
    nrf_ringbuf_init(&ss_data_queue);
#endif

    err_code = nrf_drv_spis_init(&spis, &spis_config, spis_event_handler);
    APP_ERROR_CHECK(err_code);

//    if(err_code == NRF_SUCCESS)
//    {
//      m_stm_spis_callback = events_handler;
//    }else{
//      return err_code;
//    }


    memset(m_rx_buf, 0, SPIS_RX_BUFF_LENGTH);
    err_code = nrf_drv_spis_buffers_set(&spis, m_tx_buf, SPIS_TX_BUFF_LENGTH, m_rx_buf, SPIS_RX_BUFF_LENGTH);
    APP_ERROR_CHECK(err_code);

    return err_code;

}

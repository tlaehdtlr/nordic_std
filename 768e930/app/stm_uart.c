#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "nordic_common.h"
#include "sdk_config.h"
#include "nrf_libuarte_async.h"
#include "stm_uart.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_ringbuf.h"
#include "ble_gap.h"

/* buffer */
#define UART_TX_BUF uart_tx_data
#define UART_RX_BUF uart_rx_data
#define UART_TX_BUF_SIZE (STM_UART_PKT_LENGHT*STM_UART_BUF_SAMPLE_NUM)
#define UART_RX_BUF_SIZE (STM_UART_PKT_LENGHT*STM_UART_BUF_SAMPLE_NUM)

/* uarte instance */
#define STM_UARTE libuarte

NRF_RINGBUF_DEF(UART_TX_BUF, UART_TX_BUF_SIZE);
NRF_RINGBUF_DEF(UART_RX_BUF, UART_RX_BUF_SIZE);

NRF_LIBUARTE_ASYNC_DEFINE(STM_UARTE, 1, 1, NRF_LIBUARTE_PERIPHERAL_NOT_USED, NRF_LIBUARTE_PERIPHERAL_NOT_USED, UART_TX_BUFFER_LENGTH, 10);

static stm_uart_events_handler_t m_stm_uart_callback = NULL;

static uint8_t u_tx_buf[UART_TX_BUFFER_LENGTH];

static stm_uart_tx_state_t stm_uart_tx_status = STM_UART_TX_IDLE;

bool stm_uart_get_tx_status(void)
{
    return stm_uart_tx_status;
}

void stm_uart_set_tx_status(stm_uart_tx_state_t state)
{
    stm_uart_tx_status = state;
}

void stm_uart_process_request()
{
    return ;
}

void stm_uart_adjust_ring_wr(stm_uart_ringbuf_type_t ringbuf, size_t len)
{
    if (ringbuf == STM_UART_RINGBUF_TX)
    {
        UART_TX_BUF.p_cb->wr_idx += (STM_UART_PKT_LENGHT - len);
    }
    else if (ringbuf == STM_UART_RINGBUF_RX)
    {
        UART_RX_BUF.p_cb->wr_idx += (STM_UART_PKT_LENGHT - len);
    }
}

void stm_uart_adjust_ring_rd(stm_uart_ringbuf_type_t ringbuf, size_t len)
{
    if (ringbuf == STM_UART_RINGBUF_TX)
    {
        UART_TX_BUF.p_cb->rd_idx += (STM_UART_PKT_LENGHT - len);
    }
    else if (ringbuf == STM_UART_RINGBUF_RX)
    {
        UART_RX_BUF.p_cb->rd_idx += (STM_UART_PKT_LENGHT - len);
    }
}

bool stm_uart_check_ringbuf(stm_uart_ringbuf_type_t ringbuf)
{
    if (ringbuf == STM_UART_RINGBUF_TX)
    {
        if (UART_TX_BUF.p_cb->wr_idx != UART_TX_BUF.p_cb->rd_idx)
        {
            return true;
        }
        return false;
    }
    else if (ringbuf == STM_UART_RINGBUF_RX)
    {
        if (UART_RX_BUF.p_cb->wr_idx != UART_RX_BUF.p_cb->rd_idx)
        {
            return true;
        }
        return false;
    }

    return true;
}

void stm_uart_get_ble_address(uint8_t * p_data)
{
    ble_gap_addr_t ble_address;
    sd_ble_gap_addr_get(&ble_address);
//    NRF_LOG_INFO("ble address func : %02X %02X %02X %02X %02X %02X", ble_address.addr[5], ble_address.addr[4], ble_address.addr[3], ble_address.addr[2], ble_address.addr[1], ble_address.addr[0]);
    /* LSB -> MSB */
    p_data[0] = ble_address.addr[5];
    p_data[1] = ble_address.addr[4];
    p_data[2] = ble_address.addr[3];
    p_data[3] = ble_address.addr[2];
    p_data[4] = ble_address.addr[1];
    p_data[5] = ble_address.addr[0];

    /* direct acccess FICR reg. but first MSB is strange*/
#if 0
    uint32_t FICR_value_0 = NRF_FICR->DEVICEADDR[1];
    uint32_t FICR_value_1 = NRF_FICR->DEVICEADDR[0];
    uint8_t ficr_ble_address[6];
    ficr_ble_address[0] = FICR_value_0>>8;
    ficr_ble_address[1] = FICR_value_0;
    ficr_ble_address[2] = FICR_value_1>>24;
    ficr_ble_address[3] = FICR_value_1>>16;
    ficr_ble_address[4] = FICR_value_1>>8;
    ficr_ble_address[5] = FICR_value_1;
    NRF_LOG_INFO("NRF_FICR : %02x %02x %02x %02x %02x %02x", ficr_ble_address[0], ficr_ble_address[1], ficr_ble_address[2], ficr_ble_address[3], ficr_ble_address[4], ficr_ble_address[5]);
#endif
}

#if 0
void stm_uart_tx_buff_is_empty(void)
{
    if(UART_TX_BUF.p_cb->wr_idx==UART_TX_BUF.p_cb->rd_idx)
    {
        return true;
    }else{
        return false;
    }
}
#endif

static void check_uart_rx_data(uint8_t * rx_data)
{
    if(rx_data[0] == STM_UART_PKT) //check uart mode
    {
        uint8_t cmd       = rx_data[1];
        uint8_t sub_cmd   = rx_data[2];
        uint8_t data_len  = rx_data[3];
        uint8_t result    = rx_data[4];
        NRF_LOG_INFO("check uart rx data cmd : %d, sub : %d, len : %d, result : %d", cmd, sub_cmd, data_len, result);

        if(data_len == 0)
        {
            NRF_LOG_ERROR("uart rx data len wrong(0)! cmd:%d sub_cmd:%d",cmd, sub_cmd);
            return;
        }

        switch(cmd)
        {
            /*
                response from STM
                need to update service
            */
            case STM_UART_BATTERY:
            case STM_UART_DEVIATION_DET_CHECK:
            case STM_UART_IMPEDANCE_CONTROL:
            case STM_UART_STM_VERSION:
            {
                if((result == 0x00) && (sub_cmd==STM_UART_READ)) //success
                {
                    NRF_LOG_INFO("response from STM, cmd : %d", cmd);
                    uint32_t err_code;
                    size_t len = 0;
                    NRF_LOG_INFO(" uart read rx - cmd %d  length %d payload[0] %d",cmd,data_len,rx_data[5]);
                    len = (size_t)data_len+STM_UART_HEADER_LENGTH;

                    err_code= nrf_ringbuf_cpy_put(&UART_RX_BUF,rx_data, &len);

                    if (len != data_len+STM_UART_HEADER_LENGTH)
                    {
                        NRF_LOG_INFO("There is not enough space at uart rx ring buffer, data_len : %d, len : %d ", data_len+STM_UART_HEADER_LENGTH, len);
                    }
                    stm_uart_adjust_ring_wr(STM_UART_RINGBUF_RX, len);

                    if(err_code != NRF_SUCCESS)
                    {
                        NRF_LOG_INFO(" fail to put the uart rx data - cmd %d err 0x%x", cmd, err_code );
                    }
                }
                break;
            }

            /*
                request from STM
                need to response from nordic
            */
            case STM_UART_BLE_SLEEP:
            case STM_UART_NORDIC_VERSION:
            case STM_UART_BLE_ADDRESS:
            {
                /*
                    don't care about sub, result
                    if need it, add it here
                */
                NRF_LOG_INFO("request from STM, cmd : %d", cmd);
                uint32_t err_code;
                size_t len = 0;
                NRF_LOG_INFO(" uart read rx - cmd %d  length %d payload[0] %d", cmd, data_len+STM_UART_HEADER_LENGTH, rx_data[5]);
                len = (size_t)data_len+STM_UART_HEADER_LENGTH;

                err_code= nrf_ringbuf_cpy_put(&UART_RX_BUF,rx_data, &len);
                if (len != data_len+STM_UART_HEADER_LENGTH)
                {
                    NRF_LOG_INFO("There is not enough space at uart rx ring buffer, data_len : %d, len : %d ", data_len+STM_UART_HEADER_LENGTH, len);
                }
                stm_uart_adjust_ring_wr(STM_UART_RINGBUF_RX, len);

                if(err_code != NRF_SUCCESS)
                {
                    NRF_LOG_INFO(" fail to put the uart rx data - cmd %d err 0x%x", cmd, err_code );
                }

                break;
            }
            default:
            {
                NRF_LOG_INFO("It has been not registerd, cmd : %d \r\n", cmd);
                break;
            }
        }

    }
    else
    {
        NRF_LOG_INFO("receive messgae from stm uart, it does not match tag (0x7D) , %02x", rx_data[0]);
    }
}

static void uart_event_handler(void * context, nrf_libuarte_async_evt_t * p_evt)
{
    nrf_libuarte_async_t * p_libuarte = (nrf_libuarte_async_t *)context;
//    ret_code_t ret;

    switch (p_evt->type)
    {
        case NRF_LIBUARTE_ASYNC_EVT_ERROR:
        {
            NRF_LOG_INFO("NRF_LIBUARTE_ASYNC_EVT_ERROR");
            break;
        }
        case NRF_LIBUARTE_ASYNC_EVT_RX_DATA:
        {
            check_uart_rx_data(p_evt->data.rxtx.p_data);
            nrf_libuarte_async_rx_free(p_libuarte, p_evt->data.rxtx.p_data, p_evt->data.rxtx.length);
            break;
        }
        case NRF_LIBUARTE_ASYNC_EVT_TX_DONE:
#if 0
            if (!uart_tx_buff_is_empty())
            {
                ret_code_t err_code;
                size_t p_size = UART_TX_BUFFER_LENGTH;
                memset(u_tx_buf,0x0,UART_TX_BUFFER_LENGTH);
                err_code = nrf_ringbuf_cpy_get(&UART_TX_BUF,u_tx_buf, &p_size);

                if(err_code == NRF_SUCCESS)
                {
                    UNUSED_RETURN_VALUE(nrf_libuarte_async_tx(&STM_UARTE, u_tx_buf,UART_TX_BUFFER_LENGTH ));
                }
            }
            break;
#else
            {
                stm_uart_set_tx_status(STM_UART_TX_IDLE);
//                NRF_LOG_INFO("Nordic -> STM uart done");
            }
#endif

        default:
            break;
    }
}

uint32_t uart_rx_data_get(uint8_t * p_rx_data,size_t len, size_t * p_size)
{
    ret_code_t err_code;
    *p_size = len;

    err_code = nrf_ringbuf_cpy_get(&UART_RX_BUF,p_rx_data, p_size);
    return err_code;
}

void uart_rx_data_backup(size_t len)
{

    if(((UART_RX_BUF.p_cb->wr_idx-UART_RX_BUF.p_cb->rd_idx-len) <UART_RX_BUF_SIZE) || (UART_RX_BUF.p_cb->wr_idx==UART_RX_BUF.p_cb->rd_idx))
    {
        UART_RX_BUF.p_cb->rd_idx -=len;
    }else{
        NRF_LOG_INFO("uart data rollback err wr_idx:0x%x rd_idx:0x%x",UART_RX_BUF.p_cb->wr_idx,UART_RX_BUF.p_cb->rd_idx);
    }
}

void cli_stm_uart_rx_packet_test(uint8_t * data, uint8_t payload_len)
{

    uint8_t rx_data[UART_TX_BUFFER_LENGTH];

    memset(rx_data,0x0,UART_TX_BUFFER_LENGTH);

    rx_data[0] = 0x7D;    //uart cmd
    rx_data[1] = data[0]; //cmd
    rx_data[2] = data[1]; //subcmd
    rx_data[3] = payload_len; //payload_len

    if(payload_len > 0)
    {
        uint8_t i;
        for(i=0; i<payload_len;i++)
        {
            rx_data[4+i]=data[2+i];
        }
    }

    check_uart_rx_data(rx_data);


}

static void stm_uart_cmd_send(stm_uart_send_evt_t pkt)
{
    ret_code_t ret;
    uint8_t len = pkt.p_data.send_pkt.p_len;

    memset(u_tx_buf,0x0,UART_TX_BUFFER_LENGTH);
    u_tx_buf[0]=STM_UART_PKT;
    u_tx_buf[1]=pkt.main_cmd;
    u_tx_buf[2]=pkt.sub_cmd;
    u_tx_buf[3]=len;
    if(len > 0)
    {
        memcpy(&u_tx_buf[4],pkt.p_data.send_pkt.p_buffer,len);
    }

    ret = nrf_libuarte_async_tx(&STM_UARTE, u_tx_buf,UART_TX_BUFFER_LENGTH );
    stm_uart_set_tx_status(STM_UART_TX_BUSY);
    
    /* todo */
    if (ret == NRF_ERROR_BUSY)
    {
        size_t data_size=UART_TX_BUFFER_LENGTH;

        nrf_ringbuf_cpy_put(&UART_TX_BUF,u_tx_buf, &data_size);
        stm_uart_adjust_ring_wr(STM_UART_RINGBUF_TX, data_size);
        NRF_LOG_INFO(" uart tx busy");
    }

}


/* request */
void stm_uart_read_request(stm_uart_main_cmd_type_t cmd)
{
    stm_uart_send_evt_t pkt;

    pkt.main_cmd = cmd;
    pkt.sub_cmd = STM_UART_READ;
    pkt.p_data.send_pkt.p_len=0;
    pkt.p_data.send_pkt.p_buffer=NULL;

    stm_uart_cmd_send(pkt);
}

void stm_uart_write_request(stm_uart_main_cmd_type_t cmd,uint8_t len,const uint8_t * data)
{
    stm_uart_send_evt_t pkt;
    uint8_t send_data[PAYLOAD_LENGTH_MAX];

    memcpy(send_data, data, len);
    pkt.main_cmd = cmd;
    pkt.sub_cmd=STM_UART_WRITE;
    pkt.p_data.send_pkt.p_len=len;
    pkt.p_data.send_pkt.p_buffer=send_data;

    stm_uart_cmd_send(pkt);
}

void stm_uart_write_request_onoff(stm_uart_main_cmd_type_t cmd,uint8_t onoff)
{
    stm_uart_send_evt_t pkt;

    pkt.main_cmd = cmd;
    pkt.sub_cmd=STM_UART_WRITE;
    pkt.p_data.send_pkt.p_len=1;
    pkt.p_data.send_pkt.p_buffer=&onoff;

    stm_uart_cmd_send(pkt);
}

/*
    response to STM
    write tx ring buffer
    uart tranmit
*/
void stm_uart_write_tx_buffer(uint8_t * p_data)
{
    size_t len = STM_UART_PKT_LENGHT;
    APP_ERROR_CHECK(nrf_ringbuf_cpy_put(&UART_TX_BUF,p_data, &len));
    if (len != STM_UART_PKT_LENGHT)
    {
        NRF_LOG_INFO("There is not enough space at uart tx ring buffer, len : %d \r\n", len);
    }
    stm_uart_adjust_ring_wr(STM_UART_RINGBUF_TX, len);
}

void stm_uart_response(void)
{
    ret_code_t ret;
    size_t len = UART_TX_BUFFER_LENGTH;
    memset(u_tx_buf,0x0,UART_TX_BUFFER_LENGTH);

    nrf_ringbuf_cpy_get(&UART_TX_BUF, u_tx_buf, &len);
    stm_uart_adjust_ring_rd(STM_UART_RINGBUF_TX, len);
    if (len != STM_UART_PKT_LENGHT)
    {
        NRF_LOG_INFO("need to check stm uart tx buffer");
        // NRF_LOG_INFO("TX wr : %d, rd : %d ", UART_TX_BUF.p_cb->wr_idx, UART_TX_BUF.p_cb->rd_idx);
    }

    ret = nrf_libuarte_async_tx(&STM_UARTE, u_tx_buf,UART_TX_BUFFER_LENGTH );
    stm_uart_set_tx_status(STM_UART_TX_BUSY);
    
    /* todo */
    if (ret == NRF_ERROR_BUSY)
    {
        size_t data_size=UART_TX_BUFFER_LENGTH;

        nrf_ringbuf_cpy_put(&UART_TX_BUF,u_tx_buf, &data_size);
        stm_uart_adjust_ring_wr(STM_UART_RINGBUF_TX, data_size);
        NRF_LOG_INFO(" uart tx busy");
    }
}

uint32_t stm_uart_init(stm_uart_events_handler_t events_handler)
{
    ret_code_t err_code;
    nrf_libuarte_async_config_t nrf_libuarte_async_config = {
            .tx_pin     = STM_TX_PIN_NUMBER,
            .rx_pin     = STM_RX_PIN_NUMBER,
            .baudrate   = NRF_UARTE_BAUDRATE_115200,
            .parity     = NRF_UARTE_PARITY_EXCLUDED,
            .hwfc       = NRF_UARTE_HWFC_DISABLED,
            .timeout_us = 100,
            .int_prio   = _PRIO_APP_LOW_MID
    };

    if (events_handler == NULL)
    {
        return NRF_ERROR_NULL;
    }

    err_code = nrf_libuarte_async_init(&STM_UARTE, &nrf_libuarte_async_config, uart_event_handler, (void *)&STM_UARTE);

    APP_ERROR_CHECK(err_code);

    nrf_ringbuf_init(&UART_TX_BUF);
    nrf_ringbuf_init(&UART_RX_BUF);

    nrf_libuarte_async_enable(&STM_UARTE);

    if(err_code == NRF_SUCCESS)
    {
      /* it is possible to delete this */
      m_stm_uart_callback = events_handler;
    }

    return err_code;
}
#ifndef STM_UART_H__
#define STM_UART_H__

#include <stdint.h>
#define STM_TX_PIN_NUMBER 2//30//3
#define STM_RX_PIN_NUMBER 3//31//2
#define STM_UART_PKT      0x7D

#define STM_UART_HEADER_LENGTH 4
#define PAYLOAD_LENGTH_MAX  26
#define UART_TX_BUFFER_LENGTH 30
#define STM_UART_PKT_LENGHT 32
#define STM_UART_BUF_SAMPLE_NUM 8

#define STM_START 0x01
#define STM_STOP 0x00
#define ON  0x01
#define OFF 0x00

#define NORDIC_VERSION_LENGTH 21
#define BLE_ADDRESS_LENGTH 6

typedef enum
{
    STM_UART_TX_IDLE,
    STM_UART_TX_BUSY,
    STM_UART_TX_MAX
} stm_uart_tx_state_t;

typedef enum{
    BLE_STATE_ADVERTISING,
    BLE_STATE_CONNECTED,
    BLE_STATE_MAX
} stm_uart_ble_state_t;

typedef enum{
    BLE_HANDLER_SEND_DATA,
    BLE_HANDLER_SET_SLEEP,
    BLE_HANDLER_EVT_MAX
} stm_uart_handler_evt_t;


typedef enum
{
    STM_UART_BATTERY = 0,   /**< Obligatory to implement. An event indicating that a TX packet*/
    STM_UART_SENSOR_CONTROL,
    STM_UART_NIR_DATA,
    STM_UART_IMPEDANCE_CONTROL,
    STM_UART_VIB_CONTROL,
    STM_UART_DEVIATION_DET_CHECK,
    STM_UART_IMPEDANCE_RAW_DATA,
    STM_UART_STM_VERSION,
    STM_UART_BLE_CONNECT,
    STM_UART_BLE_SLEEP,
    STM_UART_RESET,
    STM_UART_NORDIC_VERSION,
    STM_UART_BLE_ADDRESS,
    STM_UART_MAIN_CMD_MAX
} stm_uart_main_cmd_type_t;

typedef enum
{
    STM_UART_READ,
    STM_UART_WRITE,
    STM_UART_SUB_CMD_MAX
} stm_uart_sub_cmd_type_t;

typedef enum
{
    STM_UART_RINGBUF_TX = 0,
    STM_UART_RINGBUF_RX,
    STM_UART_RINGBUF_MAX
} stm_uart_ringbuf_type_t;

typedef struct
{
    uint8_t p_len;
    uint8_t * p_buffer;
} stm_uart_evt_tx_pkt_params_t;

typedef struct
{
    uint8_t  p_len;
    uint8_t  result;
    uint8_t * p_buffer;

} stm_uart_evt_rx_pkt_params_t;

typedef struct
{
    stm_uart_main_cmd_type_t    main_cmd;
    stm_uart_sub_cmd_type_t     sub_cmd;
    union
    {
        stm_uart_evt_tx_pkt_params_t  send_pkt;
        stm_uart_evt_rx_pkt_params_t  received_pkt;
    } p_data;

} stm_uart_send_evt_t;

typedef void (*stm_uart_events_handler_t)(stm_uart_handler_evt_t evt);

uint32_t stm_uart_init(stm_uart_events_handler_t events_handler);
void cli_stm_uart_rx_packet_test(uint8_t * data, uint8_t payload_len);
void stm_uart_write_request(stm_uart_main_cmd_type_t cmd,uint8_t len,const uint8_t * data);
void stm_uart_write_request_onoff(stm_uart_main_cmd_type_t cmd,uint8_t onoff);
void stm_uart_read_request(stm_uart_main_cmd_type_t cmd);
uint32_t uart_rx_data_get(uint8_t * p_rx_data,size_t len, size_t * p_size);
void uart_rx_data_backup(size_t len);

void stm_uart_get_ble_address(uint8_t * p_data);
void stm_uart_write_tx_buffer(uint8_t * p_data);
void stm_uart_response(void);
bool stm_uart_get_tx_status(void);
void stm_uart_set_tx_status(stm_uart_tx_state_t state);

void stm_uart_adjust_ring_rd(stm_uart_ringbuf_type_t ringbuf, size_t len);
bool stm_uart_check_ringbuf(stm_uart_ringbuf_type_t ringbuf);

#endif
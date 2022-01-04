#ifndef STM_SPIS_H__
#define STM_SPIS_H__


#include <stddef.h>
#include <string.h>
#include "nrf_gpio.h"

#define STM_SPIS_SCK_PIN NRF_GPIO_PIN_MAP(1,9)
#define STM_SPIS_MISO_PIN 4
#define STM_SPIS_MOSI_PIN 5
#define STM_SPIS_CS_PIN 11


typedef enum
{
    STM_SPIS_EVT_IDLE = 0,
    STM_SPIS_EVT_SEND_SENSOR_DATA,   /**< Obligatory to implement. An event indicating that a TX packet*/
    STM_SPIS_EVT_SEND_IMPEDANCE_DATA,
    STM_SPIS_EVT_SEND_RAW_IMPEDANCE_DATA,
    STM_SPIS_EVT_TYPE_MAX           /**< Enumeration upper bound. */
} stm_spis_evt_type_t;

#ifndef STM_SPIS_RINGBUFF
typedef struct
{
    uint8_t * p_buffer;
    uint16_t  num_of_bytes;
} stm_spis_evt_data_t;
#endif
typedef struct
{
    stm_spis_evt_type_t evt_type;
#ifndef STM_SPIS_RINGBUFF
    stm_spis_evt_data_t rx_pkt_received;
#endif
} stm_spis_evt_t;

#ifdef STM_SPIS_RINGBUFF
uint32_t sensor_data_get(uint8_t * p_ss_data, size_t * p_size);
void sensor_data_backup(size_t len);
void sensor_buff_reset(void);
#endif


typedef void (*stm_spis_events_handler_t)(stm_spis_evt_t event);

//uint32_t stm_spis_init(stm_spis_events_handler_t events_handler);
uint32_t stm_spis_init(void);

stm_spis_evt_type_t stm_spis_get_event(void);

void stm_spis_reset_event(void);

bool stm_spis_check_ringbuf(void);

void stm_spis_adjust_ring_rd(size_t len);
#endif

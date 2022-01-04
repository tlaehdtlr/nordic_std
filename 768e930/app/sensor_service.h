
#ifndef SENSOR_SERVICE_H__
#define SENSOR_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"


#define BLE_SS_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */

/**@brief Macro for defining a ble_bas instance.
 *
 * @param   _name  Name of the instance.
 * @hideinitializer
 */
#define BLE_SS_DEF(_name)                          \
    static ble_ss_t _name;                         \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,             \
                         BLE_SS_BLE_OBSERVER_PRIO, \
                         ble_sensor_service_on_ble_evt,        \
                         &_name)



//NRF_SDH_BLE_OBSERVER(m_our_service_observer, APP_BLE_OBSERVER_PRIO, ble_sensor_service_on_ble_evt, (void*) &m_sensor_service);


#define BLE_UUID_SENSOR_BASE_UUID					{0xbb, 0x56, 0x30, 0xed, 0xfa, 0x84, 0x27, 0xba, 0xfa, 0x41, 0x8a, 0x55, 0xbd, 0xd3, 0x01, 0x53} // 128-bit base UUID
#define BLE_UUID_SENSOR_SERVICE_UUID                0x1700 // Just a random, but recognizable value
#define BLE_UUID_SENSOR_CHARACTERISTC_UUID			0x1701
//#define BLE_UUID_NIR_CHARACTERISTC_UUID				0x1702


#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define EEG_ATTR_VALUE_LENGTH			234


typedef enum
{
	BLE_SS_EVT_NOTIFICATION_ENABLED,
    BLE_SS_EVT_NOTIFICATION_DISABLED,
    BLE_SS_EVT_SENSING_START,
	BLE_SS_EVT_SENSING_STOP
}ble_ss_evt_type_t;


typedef struct
{
	ble_ss_evt_type_t evt_type;
}ble_ss_evt_t;


typedef struct ble_ss_s ble_ss_t;


typedef void (*ble_ss_write_handler_t) (uint16_t conn_handle, ble_ss_t * p_ss, ble_ss_evt_t * p_evt);

typedef struct
{
    ble_ss_write_handler_t ss_write_handler; /**< Event handler to be called when the sensor Characteristic is written. */
} ble_ss_init_t;

/**
 * @brief This structure contains various status information for sensor service.
 * The name is based on the naming convention used in Nordic's SDKs.
 * 'ble indicates that it is a Bluetooth Low Energy relevant structure and
 * ss is short for Sensor Service).
 */
struct ble_ss_s
{
    uint16_t    conn_handle;		/**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t    service_handle;     /**< Handle of sensor Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t    sensor_char_handle; /**< Handles for the sensor characteristic attributes */
    ble_gatts_char_handles_t    nir_char_handle; /**< Handles for the nir characteristic attributes */
    ble_ss_write_handler_t ss_write_handler;
    bool sensor_noti_en;
};



/**@brief Function for initializing sensor new service.
 *
 * @param[in]   p_sensor_service       Pointer to sensor Service structure.
 */
void sensor_service_init(ble_ss_t * p_sensor_service, ble_ss_init_t *p_ss_init);

/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_sensor_service        sensor Service structure.
 * @param[in]	eeg_value				eeg data structure pointer
 */
uint32_t sensor_characteristic_update(ble_ss_t *p_sensor_service, uint8_t *data);
#endif  /* _ SENSOR_SERVICE_H__ */


/**@brief Function to take care of some housekeeping of ble connections related to the sensor service and characteristic
 *
 * @param[in]   p_ble_evt       BLE Even
 * @param[in]	p_context		event context
 */
void ble_sensor_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
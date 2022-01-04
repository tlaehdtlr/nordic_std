
#ifndef CONTROL_SERVICE_H__
#define CONTROL_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#define BLE_CS_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */

/**@brief Macro for defining a ble_bas instance.
 *
 * @param   _name  Name of the instance.
 * @hideinitializer
 */
#define BLE_CS_DEF(_name)                          \
    static ble_cs_t _name;                         \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,             \
                         BLE_CS_BLE_OBSERVER_PRIO, \
                         ble_control_service_on_ble_evt,        \
                         &_name)



//NRF_SDH_BLE_OBSERVER(m_our_service_observer, APP_BLE_OBSERVER_PRIO, ble_control_service_on_ble_evt, (void*) &m_control_service);
#define BLE_UUID_CONTROL_BASE_UUID		{0xbb, 0x56, 0x30, 0xed, 0xfa, 0x84, 0x27, 0xba, 0xfa, 0x41, 0x8a, 0x55, 0xbd, 0xd3, 0x02, 0x53} // 128-bit base UUID
#define BLE_UUID_CONTROL_SERVICE_UUID           0x2700 // Just a random, but recognizable value
#define BLE_UUID_NIR_CHAR_UUID			0x2701
#define BLE_UUID_IC_CHAR_UUID			0x2702
#define BLE_UUID_IV_CHAR_UUID			0x2703
#define BLE_UUID_VIB_CHAR_UUID			0x2704
#define BLE_UUID_DD_CHAR_UUID			0x2705


#define APP_BLE_OBSERVER_PRIO                   3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define NIR_ATTR_VALUE_LENGTH			8
#define NIR_ATTR_VALUE_DEFAULT			{0x00,0x01,0x02,0x03,0x14,0x32,0x03,0xff}
#define IC_ATTR_VALUE_LENGTH			1
#define IV_ATTR_VALUE_LENGTH			19
#define VIB_ATTR_VALUE_LENGTH			1
#define DD_ATTR_VALUE_LENGTH			1
#define CTRL_DATA_SIZE				10

typedef enum
{
    BLE_CONTROL_EVT_INVALID,
    BLE_CONTROL_EVT_NIR_ON,
    BLE_CONTROL_EVT_NIR_OFF,
    BLE_CONTROL_EVT_VIB_CONTROL,
    BLE_CONTROL_EVT_IV_NOTI_ON,
    BLE_CONTROL_EVT_IV_NOTI_OFF,
    BLE_CONTROL_EVT_DEV_DETECTED_NOTI_ON,
    BLE_CONTROL_EVT_DEV_DETECTED_NOTI_OFF
} ble_control_evt_type_t;


typedef struct
{
    ble_control_evt_type_t evt_type;                                            /**< Type of event. */
    uint8_t ctl_data[CTRL_DATA_SIZE];
} ble_control_evt_t;

typedef struct ble_cs_s ble_cs_t;


typedef void (*ble_cs_write_handler_t) (uint16_t conn_handle, ble_cs_t * p_cs, const ble_control_evt_t * ctrl_evt);

typedef struct
{
    ble_cs_write_handler_t cs_write_handler; /**< Event handler to be called when the control Characteristic is written. */
} ble_cs_init_t;

/**
 * @brief This structure contains various status information for control service.
 * The name is based on the naming convention used in Nordic's SDKs.
 * 'ble indicates that it is a Bluetooth Low Energy relevant structure and
 * cs is short for Control Service).
 */
struct ble_cs_s
{
    uint16_t			conn_handle;		/**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t			service_handle;     /**< Handle of control Service (as provided by the BLE stack). */
    uint8_t			uuid_type;          /**< UUID type. */
    ble_gatts_char_handles_t    nir_char_handle; /**< Handles for the nir characteristic attributes */
    ble_gatts_char_handles_t    iv_char_handle; /**< Handles for the impedance value characteristic attributes */
    ble_gatts_char_handles_t    vib_char_handle; /**< Handles for the vibrator contro characteristic attributes */
    ble_gatts_char_handles_t    dd_char_handle; /**< Handles for the deviation detect characteristic attributes */
    ble_cs_write_handler_t      cs_write_handler; /**< Handler for the control service write events */
    bool                        imp_value_noti_en;
    bool                        deviation_detect_noti_en;
    uint8_t                     deviation_detect_status_last;

};



/**@brief Function for initializing control new service.
 *
 * @param[in]   p_control_service       Pointer to control Service structure.
 */
void control_service_init(ble_cs_t * p_control_service, ble_cs_init_t *p_cs_init);

/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_control_service        control Service structure.
 * @param[in]	eeg_value				eeg data structure pointer
 */
ret_code_t control_dd_characteristic_update(ble_cs_t *p_control_service, uint8_t data);

ret_code_t control_iv_characteristic_update(ble_cs_t *p_control_service, uint8_t *data);

/**@brief Function to take care of some housekeeping of ble connections related to the control service and characteristic
 *
 * @param[in]   p_ble_evt       BLE Even
 * @param[in]	p_context		event context
 */
void ble_control_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

#endif  /* _ CONTROL_SERVICE_H__ */



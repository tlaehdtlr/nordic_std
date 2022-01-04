
#ifndef TEST_SERVICE_H__
#define TEST_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"


#define BLE_TEST_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */


#define BLE_TT_DEF(_name)                          \
    static ble_tt_t _name;                         \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,             \
                         BLE_TEST_BLE_OBSERVER_PRIO, \
                         ble_test_service_on_ble_evt,        \
                         &_name)



#define BLE_UUID_TEST_BASE_UUID					{0xbb, 0x56, 0x30, 0xed, 0xfa, 0x84, 0x27, 0xba, \
                                                    0xfa, 0x41, 0x8a, 0x55, 0xbd, 0xd3, 0x01, 0x53} // 128-bit base UUID
#define BLE_UUID_TEST_SERVICE_UUID                0x3700 // Just a random, but recognizable value
#define BLE_UUID_TEST_NOTI_CHARACTERISTIC_UUID			0x3701
#define BLE_UUID_TEST_WRITE_CHARACTERISTIC_UUID                 0x3702

#define TEST_NOTI_DATA_MAX_LENGTH  100
#define TEST_NOTI_DATA_LENGTH  76

#define TEST_WRITE_DATA_MAX_LENGTH  50
#define TEST_WRITE_DATA_LENGTH      4

typedef enum
{
    BLE_TT_EVT_NOTIFICATION_ENABLED,
    BLE_TT_EVT_NOTIFICATION_DISABLED,
    BLE_TT_EVT_WRITE_DATA,
}ble_tt_evt_type_t;


typedef struct
{
    ble_tt_evt_type_t evt_type;
    uint8_t write_data[TEST_WRITE_DATA_MAX_LENGTH];
    uint16_t wr_data_len;
}ble_tt_evt_t;


typedef struct ble_tt_s ble_tt_t;


typedef void (*ble_tt_app_handler_t) (ble_tt_t * p_tt, ble_tt_evt_t * p_evt);

typedef struct
{
    ble_tt_app_handler_t tt_app_handler; /**< Event handler to be called when the test Characteristic is written. */
} ble_tt_init_t;

/**
 * @brief This structure contains various status information for sensor service.
 * The name is based on the naming convention used in Nordic's SDKs.
 * 'ble indicates that it is a Bluetooth Low Energy relevant structure and
 * ss is short for Sensor Service).
 */
struct ble_tt_s
{
    uint8_t                     uuid_type;
    uint16_t                    conn_handle;		/**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t                    service_handle;     /**< Handle of test Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t    test_noti_char_handle; /**< Handles for the test notification characteristic attributes */
    ble_gatts_char_handles_t    test_write_char_handle;
    ble_tt_app_handler_t        tt_app_handler;
    bool                        test_noti_en;
};



/**@brief Function for initializing test new service.
 *
 * @param[in]   p_test_service       Pointer to test Service structure.
 */
void test_service_init(ble_tt_t * p_test_service, ble_tt_init_t *p_tt_init);

/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_test_service        test Service structure.
 * @param[in]	test_value				test data structure pointer
 */
uint32_t test_noti_characteristic_update(ble_tt_t *p_test_service, uint8_t *data, uint16_t data_len);

/**@brief Function to take care of some housekeeping of ble connections related to the test service and characteristic
 *
 * @param[in]   p_ble_evt       BLE Even
 * @param[in]	p_context		event context
 */
void ble_test_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
void test_write_data_set(ble_tt_t *p_test_service, uint8_t *data, uint16_t data_len);
#endif  /* _ TEST_SERVICE_H__ */
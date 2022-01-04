#include <stdint.h>
#include <string.h>
#include "test_service.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "nrf_log.h"
#include "boards.h"


static void on_write(ble_tt_t * p_test_service, ble_evt_t const * p_ble_evt)
{

    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_tt_evt_t ctrl_evt;
    memset(&ctrl_evt,0,sizeof(ctrl_evt));

    switch(p_evt_write->uuid.uuid)
    {
	case BLE_UUID_TEST_WRITE_CHARACTERISTIC_UUID:
            if((p_evt_write->handle != p_test_service->test_write_char_handle.value_handle) || (p_evt_write->len >TEST_WRITE_DATA_MAX_LENGTH))
            {
                NRF_LOG_INFO("Not an event for test service : handle= 0x%x  ",p_evt_write->handle)
                return;
            }

            ctrl_evt.evt_type = BLE_TT_EVT_WRITE_DATA;
            ctrl_evt.wr_data_len = p_evt_write->len;
            memcpy(ctrl_evt.write_data,p_evt_write->data,p_evt_write->len);

            p_test_service->tt_app_handler(p_test_service,&ctrl_evt);

            break;

        case BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG:
            if(p_evt_write->len == 2)
            {
              if(p_evt_write->handle == p_test_service->test_noti_char_handle.cccd_handle) //iv noti
              {
                  if (ble_srv_is_notification_enabled(p_evt_write->data))
                  {
                      ctrl_evt.evt_type = BLE_TT_EVT_NOTIFICATION_ENABLED;
                      p_test_service->test_noti_en = true;
                  }
                  else
                  {
                      ctrl_evt.evt_type = BLE_TT_EVT_NOTIFICATION_DISABLED;
                      p_test_service->test_noti_en = false;

                  }
                  p_test_service->tt_app_handler(p_test_service,&ctrl_evt);
              }

            }
            break;
        default:
            break;
    }

    NRF_LOG_INFO("test service on_write UUID : %x",p_evt_write->uuid.uuid);
}



// ALREADY_DONE_FOR_YOU: Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
void ble_test_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_tt_t * p_test_service =(ble_tt_t *) p_context;

    switch(p_ble_evt->header.evt_id)
    {
	case BLE_GAP_EVT_CONNECTED:
            p_test_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            p_test_service->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_test_service,p_ble_evt);
            break;

	case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            break;

        default:
            //NRF_LOG_INFO("[TEST EVT] 0x%02x",p_ble_evt->header.evt_id);
            break;
	}
}


/**@brief Function for adding our new characterstic to "Test service"
 *
 * @param[in]   p_test_service        Our Service structure.
 *
 */
static uint32_t test_noti_char_add(ble_tt_t * p_test_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_add_char_params_t  add_char_params;
    uint8_t value[TEST_NOTI_DATA_LENGTH];


    memset(&add_char_params, 0, sizeof(add_char_params));
    memset(value, 0, TEST_NOTI_DATA_LENGTH);

    /*characteristic 1.A*/
    add_char_params.uuid                     = BLE_UUID_TEST_NOTI_CHARACTERISTIC_UUID;
    add_char_params.uuid_type                = p_test_service->uuid_type;

    /*characteristic 1.B*/
    add_char_params.is_value_user            = false; /*false :BLE_GATTS_VLOC_STACK, true :  BLE_GATTS_VLOC_USER*/


    /*characteristic 1.D*/
    add_char_params.char_props.read          = 1;

    /*characteristic 1.E*/
    add_char_params.read_access  = SEC_OPEN;


    /*characteristic 1.F*/
    add_char_params.is_var_len               = true;
    add_char_params.max_len                  = TEST_NOTI_DATA_MAX_LENGTH;
    add_char_params.init_len                 = TEST_NOTI_DATA_LENGTH;
    add_char_params.p_init_value             = value;

    /*characteristic 2.A*/
    add_char_params.char_props.notify        = 1;
    add_char_params.cccd_write_access        = SEC_OPEN;
    /*sdk 17.0.0 - cccd �Ӽ� ���� ��ġ �ڵ����� BLE_GATTS_VLOC_STACK ���*/


    /*characteristic 1.G*/
    err_code = characteristic_add(p_test_service->service_handle,&add_char_params,&(p_test_service->test_noti_char_handle));

    return err_code;
}


static uint32_t test_write_char_add(ble_tt_t * p_test_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_add_char_params_t  add_char_params;
    uint8_t value[TEST_WRITE_DATA_LENGTH]={0};


    memset(&add_char_params, 0, sizeof(add_char_params));

    /*characteristic 1.A*/
    add_char_params.uuid                     = BLE_UUID_TEST_WRITE_CHARACTERISTIC_UUID;
    add_char_params.uuid_type                = p_test_service->uuid_type;

    /*characteristic 1.B*/
    add_char_params.is_value_user            = false; /*false :BLE_GATTS_VLOC_STACK, true :  BLE_GATTS_VLOC_USER*/


    /*characteristic 1.D*/
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.read          = 1;

    /*characteristic 1.E*/
    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;


    /*characteristic 1.F*/
    add_char_params.is_var_len               = true;
    add_char_params.max_len                  = TEST_WRITE_DATA_MAX_LENGTH;
    add_char_params.init_len                 = TEST_WRITE_DATA_LENGTH;
    add_char_params.p_init_value             = value;


    /*characteristic 1.G*/
    err_code = characteristic_add(p_test_service->service_handle,&add_char_params,&(p_test_service->test_write_char_handle));

    return err_code;
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_test_service        test Service structure.
 *
 */
void test_service_init(ble_tt_t * p_test_service, ble_tt_init_t *p_tt_init)
{


    /*service 4*/
    ret_code_t   err_code;
    ble_uuid_t            service_uuid;
    ble_uuid128_t     base_uuid = {BLE_UUID_TEST_BASE_UUID};

    /*characteristic 2.B*/
    p_test_service->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_test_service->tt_app_handler = p_tt_init->tt_app_handler;
    p_test_service->test_noti_en=false;

    /*service 4*/
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_test_service->uuid_type);
    APP_ERROR_CHECK(err_code);

    /*service 5*/
    service_uuid.type = p_test_service->uuid_type;
    service_uuid.uuid = BLE_UUID_TEST_SERVICE_UUID;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_test_service->service_handle);
    APP_ERROR_CHECK(err_code);

    test_noti_char_add(p_test_service);
    test_write_char_add(p_test_service);

}

static void error_print(uint32_t err)
{
    static ret_code_t new_error=0;

    if(err != new_error)
    {
	new_error = err;
    }
    else
    {
	return;
    }

    switch(new_error)
    {
	case BLE_ERROR_INVALID_CONN_HANDLE:
            NRF_LOG_ERROR("[HVX]Invalid Connection Handle.");
            break;
	case NRF_ERROR_INVALID_STATE:
            NRF_LOG_ERROR("[HVX] - Invalid Connection State\n - Notifications and/or indications not enabled in the CCCD\n - An ATT_MTU exchange is ongoing");
            break;
	case NRF_ERROR_INVALID_ADDR:
            NRF_LOG_ERROR("[HVX]Invalid pointer supplied.");
            break;
	case NRF_ERROR_INVALID_PARAM:
            NRF_LOG_ERROR("[HVX]Invalid parameter(s) supplied.");
            break;
	case BLE_ERROR_INVALID_ATTR_HANDLE:
            NRF_LOG_ERROR("[HVX]Invalid attribute handle(s) supplied. Only attributes added directly by the application are available to notify and indicate.");
            break;
	case BLE_ERROR_GATTS_INVALID_ATTR_TYPE:
            NRF_LOG_ERROR("[HVX]Invalid attribute type(s) supplied, only characteristic values may be notified and indicated.");
            break;
	case NRF_ERROR_NOT_FOUND:
            NRF_LOG_ERROR("[HVX]Attribute not found.");
            break;
	case NRF_ERROR_FORBIDDEN:
            NRF_LOG_ERROR("[HVX]The connection's current security level is lower than the one required by the write permissions of the CCCD associated with this characteristic");
            break;
	case NRF_ERROR_DATA_SIZE:
            NRF_LOG_ERROR("[HVX]nvalid data size(s) supplied.");
            break;
	case NRF_ERROR_BUSY:
            NRF_LOG_ERROR("[HVX] For @ref BLE_GATT_HVX_INDICATION Procedure already in progress. Wait for a @ref BLE_GATTS_EVT_HVC event and retry.");
            break;
	case BLE_ERROR_GATTS_SYS_ATTR_MISSING:
            NRF_LOG_ERROR("[HVX]System attributes missing, use @ref sd_ble_gatts_sys_attr_set to set them to a known value.");
            break;
	case NRF_ERROR_RESOURCES:
            NRF_LOG_ERROR("[HVX]Too many notifications queued.\n Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry.");
            break;
	case NRF_ERROR_TIMEOUT:
            NRF_LOG_ERROR("[HVX]here has been a GATT procedure timeout. No new GATT procedure can be performed without reestablishing the connection.");
            break;
        default:
            NRF_LOG_ERROR("[HVX]test_characteristic_update error : 0x%04x",err);
            break;
    }
}

uint32_t test_noti_characteristic_update(ble_tt_t *p_test_service, uint8_t *data, uint16_t data_len)
{

    ret_code_t  err_code = NRF_ERROR_NOT_SUPPORTED;

    if(p_test_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

        ble_gatts_hvx_params_t	hvx_params;//hvx stands for Handle Value X, where X symbolize either notification or indication
        memset(&hvx_params,0,sizeof(hvx_params));

        hvx_params.handle = p_test_service->test_noti_char_handle.value_handle;
        hvx_params.type	  = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &data_len;
        hvx_params.p_data = (uint8_t*)data;
        err_code =  sd_ble_gatts_hvx(p_test_service->conn_handle, &hvx_params);
        if(err_code != NRF_SUCCESS)
        {
            error_print(err_code);
        }

    }

    return err_code;
}

void test_write_data_set(ble_tt_t *p_test_service, uint8_t *data, uint16_t data_len)
{
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = data_len;
    gatts_value.offset  = 0;
    gatts_value.p_value = data;

    sd_ble_gatts_value_set(p_test_service->conn_handle, p_test_service->test_write_char_handle.value_handle, &gatts_value);
}




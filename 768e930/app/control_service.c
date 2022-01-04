#include <stdint.h>
#include <string.h>
#include "control_service.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "nrf_log.h"
#include "boards.h"
#include "ble_err.h"
#include "ble_srv_common.h"


static void on_write(ble_cs_t * p_control_service, ble_evt_t const * p_ble_evt)
{
    ble_control_evt_t ctrl_evt;
    memset(&ctrl_evt,0,sizeof(ctrl_evt));

    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    NRF_LOG_INFO("[cs]write event attribute handle : 0x%x",p_evt_write->handle);
    switch(p_evt_write->uuid.uuid)
    {
	case BLE_UUID_NIR_CHAR_UUID:
            if(p_evt_write->handle != p_control_service->nir_char_handle.value_handle)
            {
                NRF_LOG_INFO("Not an event for control service : handle= 0x%x  ",p_evt_write->handle)
                return;
            }
            if(p_evt_write->data[0] == 0x00)
              ctrl_evt.evt_type = BLE_CONTROL_EVT_NIR_OFF;
            else if(p_evt_write->data[0] == 0x01)
            {
              ctrl_evt.evt_type = BLE_CONTROL_EVT_NIR_ON;
              memcpy(ctrl_evt.ctl_data,p_evt_write->data,p_evt_write->len);
            }
            else
              NRF_LOG_ERROR("[NIR control] In valid data : %d",p_evt_write->data[0]);

            break;

	case BLE_UUID_VIB_CHAR_UUID:
            ctrl_evt.evt_type = BLE_CONTROL_EVT_VIB_CONTROL;
            ctrl_evt.ctl_data[0] = (p_evt_write->data[0] & 0x1F);
            break;

        case BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG:
        if(p_evt_write->len == 2)
        {
          if(p_evt_write->handle == p_control_service->iv_char_handle.cccd_handle) //iv noti
          {
              if (ble_srv_is_notification_enabled(p_evt_write->data))
              {
                  ctrl_evt.evt_type = BLE_CONTROL_EVT_IV_NOTI_ON;
                  p_control_service->imp_value_noti_en=true;
              }
              else
              {
                  ctrl_evt.evt_type = BLE_CONTROL_EVT_IV_NOTI_OFF;
                  p_control_service->imp_value_noti_en=false;

              }
          }else if(p_evt_write->handle == p_control_service->dd_char_handle.cccd_handle) //dd noti
          {
              if (ble_srv_is_notification_enabled(p_evt_write->data))
              {
                  ctrl_evt.evt_type = BLE_CONTROL_EVT_DEV_DETECTED_NOTI_ON;
                  p_control_service->deviation_detect_noti_en=true;

              }
              else
              {
                  ctrl_evt.evt_type = BLE_CONTROL_EVT_DEV_DETECTED_NOTI_OFF;
                  p_control_service->deviation_detect_noti_en=false;
              }
          }

        }
        break;
        default:
            break;
    }

    p_control_service->cs_write_handler(p_control_service->conn_handle,p_control_service,&ctrl_evt);

    NRF_LOG_INFO("control service on_write UUID : %x",p_evt_write->uuid.uuid);
}



static void on_disconnect(ble_cs_t * p_control_service, ble_evt_t const * p_ble_evt)
{
    ble_control_evt_t ctrl_evt;

    if(p_control_service->imp_value_noti_en==true)
    {
        memset(&ctrl_evt,0,sizeof(ctrl_evt));
        ctrl_evt.evt_type = BLE_CONTROL_EVT_IV_NOTI_OFF;
        p_control_service->imp_value_noti_en=false;
        p_control_service->cs_write_handler(p_control_service->conn_handle,p_control_service,&ctrl_evt);
    }

    if(p_control_service->deviation_detect_noti_en==true)
    {
        memset(&ctrl_evt,0,sizeof(ctrl_evt));
        ctrl_evt.evt_type = BLE_CONTROL_EVT_DEV_DETECTED_NOTI_OFF;
        p_control_service->deviation_detect_noti_en=false;
        p_control_service->cs_write_handler(p_control_service->conn_handle,p_control_service,&ctrl_evt);
    }

    p_control_service->conn_handle = BLE_CONN_HANDLE_INVALID;

}


// Take care of some housekeeping of ble connections related to our service and characteristic
void ble_control_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_cs_t * p_control_service =(ble_cs_t *) p_context;

    switch(p_ble_evt->header.evt_id)
    {
	case BLE_GAP_EVT_CONNECTED:
            p_control_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

	case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_control_service, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_control_service,p_ble_evt);
            break;

	case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            break;

	default:
            //NRF_LOG_INFO("[CONTROL EVT] 0x%02x",p_ble_evt->header.evt_id);
            break;
    }
}



/* Add characteristics using BLE common component function*/
static uint32_t control_service_chars_add(ble_cs_t * p_control_service)
{
    // Add a custom characteristic UUID
    ret_code_t          err_code;
    int16_t		service_handle = p_control_service->service_handle;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = {BLE_UUID_CONTROL_BASE_UUID};
    uint8_t iv_init_value[IV_ATTR_VALUE_LENGTH];
    uint8_t nir_init_value[NIR_ATTR_VALUE_LENGTH];
    uint8_t init_value = 0;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    //char_uuid.uuid = BLE_UUID_NIR_CHAR_UUID;

    ble_add_char_params_t nir_char_param; //NIR control characteristic
    memset(&nir_char_param, 0, sizeof(nir_char_param));
    memset(nir_init_value,0,NIR_ATTR_VALUE_LENGTH);

    nir_char_param.uuid				= BLE_UUID_NIR_CHAR_UUID;
    nir_char_param.uuid_type 			= char_uuid.type;
    nir_char_param.max_len			= NIR_ATTR_VALUE_LENGTH;
    nir_char_param.init_len			= NIR_ATTR_VALUE_LENGTH;
    nir_char_param.p_init_value                 = nir_init_value;
    nir_char_param.is_var_len			= 1;
    nir_char_param.char_props.read		= 1;
    nir_char_param.char_props.write		= 1;
    nir_char_param.read_access			= SEC_OPEN;
    nir_char_param.write_access			= SEC_OPEN;

    err_code = characteristic_add(service_handle, &nir_char_param, &(p_control_service->nir_char_handle));
    APP_ERROR_CHECK(err_code);

    ble_add_char_params_t iv_char_param; //Impedance value characteristic
    memset(&iv_char_param, 0, sizeof(iv_char_param));
    memset(iv_init_value,0,IV_ATTR_VALUE_LENGTH);
    iv_char_param.uuid					= BLE_UUID_IV_CHAR_UUID;
    iv_char_param.uuid_type 			= char_uuid.type;
    iv_char_param.max_len				= IV_ATTR_VALUE_LENGTH;
    iv_char_param.init_len				= IV_ATTR_VALUE_LENGTH;
    iv_char_param.p_init_value                      = iv_init_value;
    iv_char_param.char_props.notify		= 1;
    iv_char_param.cccd_write_access		= SEC_OPEN;
    iv_char_param.char_props.read		= 1;
    iv_char_param.read_access			= SEC_OPEN;


    err_code = characteristic_add(service_handle, &iv_char_param, &(p_control_service->iv_char_handle));
    APP_ERROR_CHECK(err_code);

    ble_add_char_params_t vib_char_param;//vibrator control characteristic
    memset(&vib_char_param, 0, sizeof(vib_char_param));

    vib_char_param.uuid					= BLE_UUID_VIB_CHAR_UUID;
    vib_char_param.uuid_type 			= char_uuid.type;
    vib_char_param.max_len				= VIB_ATTR_VALUE_LENGTH;
    vib_char_param.init_len				= VIB_ATTR_VALUE_LENGTH;
    vib_char_param.p_init_value                     = &init_value;
    vib_char_param.char_props.read		= 1;
    vib_char_param.char_props.write		= 1;
    vib_char_param.read_access			= SEC_OPEN;
    vib_char_param.write_access			= SEC_OPEN;

    err_code = characteristic_add(service_handle, &vib_char_param, &(p_control_service->vib_char_handle));
    APP_ERROR_CHECK(err_code);

    ble_add_char_params_t dd_char_param; //Deviation detect value characteristic
    memset(&dd_char_param, 0, sizeof(dd_char_param));

    dd_char_param.uuid					= BLE_UUID_DD_CHAR_UUID;
    dd_char_param.uuid_type 			= char_uuid.type;
    dd_char_param.max_len				= DD_ATTR_VALUE_LENGTH;
    dd_char_param.init_len				= DD_ATTR_VALUE_LENGTH;
    dd_char_param.p_init_value                     = &init_value;
    dd_char_param.char_props.notify		= 1;
    dd_char_param.cccd_write_access	 = SEC_OPEN;
    dd_char_param.char_props.read		= 1;
    dd_char_param.read_access			= SEC_OPEN;

    err_code = characteristic_add(service_handle, &dd_char_param, &(p_control_service->dd_char_handle));
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_control_service        control Service structure.
 *
 */
void control_service_init(ble_cs_t * p_control_service, ble_cs_init_t *p_cs_init)
{

    uint32_t   err_code;

    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = {BLE_UUID_CONTROL_BASE_UUID};

    service_uuid.uuid = BLE_UUID_CONTROL_SERVICE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    p_control_service->cs_write_handler = p_cs_init->cs_write_handler;
    p_control_service->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_control_service->uuid_type = service_uuid.type;

    p_control_service->imp_value_noti_en=false;
    p_control_service->deviation_detect_noti_en=false;
    p_control_service->deviation_detect_status_last=0;

    // Add control service : create a table containing control services
    // service_handle is simply an index pointing to control particular service in the table.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
					&service_uuid,					// a pointer to the service UUID that we created.
                                        &p_control_service->service_handle);//a pointer to where the service_handle number of this unique service should be stored
    APP_ERROR_CHECK(err_code);

    control_service_chars_add(p_control_service);

}

static void error_print(uint32_t err);
/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_control_service        control Service structure.
 * @param[in]	temperature				eeg data structure pointer
 */

ret_code_t control_dd_characteristic_update(ble_cs_t *p_control_service, uint8_t data)
{
    ret_code_t         err_code = NRF_SUCCESS;
    if(p_control_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        if(p_control_service->deviation_detect_status_last!= data)
        {
            ble_gatts_value_t gatts_value;
            // Initialize value struct.
            memset(&gatts_value, 0, sizeof(gatts_value));

            gatts_value.len     = sizeof(uint8_t);
            gatts_value.offset  = 0;
            gatts_value.p_value = &data;

            // Update database.
            err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID,
                                              p_control_service->dd_char_handle.value_handle,
                                              &gatts_value);
            if (err_code == NRF_SUCCESS)
            {
                // Save new status.
                p_control_service->deviation_detect_status_last = data;
            }
            else
            {
                NRF_LOG_DEBUG("Error during dd state update: 0x%08X", err_code)

                return err_code;
            }


            if(p_control_service->deviation_detect_noti_en == true)
            {
//                uint16_t  len = DD_ATTR_VALUE_LENGTH;
                ret_code_t  err_code;

                ble_gatts_hvx_params_t	hvx_params;//hvx stands for Handle Value X, where X symbolize either notification or indication
                memset(&hvx_params,0,sizeof(hvx_params));

                hvx_params.handle = p_control_service->dd_char_handle.value_handle;
                hvx_params.type	  = BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = gatts_value.offset;
                hvx_params.p_len  = &gatts_value.len;
                hvx_params.p_data = gatts_value.p_value;

                err_code =  sd_ble_gatts_hvx(p_control_service->conn_handle, &hvx_params);

                if(err_code != NRF_SUCCESS)
                {
                    error_print(err_code);
                }

                return err_code;
            }
        }
    }

    return err_code;
}

ret_code_t control_iv_characteristic_update(ble_cs_t *p_control_service, uint8_t *data)
{
    ret_code_t         err_code = NRF_SUCCESS;
    if(p_control_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

        ble_gatts_value_t gatts_value;

        // Initialize value struct.
        memset(&gatts_value, 0, sizeof(gatts_value));

        gatts_value.len     = IV_ATTR_VALUE_LENGTH;
        gatts_value.offset  = 0;
        gatts_value.p_value = data;

        err_code = sd_ble_gatts_value_set(p_control_service->conn_handle,p_control_service->iv_char_handle.value_handle, &gatts_value);

        if(p_control_service->imp_value_noti_en == true)
        {
            uint16_t  len = IV_ATTR_VALUE_LENGTH;

            ble_gatts_hvx_params_t	hvx_params;//hvx stands for Handle Value X, where X symbolize either notification or indication
            memset(&hvx_params,0,sizeof(hvx_params));

            hvx_params.handle = p_control_service->iv_char_handle.value_handle;
            hvx_params.type	  = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset = 0;
            hvx_params.p_len  = &len;
            hvx_params.p_data = (uint8_t*)data;

            err_code =  sd_ble_gatts_hvx(p_control_service->conn_handle, &hvx_params);

            if(err_code != NRF_SUCCESS)
            {
              error_print(err_code);
            }

            return err_code;
        }
    }

    return err_code;
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
            NRF_LOG_ERROR("[HVX]control_characteristic_update error : 0x%04x",err);
            break;
    }
}


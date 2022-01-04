#include <stdint.h>
#include <string.h>
#include "sensor_service.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "nrf_log.h"
#include "boards.h"


static void on_write(ble_ss_t * p_sensor_service, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    NRF_LOG_INFO("[SS]write event attribute handle : 0x%x",p_evt_write->handle);
    if (p_evt_write->handle == p_sensor_service->sensor_char_handle.cccd_handle && (p_evt_write->len == 2))
    {
	if(p_sensor_service->ss_write_handler != NULL)
        {
            ble_ss_evt_t sensor_evt;

            if(p_evt_write->data[0] == 0x01)
            {
		sensor_evt.evt_type = BLE_SS_EVT_NOTIFICATION_ENABLED;
                p_sensor_service->sensor_noti_en=true;
            }
            else
            {
		sensor_evt.evt_type = BLE_SS_EVT_NOTIFICATION_DISABLED;
                p_sensor_service->sensor_noti_en=false;
            }
            p_sensor_service->ss_write_handler(p_sensor_service->conn_handle,p_sensor_service,&sensor_evt);
        }

    }

}


static void on_disconnect(ble_ss_t * p_sensor_service)
{
    ble_ss_evt_t sensor_evt;

    if(p_sensor_service->sensor_noti_en==true)
    {
        memset(&sensor_evt,0,sizeof(sensor_evt));
        sensor_evt.evt_type = BLE_SS_EVT_NOTIFICATION_DISABLED;
        p_sensor_service->sensor_noti_en=false;
        p_sensor_service->ss_write_handler(p_sensor_service->conn_handle,p_sensor_service,&sensor_evt);
    }

    p_sensor_service->conn_handle = BLE_CONN_HANDLE_INVALID;

}


// ALREADY_DONE_FOR_YOU: Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
void ble_sensor_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_ss_t * p_sensor_service =(ble_ss_t *) p_context;

    switch(p_ble_evt->header.evt_id)
    {
	case BLE_GAP_EVT_CONNECTED:
            p_sensor_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_sensor_service);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_sensor_service,p_ble_evt);
            break;

	case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            break;

	default:
            //NRF_LOG_INFO("[SENSOR EVT] 0x%02x",p_ble_evt->header.evt_id);
            break;
    }
}


/**@brief Function for adding our new characterstic to "Sensor service"
 *
 * @param[in]   p_sensor_service        Our Service structure.
 *
 */
static uint32_t sensor_data_char_add(ble_ss_t * p_sensor_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = {BLE_UUID_SENSOR_BASE_UUID};

    char_uuid.uuid = BLE_UUID_SENSOR_CHARACTERISTC_UUID;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    //Add minimum code for defining a characteristic - decide the location of the attributes in memory
    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK; //Attribute Value is located in stack memory, no user memory is required.

    //Store  previously defined UUID and attribute metadata in the Characteristic Value Attribute
    //Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    //Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;


    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;  //declare a metadata structure for the CCCD to hold our configuration.
    memset(&cccd_md, 0, sizeof(cccd_md)); //Initialize the attribute metadata structure

    cccd_md.vloc		= BLE_GATTS_VLOC_STACK;	// decide where to store the descriptor
    char_md.p_cccd_md		= &cccd_md;	//store the CCCD metadata structure in the characteristic metadata structure.
    char_md.char_props.notify	= 1;  //Enable notification by setting the notify property bit.

    // Set read/write security levels to CCCD
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    // Set characteristic length in number of bytes
    attr_char_value.max_len     = EEG_ATTR_VALUE_LENGTH;
    attr_char_value.init_len    = EEG_ATTR_VALUE_LENGTH;
    uint8_t value[EEG_ATTR_VALUE_LENGTH];
    memset(value,0x77,EEG_ATTR_VALUE_LENGTH);
    attr_char_value.p_value     = value;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_sensor_service->service_handle,
                                                &char_md,
						&attr_char_value,
						&p_sensor_service->sensor_char_handle);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}




/**@brief Function for initiating our new service.
 *
 * @param[in]   p_sensor_service        sensor Service structure.
 *
 */
void sensor_service_init(ble_ss_t * p_sensor_service, ble_ss_init_t *p_ss_init)
{

    uint32_t   err_code;

    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = {BLE_UUID_SENSOR_BASE_UUID};

    service_uuid.uuid = BLE_UUID_SENSOR_SERVICE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    p_sensor_service->ss_write_handler = p_ss_init->ss_write_handler;
    p_sensor_service->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_sensor_service->sensor_noti_en=false;

    // Add sensor service : create a table containing sensor services
    // service_handle is simply an index pointing to sensor particular service in the table.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
					&service_uuid,					// a pointer to the service UUID that we created.
                                        &p_sensor_service->service_handle);//a pointer to where the service_handle number of this unique service should be stored
    APP_ERROR_CHECK(err_code);

    sensor_data_char_add(p_sensor_service);
}

void error_print(uint32_t err);
/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_sensor_service        sensor Service structure.
 * @param[in]	temperature				eeg data structure pointer
 */

//#define BUFFERING 1

uint32_t sensor_characteristic_update(ble_ss_t *p_sensor_service, uint8_t *data)
{

    ret_code_t err_code = NRF_ERROR_NOT_SUPPORTED;

    if(p_sensor_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
	uint16_t len = EEG_ATTR_VALUE_LENGTH;


        ble_gatts_hvx_params_t	hvx_params;//hvx stands for Handle Value X, where X symbolize either notification or indication
        memset(&hvx_params,0,sizeof(hvx_params));

        hvx_params.handle = p_sensor_service->sensor_char_handle.value_handle;
        hvx_params.type	  = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)data;
#ifndef BUFFERING
        err_code =  sd_ble_gatts_hvx(p_sensor_service->conn_handle, &hvx_params);
        if(err_code != NRF_SUCCESS)
        {
            error_print(err_code);
        }    
#else
        uint8_t test_sim = 1;
        do
        {
          if (test_sim == 1)
          {
            err_code =  sd_ble_gatts_hvx(p_sensor_service->conn_handle, &hvx_params);
            if(err_code != NRF_SUCCESS)
            {
                error_print(err_code);
            }
            else
            {
              test_sim = 0;
            }
          }
        }
        while (err_code == NRF_ERROR_RESOURCES);
#endif
    }

    return err_code;
}


void error_print(uint32_t err)
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
            NRF_LOG_ERROR("[HVX]sensor_characteristic_update error : 0x%04x",err);
            break;
    }
}


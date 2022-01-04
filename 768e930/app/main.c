
/** @file
 *
 * @defgroup imedi-eeg main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "sensor_service.h"
#include "control_service.h"
#include "ble_bas.h"
#include "cli_app.h"
#include "test_service.h"

#include "nrf_drv_wdt.h"      //for use wdt
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "app_util_platform.h"

#include "ble_dfu.h"
#if NRF_MODULE_ENABLED(BLE_DIS)
#include "ble_dis.h"
#endif
#ifdef STM_SPIS
#include "stm_spis.h"
#endif
#ifdef STM_UART
#include "stm_uart.h"
#endif

#define DEVICE_NAME                     "iSyncWave"                       /**< Name of device. Will be included in the advertising data. */
#if NRF_MODULE_ENABLED(BLE_DIS)
#define MANUFACTURER_NAME               "EM-TECH"                   //!< Manufacturer. Will be passed to Device Information Service.

/* if change sw_version, check and change NORDIC_VERSION_LENGTH at stm_uart.h */
#define SW_VERSION                      "N100_DV1_BLE_v0.02.06"                   //!< SW version. Will be passed to Device Information Service.

#endif
#define APP_ADV_INTERVAL                300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION                0                                   /**< The advertising mode keep going. */
#define APP_BLE_OBSERVER_PRIO_2         2                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define DATA_LENGTH_DEFAULT             27                                              /**< The stack default data length. */
#define DATA_LENGTH_MAX                 251

#define MIN_CONN_INTERVAL               (uint16_t)MSEC_TO_UNITS(15, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.015 seconds). */
#define MAX_CONN_INTERVAL               (uint16_t)MSEC_TO_UNITS(15, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.015 seconds). */
#define SLAVE_LATENCY                   0												/**< Slave latency. */
#define CONN_SUP_TIMEOUT                (uint16_t)MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define CONN_INTERVAL_MIN

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                      /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */
BLE_SS_DEF(m_sensor_service);													/**< Sensor Serivce instance. */
BLE_CS_DEF(m_control_service);													/**< Control Serivce instance. */
BLE_BAS_DEF(m_bas);                                                     /**< Structure used to identify the battery service. */
BLE_TT_DEF(m_test_service);
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static uint32_t m_spi_sensor_pkt_cnt = 0;

static uint8_t m_ble_address[6] = {0};

static int wdt_tick_cnt = 0;

#ifdef SENSOR_DUMMY_DATA
APP_TIMER_DEF(m_eeg_char_timer_id);
#endif
APP_TIMER_DEF(m_bas_timer_id);
APP_TIMER_DEF(m_stm_ver_timer_id);
APP_TIMER_DEF(m_sleep_timer_id);
APP_TIMER_DEF(m_wdt_kick_timer_id);
#ifdef DD_ENABLE
APP_TIMER_DEF(m_dd_timer_id);
#endif

#ifdef SENSOR_DUMMY_DATA
#define EEG_CHAR_TIMER_INTERVAL     APP_TIMER_TICKS(16) // xx ms intervals
#endif

#define BAS_TIMER_INTERVAL          APP_TIMER_TICKS(1000) // xx ms intervals
#define STM_VER_INTERVAL          APP_TIMER_TICKS(3000) // 1000ms
#define SLEEP_TIMER_INTERVAL        APP_TIMER_TICKS(1000) //1000ms
#define WDT_KICK_INTERVAL           APP_TIMER_TICKS(1000) //1sec

#ifdef DD_ENABLE
#define DD_INTERVAL                 APP_TIMER_TICKS(1000) // 1000ms intervals
#endif
static void advertising_start(bool erase_bonds);
static void bsp_event_handler(bsp_event_t event);

static nrfx_wdt_channel_id m_channel_id;

/**@brief Callback function for asserts in the SoftDevice.
 *



 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
// *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */

static void wdt_event_handler(void)
{
     //write indication 
    /* NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs */
}

static void wdt_init(void)
{
     uint32_t err_code = NRF_SUCCESS;

     //Configure WDT.
     nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
     err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
     APP_ERROR_CHECK(err_code);

     err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
     APP_ERROR_CHECK(err_code); 
}     

static void lfclk_request(void)
{
    ret_code_t err_code;
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start(false);
            break;

        default:
            break;
    }
}


#ifdef SENSOR_DUMMY_DATA
/**@brief Function for handling time-out event
 *
 * @details Function to be executed when the timer expires.
 */
static void eeg_timer_timeout_handler(void * p_context)
{
	uint8_t data[EEG_ATTR_VALUE_LENGTH];
    static uint8_t i=1;
	//memset(data,(EEG_ATTR_VALUE_LENGTH/16),EEG_ATTR_VALUE_LENGTH);
    memset(data,0,EEG_ATTR_VALUE_LENGTH);

    for(int j=0;j<i;j++)
    {
		data[j]=0x77;

        data[j%10]=0;

    }
    data[0]= i;
    if(i > EEG_ATTR_VALUE_LENGTH)
		i=1;
	else
		i++;
    UNUSED_RETURN_VALUE(sensor_characteristic_update(&m_sensor_service, data));

}
#endif



static void bas_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

#ifdef STM_UART
    stm_uart_read_request(STM_UART_BATTERY);
#endif
}

static void stm_ver_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

#ifdef STM_UART
    //stm_uart_read_request(STM_UART_STM_VERSION);
#endif
    NRF_LOG_INFO("sleep");
}

static void sleep_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    bsp_event_handler(BSP_EVENT_SLEEP);
}

#ifdef DD_ENABLE
static void dd_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

#ifdef STM_UART
    stm_uart_read_request(STM_UART_DEVIATION_DET_CHECK);
#endif
}
#endif


static void wdt_kick_timeout_handler(void * p_context)      
{
    UNUSED_PARAMETER(p_context);
    wdt_tick_cnt++;
    //NRF_LOG_INFO("count plus %d", wdt_tick_cnt);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
	// Initialize timer module.
	ret_code_t err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);

	// Create timers.
#ifdef SENSOR_DUMMY_DATA
	err_code = app_timer_create(&m_eeg_char_timer_id, APP_TIMER_MODE_REPEATED, eeg_timer_timeout_handler);
	APP_ERROR_CHECK(err_code);
#endif
	err_code = app_timer_create(&m_bas_timer_id, APP_TIMER_MODE_REPEATED, bas_timer_timeout_handler);
	APP_ERROR_CHECK(err_code);

        err_code = app_timer_create(&m_stm_ver_timer_id, APP_TIMER_MODE_REPEATED, stm_ver_timer_timeout_handler);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_create(&m_sleep_timer_id, APP_TIMER_MODE_SINGLE_SHOT, sleep_timer_timeout_handler);
        APP_ERROR_CHECK(err_code);
#ifdef DD_ENABLE
        err_code = app_timer_create(&m_dd_timer_id, APP_TIMER_MODE_REPEATED, dd_timer_timeout_handler);
        APP_ERROR_CHECK(err_code);
#endif
        err_code = app_timer_create(&m_wdt_kick_timer_id, APP_TIMER_MODE_REPEATED, wdt_kick_timeout_handler);
        APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting timers.
 *
 * @details Start timers used in current application
 */
static void application_timers_start(void)
{
    ret_code_t err_code;
    err_code = app_timer_start(m_stm_ver_timer_id, STM_VER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_wdt_kick_timer_id, WDT_KICK_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_gatt_data_length_set(&m_gatt, BLE_CONN_HANDLE_INVALID, DATA_LENGTH_MAX);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling write events to the sensor characteristic.
 *
 * @param[in] p_ss     Instance of Sensor Service to which the write applies.
 * @param[in] sensor_state Written/desired state of the sensor.
 */
static void sensor_event_handler(uint16_t conn_handle, ble_ss_t * p_ss, ble_ss_evt_t* p_evt)
{
//    ret_code_t err_code;
    UNUSED_PARAMETER(conn_handle);
    if(p_evt->evt_type==BLE_SS_EVT_NOTIFICATION_ENABLED)
    {
#ifdef SENSOR_DUMMY_DATA
	err_code = app_timer_start(m_eeg_char_timer_id, EEG_CHAR_TIMER_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
        nrf_gpio_pin_set(LED_1);
#endif
        m_spi_sensor_pkt_cnt = 0;
#ifdef STM_SPIS
#ifdef STM_SPIS_RINGBUFF
        sensor_buff_reset();
#endif
#endif
#ifdef STM_UART
        stm_uart_write_request_onoff(STM_UART_SENSOR_CONTROL,ON);
#endif
        NRF_LOG_INFO("sensor_write_handler  : BLE_SS_EVT_NOTIFICATION_ENABLED");

    }
    else if(p_evt->evt_type==BLE_SS_EVT_NOTIFICATION_DISABLED)
    {
#ifdef SENSOR_DUMMY_DATA
	err_code = app_timer_stop(m_eeg_char_timer_id);
	APP_ERROR_CHECK(err_code);
        nrf_gpio_pin_set(LED_3);
        nrf_gpio_pin_clear(LED_1);
#endif

#ifdef STM_UART
        stm_uart_write_request_onoff(STM_UART_SENSOR_CONTROL,OFF);
#endif
        NRF_LOG_INFO("sensor_write_handler  : BLE_SS_EVT_NOTIFICATION_DISABLED");
    }

}


/**@brief Function for handling write events to the sensor characteristic.
 *
 * @param[in] p_ss     Instance of Sensor Service to which the write applies.
 * @param[in] sensor_state Written/desired state of the sensor.
 */
static void control_write_handler(uint16_t conn_handle, ble_cs_t * p_cs, const ble_control_evt_t * ctrl_evt)
{
//	ret_code_t err_code;
	UNUSED_PARAMETER(conn_handle);

	switch(ctrl_evt->evt_type)
        {
          case BLE_CONTROL_EVT_NIR_ON:
            NRF_LOG_INFO("control event : BLE_CONTROL_EVT_NIR_ON");
#ifdef STM_UART
            stm_uart_write_request(STM_UART_NIR_DATA,NIR_ATTR_VALUE_LENGTH,ctrl_evt->ctl_data);
#endif
            break;

          case BLE_CONTROL_EVT_NIR_OFF:
            NRF_LOG_INFO("control event : BLE_CONTROL_EVT_NIR_OFF");
#ifdef STM_UART
            stm_uart_write_request(STM_UART_NIR_DATA,NIR_ATTR_VALUE_LENGTH,ctrl_evt->ctl_data);
#endif
            break;

          case BLE_CONTROL_EVT_VIB_CONTROL:
            NRF_LOG_INFO("control event : BLE_CONTROL_EVT_VIB_CTRL 0x%02X",ctrl_evt->ctl_data[0] );
#ifdef STM_UART
            stm_uart_write_request_onoff(STM_UART_VIB_CONTROL,ctrl_evt->ctl_data[0]);
#endif
            break;

          case BLE_CONTROL_EVT_IV_NOTI_ON:
            sensor_buff_reset();
#ifdef STM_UART
            stm_uart_write_request_onoff(STM_UART_IMPEDANCE_CONTROL,ON);
#endif
            //err_code = app_timer_start(m_iv_timer_id, IMP_VALUE_INTERVAL, NULL);
            //APP_ERROR_CHECK(err_code);
            NRF_LOG_INFO("control event : BLE_CONTROL_EVT_IV_NOTI_ON");

            break;
          case BLE_CONTROL_EVT_IV_NOTI_OFF:
            //err_code = app_timer_stop(m_iv_timer_id);
            //APP_ERROR_CHECK(err_code);
#ifdef STM_UART
            stm_uart_write_request_onoff(STM_UART_IMPEDANCE_CONTROL,OFF);
#endif
            NRF_LOG_INFO("control event  : BLE_CONTROL_EVT_IV_NOTI_OFF");
            break;

          default:
          NRF_LOG_INFO("control event default : %x",ctrl_evt->evt_type);
		break;
    }

}

static void test_service_handler(ble_tt_t * p_tt, ble_tt_evt_t * p_evt)
{

        switch(p_evt->evt_type)
        {
          case BLE_TT_EVT_NOTIFICATION_ENABLED:
            NRF_LOG_INFO("test event : BLE_TT_EVT_NOTIFICATION_ENABLED");
            sensor_buff_reset();
#ifdef STM_UART
            stm_uart_write_request_onoff(STM_UART_IMPEDANCE_RAW_DATA,ON);
#endif
            //err_code = app_timer_start(m_iv_timer_id, IMP_VALUE_INTERVAL, NULL);
            //APP_ERROR_CHECK(err_code);
            break;

          case BLE_TT_EVT_NOTIFICATION_DISABLED:
            NRF_LOG_INFO("test event : BLE_TT_EVT_NOTIFICATION_DISABLED");
            //err_code = app_timer_stop(m_iv_timer_id);
            //APP_ERROR_CHECK(err_code);
#ifdef STM_UART
            stm_uart_write_request_onoff(STM_UART_IMPEDANCE_RAW_DATA,OFF);
#endif
            break;
          case BLE_TT_EVT_WRITE_DATA:
#ifdef STM_UART
              if(p_evt->wr_data_len < 2)
              {
                  stm_uart_read_request(p_evt->write_data[0]);
              }else{
                  stm_uart_write_request(p_evt->write_data[0],(p_evt->wr_data_len-1),&p_evt->write_data[1]);
              }
#endif
              break;
          default:
          NRF_LOG_INFO("test event default : %x",p_evt->evt_type);
		break;
    }
}

static void ble_bas_evt_handler(ble_bas_t * p_bas, ble_bas_evt_t * p_evt)
{
//    ret_code_t err_code;

    if(p_evt->evt_type == BLE_BAS_EVT_NOTIFICATION_ENABLED)
    {
	NRF_LOG_INFO("BLE_BAS_EVT_NOTIFICATION_ENABLED");
    }
    else if(p_evt->evt_type == BLE_BAS_EVT_NOTIFICATION_DISABLED)
    {
	NRF_LOG_INFO("BLE_BAS_EVT_NOTIFICATION_DISABLED");
    }
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t         err_code;
    ble_ss_init_t		ss_init = {0};
    ble_cs_init_t		cs_init = {0};
    ble_bas_init_t		bas_init = {0};
    nrf_ble_qwr_init_t qwr_init = {0};
    ble_dfu_buttonless_init_t dfus_init = {0};
#if NRF_MODULE_ENABLED(BLE_DIS)
    ble_dis_init_t     dis_init;
#endif
    ble_tt_init_t     ts_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    memset(&m_sensor_service,0 ,sizeof(m_sensor_service));
    ss_init.ss_write_handler = sensor_event_handler;
    sensor_service_init(&m_sensor_service,&ss_init);

    memset(&m_control_service,0 ,sizeof(m_control_service));
    cs_init.cs_write_handler = control_write_handler;
    control_service_init(&m_control_service, & cs_init);

    memset(&m_test_service, 0, sizeof(m_test_service));
    ts_init.tt_app_handler = test_service_handler;
    test_service_init(&m_test_service, &ts_init);

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    bas_init.bl_rd_sec        = SEC_OPEN;
    bas_init.bl_cccd_wr_sec   = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;

    bas_init.evt_handler          = ble_bas_evt_handler;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);

#if NRF_MODULE_ENABLED(BLE_DIS)
// Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.sw_rev_str, SW_VERSION);
    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
#endif
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */




/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;


    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        default:
            break;
    }

   
}




static void on_ble_gap_evt_connected(ble_gap_evt_t const * p_gap_evt)
{
	ret_code_t err_code;

	NRF_LOG_INFO("BLE_GAP_EVT_CONNECTED.");
	err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
	APP_ERROR_CHECK(err_code);

	m_conn_handle = p_gap_evt->conn_handle;
        cli_set_ble_connect_state(m_conn_handle);

    // Stop scanning and advertising.
    (void) sd_ble_gap_adv_stop(m_advertising.adv_handle);

    // Assign connection handle to the Queued Write module.
	err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
	APP_ERROR_CHECK(err_code);

	NRF_LOG_DEBUG("PHY update request.");
	ble_gap_phys_t const phys =
	{
		.rx_phys = BLE_GAP_PHY_2MBPS,
		.tx_phys = BLE_GAP_PHY_2MBPS,
	};
	err_code = sd_ble_gap_phy_update(p_gap_evt->conn_handle, &phys);
	APP_ERROR_CHECK(err_code);

}



/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
  ret_code_t err_code = NRF_SUCCESS;
  ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("BLE_GAP_EVT_DISCONNECTED");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            cli_set_ble_connect_state(m_conn_handle);
            NRF_LOG_INFO("\tConn Handle 0x%02x",m_conn_handle);
            // LED indication will be changed when advertising starts.

            err_code = app_timer_stop(m_bas_timer_id);
            APP_ERROR_CHECK(err_code);
#ifdef DD_ENABLE
            err_code = app_timer_stop(m_dd_timer_id);
            APP_ERROR_CHECK(err_code);
#endif
#ifdef STM_UART
            stm_uart_write_request_onoff(STM_UART_BLE_CONNECT,BLE_STATE_ADVERTISING);
#endif
            break;

        case BLE_GAP_EVT_CONNECTED:
            on_ble_gap_evt_connected(p_gap_evt);

            err_code = app_timer_start(m_bas_timer_id, BAS_TIMER_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);

#ifdef DD_ENABLE
            err_code = app_timer_start(m_dd_timer_id, DD_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
#endif
#ifdef STM_UART
            stm_uart_write_request_onoff(STM_UART_BLE_CONNECT,BLE_STATE_CONNECTED);
#endif
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_INFO("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_2MBPS,
                .tx_phys = BLE_GAP_PHY_2MBPS,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_INFO("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
		case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:

			break;
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_INFO("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
		case BLE_GAP_EVT_CONN_PARAM_UPDATE:
			NRF_LOG_INFO("BLE_GAP_EVT_CONN_PARAM_UPDATE");
			NRF_LOG_DEBUG("Connection interval updated: min %dms, max %d, %d, %d",
							p_gap_evt->params.conn_param_update.conn_params.min_conn_interval*1.25,
							p_gap_evt->params.conn_param_update.conn_params.max_conn_interval*1.25,
                            p_gap_evt->params.conn_param_update.conn_params.slave_latency,
							p_gap_evt->params.conn_param_update.conn_params.conn_sup_timeout*10);
            break;

		case BLE_GATTS_EVT_HVN_TX_COMPLETE:
//      NRF_LOG_ERROR("BLE_GATTS_EVT_HVN_TX_COMPLETE ~~ \r\n");
            break;

        default:
          // NRF_LOG_INFO("[STACK EVT] 0x%02x",p_ble_evt->header.evt_id);
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO_2, ble_evt_handler, NULL);

}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);

}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated when button is pressed.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            NRF_LOG_INFO("BSP_EVENT_SLEEP");
            sleep_mode_enter();
            break; // BSP_EVENT_SLEEP

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            NRF_LOG_INFO("BSP_EVENT_DISCONNECT");
            break; // BSP_EVENT_DISCONNECT

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            NRF_LOG_INFO("BSP_EVENT_WHITELIST_OFF");
            break;
	case BSP_EVENT_KEY_0:
            break;
        default:
            NRF_LOG_INFO("BSP_EVENT : 0x%02x",event);
            break;
    }
}

#if 0
static void use_random_device_address(void)
{
    ret_code_t err_code;
    ble_gap_privacy_params_t prvt_conf;

    memset(&prvt_conf, 0, sizeof(prvt_conf));

    prvt_conf.privacy_mode = BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY;
    prvt_conf.private_addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE; //generated from an identity resolving key (IRK) and a random number

    prvt_conf.private_addr_cycle_s = 0;

    err_code = sd_ble_gap_privacy_set(&prvt_conf);
    APP_ERROR_CHECK(err_code);
}
#endif


static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_BATTERY_SERVICE,            BLE_UUID_TYPE_BLE}
};

static ble_uuid_t m_scrd_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_SENSOR_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN }
};

#if 0
static void set_manufacturer_data(ble_advdata_manuf_data_t * m_data)
{
	static uint8_t data[]        = "Em-tech";
	m_data->company_identifier = 0x9904;//2 bytes(LSB) id assigned by BlueTooth SIG for member company
	m_data->data.p_data = data;
    m_data->data.size = sizeof(data);
}
#endif

static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    //use_random_device_address();
    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = false;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.advdata.uuids_more_available.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_more_available.p_uuids  = m_adv_uuids;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_scrd_uuids) / sizeof(m_scrd_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_scrd_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}




/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
   ret_code_t err_code = NRF_LOG_INIT(app_timer_cnt_get);
   APP_ERROR_CHECK(err_code);
#if !(NRF_CLI_ENABLED == 1)
   NRF_LOG_DEFAULT_BACKENDS_INIT();
#endif
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    cli_process();

    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
    
}

static void dfu_init(void)
{
	ret_code_t err_code;

	err_code = ble_dfu_buttonless_async_svci_init();
	APP_ERROR_CHECK(err_code);
}

#ifdef STM_SPIS
static void stm_spis_event_handler(void)
{
#ifdef STM_SPIS_RINGBUFF
    uint8_t p_ss_data[EEG_ATTR_VALUE_LENGTH];
    size_t p_size;
    ret_code_t err_code;
#endif
    switch(stm_spis_get_event())
    {
        case STM_SPIS_EVT_SEND_SENSOR_DATA:
        {
#ifdef STM_SPIS_RINGBUFF
            p_size = EEG_ATTR_VALUE_LENGTH;
            memset(p_ss_data, 0,p_size);
            err_code = sensor_data_get(p_ss_data,&p_size);

            if(p_size != EEG_ATTR_VALUE_LENGTH)
            {
                NRF_LOG_ERROR(" sensor data get err len:%d", p_size);
            }
            else
            {
                stm_spis_adjust_ring_rd(p_size);
            }

            if((err_code==NRF_SUCCESS) && (m_sensor_service.sensor_noti_en==true))
            {
                /*sensor data display start*/
                float vol_ref = 4.548;
                uint32_t bit_resolution = 16777216;
                double vol_tmp = vol_ref/bit_resolution;
                float g_rx_data_test=0;
                sensor_display_t param = get_sensor_display_param();
                m_spi_sensor_pkt_cnt++;
                g_rx_data_test = ((p_ss_data[(param.sample*57)+(param.channel*3)]<<24) | (p_ss_data[(param.sample*57)+(param.channel*3+1)] << 16) | (p_ss_data[(param.sample*57)+(param.channel*3+2)] << 8)) / 256;

                NRF_LOG_INFO(" sensor spi sample:%d, channel:%d, cnt:%d, data:"NRF_LOG_FLOAT_MARKER,param.sample,param.channel, m_spi_sensor_pkt_cnt,NRF_LOG_FLOAT(g_rx_data_test*vol_tmp*1000));
                /*sensor data display end*/
                err_code = sensor_characteristic_update(&m_sensor_service,p_ss_data);

                if(err_code != NRF_SUCCESS)
                {
          //              sensor_data_backup(p_size); //rd_idx rollback
                    NRF_LOG_INFO(" characteristic update fail ");
                }

            }
            else
            {
                if(err_code !=NRF_SUCCESS )
                {
                    NRF_LOG_INFO(" Slave Transfer failed. buffer get error");
                }
                else
                {
                   NRF_LOG_INFO(" Slave Transfer failed. noti disabled");
                }
            }
#else
    //        if(m_sensor_service.sensor_noti_en==true)
    //        {
    //            UNUSED_RETURN_VALUE(sensor_characteristic_update(&m_sensor_service, event.rx_pkt_received.p_buffer));
    //        }
#endif
      //NRF_LOG_INFO("sensor_characteristic_update.");
            break;
        }

        case STM_SPIS_EVT_SEND_IMPEDANCE_DATA:
        {
            p_size = IV_ATTR_VALUE_LENGTH;
            memset(p_ss_data, 0,p_size);
            err_code = sensor_data_get(p_ss_data,&p_size);

            if(p_size != EEG_ATTR_VALUE_LENGTH)
            {
                NRF_LOG_ERROR(" sensor data get err len:%d", p_size);
            }
            else
            {
                stm_spis_adjust_ring_rd(p_size);
            }

            if((err_code==NRF_SUCCESS) && (m_control_service.imp_value_noti_en==true))
            {
#if 1 //log
                NRF_LOG_INFO("[1]%d [2]%d [3]%d [4]%d [5]%d",p_ss_data[0], p_ss_data[1], p_ss_data[2], p_ss_data[3], p_ss_data[4]);
                NRF_LOG_INFO("[6]%d [7]%d [8]%d [9]%d [10]%d",p_ss_data[5], p_ss_data[6], p_ss_data[7], p_ss_data[8], p_ss_data[9]);
                NRF_LOG_INFO("[11]%d [12]%d [13]%d [14]%d [15]%d",p_ss_data[10], p_ss_data[11], p_ss_data[12], p_ss_data[13], p_ss_data[14]);
                NRF_LOG_INFO("[16]%d [17]%d [18]%d [19]%d",p_ss_data[15], p_ss_data[16], p_ss_data[17], p_ss_data[18]);
#endif
        //NRF_LOG_INFO(" impedance data recieve =  0x%x len:%d", p_ss_data[0],p_size);
                err_code = control_iv_characteristic_update(&m_control_service, p_ss_data);

                if(err_code != NRF_SUCCESS)
                {
        //          sensor_data_backup(p_size); //rd_idx rollback
                    NRF_LOG_INFO(" characteristic update fail ");
                }

            }
            else
            {
                if(err_code !=NRF_SUCCESS )
                {
                    NRF_LOG_INFO(" Slave Transfer failed. buffer get error");
                }
                else
                {
                    NRF_LOG_INFO(" Slave Transfer failed. noti disabled");
                }
            }
            break;
        }

        case STM_SPIS_EVT_SEND_RAW_IMPEDANCE_DATA:
        {
            p_size = (IV_ATTR_VALUE_LENGTH*4);
            memset(p_ss_data, 0,p_size);
            err_code = sensor_data_get(p_ss_data,&p_size);

            if(p_size != EEG_ATTR_VALUE_LENGTH)
            {
                NRF_LOG_ERROR(" sensor data get err len:%d", p_size);
            }
            else
            {
                stm_spis_adjust_ring_rd(p_size);
            }

            if((err_code==NRF_SUCCESS) && (m_test_service.test_noti_en==true))
            {
#if 1 //float log
                float imp[19]={0,};
                uint8_t cnt = (p_size/4);

                for(uint8_t i=0 ; i < cnt ; i++)
                {
                    memcpy(&imp[i], &p_ss_data[i*4], sizeof(float));
                    NRF_LOG_INFO("imp[%d] : "NRF_LOG_FLOAT_MARKER,i+1,NRF_LOG_FLOAT(imp[i]) );
                }
#endif
        //NRF_LOG_INFO("raw impedance data recieve =  0x%x len:%d", p_ss_data[0],p_size);
                err_code = test_noti_characteristic_update(&m_test_service, p_ss_data, p_size);

                if(err_code != NRF_SUCCESS)
                {
        //          sensor_data_backup(p_size); //rd_idx rollback
                    NRF_LOG_INFO(" characteristic update fail ");
                }
            }
            else
            {
                if(err_code !=NRF_SUCCESS )
                {
                    NRF_LOG_INFO(" Slave Transfer failed. buffer get error");
                }
                else
                {
                    NRF_LOG_INFO(" Slave Transfer failed. noti disabled");
                }
            }
            break;
        }

        default:
            break;
    }
}
#endif


#ifdef STM_UART

/* read rx_ringbuf of uart */

static void stm_uart_event_handler(stm_uart_handler_evt_t evt)
{
    uint8_t read_cmd;
    uint8_t p_uart_data[STM_UART_PKT_LENGHT];
    memset(p_uart_data, 0,STM_UART_PKT_LENGHT);
    size_t p_size = 0;
    ret_code_t err_code;

    err_code = uart_rx_data_get(p_uart_data,STM_UART_PKT_LENGHT, &p_size);
    if (p_size != STM_UART_PKT_LENGHT)
    {
        NRF_LOG_INFO("check stm uart rx ring buf read index");
        stm_uart_adjust_ring_rd(STM_UART_RINGBUF_RX, p_size);
    }
//    NRF_LOG_INFO("tag : %d, cmd:%d, sub:%d", p_uart_data[0], p_uart_data[1], p_uart_data[2]);
//    NRF_LOG_INFO("len:%d, result:%d, first:%d, second:%d", p_uart_data[3], p_uart_data[4], p_uart_data[5], p_uart_data[6]);

    read_cmd = p_uart_data[1];
    if(err_code==NRF_SUCCESS)
    {
        switch(read_cmd) //main cmd check
        {
            /*
                response from STM
                battery, imp control, stm version
            */
            case STM_UART_BATTERY:
            {
                ble_bas_battery_level_update(&m_bas, p_uart_data[5], m_conn_handle);
                break;
            }
            case STM_UART_DEVIATION_DET_CHECK:
            {
                control_dd_characteristic_update(&m_control_service, p_uart_data[5]);                
                break;
            }
            case STM_UART_IMPEDANCE_CONTROL:
            {
                err_code = control_iv_characteristic_update(&m_control_service, &p_uart_data[5]);

                if(err_code != NRF_SUCCESS)
                {
                  uart_rx_data_backup(20); //rd_idx rollback
                  NRF_LOG_INFO(" fail to send the imp data . buffer idx rollback");
                }
                break;
            }
            case STM_UART_STM_VERSION:
            {
                /* need to check this */
                test_write_data_set(&m_test_service, &p_uart_data[5], p_uart_data[3]-1);
                NRF_LOG_INFO("stm ver : %s ", nrf_log_push((char *const)&p_uart_data[5]));
                break;
            }

            /*
                request from STM
                sleep, nordic version, ble address

                process tx_ringbuf
            */
            case STM_UART_BLE_SLEEP:
            {
                if(m_conn_handle == BLE_CONN_HANDLE_INVALID)
                {
                  //if disconnect -> go to sleep
                  bsp_event_handler(BSP_EVENT_SLEEP);
                }
                else
                {
                  //if connected-> disconnect -> go to sleep
                  bsp_event_handler(BSP_EVENT_DISCONNECT);

                  APP_ERROR_CHECK(app_timer_start(m_sleep_timer_id, SLEEP_TIMER_INTERVAL, NULL));
                }

                return ;
            }

            case STM_UART_NORDIC_VERSION:
            {
                memcpy(&p_uart_data[5], SW_VERSION, NORDIC_VERSION_LENGTH);
                 /* len = result(1) + data */
                p_uart_data[3] = 1 + NORDIC_VERSION_LENGTH;

                stm_uart_write_tx_buffer(p_uart_data);
                break;
            }
            case STM_UART_BLE_ADDRESS:
            {
                 /* len = result(1) + data */
                p_uart_data[3] = 1 + BLE_ADDRESS_LENGTH;

                stm_uart_get_ble_address(&p_uart_data[5]);
                stm_uart_write_tx_buffer(p_uart_data);
                break;
            }
            default:
            {
                NRF_LOG_INFO("stm_uart_event_handler nothing to do. cmd:0x%x  ", read_cmd);
                break;
            }
        }
    }
    else
    {
        NRF_LOG_INFO("stm_uart_event_handler get cmd error");
    }
    
   
   
}
#endif

/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;
    ret_code_t err_code;

    // Initialize.
    lfclk_request();
    log_init();
    dfu_init();
    timers_init();
    cli_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();
    wdt_init();

#ifdef STM_SPIS
//    err_code = stm_spis_init(stm_spis_event_handler);
    err_code = stm_spis_init();

    APP_ERROR_CHECK(err_code);
#endif
#ifdef STM_UART
    err_code = stm_uart_init(stm_uart_event_handler);
    APP_ERROR_CHECK(err_code);

#endif
    
    // Start execution.
    NRF_LOG_INFO("wfe_n100_ble started. ver : %s",SW_VERSION);
    stm_uart_get_ble_address(m_ble_address);
    NRF_LOG_INFO("ble address : %02X:%02X:%02X:%02X:%02X:%02X", m_ble_address[0], m_ble_address[1], m_ble_address[2], m_ble_address[3], m_ble_address[4], m_ble_address[5]);

    application_timers_start();
    nrf_drv_wdt_enable(); 

    advertising_start(erase_bonds);
    cli_start();

        
   
    // Enter main loop.
    for (;;)
    {
        idle_state_handle();      
        
        
        /* stm spi */
        if (stm_spis_check_ringbuf())
        {
            stm_spis_event_handler();
        }
        else
        {
            stm_spis_reset_event();
        }

        /* ble request to STM or response to STM */
        if (stm_uart_check_ringbuf(STM_UART_RINGBUF_TX) && stm_uart_get_tx_status() == STM_UART_TX_IDLE)
        {
            stm_uart_response();
        }

        /* request from STM */
        if (stm_uart_check_ringbuf(STM_UART_RINGBUF_RX))
        {
            stm_uart_handler_evt_t evt = BLE_HANDLER_EVT_MAX;
            stm_uart_event_handler(evt);
        }
      
        if((wdt_tick_cnt % 3) == 0)
        {
            nrf_drv_wdt_channel_feed(m_channel_id);
            wdt_tick_cnt = 0;
        }
          
    }
}



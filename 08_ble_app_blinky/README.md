# BLE 개발

- ble_app_blinky SDK example 분석을 통해 ble 서비스 구축을 이해한다.





## SDK 분석

D:\nRF5_SDK_17.0.0_9d13099\examples\peripheral\ble_app_blinky\pca10100\s140\ses



### Peripheral

#### LED, Button

- leds_init 은 bsp_board_init 를 호출하는데
- buttons_init 은 app_button_init 을 호출한다.
  - 인자 : 버튼 배열, 갯수, push 이벤트까지 delay
  - GPIOTE
  - app timer create



- button 초기화
  - 버튼 누르면 button_event_handler
    - ble_lbs_on_button_change 버튼 상태 GATT 로 notify 한다.
  - app_button_init 으로 GPIOTE 등록 (interrupt, event)

```c
static void buttons_init(void)
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {LEDBUTTON_BUTTON, false, BUTTON_PULL, button_event_handler}
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}
```

```c
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    switch (pin_no)
    {
        case LEDBUTTON_BUTTON:
            NRF_LOG_INFO("Send button state change.");
            err_code = ble_lbs_on_button_change(m_conn_handle, &m_lbs, button_action);
            if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE &&
                err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}
```

```c
uint32_t ble_lbs_on_button_change(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t button_state)
{
    ble_gatts_hvx_params_t params;
    uint16_t len = sizeof(button_state);

    memset(&params, 0, sizeof(params));
    params.type   = BLE_GATT_HVX_NOTIFICATION;
    params.handle = p_lbs->button_char_handles.value_handle;
    params.p_data = &button_state;
    params.p_len  = &len;

    return sd_ble_gatts_hvx(conn_handle, &params);
}
```







### BLE

- ble_stack_init
  - ble stack을 초기화하는거래 (softdevice, ble event interrupt 초기화)
  - nrf_sdh_enable_request
    - softdevice 활성화해서 request 가능하게함
    - observer 에게 softdevice 상태를 알려준다. (request 활성화, start, event interrupt 활성화 같은?)
  - nrf_sdh_ble_default_cfg_set
    - ble stack defaul 설정
  - nrf_sdh_ble_enable
    - ble stack 활성화
    - application 의 ram 시작 주소 인자로 받음
  - NRF_SDH_BLE_OBSERVER
    - observer 와 ble event 핸들러를 등록

```c
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
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}
```

- ble_evt_handler
  - ble stack event (연결 상태)에 따라 동작 설정
    - GAP 연결 상태
    - GAP security? 요청 
    - GAP physical 상태
    - GATT... 



- gap_params_init
  - sd_ble_gap_device_name_set
    - advertising 되는 장치 이름 설정
  - sd_ble_gap_ppcp_set
    - PPCP 동작 설정



- gatt_init
  - GATT 모듈 활성화



- services_init
  - nrf_ble_qwr_init
    - queue write 모듈 시작
  - ble_lbs_init
    - service 등록
- advertising_init
  - advertise 시작이긴한데 뭐... 특별한게 아닌듯
- conn_params_init
  - connection parameter 모듈 시작
- advertising_start
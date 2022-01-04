
#include "cli_app.h"
#include "nrf_cli.h"
#include "nrf_cli_uart.h"
#include "nrf_cli_types.h"
#include "nrf_delay.h"
#include "boards.h"
#include "ble_srv_common.h"
#ifdef STM_UART
#include "stm_uart.h"
#endif

#define CLI_EXAMPLE_LOG_QUEUE_SIZE  (4)
NRF_CLI_UART_DEF(cli_uart, 0, 256, 256);
NRF_CLI_DEF(m_cli_uart,
			"cli_uart:~$ ",
			&cli_uart.transport,
			'\r',
			CLI_EXAMPLE_LOG_QUEUE_SIZE);

/* CLI support   */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

static sensor_display_t sensor_param = {0,};

sensor_display_t get_sensor_display_param(void)
{
    return sensor_param;
}
static void cmd_sensor(nrf_cli_t const * p_cli, size_t arg, char ** argv)
{
//    uint8_t sample=0;
//    uint8_t channel=0;

    if(strcmp(argv[1], "display"))
    {
      nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "sensor sample:%d, ch:%d\n-----------------\n",sensor_param.sample,sensor_param.channel);
    }else if(arg==3)
    {
      sensor_param.sample = atoi(argv[1]);
      sensor_param.channel = atoi(argv[2]);
      nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "sensor set sample:%d, ch:%d\n-----------------\n",sensor_param.sample,sensor_param.channel);
    }else{
      nrf_cli_fprintf(p_cli, NRF_CLI_OPTION, "sensor cmd fail : sensor [sample(0~3)] [ch(0~18)]  or sensor display\n-----------------\n");
    }
}

static void cmd_uart_tx(nrf_cli_t const * p_cli, size_t arg, char ** argv)
{
#ifdef STM_UART
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[PAYLOAD_LENGTH_MAX];

    if(arg==2)
    {

        cmd = atoi(argv[1]);
        nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "uart read cmd %d\n-----------------\n",cmd);

        stm_uart_read_request(cmd);

    }else if(arg>2 && arg < 29)
    {
        cmd = atoi(argv[1]);
        len = arg-2;
        memset(payload,0x0,PAYLOAD_LENGTH_MAX);

        nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "uart tx cmd: %d subcmd: 1 len: %d \npayload:",cmd, len);
        for(uint8_t i=0;i<len;i++)
        {
            payload[i]=atoi(argv[2+i]);
            nrf_cli_fprintf(p_cli, NRF_CLI_INFO, " 0x%02x",payload[i]);
        }
        nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "\n-----------------\n");

        stm_uart_write_request(cmd,len,payload);

    }else
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_OPTION, "uart tx send cmd fail : uart_tx cmd [payload1, 2, ...]\n-----------------\n");
    }
#else
        nrf_cli_fprintf(p_cli, NRF_CLI_OPTION, "uart not support\n");
#endif
}

static void cmd_uart_rx(nrf_cli_t const * p_cli, size_t arg, char ** argv)
{
#ifdef STM_UART

    uint8_t payload_len;
    uint8_t data_len;
    uint8_t data[UART_TX_BUFFER_LENGTH];

    if(arg > 3 && arg < 31)
    {
        payload_len   = arg-3;
        data_len      = arg-1;
        memset(data,0x0,UART_TX_BUFFER_LENGTH);


        nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "uart rx payload_len : %d \ndata: ",payload_len);
        for(uint8_t i=0;i<data_len;i++)
        {
            data[i]=atoi(argv[1+i]);
            nrf_cli_fprintf(p_cli, NRF_CLI_INFO, " 0x%02x",data[i]);
        }
        nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "\n-----------------\n");

        cli_stm_uart_rx_packet_test(data,payload_len);

    }else
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_OPTION, "uart rx cmd fail : uart_rx cmd subcmd result [payload1, 2, ..] \n-----------------\n");
    }
#else
        nrf_cli_fprintf(p_cli, NRF_CLI_OPTION, "uart not support\n");
#endif
}
/* Command handlers */
static void cmd_print_param(nrf_cli_t const * p_cli, size_t argc, char **argv)
{

    for (size_t i = 1; i < argc; i++)
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "argv[%d] = %s\n", i, argv[i]);
    }
}

static void cmd_print_all(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    for (size_t i = 1; i < argc; i++)
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "%s ", argv[i]);
    }
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "\n");
}

static void cmd_print(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    ASSERT(p_cli);
    ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

    if ((argc == 1) || nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    if (argc != 2)
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\n", argv[0]);
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\n", argv[0], argv[1]);
}



static void cmd_reset(nrf_cli_t const * p_cli, size_t argc, char ** argv)
{
	UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);
	nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "NVIC_SystemReset\n");
    nrf_delay_ms(200);
	NVIC_SystemReset();
}


static void cmd_ble_state(nrf_cli_t const * p_cli, size_t arg, char ** argv)
{
	UNUSED_PARAMETER(arg);
    UNUSED_PARAMETER(argv);
	nrf_cli_fprintf(p_cli, NRF_CLI_OPTION, "Current BLE state \n-----------------\n");
	nrf_cli_fprintf(p_cli, NRF_CLI_INFO, "Connection : %s\n",(m_conn_handle == BLE_CONN_HANDLE_INVALID) ?  "Disconnected" : "Connected" );
	nrf_cli_fprintf(p_cli, NRF_CLI_OPTION, "new_version\n");
}

static void cmd_wdt_test(nrf_cli_t const * p_cli, size_t argc, char ** argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);
    nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "WDT test : Enter infinite loop \n");
    while(1);
}


void cli_set_ble_connect_state(uint16_t state)
{
    m_conn_handle = state;
}


void cli_process(void)
{
    nrf_cli_process(&m_cli_uart);
    
}

void cli_start(void)
{
    ret_code_t err_code =  nrf_cli_start(&m_cli_uart);
    APP_ERROR_CHECK(err_code);
}

void cli_init(void)
{

    nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
    uart_config.pseltxd = TX_PIN_NUMBER;
    uart_config.pselrxd = RX_PIN_NUMBER;
    uart_config.hwfc    = NRF_UART_HWFC_DISABLED;
    uart_config.baudrate = NRF_UARTE_BAUDRATE_115200;
    uart_config.parity = NRF_UART_PARITY_EXCLUDED;

    ret_code_t err_code = nrf_cli_init(&m_cli_uart, &uart_config, true, true, NRF_LOG_SEVERITY_DEBUG);
    APP_ERROR_CHECK(err_code);
}


NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_print)
{
    NRF_CLI_CMD(all,   NULL, "Print all entered parameters.", cmd_print_all),
    NRF_CLI_CMD(param, NULL, "Print each parameter in new line.", cmd_print_param),
    NRF_CLI_SUBCMD_SET_END
};
NRF_CLI_CMD_REGISTER(print, &m_sub_print, "print", cmd_print);
NRF_CLI_CMD_REGISTER(reset, NULL, "Software reset", cmd_reset);
NRF_CLI_CMD_REGISTER(ble_state, NULL, "BLE state", cmd_ble_state);
NRF_CLI_CMD_REGISTER(uart_tx,  NULL, "uart tx test", cmd_uart_tx);
NRF_CLI_CMD_REGISTER(uart_rx, NULL, "uart rx test", cmd_uart_rx);
NRF_CLI_CMD_REGISTER(sensor, NULL, "sensor display test", cmd_sensor);
NRF_CLI_CMD_REGISTER(wdt_test, NULL, "Enter Infinite Loop", cmd_wdt_test);

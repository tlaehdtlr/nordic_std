#ifndef CLI_APP_H__
#define CLI_APP_H__

#include <stdint.h>

typedef struct
{
    uint8_t sample;
    uint8_t channel;
}sensor_display_t;

void cli_init(void);
void cli_start(void);
void cli_process(void);
void cli_set_ble_connect_state(uint16_t state);
sensor_display_t get_sensor_display_param(void);
#endif

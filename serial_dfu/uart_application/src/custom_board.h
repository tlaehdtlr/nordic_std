#ifndef IMEDI_H
#define IMEDI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "nrf_gpio.h"
// LEDs definitions for IMEDI_H
#define LEDS_NUMBER    4

#define LED_1          NRF_GPIO_PIN_MAP(0,13)
#define LED_2          NRF_GPIO_PIN_MAP(0,14)
#define LED_3          NRF_GPIO_PIN_MAP(0,15)
#define LED_4          NRF_GPIO_PIN_MAP(0,16)
#define LED_START      LED_1
#define LED_STOP       LED_4

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      13
#define BSP_LED_1      14
#define BSP_LED_2      15
#define BSP_LED_3      16

#define BUTTONS_NUMBER 1
#define BUTTON_1       20//11//20
#define BUTTON_2       1//20
#define BUTTON_3       11
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP
#define BUTTONS_ACTIVE_STATE 0
#define BUTTONS_LIST { BUTTON_1}
#define BSP_BUTTON_0   BUTTON_1

#define RX_PIN_NUMBER  3
#define TX_PIN_NUMBER  2
#define CTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define RTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define HWFC           false

#ifdef __cplusplus
}
#endif
#endif // IMEDI_H
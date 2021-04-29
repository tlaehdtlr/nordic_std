/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

/* interrupt */
#include "nrf_drv_gpiote.h"
#include "app_error.h"

/**
 * @brief Function for application main entry.
 */


/* polling & interrupt */
//#define ver_polling 1
#define ver_interrupt 1


#if defined(ver_polling)
int main(void)
{      
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);
    bsp_board_init(BSP_INIT_BUTTONS);
    while (true)
    {                
        for (int i = 0; i < BUTTONS_NUMBER; i++)
        {
          if (bsp_board_button_state_get(i) == true)
          {              
              bsp_board_led_on(i);
          }
          else
          {              
              bsp_board_led_off(i);
          }
        }
        nrf_delay_ms(1);
    }
}
#elif defined(ver_interrupt)


uint8_t LED_PIN_LIST[] = {BSP_LED_0, BSP_LED_1, BSP_LED_2, BSP_LED_3};
uint8_t BUTTON_PIN_LIST[] = {BSP_BUTTON_0, BSP_BUTTON_1, BSP_BUTTON_2, BSP_BUTTON_3};

void in_pin_1_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{        
    nrf_drv_gpiote_out_toggle(LED_PIN_LIST[0]);            
}

void in_pin_2_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{    
    nrf_drv_gpiote_out_toggle(LED_PIN_LIST[1]);            
}

void in_pin_3_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{    
    nrf_drv_gpiote_out_toggle(LED_PIN_LIST[2]);            
}

void in_pin_4_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{    
    nrf_drv_gpiote_out_toggle(LED_PIN_LIST[3]);            
}



static void gpio_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
      err_code = nrf_drv_gpiote_out_init(LED_PIN_LIST[i], &out_config);
      APP_ERROR_CHECK(err_code);
    }

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    
    err_code = nrf_drv_gpiote_in_init(BUTTON_PIN_LIST[0], &in_config, in_pin_1_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN_LIST[0], true);     
    nrf_delay_ms(10);    

    err_code = nrf_drv_gpiote_in_init(BUTTON_PIN_LIST[1], &in_config, in_pin_2_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN_LIST[1], true);     
    nrf_delay_ms(10);

    err_code = nrf_drv_gpiote_in_init(BUTTON_PIN_LIST[2], &in_config, in_pin_3_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN_LIST[2], true);     
    nrf_delay_ms(10);
    
    err_code = nrf_drv_gpiote_in_init(BUTTON_PIN_LIST[3], &in_config, in_pin_4_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN_LIST[3], true);     
    nrf_delay_ms(10);
}


int main(void)
{          
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);
    bsp_board_init(BSP_INIT_BUTTONS);

    gpio_init();
    bsp_board_leds_off();
    while (true)
    {                
        ;
    }
}

#else
int main(void)
{      
    while (true)
    {                
        for (int i = 0; i < BUTTONS_NUMBER; i++)
        {
            bsp_board_led_invert(i);        
            nrf_delay_ms(500);
        }
    }      
}
#endif


/**
 *@}
 **/

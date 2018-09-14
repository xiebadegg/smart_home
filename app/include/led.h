#ifndef __LED_H__
#define __LED_H__
#include "esp_common.h"
#define LED_Pin_NUM         4
#define LED_Pin_FUNC        FUNC_GPIO4
#define LED_Pin_MUX         PERIPHS_IO_MUX_GPIO4_U
#define RELAY_Pin_NUM         5
#define RELAY_Pin_FUNC        FUNC_GPIO5
#define RELAY_Pin_MUX         PERIPHS_IO_MUX_GPIO5_U
#define LED_FREQUENCY       300 //ms
#endif


void led_cmd(bool led);
void relay_cmd(bool relay);
void led_task( void *pvParameters );
void on_off_led_relay();


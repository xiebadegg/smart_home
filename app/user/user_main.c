/*
 *
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
lt *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/

#include "esp_common.h"
#include"user_config.h"
static  os_timer_t timer, led_timer;
LOCAL struct espconn pssdpudpconn;
LOCAL os_timer_t ssdp_time_serv;
 /*
 *******************************************************************************
 * @brief     按键相关变量
 **************************** ***************************************************
 */
static struct keys_param switch_param;
static struct single_key_param *switch_signle;
 bool status = true;
 /**
 *******************************************************************************
 * @brief     任务相关变量
 *******************************************************************************
 */
extern uint32 minute;
LOCAL xTaskHandle xHandle_smartconfig;

void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
            printf("SYSTEMSC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            printf("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            printf("SC_STATUS_GETTING_SSID_PSWD\n");
            sc_type *type = pdata;
                printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            break;
        case SC_STATUS_LINK:
            printf("SC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;

	        wifi_station_set_config(sta_conf);
	        wifi_station_disconnect();
	        wifi_station_connect();
            //wifi_station_dhcpc_stop ();

            break;
        case SC_STATUS_LINK_OVER:
            os_printf("SC_STATUS_LINK_OVER\n");
            uint8 phone_ip[4] = {0};
            memcpy(phone_ip, (uint8*)pdata, 4096);
            printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            os_timer_disarm(&led_timer);
            led_cmd(false);
            relay_cmd(false);
            smartconfig_stop();
            break;
    }

}

void ICACHE_FLASH_ATTR
smartconfig_task(void *pvParameters)
{
    smartconfig_start(smartconfig_done);
         vTaskDelete(NULL);
}



/**
 *******************************************************************************
 * @brief       开关长按状态处理函数
 * @param       [in/out]  void
 * @return      void
 * @note        None
 *******************************************************************************
 */


static void swich_longpress_handler(void )
{
  smartconfig_stop();
  xTaskCreate( smartconfig_task, SMARTCONFIG_NAME, SMARTCONFIG_STACK_WORDS, NULL, SMARTCONFIG_PRIO, &xHandle_smartconfig );
  os_timer_disarm( &led_timer );
  os_timer_setfn( &led_timer, led_task, NULL);
  os_timer_arm( &led_timer, LED_FREQUENCY, true);
}
/**
 *******************************************************************************
 * @brief       开关短按状态处理函数
 * @param       [in/out]  void
 * @return      void
 * @note        None
 *******************************************************************************
 */
static void Switch_ShortPress_Handler( void )
{
    if( status == true )
    {
        status = false;

    }
    else
    {
        status = true;
    }
    led_cmd(status);
    relay_cmd(status);
}

/**
*******************************************************************************
 * @brief       输入初始化函数
 * @param       [in/out]  void
 * @return      void
 * @note        None
*******************************************************************************
 */
void drv_Switch_Init( void )
{
    switch_signle = key_init_single( SWITCH_Pin_NUM, SWITCH_Pin_MUX,
                                     SWITCH_Pin_FUNC,
                                     &swich_longpress_handler,
                                     &Switch_ShortPress_Handler );
    switch_param.key_num = 1;
    switch_param.single_key = &switch_signle;

    key_init( &switch_param );
}

void wifi_event_handler_cb(System_Event_t *event)
{
    if (event == NULL) {
        return;
    }

    switch (event->event_id) {
        case EVENT_STAMODE_GOT_IP:
        //创建mqtt任务
        user_conn_init();
            os_printf("sta got ip ,create task and free heap size is %d\n", system_get_free_heap_size());
        break;

        case EVENT_STAMODE_CONNECTED:
            os_printf("sta connected\n");
        break;

        /*case EVENT_STAMODE_DISCONNECTED:
            wifi_station_connect();
        break;*/

        default:
        break;
    }
}

/******************************************************************************
 * FLASH_SIZE_32M_MAP_1024_1024unctionName : user_rf_cal_sector_set
 * Description  : SWITCH_Pin_StateDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


void ICACHE_FLASH_ATTR
user_init(void)
{   
    minute =0;
    wifi_station_set_auto_connect (true);
    //串口初始化
    uart_init_new();
    UART_SetBaudrate(UART0,BIT_RATE_115200);
    PIN_FUNC_SELECT(RELAY_Pin_MUX, RELAY_Pin_FUNC);
    PIN_FUNC_SELECT(LED_Pin_MUX, LED_Pin_FUNC);
    os_printf("SDK version:%s\n", system_get_sdk_version());
        //按键初始化
    drv_Switch_Init();
    //vTaCskSuspend(xHandle_mqtt);
       //WiFi连接事件，连接成功调用MQTT

	wifi_set_event_handler_cb(wifi_event_handler_cb);
}

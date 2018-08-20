/*
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
 * 
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
static  os_timer_t timer;
LOCAL struct espconn pssdpudpconn;
LOCAL os_timer_t ssdp_time_serv;

 /**
 *******************************************************************************
 * @brief     按键相关变量
 *******************************************************************************
 */
static struct keys_param switch_param;
static struct single_key_param *switch_signle;
static bool status = true;
 /**
 *******************************************************************************
 * @brief     任务相关变量
 *******************************************************************************
 */


#define DEVICE_TYPE 		"gh_9e2cff3dfa51" //wechat public number
#define DEVICE_ID 			"122475" //model ID

#define DEFAULT_LAN_PORT 	12476
#define SWITCH_Pin_NUM         5
#define SWITCH_Pin_FUNC        FUNC_GPIO5
#define SWITCH_Pin_MUX         PERIPHS_IO_MUX_GPIO5_U

#define SWITCH_Pin_Rd_Init()   GPIO_DIS_OUTPUT(SWITCH_Pin_NUM)
#define SWITCH_Pin_Wr_Init()   GPIO_OUTPUT_SET(SWITCH_Pin_NUM,0)
#define SWITCH_Pin_Set_High()  GPIO_OUTPUT_SET(SWITCH_Pin_NUM,1)
#define SWITCH_Pin_Set_Low()   GPIO_OUTPUT_SET(SWITCH_Pin_NUM,0)
#define SWITCH_Pin_State       ( GPIO_INPUT_GET(SWITCH_Pin_NUM) != 0 )


LOCAL xTaskHandle xHandle_smartconfig;
void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
            printf("SC_STATUS_WAIT\n");
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
            printf("SC_STATUS_LINK_OVER\n");
            uint8 phone_ip[4] = {0};    
            memcpy(phone_ip, (uint8*)pdata, 4);
            printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            smartconfig_stop();
            break;
    }
	
}

void ICACHE_FLASH_ATTR
smartconfig_task(void *pvParameters)
{
    smartconfig_start(smartconfig_done);
    printf("system restart");
      // system_restart();
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
    //wifi_station_disconnect();
    //wifi_set_opmode(STATION_MODE);

    smartconfig_stop();
    vTaskResume(xHandle_smartconfig);
    
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
            vTaskResume(xHandle_mqtt);
            os_printf("sta got ip ,create task and free heap size is %d\n", system_get_free_heap_size());
                       break;

        case EVENT_STAMODE_CONNECTED:
            os_printf("sta connected\n");
           break;

        case EVENT_STAMODE_DISCONNECTED:
            wifi_station_connect();
            break;

        default:
            break;
    }
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
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
    wifi_station_set_auto_connect (true);    
    //串口初始化
    uart_init_new();
    UART_SetBaudrate(UART0,BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    //创建mqtt任务
    user_conn_init(); 
    xTaskCreate(smartconfig_task, "smartconfig_task", 256, NULL,SMARTCONFIG_PRIO, &xHandle_smartconfig);
    //按键初始化
    drv_Switch_Init();   
    vTaskSuspend(xHandle_mqtt);
    vTaskSuspend(xHandle_smartconfig);
    //WiFi连接事件，连接成功调用MQTT
	wifi_set_event_handler_cb(wifi_event_handler_cb);
}

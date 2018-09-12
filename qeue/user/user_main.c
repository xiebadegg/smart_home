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
 */

#include "esp_common.h"
#include"driver/uart.c"
#include"driver_h/uart.h"  
#include "freertos/queue.h"
#define task_prio 2
#define task_prio2 2
#define task_prio3 2


LOCAL xTaskHandle xHandle_task1;
LOCAL xTaskHandle xHandle_task2;
LOCAL xTaskHandle xHandle_task3;


void ICACHE_FLASH_ATTR
task1(void *pvParameters) 
{
        portBASE_TYPE xTaskWokenByReceive = pdFALSE;
     portCHAR cRxedChar; 
     while( xQueueReceiveFromISR(xQueueUart, ( void *) &cRxedChar, &xTaskWokenByReceive) )
    {
       /* 接收到一个字符串，输出.*/
        printf("this task1 %s",cRxedChar);
       /* 如果从队列移除一个字符串后唤醒了向此队列投递字符的任务，那么参数xTaskWokenByReceive将会设置成pdTRUE，这个循环无论重复多少次，仅会
          有一个任务被唤醒。*/
    }
    printf("this is task1_1\n");
    printf("this is task1_2\n");
    printf("this is task1_3\n");
    vTaskDelete(NULL);    
}
void ICACHE_FLASH_ATTR
task2(void *pvParameters) 
{
    printf("this is task2_1\n");
    printf("this is task2_2\n");
    printf("this is task2_3\n");
    vTaskDelete(NULL);    
}
void ICACHE_FLASH_ATTR
task3(void *pvParameters) 
{
    for(;;)
    {
        printf("this is task3_1\n");
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
    printf("this is task3_2\n");
    printf("this is task3_3\n");
    vTaskDelete(NULL);    
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
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{  
    uart_init_new();
    UART_SetBaudrate(UART0,BIT_RATE_115200);
    printf("SDK version:%s\n", system_get_sdk_version());
    xTaskCreate(task1, "task1", 256, NULL,task_prio, &xHandle_task1);
    xTaskCreate(task2, "task2", 256, NULL,task_prio2, &xHandle_task2);
    xTaskCreate(task3, "task3", 256, NULL,task_prio3, &xHandle_task3);
}


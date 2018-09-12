/******************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "MQTTEcho.h"
//#include "third_party/mqtt/library/MQTTClient.c"
//#include "mqtt/MQTTClient.h"
#include "stdio.h"
#include "cJSON.h"
#include "cJSON.c"
#include <setjmp.h>
#define TIMING_TASK 10000
#define COUNTDOWN_TASK 10001
#define CYCLE_COUNTDOWN_TASK 10002
#define CURRENT_TIME_TASK 10003 
#define MQTT_TASK 1 
static MQTTClient client;
static Network network;
xTaskHandle  xHandle_json;
os_timer_t delay_timer, rtc_update_timer;
extern void uart0_tx_buffer(char *buf, uint16 len);
 xTaskHandle xHandle_mqtt;
xTaskHandle xHandle_sntp;
void delay_task( void *pvParameters )
{
    char* local_date;
    static uint32 minute = 0;
    local_date = pvParameters;
    minute++; 
    printf("task time number%s\n", (local_date) );
    printf("minute past%d\n", minute);
    if ( atoi(local_date) <= minute ){
    led_cmd(true);
    minute=0;
    os_timer_disarm(&delay_timer);
  }
}
void ICACHE_FLASH_ATTR
sntp_read_timer_task(void *pvParameters)
{
	uint32_t sntp_time, local_time;
  sntp_tm broken_down_time;
  sntp_setservername(0,"0.cn.pool.ntp.org");
	sntp_setservername(1,"1.cn.pool.ntp.org");
	sntp_setservername(2,"2.cn.pool.ntp.org");
	sntp_init();
  while(!sntp_time)
    sntp_time = sntp_get_current_timestamp();
  if( system_rtc_mem_write(64, &sntp_time, sizeof(sntp_time)) )
  {
    //system_rtc_mem_read(64, &local_time, sizeof(local_time));
    local_time = system_get_rtc_time();  
	  os_printf("thisime:%d\r\n", sntp_time);
	  os_printf("date:%s\r\n", sntp_get_real_time(sntp_time));
	  os_printf("local_time:%d\r\n", local_time);
	  os_printf("local_date:%s\r\n", sntp_get_real_time(local_time));
  }
  else{
    printf("write error\n");
 }
  vTaskDelete(NULL);
}

LOCAL command_execution_function(struct my_task_json* data)
{
  struct my_task_json* json_date;
  json_date = data;
  switch(10000){
    case TIMING_TASK:
      os_timer_disarm(&delay_timer);
      os_timer_setfn( &delay_timer, delay_task, "23" );
      os_timer_arm( &delay_timer, 2000, true);
      break;
    case COUNTDOWN_TASK:

      break;
    case CYCLE_COUNTDOWN_TASK:
      break;
    case CURRENT_TIME_TASK:
      break;
    default :
      break;
  }

}

LOCAL void json_parse_task(void* pvParameters)
{
  struct my_task_json* task_json;
  char* json_string = "{\"test_1\":\"10001\",\"test_2\":\"1\",\"test_3\":\"2\"}";//pvParameters;
//JSON字符串到cJSON格式
  cJSON* cjson = cJSON_Parse(json_string); 
  printf("json pack into cjson error...1");
//判断cJSON_Parse函数返回值确定是否打包成功
  if(cjson == NULL){
    printf("json pack into cjson error...");
    cJSON_Delete(cjson);
    vTaskDelete(NULL);
  }
  printf("json pack into cjson error...2");
  cJSON* cjson_test1 = cJSON_GetObjectItem(cjson,"test_1");
  cJSON* cjson_test2 = cJSON_GetObjectItem(cjson,"test_2");
  cJSON* cjson_test3 = cJSON_GetObjectItem(cjson,"test_3");
  printf("json pack into cjson error...3");
  task_json->task_num = cjson_test1->valuestring;
  task_json->task_control->task_attributes_date = cjson_test2->valuestring;
  task_json->task_control->task_parameter_date = cjson_test3->valuestring;
  os_printf("%s\n", task_json->task_num);
  os_printf("%s\n", task_json->task_control->task_attributes_date);
  os_printf("%s\n", task_json->task_control->task_parameter_date);
  printf("json pack into cjson error...4");
  command_execution_function(task_json);
  printf("jsonson pack into cjson error...5");
  vTaskDelete(NULL);
}

//LTDOCAL  struct marky_task_json* 
 void ICACHE_FLASH_ATTR
my_sntp_init(void)
{
	  xTaskCreate(sntp_read_timer_task, "sntp_task", 512, NULL, tskIDLE_PRIORITY+2, &xHandle_sntp);

}
 
LOCAL char* my_get_rtc_time(void)
{
  uint32_t  rtc_time;
  uint32 local_time;
  local_time = system_get_rtc_time();
  local_time = ( ( local_time * 5.4 ) /1000000 );
  if( system_rtc_mem_read(64, &rtc_time, sizeof(rtc_time)) )
  {
    local_time = rtc_time + local_time;
    os_printf("now_date:%s\r\n", sntp_get_real_time(local_time));
  }
  
  return sntp_get_real_time(local_time);
}
LOCAL void messageArrived(MessageData* data)
{
  
  os_printf("%s\n", data->message->payload);
 // xTaskCreate(json_parse_task, "json_task", 512, NULL, tskIDLE_PRIORITY+2, &xHandle_json );
}


LOCAL ICACHE_FLASH_ATTR
void mqtt_client_thread(void* pvParameters)
{
    struct rst_info *rst_info = system_get_rst_info();        
    printf("mqtt client thread starts\n");
    unsigned char sendbuf[80], readbuf[80] = {0};
    int rc = 0, count = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    pvParameters = 0;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    char* address = MQTT_BROKER;

    if ((rc = NetworkConnect(&network, address, MQTT_PORT)) != 0) {
        printf("Return code from network connect is %d\n", rc);
    }
#if defined(MQTT_TASK)

    if ((rc = MQTTStartTask(&client)) != pdPASS) {
        printf("Return code from start tasks is %d\n", rc);
    
    } else {
        printf("Use MQTTStartTask\n");
    }

#endif

  connectData.MQTTVersion = 3;
  connectData.clientID.cstring = client_id;
	connectData.username.cstring = mqtt_user_name;
	connectData.password.cstring = mqtt_user_passwd;
  connectData.keepAliveInterval= 5000;
    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    } else {
          rc = MQTTSubscribe(&client, "test001", QOS2, messageArrived);
          //deliverMessage(&client, "test001", messageArrived);
          printf("%d\n",rc);
          printf("MQTT Connected\n");
          my_sntp_init();
    }
    vTaskDelete(NULL);
}
void user_conn_init(void)
{
    int ret;
    ret = xTaskCreate(mqtt_client_thread,
                      MQTT_CLIENT_THREAD_NAME,
                      MQTT_CLIENT_THREAD_STACK_WORDS,
                      NULL,
                      MQTT_CLIENT_THREAD_PRIO,
                      &xHandle_mqtt);
    
    if (ret != pdPASS)  {
        printf("mqtt create client thread %s failed\n", MQTT_CLIENT_THREAD_NAME);
    }
    vTaskDelay(1000 / portTICK_RATE_MS);  
}


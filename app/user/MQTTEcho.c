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
#include "mqtt/MQTTClient.h"
#include "stdio.h"
#include "cJSON.h"
#include "cJSON.c"
#include "uart.h"
#include "third_party/mqtt/library/MQTTClient.c"
#include <setjmp.h>
#include "sntp_time.c"
/*定时器调度的倒计时任务，通过累加minute变量统计时间*/
void delay_task( void *pvParameters )
{
    //定时时间
    long count;
    struct task_data* task_delay_data = pvParameters;
    static long minute = 0;
    minute++;
    printf("task time number%s\n", (task_delay_data->task_attributes_data));
    printf("minute past%d\n", minute);
    count = strtol(task_delay_data->task_attributes_data, NULL, 10);
    if ( count <= minute ){
    on_off_led_relay();
    os_printf("%s\n", task_delay_data->task_parameter_data);
    minute=0;
    os_timer_disarm(&delay_timer);
  }
}
/*
 *定时器调度的固定时段任务.
 * */
void week_hour_minu_task(void *pvParameters)
{
  
    struct week_task_data* task_data = pvParameters;
    static int week_minute = 0;
    week_minute++;
    int count = task_data->task_attributes_data;
    printf("minute past%d\n", week_minute);
    printf("task time number%d\n", count);
    if ( count <= week_minute ){
    on_off_led_relay();
    os_printf("%s\n", task_data->task_parameter_data);
    week_minute=0;
    os_timer_disarm(&week_hour_minu_timer);
    }
}
/*获取网络实时时间任务*/
void ICACHE_FLASH_ATTR
sntp_read_timer_task(void *pvParameters)
{
	long sntp_time, local_time;
  sntp_tm broken_down_time;
  sntp_setservername(0,"0.cn.pool.ntp.org");
	sntp_setservername(1,"1.cn.pool.ntp.org");
	sntp_setservername(2,"2.cn.pool.ntp.org");
	sntp_init();
  //获取网络实时时间戳
  while(!sntp_time){
    sntp_time = sntp_get_current_timestamp();
  }
  //将网络时间戳写入rtc
  if( system_rtc_mem_write(64, &sntp_time, sizeof(sntp_time)) )
  {
    local_time = system_get_rtc_time();
	  os_printf("thisime:%d\r\n", sntp_time);
	  os_printf("data:%s\r\n", sntp_get_real_time(sntp_time));
	  os_printf("local_time:%d\r\n", local_time);
	  os_printf("local_data:%s\r\n", sntp_get_real_time(local_time));
  }
  else{
    printf("write error\n");
 }
  vTaskDelete(NULL);
}
/***********************************
 *获取成功，返回当
 *前时间的时间戳
 *获取失败返回NULL
 *
 *
 *
 *
 ***********************************/
LOCAL long* get_local_time_t(void)
{
  long  local_time,rtc_time;
  long* t = NULL;
  local_time = system_get_rtc_time();
  local_time = ( ( local_time * 5.4 )/1000000 );
   if(system_rtc_mem_read(64, &rtc_time, sizeof(rtc_time))){
    local_time = rtc_time + local_time;
  //  
    sntp_get_real_time(local_time);
    t = &local_time;
    return t;
   }
    else{
      return t;
    }
}
/*************************************
 *将字符串转换为long型数据
 *
 *
 * ***********************************/
LOCAL struct delay_flash_data* string_conversion_number(struct task_json*  data)
{
  struct task_json* json_data =  data;  
  struct delay_flash_data flash_data;
  char* stop_str = NULL;
  char str = '_';
  char* wday_str = strtok_r(json_data->task_control->task_attributes_data, &str, &stop_str);
  char* hour_str = strtok_r(NULL, &str,&stop_str );
  char* min_str = strtok_r(NULL, &str,&stop_str );
  flash_data.day = strtol(wday_str, NULL, 10 );
  flash_data.hour = strtol( hour_str, NULL, 10 );
  flash_data.minu = strtol( min_str, NULL, 10 );
  flash_data.task_parameter_data = data->task_control->task_parameter_data; 
  struct delay_flash_data* flash_data_ptr = &flash_data; 
  return flash_data_ptr;
}
/****************************************
 *根据星期几来判断定时时间
 *传入参数为解析json后得到
 *的数据.
 *函数返回定时定时时间,以及
 *指令.
 *
 *
 * ***************************************/
LOCAL struct week_task_data* conversion_of_weeks_to_delay_time( struct delay_flash_data* data  )
{
  long long_task_num, delay_wday_time,delay_hour_time,delay_wmin_time;
  long day_minu = 0, hour_minu = 0, minu = 0, week_hour_minu = 0;

  long* local_time_t;
  struct delay_flash_data* week_flash_data_ptr = data;
  struct week_task_data week_delay_time;
        delay_wday_time = week_flash_data_ptr->day;
        delay_hour_time = week_flash_data_ptr->day;
        delay_wmin_time = week_flash_data_ptr->day;
        local_time_t = get_local_time_t();
        if(NULL != local_time_t){
        sntp_tm* rtc_tm = sntp_localtime(local_time_t);
        os_printf("wday:%d\n", rtc_tm->tm_wday);
        os_printf("min:%d\n", rtc_tm->tm_min);
        os_printf("hour:%d\n", rtc_tm->tm_hour);
        //根据定时星期几几时几分计算距离当前时间间隔多少分钟
        //根据时间间隔启用定时器
          if((rtc_tm->tm_wday <= delay_wday_time)&& (rtc_tm->tm_hour <= delay_hour_time)&&(rtc_tm->tm_min <= delay_wmin_time)){
                  day_minu = ( delay_wday_time-rtc_tm->tm_wday )*24*60; 
                  hour_minu = ( delay_hour_time -rtc_tm->tm_hour )*60; 
                  minu = (delay_wmin_time - rtc_tm->tm_min);
                  week_hour_minu = hour_minu + minu + day_minu;
          }
          else {
            if (rtc_tm->tm_wday < delay_wday_time)
            {
             day_minu = (delay_wday_time - rtc_tm->tm_wday)*24*60; 
            }
            else if(rtc_tm->tm_wday == delay_wday_time){
              if (rtc_tm->tm_hour > delay_hour_time){
                day_minu = ((7 - rtc_tm->tm_wday)+delay_wday_time )*24*60;
              }
              if(rtc_tm->tm_min > delay_wmin_time){
                day_minu = ((7 - rtc_tm->tm_wday)+delay_wday_time )*24*60;
              }

            }
            else{
            day_minu = ((7 - rtc_tm->tm_wday)+delay_wday_time )*24*60;
            }
            if(rtc_tm->tm_hour <= delay_hour_time){
            hour_minu = ( delay_hour_time -rtc_tm->tm_hour )*60;
            }
            else{
            hour_minu = ( (24 - rtc_tm->tm_hour)+ delay_hour_time )*60;
            }
            if(rtc_tm->tm_min <= delay_wmin_time){
            minu = delay_wmin_time - rtc_tm->tm_min;
            }
            else{
            minu = (60 - rtc_tm->tm_min) + delay_wmin_time;
            }
            week_hour_minu = hour_minu + minu + day_minu;
          }
        }
        else{
        printf("read rtc_mem error");
        }
        week_delay_time.task_parameter_data =  week_flash_data_ptr->task_parameter_data;
        week_delay_time.task_attributes_data = week_hour_minu;
        struct week_task_data* week_delay_time_ptr = &week_delay_time;
        return week_delay_time_ptr;
}

/*****************************
 *
 * 根据解析的指令执行相关操作
 * 传入的参数为解析后得到的指
 * 令结构体，包含任务序号，任
 * 务属性，任务参数．
 *
 ******************************/
LOCAL command_execution_function(struct task_json* data)
{
  struct task_json* json_data =  data;  

  long  long_task_num = strtol(json_data->task_num, NULL, 10);
  os_printf("%d\n", long_task_num);
  switch(long_task_num){
    case TIMING_TASK:
      printf("timeing task_dataask\n");
      //打开定时器，启动定时任务
      os_timer_disarm(&delay_timer);
      os_timer_setfn( &delay_timer, delay_task, json_data->task_control);
      os_timer_arm( &delay_timer, unit_of_time, true);

      break;
    case CYCLE_COUNTDOWN_TASK:
      printf("cycle task\n");
    case COUNTDOWN_TASK:
        printf("countdown task\n");
      struct delay_flash_data* flash_data_ptr = string_conversion_number(json_data);
      struct week_task_data* week_data_ptr = conversion_of_weeks_to_delay_time(flash_data_ptr);
      os_timer_disarm(&week_hour_minu_timer);
      os_timer_setfn( &week_hour_minu_timer, week_hour_minu_task, week_data_ptr);
      os_timer_arm( &week_hour_minu_timer, unit_of_time, true);
      if( long_task_num == CYCLE_COUNTDOWN_TASK){
      
      }
      break;
    case CURRENT_TIME_TASK:
    os_printf("%s", json_data->task_control->task_parameter_data);
    on_off_led_relay();
    default :
    printf("printf task num  error\n");
      break;
  }
  return;
}
/************************
*
* 解析队列传过来的数据，
 解析完成后调用指令判断
 函数，函数接送的参数是
 mqtt任务通过队列传来的
 消息体，是一个json格式
 的字符串
*
 ***********************/
LOCAL void json_parse_task(void* pvParameters)
{
  struct  task_json* task_json = (struct task_json*)os_malloc(sizeof(struct task_json));
  if(NULL != task_json){
     task_json->task_control = (struct task_data*)os_malloc(sizeof(struct task_data));
  }
  MQTTMessage* me;
  for(; ;) {
    if (xQueueReceive(xQueue_json,(void*)&me, (portTickType)portMAX_DELAY),0){
    }
        char *tmp_string = (char *)os_malloc(me->payloadlen+1);
        strncpy(tmp_string, me->payload, me->payloadlen);
        tmp_string[me->payloadlen] = '\0';
        cJSON* cjson = cJSON_Parse(tmp_string);
        printf("%s\n", cJSON_Print( cjson));
        if(cjson == NULL){
          cJSON_Delete(cjson);
          os_free(tmp_string);
          tmp_string = NULL;
        }
        else{
        printf("%s\n",tmp_string);
          cJSON* cjson_test1 = cJSON_GetObjectItem(cjson,"test_1");
           if (cJSON_IsString(cjson_test1)){
           task_json->task_num = cjson_test1->valuestring;
           printf("%s\n",task_json->task_num);
           }else{
           printf("it's no a string\n");
           }
          cJSON* cjson_test2 = cJSON_GetObjectItem(cjson,"test_2");
             if (cJSON_IsString(cjson_test2)){
              task_json->task_control->task_attributes_data = cjson_test2->valuestring;
              printf("%s\n", task_json->task_control->task_attributes_data);
           }else{
           printf("it's no a string\n");
           }
          cJSON* cjson_test3 = cJSON_GetObjectItem(cjson,"test_3");
           if (cJSON_IsString(cjson_test3)){
              task_json->task_control->task_parameter_data = cjson_test3->valuestring;
           }
           else{
           printf("it's no a string\n");
           }
          command_execution_function(task_json);
          cJSON_Delete(cjson);
          os_free(tmp_string);
          tmp_string = NULL;
        }
  }
    os_free(task_json);
    os_free(task_json->task_control);
    task_json->task_control=NULL;
    task_json = NULL;
    vTaskDelete(NULL);
}
//LTDOCAL  struct marky_task_json*
 void ICACHE_FLASH_ATTR
my_sntp_init(void)
{
	  xTaskCreate(sntp_read_timer_task, "sntp_task", 512, NULL, tskIDLE_PRIORITY+2, &xHandle_sntp);
}
//mqtt订阅消息的回调函数
void messageArrived(MessageData* data)
{
  xQueueSend(xQueue_json, (void *)&(data->message),0);
  return;
}

  LOCAL ICACHE_FLASH_ATTR
void mqtt_client_thread(void* pvParameters)
{
    MQTTClient client;
    Network network;
    os_event_t e;
    struct rst_info *rst_info = system_get_rst_info();
    printf("mqtt client thread starts\n");
    static unsigned char sendbuf[256], readbuf[256] = {0};
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
  connectData.keepAliveInterval= 10;
  connectData.cleansession = TRUE;
    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
        system_restart();
    } else {

          rc = MQTTSubscribe(&client, "test001", QOS0, messageArrived);
          printf("MQTT Connected\n");
          my_sntp_init();
    }
for (;;){
        if (xQueueReceive(xQueueUart,(void*) &e, (portTickType)portMAX_DELAY),1) {
        }
  MQTTMessage message_pub;
  message_pub.qos = QOS0;
  message_pub.retained = 0x00;
  message_pub.payload = e.fifo_tmp;
  message_pub.payloadlen = e.fifo_tmp_len;
  if ((rc = MQTTPublish(&client, public_topic, &message_pub)) != 0) {
  printf("Return code from MQTT publish is %d\n", rc);
  system_restart();
  }
  else {
  }
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
    /*mqtt任务是在获取到ip后启动,为了在smartconfig时避免
     *模块获取到IP但smartconfig未结束,故延时一秒启动
     * */
    vTaskDelay(1000 / portTICK_RATE_MS);
}


#ifndef __MQTTECHO_H__
#define __MQTTECHO_H__
#include <stddef.h>
#include "esp_common.h"
#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include "lwip/apps/sntp_time.h"
#include "lwip/apps/sntp.h"

#define client_id "001"
#define MQTT_BROKER  "211.149.191.185"  /* MQTT Broker Address*/
#define MQTT_PORT     8810            /* MQTT Port*/
#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         tskIDLE_PRIORITY + 2
#define mqtt_user_name 	"xfycloud"
#define mqtt_user_passwd	"xfy20080301"
struct task_date{
  char* task_attributes_date;
  char* task_parameter_date;
};
struct my_task_json{
  char* task_num;
  struct task_date* task_control;
  };
LOCAL char* my_get_rtc_time(void);
LOCAL void json_parse_task(void* pvParameters);
LOCAL command_execution_function(struct my_task_json * date);


#endif

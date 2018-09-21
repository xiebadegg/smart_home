#ifndef MQTTECHO_H
#define MQTTECHO_H
#include <stddef.h>
#include "esp_common.h"
#include <stdio.h>
#include <stdlib.h>
#include "lwip/apps/sntp_time.h"
#include "lwip/apps/sntp.h"
#include "lwip/apps/time.h"
#include "lwip/apps/sntp_opts.h"
#define client_id "001"
#define MQTT_BROKER  "211.149.191.185"  /* MQTT Broker Address*/
#define MQTT_PORT     8810            /* MQTT Port*/
#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         tskIDLE_PRIORITY + 2
#define mqtt_user_name 	"xfycloud"
#define mqtt_user_passwd	"xfy20080301"
#define MQTT_PUBLISH_NAME "uart_mqtt_publish"
#define MQTT_PUBLISH_STACK_WORDS 512
#define MQTT_PUBLISH_PRIO tskIDLE_PRIORITY + 2
#define public_topic "test002"
#define TIMING_TASK 10000
#define COUNTDOWN_TASK 10001
#define CYCLE_COUNTDOWN_TASK 10002
#define CURRENT_TIME_TASK 10003
#define MQTT_TASK 1
#define unit_of_time 1000 //unit is minute

struct task_data{
  char* task_attributes_data;
  char* task_parameter_data;
};
struct my_task_json{
  char* task_num;
  struct task_data* task_control;
  };
struct week_task_data{
char* task_parameter_data;
long  task_attributes_data;
};
LOCAL long* get_local_time_t(void);
LOCAL void json_parse_task(void* pvParameters);
LOCAL command_execution_function(struct my_task_json* data);
LOCAL xTaskHandle xHandle_publish;
extern xQueueHandle xQueue_json;
extern int sub_tags;
extern xQueueHandle xQueueUart;
extern xTaskHandle  xHandle_json;
extern sntp_tm *sntp_localtime(const sntp_time_t *tim_p);
extern void uart0_tx_buffer(char *buf, uint16 len);
struct delay_flash_data{
  long day;
  long hour;
  long minu;
  char* task_parameter_data;
 struct delay_flash_data *next;
};


os_timer_t delay_timer, rtc_updata_timer, week_hour_minu_timer;
xTaskHandle xHandle_mqtt;
xTaskHandle xHandle_sntp;

#endif

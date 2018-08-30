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
#include "stdio.h"
os_timer_t delay_timer;
uint32 minute;
extern void uart0_tx_buffer(char *buf, uint16 len);
LOCAL MQTTClient client;
Network network;
LOCAL xTaskHandle xHandle_mqtt;
LOCAL xTaskHandle xHandle_sntp;

  
void ICACHE_FLASH_ATTR
sntp_read_timer_task(void *pvParameters)
{
	long rtc_time, local_time;
  sntp_setservername(0,"0.cn.pool.ntp.org");
	sntp_setservername(1,"1.cn.pool.ntp.org");
	sntp_setservername(2,"2.cn.pool.ntp.org");
	sntp_init();
  while(!rtc_time)
    rtc_time = sntp_get_current_timestamp();
  if( system_rtc_mem_write(64, &rtc_time, sizeof(rtc_time)) )
  {
    system_rtc_mem_read(64, &local_time, sizeof(rtc_time));
	  os_printf("time:%d\r\n",rtc_time);
	  os_printf("date:%s\r\n",sntp_get_real_time(rtc_time));
	  os_printf("local_time:%d\r\n",local_time);
	  os_printf("local_date:%s\r\n", asctime( gmtime (&local_time)));
  }
  vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
my_sntp_init(void)
{
	  xTaskCreate(sntp_read_timer_task, "sntp_task", 512, NULL, tskIDLE_PRIORITY+2, &xHandle_sntp);
}
void delay_task( void *pvParameters )
{
    minute++; 
    printf("task time number%d\n",atoi(pvParameters));
    printf("minute past%d\n", minute);
  if ( atoi(pvParameters) <= minute ){
    led_cmd(true);
    minute=0;
    os_timer_disarm(&delay_timer);
  }
}

LOCAL void messageArrived(MessageData* data)
{
  os_timer_disarm(&delay_timer);
  os_timer_setfn( &delay_timer, delay_task, data->message->payload);
  os_timer_arm( &delay_timer, 2000, true);
  minute = 0;

}


LOCAL void mqtt_client_thread(void* pvParameters)
{
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
  connectData.keepAliveInterval= 500;
    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    } else {
        printf("MQTT Connected\n");
        if ((rc = MQTTSubscribe(&client, "test001", QOS2, messageArrived)) != 0) {
        printf("Return code from MQTT subscribe is %d\n", rc);
  
        } 
        else {my_sntp_init();
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
    vTaskDelay(1000 / portTICK_RATE_MS);  
}


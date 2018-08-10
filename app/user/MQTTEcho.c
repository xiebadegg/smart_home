/*******************************************************************************
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

#include <stddef.h>
#include "esp_common.h"
#include "mqtt/MQTTClient.h"
#include "user_config.h"

#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         8
#define mqtt_user_name 	"xfy20180000001"
#define mqtt_user_passwd	"ay20080301" 
LOCAL xTaskHandle mqttc_client_handle;

static void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}

static void mqtt_client_thread(void* pvParameters)
{
	   bool isNeedQueue = true;

	    Network network;
	    unsigned char sendbuf[2048], readbuf[2048] = { 0 };
	    int rc = 0, count = 0;
	    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	    pvParameters = 0;
	    NetworkInit(&network);
	    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf,
	            sizeof(readbuf));

	    //！！！！！不要把初始化放在里面
	    for (;;) {

	        //判断是否已经获取了路由器分配的IP
	        while (wifi_station_get_connect_status() != STATION_GOT_IP) {
	            vTaskDelay(1000 / portTICK_RATE_MS);
	        }

	        char* address = MQTT_SERVICE;
	        connectData.MQTTVersion = 3;
	        connectData.clientID.cstring = checkTopic;
	        connectData.username.cstring = MQTT_USER_NAME;
	        connectData.password.cstring = MQTT_USER_PAW;
	        connectData.keepAliveInterval = 40;
	        connectData.cleansession = true;

	        if ((rc = NetworkConnect(&network, address, MQTT_PORT)) != 0) {
	            printf("MClouds NetworkConnect connect is %d\n", rc);
	        }

	        if ((rc = MQTTStartTask(&client)) != pdPASS) {
	            printf("Return code from start tasks is %d\n", rc);
	        } else {
	            printf("Use MQTTStartTask\n");
	        }

	        if ((rc = MQTTConnect(&client, &connectData)) != 0) {
	            printf("[SY] MClouds connect is %d\n", rc);
	            network.disconnect(&network);
	            vTaskDelay(1000 / portTICK_RATE_MS);
	        }

	        if ((rc = MQTTSubscribe(&client, subTopic, QOS0, MessageArrived))
	                != 0) {
	            printf("[SY] MClouds sub fail is %d\n", rc);
	            network.disconnect(&network);
	            vTaskDelay(1000 / portTICK_RATE_MS);
	        }

	        printf("MQTT subscribe to topic -> %s\n", subTopic);
	        xQueueReset(MqttMessageQueueHandler);

	        while (1) {

	            char payload[2048];
	             //是否需要消息队列
	            if (isNeedQueue) {

	                struct esp_mqtt_msg_type *pMsg;
	                printf("MqttMessageQueueHandler waitting ..\n");
	                //阻塞等待
	                xQueueReceive(MqttMessageQueueHandler, &pMsg, portMAX_DELAY);
	                sprintf(payload, "%s", pMsg->allData);
	                //printf("MQTT  publish payload: %s\n", payload);
	                os_printf(" [SY] 1 MQTT get freeHeap: %d\n",
	                        system_get_free_heap_size());
	            } else {
	                sprintf(payload, "%s", tempMsg.allData);
	                printf("MQTT will publish tempMsg.allData: %s, payload: %s\n",
	                        tempMsg.allData, payload);
	                os_printf(" [SY] 2 MQTT get freeHeap: %d\n",
	                        system_get_free_heap_size());
	            }

	            MQTTMessage message;
	            message.qos = QOS0;
	            message.retained = false;
	            message.payload = (void*) payload;
	            message.payloadlen = strlen(payload) + 1;

	            if ((rc = MQTTPublish(&client, pubTopic, &message)) != 0) {
	                printf("Return code from MQTT publish is %d\n", rc);
	            } else {
	                printf("MQTT publish succeed ..\n");
	            }

	            if (rc != 0) {
	                isNeedQueue = false;
	                break;
	            } else {
	                isNeedQueue = true;
	            }

	        }
	        network.disconnect(&network);
	    }

	    printf("mqtt_client_thread going to be deleted\n");
	    vTaskDelete(NULL);
	    return;

}

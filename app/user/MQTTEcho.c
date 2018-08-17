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

#include <stddef.h>
#include "esp_common.h"
#include "mqtt/MQTTClient.h"
#include "MQTTEcho.h"
#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         8
#define mqtt_user_name 	"xfy20180000001"
#define mqtt_user_passwd	"ay20080301" 

LOCAL xTaskHandle xHandle_mqtt;
LOCAL void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}

LOCAL void mqtt_client_thread(void* pvParameters)
{
    printf("mqtt client thread starts\n");
    MQTTClient client;
    Network network;
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
    connectData.clientID.cstring = "client001";
	connectData.username.cstring = mqtt_user_name;
	connectData.password.cstring = mqtt_user_passwd;
    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    } else {
        printf("MQTT Connected\n");
    }

    if ((rc = MQTTSubscribe(&client, "test001", QOS2, messageArrived)) != 0) {
        printf("Return code from MQTT subscribe is %d\n", rc);
    } else {
        printf("MQTT subscribe to topic \"ESP8266/sample/sub\"\n");
    }

    while (++count) {
        MQTTMessage message_pub;
        char payload[30] = "xie";
        message_pub.qos = QOS2;
        message_pub.retained = 0;
        message_pub.payload = payload;
        sprintf(payload, "message number %d", count);
        message_pub.payloadlen = strlen(payload);
        
        if ((rc = MQTTPublish(&client, "test001", &message_pub)) != 0) {
            printf("Return code from MQTT publish is %d\n", rc);
        } 
        else {
            printf("MQTT publish topic \"test001\", message number is %d\n", count);
        }
        if ((rc = MQTTSubscribe(&client, "test001", 2, messageArrived)) != 0) {
        printf("Return code from MQTT subscribe is %d\n", rc);
        } 
        else {
        printf("MQTT subscribe to topic \"ESP8266/sample/sub\"\n");
        }


        vTaskDelay(1000 / portTICK_RATE_MS);  //send every 1 seconds
    }

    printf("mqtt_client_thread going to be deleted\n");
    vTaskDelete(NULL);
    return;
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
}

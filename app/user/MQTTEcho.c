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
#include "driver_h/uart.h"
#define MQTT_PUBLISH_NAME "mqtt_publish"
#define MQTT_PUBLISH_STACK_WORDS 256
#define MQTT_PUBLISH_PRIO tskIDLE_PRIORITY + 2
LOCAL xTaskHandle xhandle_publish;
#define MQTT_SUBLISH_NAME "mqtt_sublish"
#define MQTT_SUBLISH_STACK_WORDS 256
#define MQTT_SUBLISH_PRIO tskIDLE_PRIORITY + 2
LOCAL xTaskHandle xhandle_sublish;


xQueueHandle xQueueUart;
MQTTClient client;
  os_event_t e; 
LOCAL xTaskHandle xHandle_mqtt;
LOCAL void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}
LOCAL void mqtt_publish(void* pvParameters)
{   
    int rc = 0, count = 0;
    for (;;) { 
        char payload[30];
        MQTTMessage message_pub;

        if (xQueueReceive(xQueueUart, payload, (portTickType)portMAX_DELAY)) {
            switch (e.event) {
                case UART_EVENT_RX_CHAR:
                    message_pub.qos = QOS2;
                    message_pub.retained = 0;
                    message_pub.payload = payload;
                    message_pub.payloadlen = strlen(payload);
                    if ((rc = MQTTPublish(&client, "test001", &message_pub)) != 0) {
                        printf("Return code from MQTT publish is %d\n", rc);
                    } 
                    else {
                        printf("MQTT publish topic \"test001\", message number is %d\n", count);
                    }
                    count++;
                    break;

                default:
                    break;
            }
        }
    }
}
LOCAL void mqtt_sublish(void* pvParameters)
{    
    
    int rc = 0, count = 0;
    for(; ;) {
        if ((rc = MQTTSubscribe(&client, "test001", QOS2, messageArrived)) != 0) {
       // printf("Return code from MQTT subscribe is %d\n", rc);
        } 
        else {
        printf("MQTT subscribe to topic \"ESP8266/sample/sub\"\n");
        }


        vTaskDelay(1000 / portTICK_RATE_MS);  //send every 1 seconds
    }

    printf("mqtt_client_thread going to be deleted\n");
    vTaskDelete(NULL);
}


LOCAL void mqtt_client_thread(void* pvParameters)
{
    printf("mqtt client thread starts\n");
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
    connectData.clientID.cstring = client_id;
	connectData.username.cstring = mqtt_user_name;
	connectData.password.cstring = mqtt_user_passwd;
    connectData.keepAliveInterval= 8000;
    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    } else {
        printf("MQTT Connected\n");
        xTaskCreate(mqtt_publish,MQTT_PUBLISH_NAME,MQTT_PUBLISH_STACK_WORDS,NULL,MQTT_PUBLISH_PRIO,&xhandle_publish);
        xTaskCreate(mqtt_sublish,MQTT_SUBLISH_NAME,MQTT_SUBLISH_STACK_WORDS,NULL,MQTT_SUBLISH_PRIO,&xhandle_sublish);
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
    xQueueUart = xQueueCreate(32, sizeof(os_event_t));

}


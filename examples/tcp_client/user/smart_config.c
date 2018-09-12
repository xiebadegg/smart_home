#include "smart_config.h"	
#define DEFAULT_LAN_PORT 	12476

LOCAL esp_udp ssdp_udp;
LOCAL struct espconn pssdpudpconn;
LOCAL os_timer_t ssdp_time_serv;

uint8  lan_buf[200];
uint16 lan_buf_len;
uint8  udp_sent_cnt = 0;

//void ICACHE_FLASH_ATTR
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
            break;
        case SC_STATUS_LINK_OVER:
            printf("SC_STATUS_LINK_OVER\n");
            
			
			//SC_TYPE_ESPTOUCH
                uint8 phone_ip[4] = {0};

                memcpy(phone_ip, (uint8*)pdata, 4);
                printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            
            smartconfig_stop();
            break;
    }
	
}

//void ICACHE_FLASH_ATTR
smartconfig_task(void *pvParameters)
{
    smartconfig_start(smartconfig_done);
    
    vTaskDelete(NULL);
}


//void ICACHE_FLASH_ATTR
smartconfig_init()
{	
    xTaskCreate(smartconfig_task, "smartconfig_task", 256, NULL, 5, NULL);
}

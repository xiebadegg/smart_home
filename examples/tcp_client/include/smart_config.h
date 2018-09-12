
#ifndef _SMART_CONFIG_H_
#define _SMART_CONFIG_H_

void user_conn_init(void);

#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/espconn.h"
#include "espressif/airkiss.h"
#define server_ip "192.168.101.142"
#define server_port 9669


#define DEVICE_TYPE 		"gh_9e2cff3dfa51" //wechat public number
#define DEVICE_ID 			"122475" //model ID


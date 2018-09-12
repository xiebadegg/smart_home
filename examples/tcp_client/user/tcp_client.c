/***********************************************************************
	> File Name: tcp_client.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2018年07月24日 星期二 14时43分25秒
 ************************************************************************/
#include "tcp_client.h"
//#include<stdio.h>
void atask_tcp_client(void *pvParameters)
{
	int32 ivariable_example = 0;
	int32 fd = -1;
	//连接端口的超时时间
	int32 net_time_ont=5000;
	int32 ret;
	struct sockaddr_in server_addr;
	char tcp_masg[48];
	STATION_STATUS sta_staus;
	//获取IP
	do
		{	
			//获取ap模式的IP地址
			sta_staus = wifi_station_get_connect_status();
			vTaskDelay(100);
		}
	while(sta_staus != STATION_GOT_IP );


	//创建一个tcp socket，标志位SOCK_STREAM，PF_INET指使用ipv4
	fd = socket(PF_INET,SOCK_STREAM,0);
	if(-1 == fd)
	{
		printf("get socket fail");
		vTaskDelete(NULL);
		
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &net_time_ont, sizeof(int32));

	memset(server_ip, 0, sizeof(server_ip));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
	server_addr.sin_len = sizeof(server_ip);

	do
		{
			ret == connect(fd, (struct sockaddr*)&server_ip, server_addr.sin_len);
			if(0 != ret)
			{
				printf("connet tcp is fail");
				vTaskDelay(100);
			}
		}
	
	while (0!=ret);
	
	for(;;)
	{
		send(fd , "I am tcp client", sizeof("I am tcp client"), 0);
		do
			{
				ret=recv
					(fd, tcp_masg, sizeof(tcp_masg), 0);
				if(ret > 0)
				{
					printf("%s\n",tcp_masg);
				}
				else
				{
					printf("tcp server data is no\n");
				}
			}
		while (-1==ret);
		
	}
	vTaskDelete(NULL);
}
	
void tcp_client_init(void)
{
	xTaskCreate(atask_tcp_client, "tcp_client", 512, NULL, 4, NULL);//
}


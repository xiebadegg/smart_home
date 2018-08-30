#include"led.h"
void led_cmd(bool led)
{
    if(led == true )
      GPIO_OUTPUT_SET(LED_Pin_NUM,1);
    else
      GPIO_OUTPUT_SET(LED_Pin_NUM,0);
}
void relay_cmd(bool relay)
{
  if(relay == true )
      GPIO_OUTPUT_SET(RELAY_Pin_NUM,1);
  else
      GPIO_OUTPUT_SET(RELAY_Pin_NUM,0);

}

void led_task( void *pvParameters )
{
  if( status == true )
    {
        status = false;

    }
    else
    {
        status = true;
    }
  led_cmd(status);

}


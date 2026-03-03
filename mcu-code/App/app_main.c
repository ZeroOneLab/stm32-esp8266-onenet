#include "app_main.h"
#include "app_onenet_mqtt.h"

#include "at_driver.h"

#include "esp8266.h"

#include "usart.h"
#include "tim.h"

#include "string.h"

wifi_info_t wifi_info = {
	.ssid = "XT-BWP",
	.password = "XT20250512",
};

mqtt_info_t mqtt_info = {
	.host = "1.12.242.248",
	.port = 1883,
	.client_id = "8888",
	.username = "xt",
	.password = "xt12345",
	.publish_topic_post = "$dev/post/8888",
	.subscribe_topic_post = "$dev/post_reply/8888",
	.publish_topic_set = "$dev/set_reply/8888",
	.subscribe_topic_set = "$dev/set/8888",
};

onenet_mqtt_topic_data_t onenet_topic_data = {0}; // mqtt主题数据

onenet_mqtt_propertySet_t onenet_propertySet = {0}; // mqtt属性设置数据

int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

/**
 * @breif   系统初始化
 * @param   无
 * @retval  无
 */
static void system_init(void)
{
	printf("\r\n");
	printf("==========================================================	\r\n");
	printf("                                                     	\r\n");
	printf("	##       ##    ##         ####  #######  ######## 	\r\n");
	printf("	##        ##  ##           ##  ##     ##    ##    	\r\n");
	printf("	##         ####            ##  ##     ##    ##    	\r\n");
	printf("	##          ##    #######  ##  ##     ##    ##    	\r\n");
	printf("	##          ##             ##  ##     ##    ##    	\r\n");
	printf("	##          ##             ##  ##     ##    ##    	\r\n");
	printf("	########    ##            ####  #######     ##    	\r\n");
	printf("                                                       \r\n");
	printf("		零壹实验室 ———— IoT OneNET MQTT 	\r\n");
	printf("                                                     			\r\n");
	printf("==========================================================		\r\n");
	printf("  # 程序名称: IoT OneNET MQTT						\r\n");
	printf("  # 固件版本: V0.0.1                                      		\r\n");
	printf("  # 编译时间: %s %s                  	 \r\n", __DATE__, __TIME__);
	printf("  # 芯片编号：%08X-%08X-%08X \r\n", *(uint32_t *)0x1FFFF7E8, *(uint32_t *)0x1FFFF7EC, *(uint32_t *)0x1FFFF7F0);
	printf("==========================================================	\r\n");
	printf("\r\n");
}

/**
 * @breif   板级初始化
 * @param   无
 * @retval  无
 */
static void board_init(void)
{
	uint8_t ret;

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

	at_init();

	ret = esp8266_init(0);
	printf("[ESP8266] init code: %d\r\n", ret);

	ret = esp8266_wifi_connect(wifi_info.ssid, wifi_info.password);
	printf("[ESP8266][wifi] connect code: %d\r\n", ret);

	ret = esp8266_mqtt_connect(mqtt_info.client_id, mqtt_info.username, mqtt_info.password, mqtt_info.host, mqtt_info.port);
	printf("[ESP8266][mqtt] connect code: %d\r\n", ret);

	ret = esp8266_mqtt_publish(mqtt_info.publish_topic_post, "{\"id\":\"8888\"}", 14, 0, 0);
	printf("[ESP8266][mqtt] publish code: %d\r\n", ret);

	ret = esp8266_mqtt_subscribe(mqtt_info.subscribe_topic_set, 0);
	printf("[ESP8266][mqtt] subscribe code: %d\r\n", ret);
}

/**
 * @breif   应用初始化
 * @param   无
 * @retval  无
 */
static void app_init(void)
{
}

/**
 * @breif   获取到MQTT数据，用户处理函数
 * @param   data :接收并解析后的数据
 * @retval  0:成功 其他：失败
 */
uint8_t onenet_mqtt_user_handle(onenet_pub_data_t *data)
{
	if (strcmp(data->identifier, "LED1") == 0) // 对比字符串，这里以"LED1"为例
	{
		onenet_data[2].value.bool_val = data->value.bool_val;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, data->value.bool_val == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	}

	if (strcmp(data->identifier, "LED2") == 0)
	{
		onenet_data[3].value.u32_val = data->value.u32_val;
		printf ("[LED2] value: %d\r\n", data->value.u32_val);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, data->value.u32_val);
	}

	if (strcmp(data->identifier, "RELAY") == 0)
	{
		onenet_data[4].value.bool_val = data->value.bool_val;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, data->value.bool_val == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}

	return 0;
}

/**
 * @breif   主函数
 * @param   无
 * @retval  无
 */
void app_main(void)
{
	uint8_t ret = 0;
	uint16_t tick = 0;
	system_init();
	board_init();
	app_init();
	while (1)
	{
		uint8_t index = at_get_monitor_match_index(AT_LUN_ESP8266); // 模组获取到预设的匹配数据的下标，at_port.c 中定义的

		if (index != 0xFF) // 如果下标不为0xFF，则说明匹配到了预设的匹配数据
		{
			char *str = at_get_recv_buffer(AT_LUN_ESP8266); // 获取接收的数据
			printf("[ESP8266][mqtt] recv: %s\r\n", str);

			// 先判断第一个主题
			ret = onenet_mqtt_get_topic_data((char *)mqtt_info.subscribe_topic_set, str, &onenet_topic_data); // 获取mqtt设备属性设置主题

			if (ret == 0) // 如果获取成功
			{
				onenet_mqtt_topic_log(&onenet_topic_data);													 // 打印mqtt主题数据
				ret = onenet_mqtt_parse_propertySet_topic(onenet_topic_data.json_data, &onenet_propertySet); // 解析mqtt主题数据
				if (ret == 0)
				{
					ret = onenet_mqtt_user_handle(&onenet_propertySet.data); // 用户处理mqtt主题数据，

					if (ret == 0) // 处理成功, 响应数据，返回成功
						onenet_mqtt_response_topic(onenet_propertySet.msg_id, 200, "user_succ");
					else // 处理失败, 响应数据，返回失败
						onenet_mqtt_response_topic(onenet_propertySet.msg_id, 200, "user_err");
				}
			}

			// 再判断第二个主题，以此类推
			ret = onenet_mqtt_get_topic_data((char *)mqtt_info.subscribe_topic_post, str, &onenet_topic_data); // 获取mqtt设备属性上报主题
			if (ret == 0)
				onenet_mqtt_topic_log(&onenet_topic_data); // 打印mqtt主题数据

			// 最后清除缓存
			at_clear_recv_buffer(AT_LUN_ESP8266);
		}

		if (tick % 20 == 0)
		{
			onenet_mqtt_publish_post_data(onenet_data, 3);
		}

		tick++;
		HAL_Delay(100);
	}
}

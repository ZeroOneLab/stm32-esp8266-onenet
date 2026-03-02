#include "app_main.h"

#include "at_driver.h"

#include "usart.h"

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
	at_init();

	uint8_t res = at_cmd_format_send_and_recv(0, NULL, AT, "");
	printf("at res: %d\r\n", res);
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
 * @breif   主函数
 * @param   无
 * @retval  无
 */
void app_main(void)
{
	system_init();
	board_init();
	app_init();
	while (1)
	{
	}
}

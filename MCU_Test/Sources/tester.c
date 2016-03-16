/**HEADER********************************************************************
 * 
 *
 *
 **************************************************************************
 *
 * $FileName: tester.c$
 * $Version : 
 * $Date    : 
 *
 * Comments:
 *
 * @brief  The file .
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include "board.h"
#include "tester.h"
#include "virtual_com.h"
#include "fsl_uart_driver.h"
#include "uart_configuration.h"
#include "fsl_flexcan_hal.h"

#include "tasks_list.h"
#include "canbus.h"
/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
#define UART_PORT 1
#define NO_TIMEOUT 0
#define UART_SIZE 10
#define EVENT_TESTER 1 //ctesterdc event

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
void clear_screan();
char* strnstr(const char *s, const char *find, size_t slen);
void menu_display();
char* strnstr(const char *s, const char *find, size_t slen);
uint32_t test_canbus(COMMAND_NUMBER_T command);
uint32_t test_acc();
uint32_t test_j1708();
uint32_t test_wiggle();
uint32_t test_a2d();
uint32_t test_uart();

/****************************************************************************
 * Global Variables
 ****************************************************************************/
uint8_t buffer_print[20] = {0};
uint8_t buffer_scan[50] = {0};
uint8_t card_id[20] = {0};

void* tester_scan_event_h;
extern _pool_id   g_out_message_pool;
extern void set_queue_target(APPLICATION_QUEUE_T queue_target);
extern uint8_t* wait_for_recieve_massage();



_queue_id   tester_qid;
APPLICATION_MESSAGE_PTR_T tester_msg_ptr;
APPLICATION_MESSAGE_PTR_T uut_msg_recieve_ptr;

TESTER_PARAMETER_LIST_T tester_parameters =
{
		FALSE,
		FALSE,
		FALSE,
		FALSE
};

/*****************************************************************************
 * Local Types - None
 *****************************************************************************/
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/

/*****************************************************************************
 * Local Variables
 *****************************************************************************/


const uart_user_config_t tester_uart_config = {
    .bitCountPerChar = kUart8BitsPerChar,
    .parityMode      = kUartParityDisabled,
    .stopBitCount    = kUartOneStopBit,
    .baudRate        = 9600
};

/*****************************************************************************
 * Local Functions
 *****************************************************************************/




void menu_display()
{
	char buf[30] = {0x0};
	uint8_t carriage_return = 0xD; //return
	uint8_t new_line = 0xA; //new line

	//memset(buf,0x0,sizeof(buf));
	//sprintf(buf, "\033[1B");
	//cdc_write((uint8_t*)buf, strlen(buf));


	cdc_write(&new_line, 1);
	cdc_write(&carriage_return, 1);

	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "please select test number:\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));

	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "1. uart\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));



	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "2. j1708\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));


	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "3. a2d\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));

	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "4. canbus1\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));



	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "5. canbus2\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));



	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "6. wiggle sensor\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));

	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "7. acc sensor\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));

}

void clear_screan()
{
	char buf[30] = {0x0};
	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "\033[2J");
	cdc_write((uint8_t*)buf, strlen(buf));

	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "\033[H");
	cdc_write((uint8_t*)buf, strlen(buf));
}


/******************************************************************************

 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 *****************************************************************************/
char* strnstr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == '\0' || slen-- < 1)
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

bool search_command_tester(bool* ack,uint8_t* massage,uint8_t* command_buffer)//UART_ACK_COMMAND_NUMBER_T* command, uint8_t* command_buffer)

{
	char* pch;

	pch = strtok ((char*)command_buffer,":");

	if(pch == NULL)
	{
		return 1;//timeout error found
	}

	if(!strcmp(pch,"ack"))
	{
		*ack = TRUE;
	}

	if(!strcmp(pch,"nack"))
	{
		*ack = FALSE;
	}

	pch = strtok (NULL,":");
	if(pch != NULL)
	{
		strcpy((char*)massage,pch);

		return 0;
	}

	return 1;
}


bool wait_for_uart_massage_tester(bool* ack, uint8_t* massage)
{
	bool command_found = false;

	memset(buffer_scan,0x0,sizeof(buffer_scan));
	//scanf(" %s", &buffer_scan);
	if (MQX_OK !=  _event_wait_all(tester_scan_event_h, 0x40, 5000))
	{
		return 1;
	}
	_event_clear(tester_scan_event_h, 0x40);
	strcpy((char*)buffer_scan,(char*)wait_for_recieve_massage());


	//check if there is valid massage:
	command_found = search_command_tester(ack, massage, buffer_scan);//command, buffer_scan);

	return command_found;

}


void tester_parser(COMMAND_NUMBER_T command)
{

	uint32_t test_status = 0;
	uint32_t error_number = 0;

	memset(buffer_print,0x0,sizeof(buffer_print));

	switch (command)
	{
	case MENU_COMMAND:

		tester_parameters.menu_mode_on = TRUE;

		if(FALSE == tester_parameters.uut_abort)
		{
			tester_parameters.uut_abort = TRUE;
			_task_abort(g_TASK_ids[UUT_TASK]);
		}

		//configure tester task waiting for response from UUT if not busy.
		menu_display();

		break;
	case MENU_TEST:

		tester_parameters.menu_mode_on = FALSE;


		if(FALSE == tester_parameters.uut_abort)
		{
			tester_parameters.uut_abort = TRUE;
			_task_abort(g_TASK_ids[UUT_TASK]);
		}

		//print card id
		memset(buffer_print,0x0,sizeof(buffer_print));
		sprintf((char*)buffer_print, "id:%s\n", card_id);
		cdc_write((uint8_t*)buffer_print, strlen((char*)buffer_print));

		test_status = test_uart();

		if(test_status)
		{
			error_number++;
		}

		test_status = test_j1708();
		if(test_status)
		{
			error_number++;
		}

		test_status = test_canbus(MENU_CANBUS1);

		if(test_status)
		{
			error_number++;
		}

		test_status = test_canbus(MENU_CANBUS2);
		if(test_status)
		{
			error_number++;
		}

		test_status = test_a2d();
		if(test_status)
		{
			error_number++;
		}
		test_status = test_acc();
		if(test_status)
		{
			error_number++;
		}

		test_status = test_wiggle();
		if(test_status)
		{
			error_number++;
		}


		if(error_number)
		{
			memset(buffer_print,0x0,sizeof(buffer_print));
			sprintf((char*)buffer_print, "full test fail, %d tests fail\r\n", error_number);
			cdc_write((uint8_t*)buffer_print, strlen((char*)buffer_print));
		}
		else
		{
			memset(buffer_print,0x0,sizeof(buffer_print));
			sprintf((char*)buffer_print, "full test pass\r\n");
			cdc_write((uint8_t*)buffer_print, strlen((char*)buffer_print));
		}
		//print end test:
		memset(buffer_print,0x0,sizeof(buffer_print));
		sprintf((char*)buffer_print, "end_test\r\n");
		cdc_write((uint8_t*)buffer_print, strlen((char*)buffer_print));

		break;
	case MENU_UART:
		test_uart();
		break;
	case MENU_J1708:
		test_j1708();
		break;
	case MENU_CANBUS1:
	case MENU_CANBUS2:
		test_canbus(command);
		break;
	case MENU_A2D:
		test_a2d();
		break;
	case MENU_ACC:
		test_acc();
		break;
	case MENU_WIGGLE:
		test_wiggle();
		break;

	default:
		break;
	}
}

uint32_t test_wiggle()
{
	bool ack = FALSE;
	char uart_massage[50] = {0};
	bool uart_status = 0;

	//send canbus:
	sprintf((char*)buffer_print, "wiggle\n",7);
	printf("%s",buffer_print);

	//wait for ack:
	uart_status = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
	if(uart_status == 1)
	{
		memset(uart_massage,0x0,sizeof(uart_massage));
		sprintf(uart_massage, "wiggle test failed - no UART ack\r\n");
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
		return 1; //timeout
	}

	sprintf(uart_massage, "%s\r\n",uart_massage);
	cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

	return !ack;

}

uint32_t test_acc()
{
	bool ack = FALSE;
	char uart_massage[50] = {0};
	bool uart_status = 0;

		//send canbus:
		sprintf((char*)buffer_print, "acc\n",4);
		printf("%s",buffer_print);

		//wait for ack:
		uart_status = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
		if(uart_status == 1)
		{
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf(uart_massage, "acc test failed - no UART ack\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
			return 1; //timeout
		}


		sprintf(uart_massage, "%s\r",uart_massage);
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

		uart_status = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
		if(uart_status == 1)
		{
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf(uart_massage, "acc test failed - no UART ack\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
			return 1; //timeout
		}

		sprintf(uart_massage, "%s\r",uart_massage);
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

		return !ack;
}

uint32_t test_a2d()
{
	bool ack = FALSE;
	char uart_massage[50] = {0};
	bool uart_status = 0;

	//send canbus:
	sprintf((char*)buffer_print, "a2d\n",4);
	printf("%s",buffer_print);

	//wait for ack:
	uart_status = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
	if(uart_status == 1)
	{
		memset(uart_massage,0x0,sizeof(uart_massage));
		sprintf(uart_massage, "A2D test failed - no UART ack\r\n");
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
		return 1; //timeout
	}

	sprintf(uart_massage, "%s\r",uart_massage);
	cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

	uart_status = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
	if(uart_status == 1)
	{
		memset(uart_massage,0x0,sizeof(uart_massage));
		sprintf(uart_massage, "A2D test failed - no UART ack\r\n");
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
		return 1; //timeout
	}

	sprintf(uart_massage, "%s\r",uart_massage);
	cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

	return !ack;

}

uint32_t test_uart()
{
	bool uart_status = 0;
	bool ack = FALSE;
	char uart_massage[50] = {0};
	//send uart:
	sprintf((char*)uart_massage, "uart\n",5);
	printf("%s",uart_massage);

	memset(uart_massage,0x0,sizeof(uart_massage));

	uart_status = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
	if(uart_status == 1)
	{
		memset(uart_massage,0x0,sizeof(uart_massage));
		sprintf(uart_massage, "uart test failed - no UART ack\r\n");
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

		return TIME_OUT; //timeout
	}

	sprintf(uart_massage, "%s\r",uart_massage);
	cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

	return !ack;
}

uint32_t test_j1708()
{
	bool ack = FALSE;
	char uart_massage[50] = {0};
	bool uart_status = 0;

	//send j1708:
	sprintf((char*)uart_massage, "j1708\n",6);
	printf("%s",uart_massage);

	//wait for ack:
	memset(uart_massage,0x0,sizeof(uart_massage));
	uart_status = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
	if(uart_status == 1)
	{
		memset(uart_massage,0x0,sizeof(uart_massage));
		sprintf(uart_massage, "j1708 test failed - no UART ack\r\n");
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

		return 1; //timeout
	}


	//send j1708 massage:
	TIME_STRUCT time;
	set_queue_target(TESTER_QUEUE);
	uint8_t string_j1708[] = "j1708";
	uint8_t string_j1708_back[] = "8071j";
	_time_get(&time);
	if ((tester_msg_ptr = (APPLICATION_MESSAGE_PTR_T) _msg_alloc (g_out_message_pool)) == NULL)
	{
		_mqx_exit(0);
	}

	sprintf((char*)tester_msg_ptr->data, (char*)string_j1708, strlen((char*)string_j1708));
	tester_msg_ptr->timestamp = time;
	tester_msg_ptr->header.SOURCE_QID = tester_qid;
	tester_msg_ptr->header.TARGET_QID = _msgq_get_id(0, J1708_TX_QUEUE);
	tester_msg_ptr->header.SIZE = sizeof (MESSAGE_HEADER_STRUCT) + strlen((char *)tester_msg_ptr->data) + 1 ;

	_msgq_send(tester_msg_ptr);

	//wait for j1708 massage:
	uut_msg_recieve_ptr = _msgq_receive(tester_qid, 6000);
	if (uut_msg_recieve_ptr != NULL)
	{
		//check getting : "8071j"
		//search for command:
		if(!strcmp((char*)uut_msg_recieve_ptr->data, (char*)string_j1708_back))
		{
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf((char*)uart_massage, "j1708 test pass \r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
			_msg_free(uut_msg_recieve_ptr);
			return 0;
		}
		else
		{
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf((char*)uart_massage, "j1708 test failed - value=%s\n",(char*)uut_msg_recieve_ptr->data);
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

			_msg_free(uut_msg_recieve_ptr);
			return 1;
		}

	}
	else
	{
		memset(uart_massage,0x0,sizeof(uart_massage));
		sprintf((char*)uart_massage, "j1708 test fail - timeout \r\n");
		cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
	}

	return 1;

}

uint32_t test_canbus(COMMAND_NUMBER_T command)
{
	bool ack = FALSE;
	char uart_massage[50] = {0};
	uint32_t can_result;

	if(MENU_CANBUS1 == command)
	{
		canbus_init(9, 8,  0x123,0x456 , 0);
	}
	else
	{
		canbus_init(9, 8,  0x123,0x456 , 1);
	}

	//send canbus:
	if(MENU_CANBUS1 == command)
	{
		sprintf((char*)buffer_print, "canbus1\n",7);
	}
	else
	{
		sprintf((char*)buffer_print, "canbus2\n",7);
	}

	printf("%s",buffer_print);
	uint32_t cansize = 0;

	//wait for ack:
	can_result = wait_for_uart_massage_tester(&ack, (uint8_t*)uart_massage);
	if(can_result)
	{
		memset(uart_massage,0x0,sizeof(uart_massage));
		if(MENU_CANBUS1 == command)
		{
			canbus_deinit(0);
			sprintf(uart_massage, "can1 test failed - no UART ack\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
		}
		else
		{
			canbus_deinit(1);
			sprintf(uart_massage, "can2 test failed - no UART ack\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
		}

		return 1; //ack fail
	}


	if((command == MENU_CANBUS1) && (ack))
	{

		uint8_t canData[8] = {0};
		sprintf((char*)canData, "canbus1");

		//delay to uut to be ready to recieve:


		can_result = canbus_transmit(canData,7);

		if(can_result)
		{
			canbus_deinit(0);
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf(uart_massage, "can1 test failed - transmit fail\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
			return 1;
		}

		sprintf(uart_massage, "1subnac");

		flexcan_msgbuff_t can_buff;


		can_result = canbus_recive(&can_buff, &cansize, 6000);
		if(can_result)
		{
			canbus_deinit(0);
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf(uart_massage, "can1 test failed - receive timeout\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
			return 1;
		}

		if(!strcmp((char*)can_buff.data,uart_massage))
		{
			canbus_deinit(0);

			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf((char*)uart_massage, "can1 test pass\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

			return 0 ;
		}
	}


	if((command == MENU_CANBUS2) && (ack))
	{

		uint8_t canData[8] = {0};
		sprintf((char*)canData, "canbus2");

		//delay to uut to be ready to recieve:


		can_result = canbus_transmit(canData,7);
		if(can_result)
		{
			canbus_deinit(1);
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf(uart_massage, "can2 test failed - transmit fail\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
			return 1;
		}

		sprintf(uart_massage, "2subnac");

		flexcan_msgbuff_t can_buff;


		can_result = canbus_recive(&can_buff, &cansize, 6000);
		if(can_result)
		{
			canbus_deinit(1);
			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf(uart_massage, "can2 test failed - receive timeout\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));
			return 1;
		}

		if(!strcmp((char*)can_buff.data,uart_massage))
		{
			canbus_deinit(1);

			memset(uart_massage,0x0,sizeof(uart_massage));
			sprintf((char*)uart_massage, "can2 test pass\r\n");
			cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

			return 0;
		}
	}


	if(MENU_CANBUS1 == command)
	{
		canbus_deinit(0);
	}
	else
	{
		canbus_deinit(1);
	}

	sprintf(uart_massage, "%s\r\n",uart_massage);
	cdc_write((uint8_t*)uart_massage, strlen(uart_massage));

	return 1;

}
/******************************************************************************
 *
 *    @name        cdc_search_command
 *
 *    @brief       This function receive USB cdc buffer and search for any relevant command
 *
 *    @param       buffer         :  cdc buffer
 *    @param       start_index    :  pointer of start buffer
 *    @param       end_index      :  pointer of end buffer
 *    @param       command        :  command if command found else command none
 *
 *    @return      command found
 *                  1  :  if found
 *                  else return 0
 *
 *****************************************************************************/
bool cdc_search_command(COMMAND_NUMBER_T* command, uint8_t* buffer, uint32_t* buffer_size)

{
	uint32_t j = 0;
	uint32_t i = 0;

	COMMAND_NUMBER_T command_type = NO_COMMAND;
	char command_string[30] = {0};
	uint8_t command_size = 0;

	for(i = 0; i < *buffer_size; i++)
	{

		if(buffer[i] == 0xD) //carriage return
		{
			//check if there is space for minimum command:
			for(j = 0; j < MAX_COMMAND; j++)
            {
				memset(command_string,0x0,sizeof(command_string));
				command_size = 0;

				switch (j)
				{
				case MENU_COMMAND:
					memcpy(command_string, command_list.menu.string, command_list.menu.size);
					command_size = command_list.menu.size;
					command_type = command_list.menu.type;
					break;
				case MENU_TEST:
					memcpy(command_string, command_list.menu_test.string, command_list.menu_test.size);
					command_size = command_list.menu_test.size;
					command_type = command_list.menu_test.type;
					break;
				case MENU_UART:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_uart.string, command_list.menu_uart.size);
						command_size = command_list.menu_uart.size;
						command_type = command_list.menu_uart.type;
					}
					break;

				case MENU_J1708:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_j1708.string, command_list.menu_j1708.size);
						command_size = command_list.menu_j1708.size;
						command_type = command_list.menu_j1708.type;
					}
					break;
				case MENU_A2D:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_a2d.string, command_list.menu_a2d.size);
						command_size = command_list.menu_a2d.size;
						command_type = command_list.menu_a2d.type;
					}
					break;
				case MENU_CANBUS1:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_canbus1.string, command_list.menu_canbus1.size);
						command_size = command_list.menu_canbus1.size;
						command_type = command_list.menu_canbus1.type;
					}
					break;

				case MENU_CANBUS2:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_canbus2.string, command_list.menu_canbus2.size);
						command_size = command_list.menu_canbus2.size;
						command_type = command_list.menu_canbus2.type;
					}
					break;

				case MENU_WIGGLE:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_wiggle.string, command_list.menu_wiggle.size);
						command_size = command_list.menu_wiggle.size;
						command_type = command_list.menu_wiggle.type;
					}
					break;
				case MENU_ACC:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_acc.string, command_list.menu_acc.size);
						command_size = command_list.menu_acc.size;
						command_type = command_list.menu_acc.type;
					}
					break;

				}

				command_string[command_size] = '\0'; //append \0 to end of the command

				//search for command:
				if(( strnstr((char*)buffer, command_string, i) !=NULL) && (command_size != 0x0))
				{
					*buffer_size = 0;
					*command = command_type;
					//found command
					memset(buffer_print,0x0,sizeof(buffer_print));
					sprintf((char*)buffer_print, "\n");
					cdc_write((uint8_t*)buffer_print, strlen((char*)buffer_print));
					return 1; //command found
				}

            }
		}
	}//for(i = 0; i < buffer_size; i++)

	//if no command found so copy last part to beginning:
	if(*buffer_size > MAX_COMMAND * 2)
	{
		memcpy(buffer, (buffer + *buffer_size - MAX_COMMAND), MAX_COMMAND);
		*buffer_size = MAX_COMMAND;
	}

	return 0; //command not found
}

uint8_t buf[64] = {0};




void tester_task()
{

	uint32_t size = 0;
	bool command_found  = FALSE;
	COMMAND_NUMBER_T command = NO_COMMAND;

	cdc_init();

	tester_qid = _msgq_open ((_queue_number)TESTER_QUEUE, 0);
	if (MSGQ_NULL_QUEUE_ID == tester_qid)
	{
	   _task_block();
	}

	_mqx_uint event_result;

	event_result = _event_create("tester_scan");
	if(MQX_OK != event_result){	}

	event_result = _event_open("tester_scan", &tester_scan_event_h);
	if(MQX_OK != event_result){	}


	tester_parameters.menu_mode_on = false;
	tester_parameters.tester_busy = false;

	while (1)
	{

		size += cdc_read(buf + size);

		command_found = cdc_search_command(&command, buf, &size);


		if(command_found)
		{
			//send massage to tester.c (tester task will start debug/release).
			if(MENU_TEST == command)
			{
			//get id:
			memcpy(card_id, buf + 5, sizeof(card_id));
			}
			//else
			//{
				tester_parser(command);
			//}
			//example word led to tester - tester check is debug than use word led.
			//if tester in middle of presses so ignore word
		}
	}
}

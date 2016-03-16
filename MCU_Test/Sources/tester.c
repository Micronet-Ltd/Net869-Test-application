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
uint8_t buffer_scan[20] = {0};
uint8_t card_id[20] = {0};

void* tester_scan_event_h;
extern _pool_id   g_out_message_pool;



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
	sprintf(buf, "0. exit\r\n");
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

bool search_command_tester(UART_ACK_COMMAND_NUMBER_T* command, uint8_t* command_buffer)

{
	uint32_t j = 0;
	UART_ACK_COMMAND_NUMBER_T command_type = NO_COMMAND;
	char command_string[20] = {0};
	uint8_t command_size = 0;

	*command = NO_ACK_COMMAND;

	//check if there is space for minimum command:
	for(j = 0; j < MAX_UART_ACK_COMMAND; j++)
	{
		switch (j)
		{
		case UART_ACK_COMMAND:
			sprintf(command_string, uart_tester_ack_command_list.uart.string, uart_tester_ack_command_list.uart.size);
			command_size = uart_tester_ack_command_list.uart.size;
			command_type = uart_tester_ack_command_list.uart.type;
			break;

		case J1708_ACK_COMMAND:
			sprintf(command_string, uart_tester_ack_command_list.j1708.string, uart_tester_ack_command_list.j1708.size);
			command_size = uart_tester_ack_command_list.j1708.size;
			command_type = uart_tester_ack_command_list.j1708.type;
			break;
		case A2D_ACK_COMMAND:

				sprintf(command_string, uart_tester_ack_command_list.a2d.string, uart_tester_ack_command_list.a2d.size);
				command_size = uart_tester_ack_command_list.a2d.size;
				command_type = uart_tester_ack_command_list.a2d.type;

			break;
		case CANBUS1_ACK_COMMAND:

				sprintf(command_string, uart_tester_ack_command_list.canbus1.string, uart_tester_ack_command_list.canbus1.size);
				command_size = uart_tester_ack_command_list.canbus1.size;
				command_type = uart_tester_ack_command_list.canbus1.type;

			break;

		case CANBUS2_ACK_COMMAND:

				sprintf(command_string, uart_tester_ack_command_list.canbus2.string, uart_tester_ack_command_list.canbus2.size);
				command_size = uart_tester_ack_command_list.canbus2.size;
				command_type = uart_tester_ack_command_list.canbus2.type;

			break;
		case WIGGLE_ACK_COMMAND:

				sprintf(command_string, uart_tester_ack_command_list.wiggle.string, uart_tester_ack_command_list.wiggle.size);
				command_size = uart_tester_ack_command_list.wiggle.size;
				command_type = uart_tester_ack_command_list.wiggle.type;

			break;
		case ACC_ACK_COMMAND:

				sprintf(command_string, uart_tester_ack_command_list.acc.string, uart_tester_ack_command_list.acc.size);
				command_size = uart_tester_ack_command_list.acc.size;
				command_type = uart_tester_ack_command_list.acc.type;

			break;

		}

		//command_string[command_size] = '\0'; //append \0 to end of the command

		//search for command:
		if(( strnstr((char*)command_buffer, command_string, 20) !=NULL) && (command_size != 0x0))
		{
			//*buffer_size = 0;
			*command = command_type;
			//found command
			return 1; //command found
		}

	}

	return 0; //command not found
}


bool wait_for_uart_massage_tester(UART_ACK_COMMAND_NUMBER_T* command )
{
	*command = NO_ACK_COMMAND;
	bool command_found = false;
	_mqx_uint timeout;
	memset(buffer_scan,0x0,sizeof(buffer_scan));
	//scanf(" %s", &buffer_scan);
	if (MQX_OK !=  _event_wait_all(tester_scan_event_h, 0x40, 5000))
	{
		return 1;
	}
	_event_clear(tester_scan_event_h, 0x40);
	strcpy(buffer_scan,wait_for_recieve_massage());


	//check if there is valid massage:
	command_found = search_command_tester(command, buffer_scan);
	if(command_found)
	{
		return 0;
	}


	return 0;
}


void tester_parser(COMMAND_NUMBER_T command)
{
	uint32_t i;

	 _time_delay(10);            // context switch
		char cdc_buffer[30] = {0};
		uint32_t test_status = 0;

		UART_ACK_COMMAND_NUMBER_T uart_command = NO_ACK_COMMAND;
	memset(buffer_print,0x0,sizeof(buffer_print));

	switch (command)
	{
	case MENU_COMMAND:

		if(FALSE == tester_parameters.menu_mode_on)
		{

			tester_parameters.menu_mode_on = TRUE;

			if(FALSE == tester_parameters.uut_abort)
			{
				tester_parameters.uut_abort = TRUE;
				_task_abort(g_TASK_ids[UUT_TASK]);
			}



			//configure tester task waiting for response from UUT if not busy.
			menu_display();
		}
		break;
	case MENU_TEST:

		if(FALSE == tester_parameters.uut_abort)
		{
			tester_parameters.uut_abort = TRUE;
			_task_abort(g_TASK_ids[UUT_TASK]);
		}

		//print card id
		memset(buffer_print,0x0,sizeof(buffer_print));
		sprintf(buffer_print, "id:%s\n", card_id);
		cdc_write((uint8_t*)buffer_print, strlen(buffer_print));
		uint32_t can_status = 0;
		test_status += test_uart();
		test_status += test_j1708();

		can_status = test_canbus(MENU_CANBUS1);
		if(can_status)
		{
			test_status++;

			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "canbus1	fail\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
		}
		else
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "canbus1	pass\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
		}

		can_status = test_canbus(MENU_CANBUS2);
		if(can_status)
		{
			test_status++;
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "canbus2	fail\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
		}
		else
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "canbus1	pass\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
		}

		test_status += test_a2d();
		test_status += test_acc();
		test_status += test_wiggle();

		if(test_status)
		{
			memset(buffer_print,0x0,sizeof(buffer_print));
			sprintf(buffer_print, "full test fail, %d tests fail\r\n", test_status);
			cdc_write((uint8_t*)buffer_print, strlen(buffer_print));
		}
		else
		{
			memset(buffer_print,0x0,sizeof(buffer_print));
			sprintf(buffer_print, "full test pass\r\n");
			cdc_write((uint8_t*)buffer_print, strlen(buffer_print));
		}
		//print end test:
		memset(buffer_print,0x0,sizeof(buffer_print));
		sprintf(buffer_print, "end_test\r\n");
		cdc_write((uint8_t*)buffer_print, strlen(buffer_print));

		break;

	case MENU_EXIT:
		if((tester_parameters.menu_mode_on) && (FALSE == tester_parameters.tester_busy))
		{
			tester_parameters.menu_mode_on = FALSE;
			clear_screan();
		}
		break;

	case MENU_UART:
		test_uart();
		break;
	case MENU_J1708:
		test_j1708();
		break;
	case MENU_CANBUS1:
	case MENU_CANBUS2:
		if(test_canbus(command))
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			if(MENU_CANBUS1 == command)
			{
				sprintf(cdc_buffer, "canbus1	fail\r\n");
				canbus_deinit(0);
			}
			else
			{
				sprintf(cdc_buffer, "canbus2	fail\r\n");
				canbus_deinit(1);
			}

			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
		}
		else
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "canbus1	pass\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
		}
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
	char cdc_buffer[30] = {0};
	bool uart_status = 0;
	UART_ACK_COMMAND_NUMBER_T uart_command = NO_ACK_COMMAND;

	if(FALSE == tester_parameters.tester_busy)
	{
		//send canbus:
		sprintf(buffer_print, "wiggle\n",7);
		printf("%s",buffer_print);

		//wait for ack:
		uart_status = wait_for_uart_massage_tester(&uart_command);
		if(uart_status == 1)
		{
			return 1; //timeout
		}

		//get ack:
		if(uart_command == WIGGLE_ACK_COMMAND)
		{

			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "wiggle	pass\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
			return 0;
		}
		else
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "wiggle	fail\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
			return 1;
		}

	}
}

uint32_t test_acc()
{
	char cdc_buffer[30] = {0};
	bool uart_status = 0;
	UART_ACK_COMMAND_NUMBER_T uart_command = NO_ACK_COMMAND;

	if(FALSE == tester_parameters.tester_busy)
		{
			//send canbus:
			sprintf(buffer_print, "acc\n",4);
			printf("%s",buffer_print);

			//wait for ack:
			uart_status = wait_for_uart_massage_tester(&uart_command);
			if(uart_status == 1)
			{
				return 1; //timeout
			}

			//get ack:
			if(uart_command == ACC_ACK_COMMAND)
			{

				memset(cdc_buffer,0x0,sizeof(cdc_buffer));
				sprintf(cdc_buffer, "ACC	pass\r\n");
				cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
				return 0;
			}
			else
			{
				memset(cdc_buffer,0x0,sizeof(cdc_buffer));
				sprintf(cdc_buffer, "ACC	fail\r\n");
				cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
				return 1;
			}

		}
}

uint32_t test_a2d()
{
	char cdc_buffer[30] = {0};
	bool uart_status = 0;
	UART_ACK_COMMAND_NUMBER_T uart_command = NO_ACK_COMMAND;

	if(FALSE == tester_parameters.tester_busy)
	{
		//send canbus:
		sprintf(buffer_print, "a2d\n",4);
		printf("%s",buffer_print);

		//wait for ack:
		uart_status = wait_for_uart_massage_tester(&uart_command);
		if(uart_status == 1)
		{
			return 1; //timeout
		}

		//get ack:
		if(uart_command == A2D_ACK_COMMAND)
		{

			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "A2D	pass\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
			return 0;
		}
		else
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "A2D	fail\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
			return 1;
		}
	}
}

uint32_t test_uart()
{
	char cdc_buffer[30] = {0};
	bool uart_status = 0;
	UART_ACK_COMMAND_NUMBER_T uart_command = NO_ACK_COMMAND;

	if(FALSE == tester_parameters.tester_busy)
	{
		//send uart:
		sprintf(buffer_print, "uart\n",5);
		printf("%s",buffer_print);

		 _time_delay(10);            // context switch
		uart_status = wait_for_uart_massage_tester(&uart_command);
		if(uart_status == 1)
		{
			return 1; //timeout
		}

		//print pass is uart ack ok:
		if(uart_command == UART_ACK_COMMAND)
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "Uart	pass\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
			return 0;

		}
		else
		{
			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "Uart	fail\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
			return 1;

		}
	}
}

uint32_t test_j1708()
{
	char cdc_buffer[30] = {0};
	bool uart_status = 0;
	UART_ACK_COMMAND_NUMBER_T uart_command = NO_ACK_COMMAND;

	if(FALSE == tester_parameters.tester_busy)
	{
		//send j1708:
		sprintf(buffer_print, "j1708\n",6);
		printf("%s",buffer_print);

		//wait for ack:
		uart_status = wait_for_uart_massage_tester(&uart_command);
		if(uart_status == 1)
		{
			return 1; //timeout
		}

		//get ack:
		if(uart_command == J1708_ACK_COMMAND)
		{
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
			//memcpy(tester_msg_ptr->.data, string_j1708, sizeof(string_j1708));
			sprintf(tester_msg_ptr->data, string_j1708, strlen(string_j1708));
			tester_msg_ptr->timestamp = time;
			tester_msg_ptr->header.SOURCE_QID = tester_qid;
			tester_msg_ptr->header.TARGET_QID = _msgq_get_id(0, J1708_TX_QUEUE);
			tester_msg_ptr->header.SIZE = sizeof (MESSAGE_HEADER_STRUCT) + strlen((char *)tester_msg_ptr->data) + 1 ;

			_msgq_send(tester_msg_ptr);


			uint8_t* j1708_data;
			bool timeout = false;

			//wait for j1708 massage:
			uut_msg_recieve_ptr = _msgq_receive(tester_qid, 6000);
			if (uut_msg_recieve_ptr != NULL)
			{
				//check getting : "8071j"
				//search for command:
				if(!strcmp(uut_msg_recieve_ptr->data, string_j1708_back))
				{
					memset(cdc_buffer,0x0,sizeof(cdc_buffer));
					sprintf(cdc_buffer, "J1708	pass\r\n");
					cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
					_msg_free(uut_msg_recieve_ptr);
					return 0;
				}

			_msg_free(uut_msg_recieve_ptr);
			}

			memset(cdc_buffer,0x0,sizeof(cdc_buffer));
			sprintf(cdc_buffer, "J1708	fail\r\n");
			cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));

			return 1;

		}

		memset(cdc_buffer,0x0,sizeof(cdc_buffer));
		sprintf(cdc_buffer, "J1708	fail\r\n");
		cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
		return 1;
	}
}

uint32_t test_canbus(COMMAND_NUMBER_T command)
{
	char cdc_buffer[30] = {0};
	uint32_t can_result;

	UART_ACK_COMMAND_NUMBER_T uart_command = NO_ACK_COMMAND;

	if(FALSE == tester_parameters.tester_busy)
	{

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
			sprintf(buffer_print, "canbus1\n",7);
		}
		else
		{
			sprintf(buffer_print, "canbus2\n",7);
		}

		printf("%s",buffer_print);
		uint32_t cansize = 0;
		uint8_t candata[64]= {0};
		uint8_t* candata_rec;

		//wait for ack:
		can_result = wait_for_uart_massage_tester(&uart_command);
		if(can_result == 1)
		{
			if(MENU_CANBUS1 == command)
			{
				canbus_deinit(0);
			}
			else
			{
				canbus_deinit(1);
			}

			return 1; //timeout
		}

		//get ack:
		if(uart_command == CANBUS1_ACK_COMMAND)
		{

			uint8_t canData[8] = {0};
			sprintf(canData, "canbus1");

			//delay to uut to be ready to recieve:


			can_result = canbus_transmit(canData,7);

			if(can_result)
			{
				canbus_deinit(0);
				return 1;
			}

			sprintf(cdc_buffer, "1subnac");

			flexcan_msgbuff_t can_buff;


			can_result = canbus_recive(&can_buff, &cansize, 6000);
			if(can_result)
			{
				canbus_deinit(0);
				return 1;
			}

			if(!strcmp(can_buff.data,cdc_buffer))
			{
				canbus_deinit(0);
				return 0 ;
			}
		}


		if(uart_command == CANBUS2_ACK_COMMAND)
		{

			bool can_status = 0;
			uint32_t i;
			uint8_t canData[8] = {0};
			sprintf(canData, "canbus2");

			//delay to uut to be ready to recieve:


			can_result = canbus_transmit(canData,7);
			if(can_status)
			{
				canbus_deinit(1);
				return 1;
			}

			sprintf(cdc_buffer, "2subnac");

			flexcan_msgbuff_t can_buff;


			can_result = canbus_recive(&can_buff, &cansize, 6000);
			if(can_status)
			{
				canbus_deinit(1);
				return 1;
			}

			if(!strcmp(can_buff.data,cdc_buffer))
			{
				canbus_deinit(1);
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

		return 1;

	}
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

				case MENU_EXIT:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_exit.string, command_list.menu_exit.size);
						command_size = command_list.menu_exit.size;
						command_type = command_list.menu_exit.type;
					}
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
					sprintf(buffer_print, "\n");
					cdc_write((uint8_t*)buffer_print, strlen(buffer_print));
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

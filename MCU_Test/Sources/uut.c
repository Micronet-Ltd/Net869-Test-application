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
#include "uut.h"
#include "virtual_com.h"
#include "fsl_uart_driver.h"
#include "uart_configuration.h"

/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
#define UART_PORT 1
#define TIMEOUT 0xfffffff
#define UART_SIZE 10
/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/

/****************************************************************************
 * Global Variables
 ****************************************************************************/
/*****************************************************************************
 * Local Types - None
 *****************************************************************************/
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
bool wait_for_uart_massage_uut(UART_COMMAND_NUMBER_T* command , uint32_t* size);
char* strnstr(const char *s, const char *find, size_t slen);
/*****************************************************************************
 * Local Variables
 *****************************************************************************/

/*****************************************************************************
 * Local Functions
 *****************************************************************************/



uint8_t buffer[20] = {0};

void execute_command(UART_COMMAND_NUMBER_T command_type)
{
	memset(buffer,0x0,sizeof(buffer));

	switch (command_type)
	{
	case ABORT_ACK_UUT_COMMAND:

		memcpy(buffer, uart_ack_command_list.abort.string, uart_ack_command_list.abort.size);
		printf("%s",buffer);

	break;
	case ABORT_UUT_COMMAND:

	  _task_block();
	break;


	case UART_UUT_COMMAND:

		//send tester side acknowledge:

		memcpy(buffer, uart_ack_command_list.uart.string, uart_ack_command_list.uart.size);

		printf("%s",buffer);
		break;

	case J1708_UUT_COMMAND:
		//
		break;

	case CANBUS1_UUT_COMMAND:
		//
		break;

	case CANBUS2_UUT_COMMAND:
		//
		break;

	case WIGGLE_UUT_COMMAND:
		//
		break;

	default:
		//error no command found
		break;
	}
}


bool search_command_uut(UART_COMMAND_NUMBER_T* command, uint8_t* command_buffer)

{
	uint32_t j = 0;
	UART_COMMAND_NUMBER_T command_type = NO_UUT_COMMAND;
	char command_string[MAX_UART_UUT_COMMAND_SIZE] = {0};
	uint8_t command_size = 0;

	//check if there is space for minimum command:
	for(j = 0; j < MAX_UUT_COMMAND; j++)
	{
		switch (j)
		{
		case UART_UUT_COMMAND:
			sprintf(command_string, uart_command_list.uart.string, uart_command_list.uart.size);
			command_size = uart_command_list.uart.size;
			command_type = uart_command_list.uart.type;
			break;

		case J1708_UUT_COMMAND:
			sprintf(command_string, uart_command_list.j1708.string, uart_command_list.j1708.size);
			command_size = uart_command_list.j1708.size;
			command_type = uart_command_list.j1708.type;
			break;

		case CANBUS1_UUT_COMMAND:
				sprintf(command_string, uart_command_list.canbus1.string, uart_command_list.canbus1.size);
				command_size = uart_command_list.canbus1.size;
				command_type = uart_command_list.canbus1.type;
			break;

		case CANBUS2_UUT_COMMAND:
				sprintf(command_string, uart_command_list.canbus2.string, uart_command_list.canbus2.size);
				command_size = uart_command_list.canbus2.size;
				command_type = uart_command_list.canbus2.type;
			break;
		case WIGGLE_UUT_COMMAND:
				sprintf(command_string, uart_command_list.wiggle.string, uart_command_list.wiggle.size);
				command_size = uart_command_list.wiggle.size;
				command_type = uart_command_list.wiggle.type;
			break;
		case ABORT_ACK_UUT_COMMAND:
							sprintf(command_string, uart_command_list.abort.string, uart_command_list.abort.size);
							command_size = uart_command_list.abort.size;
							command_type = uart_command_list.abort.type;
						break;
		case ABORT_UUT_COMMAND:
								sprintf(command_string, uart_command_list.abort_ack.string, uart_command_list.abort_ack.size);
								command_size = uart_command_list.abort_ack.size;
								command_type = uart_command_list.abort_ack.type;
							break;

		}


		//search for command:
		if(( strnstr((char*)command_buffer, (char*)command_string, 20) != NULL) && (command_size != 0x0))
		{
			*command = command_type;
			//found command
			return 1; //command found
		}

	}

	return 0; //command not found
}

uint8_t command_uart[20];

bool wait_for_uart_massage_uut(UART_COMMAND_NUMBER_T* command , uint32_t* size)
{

	*command = NO_UUT_COMMAND;
	bool command_found = false;
	memset(command_uart,0x0,sizeof(command_uart));

	while(1)
	{
		scanf(" %s", &command_uart);

		//check if there is valid massage:
		command_found = search_command_uut(command, command_uart);
		if(command_found)
		{
			return 1;
		}
	}

	return false;
}


void uut_task()
{
	uint32_t size = 0;
	bool status = false;
	UART_COMMAND_NUMBER_T command;

	while (1)
	{
		//wait for uart string massage:
		status = wait_for_uart_massage_uut(&command ,&size);

		if(status)
		{
			//execute uart command:
			execute_command(command);
		}
	}

}


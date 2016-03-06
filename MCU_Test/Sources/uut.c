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
//#include "fsl_device_registers.h"
//#include "fsl_clock_manager.h"
//#include "board.h"
//#include "fsl_debug_console.h"
//#include "fsl_port_hal.h"

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
//bool static tester_side = FALSE;
//bool static uut_side	= FALSE;
/*****************************************************************************
 * Local Types - None
 *****************************************************************************/
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
bool wait_for_uart_massage_uut(UART_COMMAND_NUMBER_T* command , uint32_t* size);

/*****************************************************************************
 * Local Variables
 *****************************************************************************/

/*****************************************************************************
 * Local Functions
 *****************************************************************************/



uint8_t buffer[20] = {0};

execute_command(UART_COMMAND_NUMBER_T command_type)
{
	memset(buffer,0x0,sizeof(buffer));
	uint32_t i;
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
	uint32_t i = 0;

	UART_COMMAND_NUMBER_T command_type = NO_UUT_COMMAND;

	char command_string[MAX_UART_UUT_COMMAND_SIZE] = {0};

	uint8_t command_size = 0;

	//while(i<10)//for(i = 0; i < *buffer_size; i++)
	//{

		//if(buffer[i] == 0xA) //carriage return

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

				//command_string[command_size] = '\n'; //append \0 to end of the command

				//search for command:
				if(( strnstr((char*)command_buffer, command_string, 20) !=NULL) && (command_size != 0x0))
				{
					//*buffer_size = 0;
					*command = command_type;
					//found command
					return 1; //command found
				}

            }
		//}//
	//i++;
	//}//for(i = 0; i < buffer_size; i++)
/*
	//if no command found so copy last part to beginning:
	if(*buffer_size > MAX_UART_UUT_COMMAND_SIZE * 2)
	{
		memcpy(buffer, (buffer + *buffer_size - MAX_UART_UUT_COMMAND_SIZE), MAX_UART_UUT_COMMAND_SIZE);
		*buffer_size = MAX_UART_UUT_COMMAND_SIZE;
	}
*/
	return 0; //command not found
}
static first_massage = true;
uint8_t command_uart[20];

bool wait_for_uart_massage_uut(UART_COMMAND_NUMBER_T* command , uint32_t* size)
{

	*command = NO_UUT_COMMAND;
	uint8_t character;
	bool command_found = false;
	uint32_t i = 0 ;
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



 uint8_t buffere[64] = {0};
 uint8_t buffered[64] = {0};

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


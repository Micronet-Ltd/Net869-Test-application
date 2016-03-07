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
/****************************************************************************
 * Global Variables
 ****************************************************************************/
uint8_t buffer_print[20] = {0};
uint8_t buffer_scan[20] = {0};
void * g_tester_wait_h;
//bool static tester_side = FALSE;
//bool static uut_side	= FALSE;
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
	sprintf(buf, "3. canbus1\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));



	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "4. canbus2\r\n");
	cdc_write((uint8_t*)buf, strlen(buf));



	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "5. wiggle sensor\r\n");
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

		case CANBUS1_ACK_COMMAND:
			if(tester_parameters.menu_mode_on)
			{
				sprintf(command_string, uart_tester_ack_command_list.canbus1.string, uart_tester_ack_command_list.canbus1.size);
				command_size = uart_tester_ack_command_list.canbus1.size;
				command_type = uart_tester_ack_command_list.canbus1.type;
			}
			break;

		case CANBUS2_ACK_COMMAND:
			if(tester_parameters.menu_mode_on)
			{
				sprintf(command_string, uart_tester_ack_command_list.canbus2.string, uart_tester_ack_command_list.canbus2.size);
				command_size = uart_tester_ack_command_list.canbus2.size;
				command_type = uart_tester_ack_command_list.canbus2.type;
			}
			break;
		case WIGGLE_ACK_COMMAND:
			if(tester_parameters.menu_mode_on)
			{
				sprintf(command_string, uart_tester_ack_command_list.wiggle.string, uart_tester_ack_command_list.wiggle.size);
				command_size = uart_tester_ack_command_list.wiggle.size;
				command_type = uart_tester_ack_command_list.wiggle.type;
			}
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
	*command = NO__ACK_COMMAND;
	bool command_found = false;

	memset(buffer_scan,0x0,sizeof(buffer_scan));
	scanf(" %s", &buffer_scan);

	//check if there is valid massage:
	command_found = search_command_tester(command, buffer_scan);
	if(command_found)
	{
		return 1;
	}


	return 0;
}


void tester_parser(COMMAND_NUMBER_T command)
{

	char cdc_buffer[30] = {0};
	bool uart_status = 0;
	 _time_delay(10);            // context switch
	 UART_ACK_COMMAND_NUMBER_T uart_command;

	memset(buffer_print,0x0,sizeof(buffer_print));

	switch (command)
	{
	case MENU_COMMAND:
		if(FALSE == tester_parameters.menu_mode_on)
		{
			tester_parameters.menu_mode_on = TRUE;

			if(FALSE == tester_parameters.uut_abort)
			{
				//send uut abort command:
				memset(buffer_print,0x0,sizeof(buffer_print));
				memcpy(buffer_print, uart_tester_command_list.abort.string, uart_tester_command_list.abort.size);
				printf("%s",buffer_print);
				tester_parameters.uut_abort = TRUE;
			}

			//configure tester task waiting for response from UUT if not busy.
			menu_display();
		}
		break;

	case MENU_EXIT:
		if((tester_parameters.menu_mode_on) && (FALSE == tester_parameters.tester_busy))
		{
			tester_parameters.menu_mode_on = FALSE;
			clear_screan();
		}
		break;

	case MENU_UART:
		if((tester_parameters.menu_mode_on) && (FALSE == tester_parameters.tester_busy))
		{
			//send uart:
			memset(buffer_print,0x0,sizeof(buffer_print));
			memcpy(buffer_print, uart_tester_command_list.uart.string, uart_tester_command_list.uart.size);
			printf("%s",buffer_print);

			 _time_delay(10);            // context switch
			uart_status = wait_for_uart_massage_tester(&uart_command);

			//print pass is uart ack ok:
			if(uart_status)
			{
				memset(cdc_buffer,0x0,sizeof(cdc_buffer));
				sprintf(cdc_buffer, "Uart	pass\r\n");
				cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));

			}
			else
			{
				memset(cdc_buffer,0x0,sizeof(cdc_buffer));
				sprintf(cdc_buffer, "Uart	fail\r\n");
				cdc_write((uint8_t*)cdc_buffer, strlen(cdc_buffer));
			}

		}
		break;

	default:
		break;
	}
}

 uint8_t buf[64] = {0};
 uint8_t buffere[64] = {0};
 uint8_t buffered[64] = {0};


void release_tester()
{
	 _event_set(g_tester_wait_h, EVENT_TESTER);
}

void tester_task()
{

	uint32_t size = 0;
	bool command_found  = FALSE;
	COMMAND_NUMBER_T command = NO_COMMAND;

	cdc_init();

	while (1)
	{
		size += cdc_read(buf + size);

		command_found = cdc_search_command(&command, buf, &size);

		if(command_found)
		{
			//send massage to tester.c (tester task will start debug/release).
			tester_parser(command);
			//example word led to tester - tester check is debug than use word led.
			//if tester in middle of presses so ignore word
		}
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
	char command_string[MAX_COMMAND_SIZE +1] = {0};
	uint8_t command_size = 0;

	for(i = 0; i < *buffer_size; i++)
	{

		if(buffer[i] == 0xD) //carriage return
		{
			//check if there is space for minimum command:
			for(j = 0; j < MAX_COMMAND; j++)
            {
				switch (j)
				{
				case TEST_COMMAND:
					memcpy(command_string, command_list.test.string, command_list.test.size);
					command_size = command_list.test.size;
					command_type = command_list.test.type;
					break;

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

				case MENU_UART:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.menu_uart.string, command_list.menu_uart.size);
						command_size = command_list.menu_uart.size;
						command_type = command_list.menu_uart.type;
					}
					break;
/*
				case MENU_J1708:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.xxx.string, command_list.xxx.size);
						command_size = command_list.xxx.size;
						command_type = command_list.xxx.type;
					}
					break;

				case MENU_CANBUS1:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.xx.string, command_list.xx.size);
						command_size = command_list.xx.size;
						command_type = command_list.xx.type;
					}
					break;

				case MENU_CANBUS2:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.xx.string, command_list.xx.size);
						command_size = command_list.xx.size;
						command_type = command_list.xx.type;
					}
					break;

				case MENU_WIGGLE:
					if(tester_parameters.menu_mode_on)
					{
						memcpy(command_string, command_list.xx.string, command_list.xx.size);
						command_size = command_list.xx.size;
						command_type = command_list.xx.type;
					}
					break;
*/
				}

				command_string[command_size] = '\0'; //append \0 to end of the command

				//search for command:
				if(( strnstr((char*)buffer, command_string, i) !=NULL) && (command_size != 0x0))
				{
					*buffer_size = 0;
					*command = command_type;
					//found command
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


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
#include "tasks_list.h"
#include "canbus.h"
#include "ADC.h"
#include "fsl_i2c_master_driver.h"
/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
#define UART_PORT 1
#define TIMEOUT 0xfffffff
#define UART_SIZE 10
#define UUT_ACC_DEVICE_ADDRESS 			0x1D
#define	UUT_I2C_BAUD_RATE				400
/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/

/****************************************************************************
 * Global Variables
 ****************************************************************************/
extern _pool_id   g_out_message_pool;
_queue_id   uut_qid;
APPLICATION_MESSAGE_PTR_T uut_msg_ptr;
APPLICATION_MESSAGE_PTR_T uut_msg_recieve_ptr;
bool uut_reset = TRUE; // uut out from reset with this default state
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

	uint32_t adc_value = 0; //adc

	uint8_t read_data =  0; //acc
	uint8_t write_data[2] = {0}; //acc
	i2c_device_t acc_device = {.address = UUT_ACC_DEVICE_ADDRESS,    .baudRate_kbps = UUT_I2C_BAUD_RATE}; //acc


	switch (command_type)
	{
	case ABORT_ACK_UUT_COMMAND:
		uut_reset = false;//mask reset
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

		//init rx queue:
		set_queue_target(UUT_QUEUE);


		//send ack:
		sprintf(buffer, "j1708_ack\n");
		printf("%s",buffer);

		uint8_t* j1708_data;
		TIME_STRUCT time;
		bool timeout = false;
		uint8_t string_j1708[] = "j1708";
		uint8_t string_j1708_back[] = "8071j";
		//wait for j1708 massage:
		uut_msg_recieve_ptr = _msgq_receive(uut_qid, 10000);
		if (uut_msg_recieve_ptr == NULL)
		{
			break;
		}

		//check getting : "8071j"
		//search for command:
		if(!strcmp(uut_msg_recieve_ptr->data, string_j1708))
		{
			//send back data in reverse:
			_time_get(&time);
			if ((uut_msg_ptr = (APPLICATION_MESSAGE_PTR_T) _msg_alloc (g_out_message_pool)) == NULL)
			{
				break;
			}
			//memcpy(uut_msg_ptr->.data, string_j1708, sizeof(string_j1708));
			sprintf(uut_msg_ptr->data, string_j1708_back, strlen(string_j1708_back));
			uut_msg_ptr->timestamp = time;
			uut_msg_ptr->header.SOURCE_QID = uut_qid;
			uut_msg_ptr->header.TARGET_QID = _msgq_get_id(0, J1708_TX_QUEUE);
			uut_msg_ptr->header.SIZE = sizeof (MESSAGE_HEADER_STRUCT) + strlen((char *)uut_msg_ptr->data) + 1 ;
			_msgq_send(uut_msg_ptr);
		}

		_msg_free(uut_msg_recieve_ptr);

		break;
	case A2D_UUT_COMMAND:



		//read a2d
		ADC_sample_input (kADC_ANALOG_IN1);
		adc_value =  ADC_get_value (kADC_ANALOG_IN1);

		//check value
		if((adc_value > 3800) && (adc_value < 4400))
		{
			//send ack:
			sprintf(buffer, "a2d_ack\n");
			printf("%s",buffer);
		}
		else
		{
			//send error ack:
			sprintf(buffer, "error_ack\n");
			printf("%s",buffer);
		}

		break;

	case CANBUS1_UUT_COMMAND:

		 //rxMailbxNum 			8
		 //txMailbxNum			9
		 //rxRemoteMailbxNum	10
		 //txRemoteMailbxNum	11
		 //rxRemoteId			0x0F0
		 //txRemoteId			0x00F
		 //rxId					0x456
		 //txId					0x123
		//canbus instance       1 //0 or 1

		canbus_init(9, 8, 11, 10, 0x00F, 0x0F0, 0x123,0x456, 1);

		uint8_t canData[8] = {0};
		sprintf(canData, "hello");
		uint32_t cansize = 0;
		uint8_t candata[64]= {0};

		//send ack:
		sprintf(buffer, "canbus1_ack\n");
		printf("%s",buffer);

		canbus_recive(candata, &cansize, 1, 50000);
		sprintf(buffer, "canbus1_ack\n");


		break;

	case CANBUS2_UUT_COMMAND:
		//
		break;

	case WIGGLE_UUT_COMMAND:

		//read wiggle

		//check value
		if(1) //
		{
			//send ack:
			sprintf(buffer, "wiggle_ack\n");
			printf("%s",buffer);
		}
		else
		{
			//send error ack:
			sprintf(buffer, "error_ack\n");
			printf("%s",buffer);
		}
		break;
		break;
	case ACC_UUT_COMMAND:

		//read acc id
		write_data[0] = 0xD; //ACC id register
		I2C_DRV_MasterReceiveDataBlocking (0, &acc_device, write_data,  1, &read_data, 1, 100);


		//check value
		if(read_data == 0x4A) //acc id value is 0x4A
		{
			//send ack:
			sprintf(buffer, "acc_ack\n");
			printf("%s",buffer);
		}
		else
		{
			//send error ack:
			sprintf(buffer, "error_ack\n");
			printf("%s",buffer);
		}
		break;
	case RESET_UUT_COMMAND:

		if(uut_reset)//parameter is  1, after chip out of reset. press menu change it to "0"
		{
			//send ack:
			uut_reset = FALSE;
			sprintf(buffer, "reset_ack\n");
			printf("%s",buffer);
		}
		else
		{
			//send error ack:
			sprintf(buffer, "error_ack\n");
			printf("%s",buffer);
		}
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

	memset(command_string,0x0, sizeof(command_string));
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
		case A2D_UUT_COMMAND:
			sprintf(command_string, uart_command_list.a2d.string, uart_command_list.a2d.size);
			command_size = uart_command_list.a2d.size;
			command_type = uart_command_list.a2d.type;
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
		case ACC_UUT_COMMAND:
								sprintf(command_string, uart_command_list.acc.string, uart_command_list.acc.size);
								command_size = uart_command_list.acc.size;
								command_type = uart_command_list.acc.type;
							break;
		case RESET_UUT_COMMAND:
								sprintf(command_string, uart_command_list.reset.string, uart_command_list.reset.size);
								command_size = uart_command_list.reset.size;
								command_type = uart_command_list.reset.type;
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

	ADC_init();

	uut_qid = _msgq_open ((_queue_number)UUT_QUEUE, 0);
	if (MSGQ_NULL_QUEUE_ID == uut_qid)
	{
	   _task_block();
	}

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


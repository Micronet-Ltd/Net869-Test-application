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
#include "fsl_flexcan_hal.h"
#include "fpga_api.h"
/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
#define UART_PORT 1
#define TIMEOUT 0xfffffff
#define UART_SIZE 10
#define UUT_ACC_DEVICE_ADDRESS 			0x1D
#define	UUT_I2C_BAUD_RATE				400
#define ACC_ID_VALUE 0x4A
#define MIN_A2D_VALUE 3800
#define MAX_A2D_VALUE 4400

#define  BIT_RATE_33_33KHZ 	2 //for swc
#define  BIT_RATE_1MHZ  	9 //for can

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/

/****************************************************************************
 * Global Variables
 ****************************************************************************/
extern _pool_id   g_out_message_pool;
extern uint32_t wiggle_sensor_cnt;
extern uint8_t* wait_for_recieve_massage();
extern _task_id   g_TASK_ids[NUM_TASKS];
extern bool button_pressed;

void set_queue_target(APPLICATION_QUEUE_T queue_target);
void help_button_interrupt_init();
void * g_help_button_event_h;
_queue_id   uut_qid;
APPLICATION_MESSAGE_PTR_T uut_msg_ptr;
APPLICATION_MESSAGE_PTR_T uut_msg_recieve_ptr;
bool uut_reset = TRUE; // uut out from reset with this default state
void* uut_scan_event_h;
bool start_led = false;

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



uint8_t buffer[50] = {0};

void execute_command(UART_COMMAND_NUMBER_T command_type)
{
	memset(buffer,0x0,sizeof(buffer));

	uint32_t adc_value = 0; //adc
	//uint32_t i;
	uint8_t read_data =  0; //acc
	uint8_t write_data[2] = {0}; //acc
	i2c_device_t acc_device = {.address = UUT_ACC_DEVICE_ADDRESS,    .baudRate_kbps = UUT_I2C_BAUD_RATE}; //acc
	uint32_t cansize;
	uint8_t candata_compare[8]= {0};

	switch (command_type)
	{

	case BUTTON_UUT_COMMAND:

		if(button_pressed)
		{
			sprintf((char*)buffer, "ack:pressed\n");
		}
		else
		{
			sprintf((char*)buffer, "ack:not pressed\n");
		}

		button_pressed = false;

		printf("%s",buffer);
		break;

	case UART_UUT_COMMAND:

		//send tester side acknowledge:
		sprintf((char*)buffer, "ack:uart test pass\n");
		printf("%s",buffer);
		break;
	case LED_UUT_START_COMMAND:
		if(start_led)
		{
			start_led = false;
		}
		else
		{
			start_led = true;
		}
		break;
	case SCUP_UUT_COMMAND:

		_time_delay(5000);

		sprintf((char*)buffer, "ack:scup");
		printf("%s",buffer);

		break;
	case J1708_UUT_COMMAND:

		//init rx queue:
		set_queue_target(UUT_QUEUE);


		//send ack:
		//sprintf((char*)buffer, "j1708_ack\n");
		sprintf((char*)buffer, "ack:j1708\n");
		printf("%s",buffer);

		// _time_delay(1000);            // context switch
		//for(i=0;i<10000;)
		//{i++;}

		TIME_STRUCT time;
		uint8_t string_j1708[] = "j1708";
		uint8_t string_j1708_back[] = "8071j";
		//wait for j1708 massage:
		uut_msg_recieve_ptr = _msgq_receive(uut_qid, 4000);
		if (uut_msg_recieve_ptr == NULL)
		{
			break;
		}

		//check getting : "8071j"
		//search for command:
		if(!strcmp((char*)uut_msg_recieve_ptr->data, (char*)string_j1708))
		{
			//send back data in reverse:
			_time_get(&time);
			if ((uut_msg_ptr = (APPLICATION_MESSAGE_PTR_T) _msg_alloc (g_out_message_pool)) == NULL)
			{
				break;
			}
			//memcpy(uut_msg_ptr->.data, string_j1708, sizeof(string_j1708));
			sprintf((char*)uut_msg_ptr->data, (char*)string_j1708_back, strlen((char*)string_j1708_back));
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
		if((adc_value > MIN_A2D_VALUE) && (adc_value < MAX_A2D_VALUE))
		{
			//send ack:
			//sprintf((char*)buffer, "a2d_ack_%d\n",adc_value);
			sprintf((char*)buffer, "ack:a2d test pass\n");
			printf("%s",buffer);
			_time_delay(100);
			sprintf((char*)buffer, "ack:voltage =%d\n",adc_value);
			printf("%s",buffer);
		}
		else
		{
			//send error ack:
			//sprintf((char*)buffer, "error_ack\n");
			sprintf((char*)buffer, "nack:a2d test failed\n");
			printf("%s",buffer);
			_time_delay(100);
			sprintf((char*)buffer, "ack:voltage =%d\n",adc_value);
			printf("%s",buffer);
		}

		break;

	case CANBUS1_UUT_COMMAND:
	case CANBUS2_UUT_COMMAND:
	case CANBUS1_TERM_UUT_COMMAND:
	case CANBUS2_TERM_UUT_COMMAND:
	case SWC1_UUT_COMMAND:
	case SWC2_UUT_COMMAND:


		 //rxMailbxNum 			8
		 //txMailbxNum			9
		 //rxId					0x456
		 //txId					0x123
		//canbus instance       1 //0 or 1
/*
		if(CANBUS1_TERM_UUT_COMMAND == command_type)
		{
			GPIO_DRV_SetPinOutput(CAN1_TERM_ENABLE);
			_time_delay(1000);
		}
		else if(CANBUS2_TERM_UUT_COMMAND == command_type)
		{
			GPIO_DRV_SetPinOutput(CAN2_TERM_ENABLE);
			_time_delay(1000);
		}
		else
		{
			GPIO_DRV_ClearPinOutput(CAN1_TERM_ENABLE);
			GPIO_DRV_ClearPinOutput(CAN2_TERM_ENABLE);
		}
*/

		//TODO:
		//if swc 33.3 kh
		//if can so 1 Mhz
		//BIT_RATE_33_33KHZ 	2 //for swc
		//BIT_RATE_1MHZ
		if((CANBUS1_UUT_COMMAND == command_type) || (CANBUS2_UUT_COMMAND == command_type))
		{
		    //set CAN termination
			GPIO_DRV_ClearPinOutput(CAN1_TERM_ENABLE);
			GPIO_DRV_ClearPinOutput(CAN2_TERM_ENABLE);

			//Set CAN2 to regular mode twisted
			GPIO_DRV_ClearPinOutput(CAN2_SWC_SELECT);

			//Set to sleep mode NCV7356
			GPIO_DRV_ClearPinOutput(SWC_MODE0);
			GPIO_DRV_ClearPinOutput(SWC_MODE1); // Configuration Sleep

			//Enable SWC
			GPIO_DRV_SetPinOutput(SWC_ENABLE);
			_time_delay(500);

			if(CANBUS1_UUT_COMMAND == command_type)
			{
				canbus_init(BIT_RATE_1MHZ, 8, 9,  0x456,0x123 , 0);
			}
			else
			{
				canbus_init(BIT_RATE_1MHZ, 8, 9,  0x456,0x123 , 1);
			}


		}
		else if((CANBUS1_TERM_UUT_COMMAND == command_type) || (CANBUS2_TERM_UUT_COMMAND == command_type))
		{
		    //Unset CAN termination
		    GPIO_DRV_SetPinOutput(CAN1_TERM_ENABLE);
		    GPIO_DRV_SetPinOutput(CAN2_TERM_ENABLE);

			//Set CAN2 to regular mode twisted
			GPIO_DRV_ClearPinOutput(CAN2_SWC_SELECT);

			//Set to sleep mode NCV7356
			GPIO_DRV_ClearPinOutput(SWC_MODE0);
			GPIO_DRV_ClearPinOutput(SWC_MODE1); // Configuration Sleep

			//Enable SWC
			GPIO_DRV_SetPinOutput(SWC_ENABLE);
			_time_delay(500);

			if(CANBUS1_TERM_UUT_COMMAND == command_type)
			{
				canbus_init(BIT_RATE_1MHZ, 8, 9,  0x456,0x123 , 0);
			}
			else
			{
				canbus_init(BIT_RATE_1MHZ, 8, 9,  0x456,0x123 , 1);
			}

		}
		else if((SWC1_UUT_COMMAND == command_type) ||(SWC2_UUT_COMMAND == command_type))
		{
			//Set SWC to operation mode
			//Set CAN2 to regular mode twisted
			GPIO_DRV_SetPinOutput(CAN2_SWC_SELECT);

			//Set Speed to Normal 33KHz of NCV7356
			GPIO_DRV_SetPinOutput(SWC_MODE0);
			//GPIO_DRV_SetPinOutput(SWC_MODE1); // Configuratio Normal 33KHz Mode Mode0 High Mode1 High
			GPIO_DRV_ClearPinOutput(SWC_MODE1); // Configuratio High up to 100Khz Mode Mode0 High Mode1 Low

			//Enable SWC
			GPIO_DRV_ClearPinOutput(SWC_ENABLE);
			//GPIO_DRV_SetPinOutput(SWC_ENABLE);
			_time_delay(500);

			if(SWC1_UUT_COMMAND == command_type)
			{
				canbus_init(BIT_RATE_33_33KHZ, 8, 9,  0x456,0x123 , 0);
			}
			else
			{
				canbus_init(BIT_RATE_33_33KHZ, 8, 9,  0x456,0x123 , 1);
			}
		}


		sprintf((char*)buffer, "ack:can\n");
		printf("%s",buffer);


		uint32_t i;
		for(i=0;i<10000;i++)
		{
			i=i;
		}
		flexcan_msgbuff_t can_buff;

		canbus_recive(&can_buff, &cansize,  4000);
		for(i=0;i<10000;i++)
		{
			i=i;
		}

		if((CANBUS1_UUT_COMMAND == command_type) || (CANBUS1_TERM_UUT_COMMAND == command_type) || (SWC1_UUT_COMMAND == command_type))
		{
			sprintf((char*)candata_compare, "canbus1");
		}
		else
		{
			sprintf((char*)candata_compare, "canbus2");
		}

		//delay for tester to be ready to recieve canbus:
		if(!strcmp((char*)can_buff.data,(char*)candata_compare))
		{
			if((CANBUS1_UUT_COMMAND == command_type) || (CANBUS1_TERM_UUT_COMMAND == command_type) || (SWC1_UUT_COMMAND == command_type))
			{
				sprintf((char*)candata_compare, "1subnac");
			}
			else
			{
				sprintf((char*)candata_compare, "2subnac");
			}
			for(i=0;i<1000000;)
					{
						i++;
					}
			canbus_transmit(candata_compare,7);
		}
		else
		{
			i++;
		}

		for(i=0;i<10000;)
		{
			i++;
		}
		//send back
		_time_delay(1000);

		if((CANBUS1_UUT_COMMAND == command_type) || (CANBUS1_TERM_UUT_COMMAND == command_type) || (SWC1_UUT_COMMAND == command_type))
		{
			canbus_deinit(0);
		}
		else
		{
			canbus_deinit(1);
		}
		break;

	case WIGGLE_UUT_COMMAND:

		//read wiggle

		//check value
		if( wiggle_sensor_cnt > 0) //
		{
			//send ack:
			sprintf((char*)buffer, "ack:wiggle pass\n");
			printf("%s",buffer);
		}
		else
		{
			//send error ack:
			sprintf((char*)buffer, "nack:wiggle fail\n");
			printf("%s",buffer);
		}
		break;
	case ACC_UUT_COMMAND:

		//read acc id
		write_data[0] = 0xD; //ACC id register
		I2C_DRV_MasterReceiveDataBlocking (0, &acc_device, write_data,  1, &read_data, 1, 100);


		//check value
		if(read_data == ACC_ID_VALUE) //acc id value is 0x4A
		{
			//send ack:
			sprintf((char*)buffer, "ack:acc test pass\n");
			printf("%s",buffer);
			_time_delay(100);
			sprintf((char*)buffer, "ack:id=%d\n",read_data);
			printf("%s",buffer);
		}
		else
		{
			//send error ack:
			sprintf((char*)buffer, "nack:acc test fail\n");
			printf("%s",buffer);
			_time_delay(100);
			sprintf((char*)buffer, "nack:id=%d\n",read_data);
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
		case CANBUS1_TERM_UUT_COMMAND:
				sprintf(command_string, uart_command_list.canbus1_term.string, uart_command_list.canbus1_term.size);
				command_size = uart_command_list.canbus1_term.size;
				command_type = uart_command_list.canbus1_term.type;
			break;
		case CANBUS2_TERM_UUT_COMMAND:
				sprintf(command_string, uart_command_list.canbus2_term.string, uart_command_list.canbus2_term.size);
				command_size = uart_command_list.canbus2_term.size;
				command_type = uart_command_list.canbus2_term.type;
			break;
		case SWC1_UUT_COMMAND:
				sprintf(command_string, uart_command_list.swc1.string, uart_command_list.swc1.size);
				command_size = uart_command_list.swc1.size;
				command_type = uart_command_list.swc1.type;
			break;
		case SWC2_UUT_COMMAND:
				sprintf(command_string, uart_command_list.swc2.string, uart_command_list.swc2.size);
				command_size = uart_command_list.swc2.size;
				command_type = uart_command_list.swc2.type;
			break;
		case SCUP_UUT_COMMAND:
				sprintf(command_string, uart_command_list.scup.string, uart_command_list.scup.size);
				command_size = uart_command_list.scup.size;
				command_type = uart_command_list.scup.type;
			break;
		case BUTTON_UUT_COMMAND:
				sprintf(command_string, uart_command_list.button.string, uart_command_list.button.size);
				command_size = uart_command_list.button.size;
				command_type = uart_command_list.button.type;
			break;
		case WIGGLE_UUT_COMMAND:
				sprintf(command_string, uart_command_list.wiggle.string, uart_command_list.wiggle.size);
				command_size = uart_command_list.wiggle.size;
				command_type = uart_command_list.wiggle.type;
			break;

		case ACC_UUT_COMMAND:
								sprintf(command_string, uart_command_list.acc.string, uart_command_list.acc.size);
								command_size = uart_command_list.acc.size;
								command_type = uart_command_list.acc.type;
							break;
		case LED_UUT_START_COMMAND:
								sprintf(command_string, uart_command_list.led.string, uart_command_list.led.size);
								command_size = uart_command_list.led.size;
								command_type = uart_command_list.led.type;
							break;
		}


		//search for command:
		if(( strstr((char*)command_buffer, (char*)command_string) != NULL) && (command_size != 0x0))
		{
			*command = command_type;
			//found command
			return 1; //command found
		}

	}

	return 0; //command not found
}

uint8_t command_uart[20];

//wait till getting massage from scanf:
bool wait_for_uart_massage_uut(UART_COMMAND_NUMBER_T* command , uint32_t* size)
{

	*command = NO_UUT_COMMAND;
	bool command_found = false;
	memset(command_uart,0x0,sizeof(command_uart));

	while(1)
	{
		//wait till get massage from scan task:
		_event_wait_all(uut_scan_event_h, 0x20, 0);
		_event_clear(uut_scan_event_h, 0x20);

		strcpy((char*)command_uart,(char*)wait_for_recieve_massage());
		//check if there is valid massage:
		command_found = search_command_uut(command, command_uart);
		if(command_found)
		{
			return 1;
		}
	}

	return false;
}

void led_task()
{
	uint8_t Br, R,G,B;
	Br = 10;


	while(1)
	{

		if(start_led)
		{

			R = 255;
			G = 0;
			B = 0;

			FPGA_write_led_status (LED_RIGHT , &Br, &R, &G, &B);
			FPGA_write_led_status (LED_MIDDLE, &Br, &R, &G, &B);
			GPIO_DRV_ClearPinOutput (LED_GREEN);
			GPIO_DRV_ClearPinOutput (LED_BLUE);
			GPIO_DRV_SetPinOutput   (LED_RED);
			if(start_led)
			{
				_time_delay(1000);
			}

			R = 0;
			G = 255;
			B = 0;

			FPGA_write_led_status (LED_RIGHT , &Br, &R, &G, &B);
			FPGA_write_led_status (LED_MIDDLE, &Br, &R, &G, &B);
			GPIO_DRV_ClearPinOutput (LED_RED);
			GPIO_DRV_SetPinOutput (LED_GREEN);
			GPIO_DRV_ClearPinOutput (LED_BLUE);
			if(start_led)
			{
				_time_delay(1000);
			}

			R = 0;
			G = 0;
			B = 255;

			FPGA_write_led_status (LED_RIGHT , &Br, &R, &G, &B);
			FPGA_write_led_status (LED_MIDDLE, &Br, &R, &G, &B);
			GPIO_DRV_ClearPinOutput (LED_RED);
			GPIO_DRV_ClearPinOutput (LED_GREEN);
			GPIO_DRV_SetPinOutput (LED_BLUE);



		}

		_time_delay(1000);
	}

	_task_block();
}


/*
void help_button_task (void)
{

	_mqx_uint event_result;
	uint32_t pin_input = 0;


	event_result = _event_create("event.HelpButtonInt");
	if(MQX_OK != event_result){	}

	event_result = _event_open("event.HelpButtonInt", &g_help_button_event_h);
	if(MQX_OK != event_result){	}


	while(1)
	{
		_event_wait_all(g_help_button_event_h, 1, 0);
		_event_clear(g_help_button_event_h, 1);

		pin_input = GPIO_DRV_ReadPinInput (BUTTON1);

		//send massage on reset:
		sprintf((char*)buffer, "MCU_button_press\n");
		printf("%s",buffer);

	}

	_task_block();

}
*/
void uut_task()
{
	uint32_t size = 0;
	bool status = false;
	UART_COMMAND_NUMBER_T command;
	_mqx_uint event_result;

	//help_button_interrupt_init();

	event_result = _event_create("uut_scan");
	if(MQX_OK != event_result){	}

	event_result = _event_open("uut_scan", &uut_scan_event_h);
	if(MQX_OK != event_result){	}

	ADC_init();

	uut_qid = _msgq_open ((_queue_number)UUT_QUEUE, 0);
	if (MSGQ_NULL_QUEUE_ID == uut_qid)
	{
	   _task_block();
	}

	//send massage on reset:
	sprintf((char*)buffer, "MCU_started\n");
	printf("%s",buffer);





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


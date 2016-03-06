/**HEADER********************************************************************
*
*
***************************************************************************
*
*
**************************************************************************
*
* $FileName: tester.h$
* $Version :
* $Date    :
*
* Comments:
*
* @brief The file contains Macro's and functions needed by the tester
*        application
*
*****************************************************************************/

#ifndef _TESTER_H
#define _TESTER_H

//command getting from cdc uart:
typedef enum {
	NO_COMMAND		=	0,
	TEST_COMMAND		 ,
	MENU_COMMAND         ,
	MENU_UART			 ,
	MENU_J1708			 ,
	MENU_CANBUS1		 ,
	MENU_CANBUS2		 ,
	MENU_WIGGLE  		 ,
	MENU_EXIT			 ,
	MAX_COMMAND			 ,
} COMMAND_NUMBER_T;


//command getting from cdc uart:
typedef enum {
	NO__ACK_COMMAND		=	0,
	UART_ACK_COMMAND		 ,
	J1708_ACK_COMMAND        ,
	CANBUS1_ACK_COMMAND		 ,
	CANBUS2_ACK_COMMAND      ,
	WIGGLE_ACK_COMMAND		 ,
	ABORT_ACK_COMMAND		 ,
	MAX_UART_ACK_COMMAND     ,
} UART_ACK_COMMAND_NUMBER_T;


#define MAX_COMMAND_SIZE  5  //DEBUG is 5 character which is max size

typedef struct
{
	char* string;
	uint32_t size;
	UART_ACK_COMMAND_NUMBER_T type;
} CDC_COMMAND_T;


typedef struct
{
	CDC_COMMAND_T uart;
	CDC_COMMAND_T j1708;
	CDC_COMMAND_T canbus1;
	CDC_COMMAND_T canbus2;
	CDC_COMMAND_T wiggle;
	CDC_COMMAND_T abort;


} UART_TESTER_ACK_COMMAND_LIST_T;

UART_TESTER_ACK_COMMAND_LIST_T uart_tester_ack_command_list =
{
		{
				"uart_ack\n",
				9,
				UART_ACK_COMMAND
		},
		{
				"8071j",
				5,
				J1708_ACK_COMMAND
		},
		{
				"canbus1_ack",
				10,
				CANBUS1_ACK_COMMAND
		},
		{
				"canbus2_ack",
				10,
				CANBUS2_ACK_COMMAND
		},
		{
				"wiggle_ack",
				10,
				WIGGLE_ACK_COMMAND
		}
};

//command from tester to uut side:
UART_TESTER_ACK_COMMAND_LIST_T uart_tester_command_list =
{
		{
				"uart\n",
				5,
				UART_ACK_COMMAND
		},
		{
				"8071j\n",
				6,
				J1708_ACK_COMMAND
		},
		{
				"canbus1\n",
				8,
				CANBUS1_ACK_COMMAND
		},
		{
				"canbus2\n",
				8,
				CANBUS2_ACK_COMMAND
		},
		{
				"wiggle\n",
				7,
				WIGGLE_ACK_COMMAND
		},
		{
				"abort\n",
				6,
				ABORT_ACK_COMMAND
		}
};



typedef struct
{
	CDC_COMMAND_T test;
	CDC_COMMAND_T menu;
	CDC_COMMAND_T menu_exit;
	CDC_COMMAND_T menu_uart;


} COMMAND_LIST_T;

//usb side commands:
COMMAND_LIST_T command_list =
{
		{
				"test",
				4,
				TEST_COMMAND
		},
		{
				"menu",
				4,
				MENU_COMMAND
		},
		{
				"0",
				1,
				MENU_EXIT
		},
		{
				"1",
				1,
				MENU_UART
		}
};




typedef struct
{
	bool menu_mode_on;
	bool test_mode_on;
	bool tester_busy;
	bool uut_abort;
} TESTER_PARAMETER_LIST_T;

TESTER_PARAMETER_LIST_T tester_parameters =
{
		FALSE,
		FALSE,
		FALSE,
		FALSE
};


void tester_parser(COMMAND_NUMBER_T command);
bool cdc_search_command(COMMAND_NUMBER_T* command, uint8_t* buffer, uint32_t* buffer_size);

extern _task_id   g_TASK_ids[19];//NUM_TASKS;

#endif //TESTER_H

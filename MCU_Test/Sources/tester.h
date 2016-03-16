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

#define TIME_OUT 1
#define ACK_FAIL 2

//command getting from cdc uart:
typedef enum {
	MENU_TEST            , // !! must be first so when changing from menu to test all it recognize string "test_"
	MENU_UART			 ,
	MENU_J1708			 ,
	MENU_A2D			 ,
	MENU_ACC  		     ,
	MENU_CANBUS1		 ,
	MENU_CANBUS2		 ,
	MAX_AUTO_TEST	 	 ,  //put all auto tests above, put all manual tests below:
	MENU_WIGGLE  		 ,
	MENU_COMMAND         ,
	NO_COMMAND			 ,
	MAX_COMMAND			 ,
} COMMAND_NUMBER_T;


//command acknowledge from uut side:
typedef enum {
	NO_ACK_COMMAND		=	0,
	UART_ACK_COMMAND		 ,
	J1708_ACK_COMMAND        ,
	CANBUS1_ACK_COMMAND		 ,
	CANBUS2_ACK_COMMAND      ,
	WIGGLE_ACK_COMMAND		 ,
	A2D_ACK_COMMAND		 	 ,
	ACC_ACK_COMMAND		 	 ,
	MAX_UART_ACK_COMMAND     ,
} UART_ACK_COMMAND_NUMBER_T;


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
	CDC_COMMAND_T a2d;
	CDC_COMMAND_T canbus1;
	CDC_COMMAND_T canbus2;
	CDC_COMMAND_T wiggle;
	CDC_COMMAND_T acc;



} UART_TESTER_ACK_COMMAND_LIST_T;

UART_TESTER_ACK_COMMAND_LIST_T uart_tester_ack_command_list =
{
		{
				"uart_ack\n",
				9,
				UART_ACK_COMMAND
		},
		{
				"j1708_ack",
				5,
				J1708_ACK_COMMAND
		},
		{
				"a2d_ack",
				7,
				A2D_ACK_COMMAND
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
		},
		{
				"acc_ack",
				7,
				ACC_ACK_COMMAND
		}
};

//usb menu commands:
typedef struct
{
	CDC_COMMAND_T menu;
	CDC_COMMAND_T menu_uart;
	CDC_COMMAND_T menu_j1708;
	CDC_COMMAND_T menu_a2d;
	CDC_COMMAND_T menu_canbus1;
	CDC_COMMAND_T menu_canbus2;
	CDC_COMMAND_T menu_wiggle;
	CDC_COMMAND_T menu_acc;
	CDC_COMMAND_T menu_test;



} COMMAND_LIST_T;

//usb menu commands:
COMMAND_LIST_T command_list =
{
		{
				"menu",
				4,
				MENU_COMMAND
		},
		{
				"1",
				1,
				MENU_UART
		},
		{
				"2",
				1,
				MENU_J1708
		},
		{
				"3",
				1,
				MENU_A2D
		},
		{
				"4",
				1,
				MENU_CANBUS1
		},
		{
				"5",
				1,
				MENU_CANBUS2
		},
		{
				"6",
				1,
				MENU_WIGGLE
		},
		{
				"7",
				1,
				MENU_ACC
		},
		{
				"test_",
				5,
				MENU_TEST
		}
};




typedef struct
{
	bool menu_mode_on;
	bool test_mode_on;
	bool tester_busy;
	bool uut_abort;
} TESTER_PARAMETER_LIST_T;




void tester_parser(COMMAND_NUMBER_T command);
bool cdc_search_command(COMMAND_NUMBER_T* command, uint8_t* buffer, uint32_t* buffer_size);

extern _task_id   g_TASK_ids[19];//NUM_TASKS;

#endif //TESTER_H

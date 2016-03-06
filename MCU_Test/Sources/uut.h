/**HEADER********************************************************************
*
*
***************************************************************************
*
*
**************************************************************************
*
* $FileName: uut.h$
* $Version :
* $Date    :
*
* Comments:
*
* @brief The file contains Macro's and functions needed by the uut
*        application
*
*****************************************************************************/

#ifndef _UUT_H
#define _UUT_H

#define MAX_UART_UUT_COMMAND_SIZE 10

//command getting from cdc uart:
typedef enum {
	NO_UUT_COMMAND		=	0,
	UART_UUT_COMMAND		 ,
	J1708_UUT_COMMAND        ,
	CANBUS1_UUT_COMMAND		 ,
	CANBUS2_UUT_COMMAND      ,
	WIGGLE_UUT_COMMAND		 ,
	ABORT_UUT_COMMAND        ,
	ABORT_ACK_UUT_COMMAND    ,
	MAX_UUT_COMMAND          ,
} UART_COMMAND_NUMBER_T;


#define MAX_COMMAND_SIZE  10  //DEBUG is 5 character which is max size

typedef struct
{
	char* string;
	uint32_t size;
	UART_COMMAND_NUMBER_T type;
} UART_COMMAND_T;

typedef struct
{
	UART_COMMAND_T uart;
	UART_COMMAND_T j1708;
	UART_COMMAND_T canbus1;
	UART_COMMAND_T canbus2;
	UART_COMMAND_T wiggle;
	UART_COMMAND_T abort;
	UART_COMMAND_T abort_ack;

} UART_COMMAND_LIST_T;

UART_COMMAND_LIST_T uart_command_list =
{
		{
				"uart",
				4,
				UART_UUT_COMMAND
		},
		{
				"j1708",
				5,
				J1708_UUT_COMMAND
		},
		{
				"canbus1",
				6,
				CANBUS1_UUT_COMMAND
		},
		{
				"canbus2",
				6,
				CANBUS2_UUT_COMMAND
		},
		{
				"wiggle",
				6,
				WIGGLE_UUT_COMMAND
		},
		{
				"abort",
				5,
				ABORT_ACK_UUT_COMMAND
		},
		{
				"abort_ack",
				9,
				ABORT_UUT_COMMAND
		}
};


UART_COMMAND_LIST_T uart_ack_command_list =
{
		{
				"uart_ack\n",
				9,
				UART_UUT_COMMAND
		},
		{
				"j1708_ack\n",
				5,
				J1708_UUT_COMMAND
		},
		{
				"canbus1_ack\n",
				11,
				CANBUS1_UUT_COMMAND
		},
		{
				"canbus2_ack\n",
				11,
				CANBUS2_UUT_COMMAND
		},
		{
				"wiggle_ack\n",
				11,
				WIGGLE_UUT_COMMAND
		},
		{
				"abort_ack\n",
				10,
				ABORT_ACK_UUT_COMMAND
		}
};


#endif //_UUT_H

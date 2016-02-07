#include "tasks_list.h"


TASK_TEMPLATE_STRUCT MQX_template_list[] =
{
//  Task number,		Entry point,			Stack,     Pri,	    					Task name       	,	    Auto start        Creation Param      Time Slice
	{ MAIN_TASK,			Main_task,			1000,       MAIN_TASK_PRIORITY,			"MAIN_TASK",			MQX_AUTO_START_TASK,		0,            	0 },
	{ ACC_TASK,				Acc_task,			2000,      	ACC_TASK_PRIORITY,			"ACC_TASK",				0,        					0,              0 },
//	{ USB_TASK,				Usb_task,			1500,      	USB_TASK_PRIORITY,			"USB_TASK",				0,					        0,              0 },
	{ J1708_RX_TASK,		J1708_Rx_task,		1500,		J1708_RX_TASK_PRIORITY,		"J1708_RX_TASK",		0,							0,				0 },
	{ J1708_TX_TASK,		J1708_Tx_task,		1500,		J1708_TX_TASK_PRIORITY,		"J1708_TX_TASK",		0,							0,				0 },
	{ FPGA_UART_RX_TASK,	FPGA_UART_Rx_task,	1500,		FPGA_UART_RX_TASK_PRIORITY,	"FPGA_UART_RX_TASK",	0,							0,				0 },
	{ 0	}
};
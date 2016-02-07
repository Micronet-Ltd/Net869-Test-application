#include <stdio.h>
#include <mqx.h>
#include <bsp.h>

#include <fsl_flexcan_driver.h>
#include <fsl_flexcan_hal.h>
#include <lwmsgq.h>
#include <mutex.h>

#include "fsl_i2c_master_driver.h"

#include "tasks_list.h"
#include "gpio_pins.h"
#include "J1708_task.h"
#include "fpga_api.h"


void MQX_I2C0_IRQHandler (void);
void MQX_PORTA_IRQHandler(void);
void MQX_PORTC_IRQHandler(void);

#define	MAIN_TASK_SLEEP_PERIOD	10			// 10 mSec sleep
#define TIME_ONE_SECOND_PERIOD	((int) (1000 / MAIN_TASK_SLEEP_PERIOD))

static i2c_master_state_t i2c_master;

_pool_id   g_out_message_pool;
_pool_id   g_in_message_pool;

uint32_t wiggle_sensor_cnt;

_task_id   g_TASK_ids[NUM_TASKS] = { 0 };


void Main_task( uint32_t initial_data ) {

    _queue_id  main_qid;    //, usb_qid, can1_qid, can2_qid, j1708_qid, acc_qid, reg_qid;
	_queue_id  j1708_rx_qid;
	//APPLICATION_MESSAGE_T *msg;

    uint8_t u8mainTaskLoopCnt = 0;

    wiggle_sensor_cnt = 0;
    _time_delay (10);


    printf("\nMain Task: Start \n");
#if 0
    PinMuxConfig ();
    ADCInit      ();
    USBInit      ();
    I2CInit      ();
    registerInit ();
#endif

    // board Initialization
    hardware_init();
    OSA_Init();
    GPIO_Config();

	NVIC_SetPriority(PORTA_IRQn, 6U);
	OSA_InstallIntHandler(PORTA_IRQn, MQX_PORTA_IRQHandler);
	NVIC_SetPriority(PORTC_IRQn, 6U);
	OSA_InstallIntHandler(PORTC_IRQn, MQX_PORTC_IRQHandler);

    // I2C0 Initialization
    NVIC_SetPriority(I2C0_IRQn, 6U);
    OSA_InstallIntHandler(I2C0_IRQn, MQX_I2C0_IRQHandler);
    I2C_DRV_MasterInit(I2C0_IDX, &i2c_master);

    // turn on device
    GPIO_DRV_SetPinOutput(POWER_3V3_ENABLE);
    GPIO_DRV_SetPinOutput(POWER_5V0_ENABLE);
//	GPIO_DRV_ClearPinOutput(ACC_ENABLE       );
	GPIO_DRV_SetPinOutput(ACC_ENABLE       );

    // FPGA Enable
    GPIO_DRV_SetPinOutput(FPGA_PWR_ENABLE);

//	BOARD_InitOsc0();
//	CLOCK_SetBootConfig_Run ();

	//Enable CAN
	GPIO_DRV_SetPinOutput(CAN_ENABLE);

    // Enable USB for DEBUG
    GPIO_DRV_ClearPinOutput(USB_ENABLE);
    GPIO_DRV_ClearPinOutput(USB_HUB_RSTN);

    GPIO_DRV_ClearPinOutput(USB_OTG_SEL);    // Connect D1 <-> D MCU or HUB
    //GPIO_DRV_SetPinOutput(USB_OTG_SEL);    // Connect D2 <-> D A8 OTG
    GPIO_DRV_ClearPinOutput(USB_OTG_OE); //Enable OTG/MCU switch

    _time_delay(10);
    GPIO_DRV_SetPinOutput(USB_HUB_RSTN);
    GPIO_DRV_SetPinOutput(USB_ENABLE);

    _time_delay(20);
/*
    g_TASK_ids[USB_TASK] = _task_create(0, USB_TASK, 0);
	if ( g_TASK_ids[USB_TASK] == MQX_NULL_TASK_ID ) {
		MIC_DEBUG_UART_PRINTF("\nMain Could not create USB_TASK\n");
	}
*/

    //Enable UART
    GPIO_DRV_SetPinOutput(UART_ENABLE);
    GPIO_DRV_SetPinOutput(FTDI_RSTN);

/*
 * RUSLAN closed for testing
	g_out_message_pool = _msgpool_create (sizeof(APPLICATION_MESSAGE_T), NUM_CLIENTS, 0, 0);
	if (g_out_message_pool == MSGPOOL_NULL_POOL_ID)
	{
		printf("\nCould not create a g_out_message_pool message pool\n");
		_task_block();
	}

	g_in_message_pool = _msgpool_create (sizeof(APPLICATION_MESSAGE_T), NUM_CLIENTS, 0, 0);
	if (g_in_message_pool == MSGPOOL_NULL_POOL_ID)
	{
		printf("\nCould not create a g_in_message_pool message pool\n");
		_task_block();
	}
*/
	main_qid = _msgq_open(MAIN_QUEUE, 0);

	_time_delay (1000);

	FPGA_init ();

	J1708_enable  (7);


#if 0
	GPIO_DRV_SetPinOutput   (LED_BLUE);

    GPIO_DRV_ClearPinOutput(CPU_ON_OFF);
    _time_delay (3000);
    GPIO_DRV_SetPinOutput(CPU_ON_OFF);

    GPIO_DRV_ClearPinOutput   (LED_BLUE);
#else
    _time_delay (1000);
#endif

	{
		uint8_t Br, R,G,B;
		R = G = B = 255;
		Br = 10;
		FPGA_write_led_status (LED_RIGHT , &Br, &R, &G, &B);
		_time_delay (10);
		FPGA_write_led_status (LED_MIDDLE, &Br, &R, &G, &B);
		_time_delay (10);
	}


	g_TASK_ids[J1708_TX_TASK] = _task_create(0, J1708_TX_TASK, 0 );
	if (g_TASK_ids[J1708_TX_TASK] == MQX_NULL_TASK_ID)
	{
		printf("\nMain Could not create J1708_TX_TASK\n");
	}

	g_TASK_ids[J1708_RX_TASK] = _task_create(0, J1708_RX_TASK, 0 );
	if (g_TASK_ids[J1708_RX_TASK] == MQX_NULL_TASK_ID)
	{
		printf("\nMain Could not create J1708_RX_TASK\n");
	}
	
	g_TASK_ids[FPGA_UART_RX_TASK] = _task_create(0, FPGA_UART_RX_TASK, 0 );
	if (g_TASK_ids[FPGA_UART_RX_TASK] == MQX_NULL_TASK_ID)
	{
		printf("\nMain Could not create FPGA_UART_RX_TASK\n");
	}

	g_TASK_ids[ACC_TASK] = _task_create(0, ACC_TASK, 0);
	if (g_TASK_ids[ACC_TASK] == MQX_NULL_TASK_ID)
	{
		printf("\nMain Could not create ACC_TASK\n");
	}

    //Disable CAN termination
    GPIO_DRV_ClearPinOutput(CAN1_TERM_ENABLE);
    GPIO_DRV_ClearPinOutput(CAN2_TERM_ENABLE);

    //Initialize CAN sample
    configure_can_pins(0);
    configure_can_pins(1);

    _time_delay (1000);


    printf("\nMain Task: Loop \n");


    while ( 1 ) {

#if 0
		GPIO_DRV_ClearPinOutput (LED_RED);
		GPIO_DRV_ClearPinOutput (LED_GREEN);
		GPIO_DRV_ClearPinOutput (LED_BLUE);
		_time_delay (1000);

		GPIO_DRV_SetPinOutput   (LED_RED);
		_time_delay (1000);

		GPIO_DRV_SetPinOutput   (LED_GREEN);
		_time_delay (1000);

		GPIO_DRV_SetPinOutput   (LED_BLUE);
		_time_delay (1000);
#endif
	    _time_delay(MAIN_TASK_SLEEP_PERIOD);            // context switch
    }

    printf("\nMain Task: End \n");
    _task_block();       // should never get here

}

#if 0
void OTG_CONTROL (void)
{
	uint8_t user_switch_status =  (GPIO_DRV_ReadPinInput (SWITCH2) << 1) + GPIO_DRV_ReadPinInput (SWITCH1);

	if (user_switch_status == user_switch)
		return;

	user_switch = user_switch_status;
	GPIO_DRV_SetPinOutput (USB_OTG_OE);
	_time_delay (1000);

	// disable OTG Switch

	case (user_switch) {
		OTG_CPU_CONNECTION :


			// select channel

			break;

		OTG_HUB_CONNECTION :
			break;

		default            : break;

		// enable OTG Switch
		GPIO_DRV_ClearPinOutput (USB_OTG_OE);
	}
}
#endif

void MQX_PORTA_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending (VIB_SENS)) {
		GPIO_DRV_ClearPinIntFlag(VIB_SENS);
		//wiggle_sensor_cnt++;
	}
}

void MQX_PORTC_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending (FPGA_GPIO0)) {
		GPIO_DRV_ClearPinIntFlag(FPGA_GPIO0);
		_event_set(g_J1708_event_h, EVENT_J1708_RX);
	}
}

//END FILE
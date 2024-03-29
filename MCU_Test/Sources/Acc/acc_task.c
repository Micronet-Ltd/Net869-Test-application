#include <mqx.h>
#include <bsp.h>
#include <message.h>
#include <mutex.h>
//#include <sem.h>

#include <event.h>

//extern const gpio_input_pin_user_config_t inputPins *;
#include "gpio_pins.h"
#include "tasks_list.h"

#include "fsl_i2c_master_driver.h"


#define ACC_DEVICE_ADDRESS 			0x1D
#define	I2C_BAUD_RATE				400

#define	ACC_I2C_PORT				I2C0_IDX

#define ACC_VALUE_ID				0x4A
#define ACC_VALUE_STATUS_WATERMARK	0x3F
#define ACC_VALUE_RESET_CMD			0x40

#define ACC_REG_STATUS				0x00
#define ACC_REG_FIFO_SAMPLES		0x01
#define ACC_REG_F_SETUP				0x09
#define ACC_REG_XYZ_DATA_CFG		0x0E
#define ACC_REG_WHO_AM_I			0x0D
#define ACC_REG_CTRL_REG1			0x2A
#define ACC_REG_CTRL_REG2			0x2B
#define ACC_REG_CTRL_REG4			0x2D
#define ACC_REG_CTRL_REG5			0x2E

#define MAX_FIFO_SIZE				192
#define ACC_MAX_POOL_SIZE			10

#define TIME_OUT					100		//  in miliseconds

#define DEBUG						TRUE

static i2c_device_t acc_device = {.address = ACC_DEVICE_ADDRESS,    .baudRate_kbps = I2C_BAUD_RATE};
bool acc_enabled_g = false;

/**************************************************************************************
* The accelerometer is connected to MCU I2C interface. When device is powered, the    *
* accelerometer is accessed every 1.25mSec (800 Hz), reading 32 samples of each axis. *
* When the device is off, accelerometer is not accessed at all and configured to be   *
* in low power mode.                                                                  *
*                                                                                     *
* The MCU resets and reconfigure the accelerometer every power up. accelerometer data *
* is read from its internal FIFO.
**************************************************************************************/
bool accInit       (void);
void AccDisable    (void);
void AccEnable     (void);
void ISR_accIrq    (void* param);
void acc_fifo_read (uint8_t *buffer, uint8_t max_buffer_size);

void * g_acc_event_h;

void acc_irq(void)
{
	GPIO_DRV_ClearPinIntFlag(ACC_INT);
	// Signal main task to read acc
	_event_set(g_acc_event_h, 1);
}

void AccIntEnable()
{
	uint32_t port;

	const gpio_input_pin_user_config_t inputPin =
			{
					  .pinName = ACC_INT,
					  .config.isPullEnable = false,
					  .config.pullSelect = kPortPullUp,
					  .config.isPassiveFilterEnabled = false,
					  .config.interrupt = kPortIntFallingEdge,
			  };

	// FIXME: hardcoded index
	port = GPIO_EXTRACT_PORT(ACC_INT);
	NVIC_SetPriority(g_portIrqId[port], 6U);
	OSA_InstallIntHandler(g_portIrqId[port], acc_irq);

	GPIO_DRV_InputPinInit(&inputPin);

}

#define MIC_LED_TEST

void Acc_task (uint32_t initial_data)
{

	/* */
	_mqx_uint event_result;


	event_result = _event_create("event.AccInt");
	if(MQX_OK != event_result){	}

	event_result = _event_open("event.AccInt", &g_acc_event_h);
	if(MQX_OK != event_result){	}

	//print yuvalf("\nACC Task: Start \n");

	GPIO_DRV_SetPinOutput   (ACC_ENABLE       );
	_time_delay(100);

	//TODO yuval -temp till fix acc int
	while(1)
	{
		_time_delay (10000);

	}

	AccIntEnable();
	// try to initialize accelerometer every 10 seconds
	while (accInit () == false)
	{
		_time_delay (10000);
	}

#ifdef MIC_LED_TEST
	GPIO_DRV_SetPinOutput(LED_GREEN);
#endif


	//TODO hack Enabling sensor by default
	AccEnable();

	//TODO: Remote Test acc message
	//test_acc_msg.header.SOURCE_QID = acc_qid;
	//test_acc_msg.header.TARGET_QID = _msgq_get_id(0, USB_QUEUE);
	//test_acc_msg.header.SIZE = sizeof(MESSAGE_HEADER_STRUCT) + strlen((char *)msg->data) + 1;
	//acc_msg = &test_acc_msg;
	while (1)
	{

		/*
		 * Ruslan Add test procedure
		if ((acc_msg = (APPLICATION_MESSAGE_PTR_T) _msg_alloc (g_out_message_pool)) == NULL)
		{
			if (MQX_OK != (err_task = _task_get_error()))
			{
				_task_set_error(MQX_OK);
			}
			printf("ACC Task: ERROR: message allocation failed %x\n", err_task);
		}

		_event_wait_all(g_acc_event_h, 1, 0);
		_event_clear(g_acc_event_h, 1);

		if(acc_msg) {
			acc_fifo_read ((uint8_t*)&(acc_msg->data), (uint8_t)(sizeof(acc_msg->data)-sizeof(uint64_t)));
			_time_get(&time);
			acc_msg->timestamp = time;
			acc_msg->header.SOURCE_QID = acc_qid;
			acc_msg->header.TARGET_QID = _msgq_get_id(0, USB_QUEUE);
			acc_msg->header.SIZE = sizeof(acc_msg->data)-sizeof(uint64_t);
			_msgq_send (acc_msg);

			if (MQX_OK != (err_task = _task_get_error()))
			{
				printf("ACC Task: ERROR: message send failed %x\n", err_task);
				_task_set_error(MQX_OK);
			}
		}
		*/

		_time_delay (100);
	}

	// should never get here
	//print yuvalf("\nACC Task: End \n");
	_task_block();
}


bool accInit (void)
{
	uint8_t read_data      =  0 ;
	uint8_t write_data [2] = {0};

	// read recognition device ID
	write_data[0] = ACC_REG_WHO_AM_I   ;
	if (I2C_DRV_MasterReceiveDataBlocking (ACC_I2C_PORT, &acc_device, write_data,  1, &read_data, 1, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;
	if (read_data == ACC_VALUE_ID)
	{
		//print yuvalf ("ACC Task: Device detected\n");
	}
	else
	{
		//print yuvalf ("ACC Task: Device NOT detected\n");
		goto _ACC_CONFIG_FAIL;
	}
	
	// reset device
	write_data[0] = ACC_REG_CTRL_REG2   ;
	write_data[1] = ACC_VALUE_RESET_CMD ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;
    OSA_TimeDelay(1);

	// set CTRL_REG1 to STANDBY Normal mode with 1.56Hz sample rates reads at SLEEP mode and 800Hz at ACTIVE mode
	write_data[0] = ACC_REG_CTRL_REG1   ;
	write_data[1] = 0x00 ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;

	write_data[0] = ACC_REG_CTRL_REG1   ;
	write_data[1] = 0xC0 ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;

	// configure device to Low power in Sleep mode and Normal power mode at Active
	write_data[0] = ACC_REG_CTRL_REG2   ;
	write_data[1] = 0x18 ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;

	// configure interrupt source to FIFO interrupt on INT1 pin with water mark of 10 samples
	// when FIFO sample count exceeding the water mark event does not stop the FIFO from accepting new data
	// FIFO always contains the most recent samples when overflowed (FMODE = 01)
	write_data[0] = ACC_REG_CTRL_REG5   ;
	write_data[1] = 0x40 ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;

	write_data[0] = ACC_REG_CTRL_REG4   ;
	write_data[1] = 0x40 ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;

	write_data[0] = ACC_REG_F_SETUP   ;
	write_data[1] = 0x4A ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;
	
	// configure output buffer data format using 8g scale range
	write_data[0] = ACC_REG_XYZ_DATA_CFG   ;
	write_data[1] = 0x02 ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;

	// set ACC to ACTIVE mode
	write_data[0] = ACC_REG_CTRL_REG1   ;
	write_data[1] = 0xC1 ;
    if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_CONFIG_FAIL;

	//printf ("ACC Task: Device Configured \n");
	return true;

_ACC_CONFIG_FAIL:
	//printf ("ACC Task: ERROR: Device NOT Configured \n");
	return false;
} 

void AccEnable (void)
{
	uint8_t read_data      =  0 ;
	uint8_t write_data [2] = {0};

	// read recognition register
	write_data[0] = ACC_REG_CTRL_REG1;
	if (I2C_DRV_MasterReceiveDataBlocking (ACC_I2C_PORT, &acc_device, write_data,  1, &read_data, 1, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_ENABLE_FAIL;

	write_data[1] = read_data |= 0x1;
	//if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)			goto _ACC_ENABLE_FAIL;
	//printf ("ACC Task: Accelerometer Enabled \n");
	acc_enabled_g = TRUE;
	return;

_ACC_ENABLE_FAIL:
_time_delay(10); //printf ("ACC Task: ERROR: Accelerometer NOT enabled \n");

}

void AccDisable (void)
{
	uint8_t read_data      =  0 ;
	uint8_t write_data [2] = {0};
	
	// read recognition register
	write_data[0] = ACC_REG_CTRL_REG1;
	if (I2C_DRV_MasterReceiveDataBlocking (ACC_I2C_PORT, &acc_device, write_data,  1, &read_data, 1, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_DISABLE_FAIL;

	write_data[1] = read_data &= ~0x1;
	if (I2C_DRV_MasterSendDataBlocking    (ACC_I2C_PORT, &acc_device, NULL,  0, write_data, 2, TIME_OUT) != kStatus_I2C_Success)			goto _ACC_DISABLE_FAIL;
	//print yuvalf ("ACC Task: Accelerometer Disabled \n");
	acc_enabled_g = FALSE;
	return;

_ACC_DISABLE_FAIL:
	_time_delay(10); //print yuvalf ("ACC Task: ERROR: Accelerometer NOT disabled \n");
}

void acc_fifo_read (uint8_t *buffer, uint8_t max_buffer_size)
{
	uint8_t u8ByteCnt      =  0 ;
	uint8_t read_data      =  0 ;
	uint8_t write_data [2] = {0};
	
	// read status register
	write_data[0] = ACC_REG_STATUS;
	if (I2C_DRV_MasterReceiveDataBlocking (ACC_I2C_PORT, &acc_device, write_data,  1, &read_data, 1, TIME_OUT*10) != kStatus_I2C_Success)		goto _ACC_FIFO_READ_FAIL;

	u8ByteCnt  = (read_data & ACC_VALUE_STATUS_WATERMARK);				// get amount of samples in FIFO
	u8ByteCnt *= 6;														// read 2 Bytes per Sample (each sample is 12 bits): Max 192 Samples
	
	if (u8ByteCnt > max_buffer_size)
		u8ByteCnt = max_buffer_size;

	write_data[0] = ACC_REG_FIFO_SAMPLES;
	if (I2C_DRV_MasterReceiveDataBlocking (ACC_I2C_PORT, &acc_device, write_data,  1, buffer, u8ByteCnt, TIME_OUT) != kStatus_I2C_Success)		goto _ACC_FIFO_READ_FAIL;
	return;

_ACC_FIFO_READ_FAIL:
_time_delay(10); //print yuvalf ("ACC Task: ERROR: Accelerometer read failure \n");
}

#if 0
void ISR_accIrq (void* param) 
{
	LWGPIO_STRUCT_PTR gpio = (LWGPIO_STRUCT_PTR) param; 
	APPLICATION_MESSAGE *msg;

	// TODO: change to event instead of message
	if ((msg = _msg_alloc_system (sizeof(*msg))) != NULL) 
		 msg->header.SOURCE_QID =  _msgq_get_id(0, ACC_QUEUE);
		 msg->header.TARGET_QID =  _msgq_get_id(0, ACC_QUEUE);
		 _msgq_send (msg);
	}
	lwgpio_int_clear_flag (gpio);
}
#endif


/**************************************************************************************
* The J1708 is implemented by the on board FPGA. There are 2 J1708 independent tasks  *
* Rx task and Tx task. The FPGA - MCU data protocol interface is implemented via UART *
* channel, while the command protocol interface is implemente via I2C channel. Both   *
* Tx and Rx tasks use the same UART and I2C channels.                                 *
*                                                                                     *
* J1708 Tx task:                                                                      *
* --------------                                                                      *
* Tx task receives data from CPU and trnasfer it "AS IS" to the on board FPGA. When   *
* transfer is completed, the MCU sends an I2C command, indicating the amount of byte  *
* in the message. By that, the MCU also indicates that package is ready to be sent.   *
*                                                                                     *
* J1708 Rx task:                                                                      *
* --------------                                                                      *
* Rx task receives data from on board FPGA and trnasfer it "AS IS" to the MCU. When   *
* FPGA has a complete packet that needs to be sent to CPU, it generates an interrupt  *
* to the MCU. if The MCU interrupt rutine detects that J1708 message is ready it      *
* notifys to Rx task that a message is ready to be read from the FPGA.                *
* The Rx task reads the amount of bytes to be read (message length) via I2C channel   *
* and reads the message via UART channel. When message is read it is being sent to    *
* USB task.                                                                           *
**************************************************************************************/

#include "tasks_list.h"
#include "J1708_task.h"

#include "fpga_api.h"

#include "gpio_pins.h"


#define J1708_DISABLED							0
#define J1708_ENABLED							1

#ifdef __cplusplus
extern "C"
{
#endif

void debug_read_j1708_status (void);

static uint8_t J1708_state    = J1708_DISABLED;
static uint8_t J1708_priority = 7;			// lowest priority by default

void * g_J1708_event_h;

APPLICATION_QUEUE_T g_queue_target;

APPLICATION_MESSAGE_T *validation_j1708_msg;
bool massage_ready = FALSE;

void J1708_reset   (void);



void J1708_Tx_task (uint32_t initial_data)
{
	static _queue_id   J1708_tx_qid;

	APPLICATION_MESSAGE_T *msg;
	J1708_tx_qid = _msgq_open (J1708_TX_QUEUE, 0);

	uint8_t       cnt;
	bool          J1708TxFull   ;
	uint8_t       uart_tx_length;
	
	//print yuvalf ("\nJ1708_Tx Task: Start \n");

	while (1) {
		while (J1708_state == J1708_DISABLED)
			_time_delay (10000);

		debug_read_j1708_status ();

		// wait 10 second for interrupt message
		msg = _msgq_receive(J1708_tx_qid, 10000);
		if (msg == NULL) {
			//printf ("\nJ1708_Tx: WARNING: Message not received in last 10 Seconds \n");
			continue;
		}

		// read J1708 package length register via I2C channel and verify there is enough space for new message
		for (cnt = 0; cnt < 10; cnt++) {
			if (FPGA_read_J1708_tx_register (&J1708TxFull, NULL))
				if (!J1708TxFull)
					break;
		}

		if (cnt >= 10) {
			//printf ("\nJ1708_Tx: ERROR: Previous transfer not completed !!!\n");
			J1708_reset ();
			_msg_free   (msg);
			continue;
		}

		// send message via UART channel
		uart_tx_length = (msg->header.SIZE - sizeof (MESSAGE_HEADER_STRUCT)) ;
		if (!FPGA_write_J1708_packet  (msg->data, uart_tx_length)) {
			_msg_free   (msg);
			J1708_reset ();
			continue;
		}
		
		_msg_free   (msg);

		// send command via I2C channel
		if (!FPGA_write_J1708_tx_length (&uart_tx_length)) {
			J1708_reset ();
			continue;
		}

		debug_read_j1708_status ();
	}

	// should never get here
	//print yuvalf ("\nJ1708_Tx Task: End \n");
	_task_block();
}

#define MIC_LED_TEST

void J1708_Rx_task (uint32_t initial_data)
{
	APPLICATION_MESSAGE_T *msg;
	_mqx_uint err_task;
	const _queue_id     J1708_rx_qid  = _msgq_open (J1708_RX_QUEUE, 0);
	//_queue_id     USB_qid    	= _msgq_get_id (0, USB_QUEUE     );
	//_queue_id     USB_qid       = _msgq_get_id (0, J1708_TX_QUEUE     );		// loop for test only

	uint32_t J1708_rx_event_bit;
	bool     J1708_rx_status;
	uint8_t  J1708_rx_len;

	_event_create ("event.J1708_RX");
	_event_open   ("event.J1708_RX", &g_J1708_event_h);
	//print yuvalf ("\nJ1708_Rx Task: Start \n");

	while (1) {
		while (J1708_state == J1708_DISABLED)
			_time_delay (10000);


		// wait 10 second for interrupt message
		_event_wait_all (g_J1708_event_h, EVENT_J1708_RX, 3000);
		_event_get_value (g_J1708_event_h, &J1708_rx_event_bit);
		if (J1708_rx_event_bit == EVENT_J1708_RX)
			_event_clear    (g_J1708_event_h, EVENT_J1708_RX);
		else {
			////print yuvalf ("\nJ1708_Rx: WARNING: No interrupt in last 10 Seconds \n");
			continue;
		}
		
		// read J1708 package length
		if (!FPGA_read_J1708_rx_register (&J1708_rx_status, &J1708_rx_len)) {
			J1708_reset ();
			continue;
		}

		// check if this is a real new message
		if (J1708_rx_status == false) {
			////print yuvalf ("\nJ1708_Rx: ERROR: Received interrupt without register bit indication\n");
			continue;
		}

		// send buffer - Since the buffer is cyclic, it might be needed to split buffer to 2 buffers
		/*
		 * Ruslan Add message pool for test
		 */

		if ((msg = _msg_alloc(g_out_message_pool)) != NULL) {
			msg->header.SOURCE_QID = J1708_rx_qid;
			msg->header.TARGET_QID = _msgq_get_id(0, get_queue_target());
			msg->header.SIZE       = J1708_rx_len;
		}
		else {
			if (MQX_OK != (err_task = _task_get_error()))
			{
				_task_set_error(MQX_OK);
			}
			//print yuvalf("J1708_Rx_task Task: ERROR: message allocation failed %x\n", err_task);
			continue;
		}


		// calculate actual buffer size
		if (!FPGA_read_J1708_packet (msg->data, msg->header.SIZE)) {
			//print yuvalf ("\nJ1708_Rx: ERROR: Could not read UART message buffer\n");
			J1708_reset ();
		}

		// add header size to message length
		msg->header.SIZE  += sizeof (MESSAGE_HEADER_STRUCT);

		_msgq_send (msg);
		if (MQX_OK != (err_task = _task_get_error()))
		{
			//print yuvalf("J1708_Rx_task Task: ERROR: message send failed %x\n", err_task);
			_task_set_error(MQX_OK);
		}

		//move massage to global pointer for hw validation:
		massage_ready = TRUE;
		validation_j1708_msg = msg;

#ifdef MIC_LED_TEST
		//GPIO_DRV_SetPinOutput(LED_BLUE);
#endif

	}
	
	// should never get here
	//print yuvalf ("\nJ1708_Rx Task: End \n");
	_task_block();
}



void J1708_reset (void)
{
	J1708_disable ();
	J1708_enable (J1708_priority);
}		

void J1708_enable (uint8_t priority)
{
	uint8_t prio = priority;

	if (FPGA_set_irq (FPGA_REG_J1708_RX_IRQ_BIT)) {}
		//print yuvalf ("\nJ1708: Set FPGA J1708 Rx IRQ\n");
//	else
		//print yuvalf ("\nJ1708: ERROR: FPGA J1708 Rx IRQ NOT Set !!!\n");

	// set enable bit
	if (!FPGA_write_J1708_priority (&prio)) {}
		//print yuvalf ("J1708: ERROR: J1708 Priority NOT SET\n");

	if (FPGA_J1708_enable ()) {}
		//print yuvalf ("\nJ1708: J1708 Enabled\n");
	//else
		//print yuvalf ("\nJ1708: ERROR: J1708 NOT Enabled !!!\n");

	GPIO_DRV_ClearPinOutput (J1708_ENABLE);

	J1708_state = J1708_ENABLED;
	J1708_priority = priority;
}		

void J1708_disable (void)
{
	GPIO_DRV_SetPinOutput   (J1708_ENABLE);

	if (FPGA_clear_irq (FPGA_REG_J1708_RX_IRQ_BIT)) {}
		//print yuvalf ("\nJ1708: Clear FPGA J1708 Rx IRQ\n");
//	else
		//print yuvalf ("\nJ1708: ERROR: FPGA J1708 Rx IRQ NOT Cleared !!!\n");

	if (FPGA_J1708_disable ()) {}
		//print yuvalf ("\nJ1708: J1708 Disabled\n");
//	else
		//print yuvalf ("\nJ1708: ERROR: J1708 NOT Disabled !!!\n");

	J1708_state = J1708_DISABLED;
}

//0 massage is ready
//1 timeout
APPLICATION_QUEUE_T get_queue_target()
{
	return g_queue_target;
}

void set_queue_target(APPLICATION_QUEUE_T queue_target)
{
	g_queue_target = queue_target;
}

void debug_read_j1708_status (void)
{
	// for debug only
	uint32_t debug_version = 0;
	bool debug_status = 0;;					// enable \ disable
	bool debug_tx_status = 0;;
	uint8_t debug_tx_len = 0;;
	uint8_t debug_prio = 0;;


	FPGA_read_J1708_status (&debug_status);
	FPGA_read_J1708_tx_register(&debug_tx_status, &debug_tx_len);
	FPGA_read_J1708_priority(&debug_prio);
	FPGA_read_version (&debug_version);
}


#ifdef __cplusplus
}
#endif


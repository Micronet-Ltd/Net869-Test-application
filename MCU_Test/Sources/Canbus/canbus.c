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
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "fsl_clock_manager.h"
#include "fsl_flexcan_driver.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "canbus.h"
/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/

/****************************************************************************
 * Global Variables
 ****************************************************************************/


/*****************************************************************************
 * Local Types - None
 *****************************************************************************/
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/

/*****************************************************************************
 * Local Variables
 *****************************************************************************/
 flexcan_user_config_t flexcanData;
 uint32_t canPeClk;
/*****************************************************************************
 * Local Functions
 *****************************************************************************/





/* Global variables*/
uint32_t txIdentifier;
uint32_t rxIdentifier;

//uint32_t txRemoteIdentifier;
//uint32_t rxRemoteIdentifier;

uint32_t txMailboxNum;
uint32_t rxMailboxNum;

//uint32_t rxRemoteMailboxNum;
//uint32_t rxRemoteMailboxNum;

flexcan_state_t canState;

uint32_t instance = BOARD_CAN_INSTANCE;
uint32_t numErrors;

/* The following tables are the CAN bit timing parameters that are calculated by using the method
 * outlined in AN1798, section 4.1.
 */
/*
 * The table contains propseg, pseg1, pseg2, pre_divider, and rjw. The values are calculated for
 * a protocol engine clock of 60MHz
 */
flexcan_time_segment_t bitRateTable60Mhz[] = {
    { 6, 7, 7, 19, 3},  /* 125 kHz */
    { 6, 7, 7,  9, 3},  /* 250 kHz */
    { 6, 7, 7,  4, 3},  /* 500 kHz */
    { 6, 5, 5,  3, 3},  /* 750 kHz */
    { 6, 5, 5,  2, 3},  /* 1   MHz */
};

/*
 * The table contains propseg, pseg1, pseg2, pre_divider, and rjw. The values are calculated for
 * a protocol engine clock of 48MHz
 */
flexcan_time_segment_t bitRateTable48Mhz[] = {
    { 6, 7, 7, 15, 3},  /* 125 kHz */
    { 6, 7, 7,  7, 3},  /* 250 kHz */
    { 6, 7, 7,  3, 3},  /* 500 kHz */
    { 6, 3, 3,  3, 3},  /* 750 kHz */
    { 6, 3, 3,  2, 3},  /* 1   MHz */
};

/*
 * The table contains propseg, pseg1, pseg2, pre_divider, and rjw. The values are calculated for
 * a protocol engine clock of 75MHz
 */
flexcan_time_segment_t bitRateTable75Mhz[] = {
    { 6, 7, 7, 25, 3},  /* 125 kHz */
    { 6, 7, 7, 12, 3},  /* 250 kHz */
    { 6, 6, 6,  6, 3},  /* 500 kHz */
    { 6, 4, 4,  5, 3},  /* 750 kHz */
    { 6, 3, 3,  4, 3},  /* 1   MHz */
};

uint32_t send_data(uint8_t* data, uint32_t size, uint32_t canInstance)
{
    //uint8_t data[8];
    uint32_t result;
    flexcan_data_info_t txInfo;

    /*Standard ID*/
    txInfo.msg_id_type = kFlexCanMsgIdStd;
    txInfo.data_length = size;



    //PRINTF("\r\nFlexCAN send config");
    result = FLEXCAN_DRV_ConfigTxMb(canInstance, txMailboxNum, &txInfo, txIdentifier);
    if (result)
    {
        return 1;
    }
    else
    {
        result = FLEXCAN_DRV_SendBlocking(instance, txMailboxNum, &txInfo, txIdentifier,
                                  data, 6000);
        if (result)
        {
           return 1;
        }
        else
        {
           return 0;
        }
    }
}


// FlexCAN receive configuration
void receive_mb_config(void)
{
    uint32_t result;
    flexcan_data_info_t rxInfo;

    rxInfo.msg_id_type = kFlexCanMsgIdStd;
    rxInfo.data_length = 1;

    //PRINTF("\r\nFlexCAN MB receive config");

    /* Configure RX MB fields*/
    result = FLEXCAN_DRV_ConfigRxMb(instance, rxMailboxNum, &rxInfo,rxIdentifier);
    if (result)
    {
        numErrors++;
        //PRINTF("\r\nFlexCAN RX MB configuration failed. result: 0x%lx", result);
    }
}


void canbus_deinit(uint32_t canInstance)
{
	FLEXCAN_DRV_Deinit(canInstance);
}

void canbus_init(
				uint32_t rxMailbxNum,
				uint32_t txMailbxNum,
				///uint32_t rxRemoteMailbxNum,
				//uint32_t txRemoteMailbxNum,
				//uint32_t rxRemoteId,
				//uint32_t txRemoteId,
				uint32_t rxId,
				uint32_t txId,
				uint32_t canInstance

			)
{
    //PRINTF("\r\nRunning the FlexCAN loopback example.");

    uint32_t result;
    flexcan_user_config_t flexcanData;
    uint32_t canPeClk;

    instance = canInstance;
	 // Select mailbox number
	rxMailboxNum = rxMailbxNum;//8;
	txMailboxNum = txMailbxNum;//9;

	//rxRemoteMailboxNum = rxRemoteMailbxNum;//10;
	//rxRemoteMailboxNum = txRemoteMailbxNum;//11;

	// Select mailbox ID
	//rxRemoteIdentifier = rxRemoteId;//0x0F0;
	//txRemoteIdentifier = txRemoteId;//0x00F;

	rxIdentifier = rxId;//0x456;//0x123;
	txIdentifier = txId;//0x123;

    numErrors = 0;

    flexcanData.max_num_mb = 16;
    flexcanData.num_id_filters = kFlexCanRxFifoIDFilters_8;
    flexcanData.is_rx_fifo_needed = false;
    flexcanData.flexcanMode = kFlexCanNormalMode;//kFlexCanLoopBackMode;kFlexCanNormalMode



	// Set rxIdentifier as same as txIdentifier to receive loopback data

	result = FLEXCAN_DRV_Init(instance, &canState, &flexcanData);
	if (result)
	{
		numErrors++;
		//PRINTF("\r\nFLEXCAN initilization. result: 0x%lx", result);
		return;
	}

	if (FLEXCAN_HAL_GetClock((g_flexcanBase[instance])))
	{
		canPeClk = CLOCK_SYS_GetFlexcanFreq(0, kClockFlexcanSrcBusClk);
	}
	else
	{
		canPeClk = CLOCK_SYS_GetFlexcanFreq(0, kClockFlexcanSrcOsc0erClk);
	}

	switch (canPeClk)
	{
		case 60000000:
			result = FLEXCAN_DRV_SetBitrate(instance, &bitRateTable60Mhz[0]); // 125kbps
			break;
		case 48000000:
			result = FLEXCAN_DRV_SetBitrate(instance, &bitRateTable48Mhz[0]); // 125kbps
			break;
		default:
			if ((canPeClk > 74990000) && (canPeClk <= 75000000))
			{
			result = FLEXCAN_DRV_SetBitrate(instance, &bitRateTable75Mhz[0]); // 125kbps
			}
			else
			{
			  //PRINTF("\r\nFLEXCAN bitrate table not available for PE clock: %d", canPeClk);
			  //return kStatus_FLEXCAN_Fail;
			}
	}
	if (result)
	{
		numErrors++;
		//PRINTF("\r\nFLEXCAN set bitrate failed. result: 0x%lx", result);
	}

	FLEXCAN_DRV_SetRxMaskType(instance, kFlexCanRxMaskIndividual);

	FLEXCAN_DRV_SetRxMaskType(instance, kFlexCanRxMaskGlobal);

	result = FLEXCAN_DRV_SetRxMbGlobalMask(instance, kFlexCanMsgIdStd,rxId);//rxId);// 0x123);
	if (result)
	{
		numErrors++;
		//PRINTF("\r\nFLEXCAN set rx MB global mask. result: 0x%lx", result);
	}

	// Standard ID
	result = FLEXCAN_DRV_SetRxIndividualMask(instance, kFlexCanMsgIdStd, rxMailboxNum, 0x7FF);
	if(result)
	{
		numErrors++;
		//PRINTF("\r\nFLEXCAN set rx individual mask with standard ID fail. result: 0x%1x", result);
	}

	// Extern ID
	result = FLEXCAN_DRV_SetRxIndividualMask(instance, kFlexCanMsgIdExt, rxMailboxNum, rxId);//0x123);//rxId);//
	if(result)
	{
		numErrors++;
		//PRINTF("\r\nFLEXCAN set rx individual mask with standard ID fail. result: 0x%1x", result);
	}
}

//return 0 pass/1 fail
bool canbus_recive(flexcan_msgbuff_t* rxMb,uint32_t* size, uint32_t timeout)
{
	// Config receive mailbox
	receive_mb_config();

	flexcan_status_t error_canbus;
	error_canbus = FLEXCAN_DRV_RxMessageBufferBlocking(instance,rxMailboxNum, rxMb,timeout);

	if(error_canbus)
	{
		return TRUE;
	}
	//result = FLEXCAN_DRV_RxMessageBuffer(canInstance, rxMailboxNum,&rxMb);

	*size = ((rxMb->cs) >> 16) & 0xF;

	//f = rxMb->data[0];
//	 for (i = 0; i < size; i++)
//	 {
//		 data[i]  = rxMb.data[result];
//	 }

	return FALSE;
}

uint32_t canbus_transmit(uint8_t* data, uint32_t size)
{
    // Config receive mailbox
    //receive_mb_config();
	return send_data(data,size, instance );
}

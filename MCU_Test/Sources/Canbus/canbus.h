/**HEADER********************************************************************
*
*
***************************************************************************
*
*
**************************************************************************
*
* $FileName: canbus.h$
* $Version :
* $Date    :
*
* Comments:
*
* @brief The file contains Macro's and functions needed by the uut
*        application
*
*****************************************************************************/

#ifndef _CANBUS_H
#define _CANBUS_H

#include "fsl_flexcan_hal.h"

void canbus_init(
				uint32_t rxMailbxNum,
				uint32_t txMailbxNum,
				//uint32_t rxRemoteMailbxNum,
				//uint32_t txRemoteMailbxNum,
				//uint32_t rxRemoteId,
				//uint32_t txRemoteId,
				uint32_t rxId,
				uint32_t txId,
				uint32_t canInstance
			);

void canbus_deinit(uint32_t canInstance);
uint32_t canbus_transmit(uint8_t* data, uint32_t size);
bool canbus_recive(flexcan_msgbuff_t* rxMb,uint32_t* size, uint32_t timeout);


#endif //_CANBUS_H

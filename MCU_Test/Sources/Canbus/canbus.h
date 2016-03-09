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


void canbus_init(
				uint32_t rxMailbxNum,
				uint32_t txMailbxNum,
				uint32_t rxRemoteMailbxNum,
				uint32_t txRemoteMailbxNum,
				uint32_t rxRemoteId,
				uint32_t txRemoteId,
				uint32_t rxId,
				uint32_t txId
			);

#endif //_CANBUS_H

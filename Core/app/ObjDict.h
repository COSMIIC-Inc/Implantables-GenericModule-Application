
/*!
** @file   ObjDict.h
** @author CAN Festival -- NNP
** @date   OD File produced: 2011-03-06 21:48:07.869000
**
** @brief This file is generated by the NNP Tool -- Object Dictionary Editor, as
** originally developed by CAN Festival and modified by the NNP Team.
** Specific contents are detailed in Object Dictionary Template.
** Editor modification: 4/29/2010 -- rev 1.0.0
**
** This ObjDict.h file is generated by hand - jeu
**
*/

/*This object dictionary file should only be modified by the Object Dictionary Editor */

#ifndef OBJDICT_H
#define OBJDICT_H

#include "data.h"

#define APP_REV 0
#define RESTORE_COUNT 6

/* Prototypes of function provided by object dictionnary */
/* Master node data struct */
extern CO_Data ObjDict_Data;
UNS32 ObjDict_valueRangeTest (UNS8 typeValue, void * value);
const indextable * ObjDict_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);
extern UNS32 ObjDict_obj1018_Serial_Number;
extern UNS8 ObjDict_obj1018_Product_Code;
extern UNS32 ObjDict_obj1018_Revision_Number;

extern UNS8 clockRate;					/* Mapped at index 0x2000, subindex 0x00*/
extern UNS8 Control_ScriptStatus;		/* Mapped at index 0x2001, subindex 0x01 */
extern UNS8 Control_CurrentGroup;		/* Mapped at index 0x2001, subindex 0x02 */
extern UNS8 X_Network;					/* Mapped at index 0x2002, subindex 0x00*/
extern UNS16 Temperature;				/* Mapped at index 0x2003, subindex 0x00*/
extern UNS8 TemperatureIMU[2];
extern UNS8 Status_modeSelect;			/* Mapped at index 0x2010, subindex 0x01 */
extern UNS8 Status_numTPDO;				/* Mapped at index 0x2010, subindex 0x02 */
extern UNS8 Status_numRPDO;				/* Mapped at index 0x2010, subindex 0x03 */
extern UNS8 Status_channelSelect;		/* Mapped at index 0x2010, subindex 0x04 */
extern UNS8 Status_profileSelect;		/* Mapped at index 0x2010, subindex 0x05 */
extern UNS8 Status_profileWrite;		/* Mapped at index 0x2010, subindex 0x06 */
extern UNS8 Status_NodeId;
extern UNS16 Status_TestValue;
extern UNS16 Status_NodeTest;
extern UNS8 Accelerometers[4];			/* Mapped at index 0x2011, subindex 0x00*/
extern UNS8 AccelerometersFiltered[4];  /* Mapped at index 0x2011, subindex 0x01 */
extern INTEGER8 AccelerometersTilt[4];	/* Mapped at index 0x2011, subindex 0x02 */
extern UNS8 Gyroscopes[6];
extern UNS8 Quaternion[16];
extern UNS8 AccelerometerSettings;  	/* Mapped at index 0x2012, subindex 0x01 */
extern UNS32 AddressRequest;
extern UNS8 memorySelect;
extern UNS8 triggerReadMemory;
extern UNS8 writeByteMemory;
extern UNS8 statusByteMemory;
extern UNS8 ReadMemoryData[36];
extern UNS16 addressCounter;
extern UNS16 CAN_FormErrors;
extern UNS16 CAN_StuffErrors;
extern UNS16 CAN_BitErrors;
extern UNS16 CAN_OtherErrors;
extern UNS16 CAN_TotalErrors;
extern UNS16 CAN_Rx_ErrCounter;
extern UNS16 CAN_Tx_ErrCounter;
extern UNS16 CAN_Receive_BEI;
extern UNS16 CAN_Receive_Messages;
extern UNS16 CAN_Transmit_Messages;
extern UNS16 CAN_Interrupts_Off;
extern UNS16 RestoreList[RESTORE_COUNT];
extern UNS8 DiagnosticsEnabled;
extern UNS8 Diagnostic_VIN;
extern UNS8 Diagnostic_VIC;
extern UNS8 Diagnostic_VOS;
extern UNS8 Diagnostic_VDD;

#endif // OBJDICT_H
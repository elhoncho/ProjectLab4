#include <stdint.h>
#include <stdio.h>
#include "MyInc/AT86RF212B_HAL.h"
#include "MyInc/AT86RF212B.h"
#include "MyInc/terminal.h"
#include "MyInc/AT86RF212B_Regesters.h"
#include "MyInc/AT86RF212B_Constants.h"
#include "MyInc/generalHAL.h"
#include "MyInc/errors_and_logging.h"

#define MSKMODE_SHOW_INT
#define MSKMODE_DONT_SHOW_INT

//------------Private Function Prototypes----------------//
//TODO:These should be uncommented as they are static
//static void AT86RF212B_ID();
static void AT86RF212B_PowerOnReset();
//static void AT86RF212B_TRX_Reset();
static void 	AT86RF212B_StateMachineReset();
static void 	AT86RF212B_BitWrite(uint8_t reg, uint8_t mask, uint8_t pos, uint8_t value);
static void 	AT86RF212B_AES_Io (uint8_t mode, uint8_t cmd, uint8_t start, uint8_t *idata, uint8_t *odata);
static void 	AT86RF212B_AES_Read (uint8_t cmd, uint8_t *odata);
static void 	AT86RF212B_AES_Write (uint8_t cmd, uint8_t start, uint8_t *idata);
static void 	AT86RF212B_AES_Wrrd (uint8_t cmd, uint8_t start, uint8_t *idata, uint8_t *odata);
static uint8_t 	AT86RF212B_BitRead (uint8_t addr, uint8_t mask, uint8_t pos);
static uint8_t 	AT86RF212B_FrameLengthRead ();
static uint8_t 	AT86RF212B_FrameRead (uint8_t *frame);
static uint8_t 	AT86RF212B_FrameReadBlm (uint8_t *d);
static void 	AT86RF212B_FrameWrite (uint8_t length, uint8_t *frame);
static void 	AT86RF212B_IrqInit ();
static void 	AT86RF212B_SramRead (uint8_t addr, uint8_t length, uint8_t *data);
static void 	AT86RF212B_Sramrite (uint8_t addr, uint8_t length, uint8_t *data);
static void 	AT86RF212B_SetPhyMode();
static void 	AT86RF212B_PhySetChannel();
static void 	PhyCalibrateFTN();
static void 	PhyCalibratePll();

//-----------External Variables--------------------//
extern uint8_t logging;


//------------Private Global Variables----------------//
static AT86RF212B_Config config;
static volatile interupt = 0;

//==============================================================================================//
//                                       Public Functions                                       //
//==============================================================================================//

void AT86RF212B_Open(){
	AT86RF212B_OpenHAL(1000);

	//Time to wait after power on
	AT86RF212B_DelayHAL(AT86RF212B_tTR1, config);
	//Run power on reset sequence
	AT86RF212B_PowerOnReset();

	//------------Configure the AT86RF212B-------------//
	//Change this to set the RF mode
	config.phyMode = AT86RF212B_O_QPSK_100;
	//scrambler configuration for O-QPSK_{400,1000}; values { 0: disabled, 1: enabled (default)}.
	config.scramen = 1;
	//transmit signal pulse shaping for O-QPSK_{250,500,1000}; values {0 : half-sine filtering (default), 1 : RC-0.8 filtering}.
	config.rcen = 0;
	//Set the TX power level (Table 9-15) 0x03 = 0dBm
	config.txPower = 0x03;
	//Set the RX sensitivity RX threshold = RSSI_BAS_VAL + rxSensLvl * 3
	//rxSensLvl = 0 - 15, 0 = max sensitivity
	config.rxSensLvl = 0;
	//Enable TX CRC generation 1 = on 0 = off
	config.txCrc = 1;
	//Address Filtering
	config.panId_7_0 = 0x01;
	config.panId_15_8 = 0x00;
	config.shortAddr_7_0 = 0x01;
	config.shortAddr_15_8 = 0x00;
	config.extAddr_7_0 = 0x01;
	config.extAddr_15_8 = 0x00;
	config.extAddr_23_16 = 0x01;
	config.extAddr_31_24 = 0x00;
	config.extAddr_39_32 = 0x01;
	config.extAddr_47_40 = 0x00;
	config.extAddr_55_48 = 0x01;
	config.extAddr_63_56 = 0x00;

	AT86RF212B_SetPhyMode();

	AT86RF212B_PhySetChannel();
}

void AT86RF212B_ISR_Callback(){
	interupt = 1;
}

//-------------------Primitive Functions from AT86RF212 Programming Manual----------------------//

uint8_t AT86RF212B_RegRead(uint8_t reg){
	uint8_t pRxData[2] = {0, 0};
	uint8_t pTxData[2] = {0, 0};

	//Set the MSB and MSB-1 of the 8 bit register to a 1 0 for read access
	reg |= 1 << 7;
	reg &= ~(1 << 6);
	pTxData[0] = reg;
	AT86RF212B_RegReadAndWriteHAL(pTxData, pRxData, 2);
	//First byte is a configurable status and the 2nd byte is the register value
	return pRxData[1];
}

uint8_t AT86RF212B_RegWrite(uint8_t reg, uint8_t value){
	uint8_t pRxData[2] = {0, 0};
	uint8_t pTxData[2] = {0, 0};

	//Set the MSB and MSB-1 of the 8 bit register to a 1 1 for write access
	reg |= 1 << 7;
	reg |= 1 << 6;
	pTxData[0] = reg;
	pTxData[1] = value;
	AT86RF212B_RegReadAndWriteHAL(pTxData, pRxData, 2);

	return pRxData[1];
}



//==============================================================================================//
//                                     Private Functions                                        //
//==============================================================================================//

//-------------------Primitive Functions from AT86RF212 Programming Manual----------------------//

static void AT86RF212B_BitWrite(uint8_t reg, uint8_t mask, uint8_t pos, uint8_t value){
	uint8_t pRxData[2] = {0, 0};
	uint8_t pTxData[2] = {0, 0};

	uint8_t currentValue = AT86RF212B_RegRead(reg);

	//Set the MSB and MSB-1 of the 8 bit register to a 1 1 for write access
	reg |= 1 << 7;
	reg |= 1 << 6;
	pTxData[0] = reg;

	pTxData[1] = (currentValue & (~mask)) | (value << pos);
	AT86RF212B_RegReadAndWriteHAL(pTxData, pRxData, 2);

	return;
}

static void 	AT86RF212B_AES_Io (uint8_t mode, uint8_t cmd, uint8_t start, uint8_t *idata, uint8_t *odata){

}
static void 	AT86RF212B_AES_Read (uint8_t cmd, uint8_t *odata){

}
static void 	AT86RF212B_AES_Write (uint8_t cmd, uint8_t start, uint8_t *idata){

}
static void 	AT86RF212B_AES_Wrrd (uint8_t cmd, uint8_t start, uint8_t *idata, uint8_t *odata){

}
static uint8_t 	AT86RF212B_BitRead (uint8_t addr, uint8_t mask, uint8_t pos){

}
static uint8_t 	AT86RF212B_FrameLengthRead (){

}
static uint8_t 	AT86RF212B_FrameRead (uint8_t *frame){

}
static uint8_t 	AT86RF212B_FrameReadBlm (uint8_t *d){

}
static void 	AT86RF212B_FrameWrite (uint8_t length, uint8_t *frame){

}

static void 	AT86RF212B_IrqInit (){

	//Set IRQ Polarity to active high
	AT86RF212B_BitWrite(SR_IRQ_POLARITY, 0);
	//Enable Awake IRQ
	AT86RF212B_RegWrite(RG_IRQ_MASK, (TRX_IRQ_AWAKE_END | TRX_IRQ_PLL_LOCK));
	//Only show enabled interrupts in the IRQ register
	AT86RF212B_BitWrite(SR_IRQ_MASK_MODE, 0);

}
static void 	AT86RF212B_SramRead (uint8_t addr, uint8_t length, uint8_t *data){

}
static void 	AT86RF212B_Sramrite (uint8_t addr, uint8_t length, uint8_t *data){

}

//--------------------------Routines from AT86RF212 Programming Manual--------------------------//

static void AT86RF212B_PowerOnReset(){
	//The following programming sequence should be executed after power-on to
	//completely reset the radio transceiver. The MCU can not count on CLKM
	//before finalization of this sequence.

	/* AT86RF212::P_ON */
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_HIGH);
	Delayus(400);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_DelayHAL(AT86RF212B_t10, config);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_HIGH);
	//Turn off CLKM clock (available as a clock reference if needed)
	AT86RF212B_RegWrite(RG_TRX_CTRL_0, 0x18);

	//*************Enable interrupts****************//
	//      These don't look like they work for     //
	//      the P_ON -> TRX_OFF transition          //
	//      but work after this transition          //
	//**********************************************//
	AT86RF212B_IrqInit();

	//Change to TRX_OFF state
	AT86RF212B_RegWrite(RG_TRX_STATE, CMD_FORCE_TRX_OFF);

	AT86RF212B_DelayHAL(AT86RF212B_tTR13, config);

	if(logging){
		char tmpStr[32];
		sprintf(tmpStr, "IRQ Mask Reg: 0x%02X\r\n", AT86RF212B_RegRead(RG_IRQ_MASK));
		LOG(LOG_LVL_DEBUG, tmpStr);
	}

	/* AT86RF212::TRX_OFF */
	ASSERT(AT86RF212B_RegRead(RG_TRX_STATUS) & TRX_OFF);
}

//TODO: This should be static
void AT86RF212B_ID(){
	/* AT86RF212::P_ON */
	config.partid = AT86RF212B_RegRead(RG_PART_NUM);
	config.version = AT86RF212B_RegRead(RG_VERSION_NUM);
	config.manid0 = AT86RF212B_RegRead(RG_MAN_ID_0);
	config.manid1 = AT86RF212B_RegRead(RG_MAN_ID_1);

	if(logging){
		char tmpStr[32];
		sprintf(tmpStr, "Part ID: 0x%02X\r\n", config.partid);
		LOG(LOG_LVL_DEBUG, tmpStr);
		sprintf(tmpStr, "Version: 0x%02X\r\n", config.version);
		LOG(LOG_LVL_DEBUG, tmpStr);
		sprintf(tmpStr, "ManID0:  0x%02X\r\n", config.manid0);
		LOG(LOG_LVL_DEBUG, tmpStr);
		sprintf(tmpStr, "ManID1:  0x%02X\r\n", config.manid1);
		LOG(LOG_LVL_DEBUG, tmpStr);
	}
}

//TODO: Should be static
void AT86RF212B_TRX_Reset(){
	//This routine will bring the radio transceiver into a known state,
	//e.g. in case of a fatal error. The use case assumes, that the
	//radio transceiver is in one of the [ACTIVE] states (any state except P_ON and SLEEP)
	//and will do a reset, so that all registers get initialized
	//with their default values.

	/* AT86RF212::[ACTIVE] */
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_DelayHAL(AT86RF212B_t10, config);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_HIGH);
	AT86RF212B_IrqInit();
	AT86RF212B_DelayHAL(AT86RF212B_tTR13, config);
	/* AT86RF212::TRX_OFF */
	ASSERT(AT86RF212B_RegRead(RG_TRX_STATUS) & TRX_OFF);
}

static void PhyCalibrateFTN(){
	/* AT86RF212::[CONFIG] */
	AT86RF212B_BitWrite(SR_FTN_START, 1);
	AT86RF212B_DelayHAL(AT86RF212B_tTR16, config);
}

static void PhyCalibratePll(){
	/* AT86RF212::PLL_ON */
	AT86RF212B_BitWrite(SR_PLL_DCU_START, 1);
	AT86RF212B_BitWrite(SR_PLL_CF_START, 1);;
	AT86RF212B_DelayHAL(AT86RF212B_tTR21, config);
}

static void AT86RF212B_PhySetChannel(){
	/* AT86RF212::TRX_OFF */
	//CC_BAND (Table 9-34) 5 for 902.02MHz - 927.5 MHz
	AT86RF212B_BitWrite(SR_CC_BAND, 5);
	//(9.8.2) Fc[MHz] = 906[MHz] + 2[MHz] x (k – 1), for k = 1, 2, ..., 10
	AT86RF212B_BitWrite(SR_CHANNEL, 0);
}

static void AT86RF212B_StateMachineReset(){
	/* AT86RF212::[ACTIVE] */
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_BitWrite(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
	AT86RF212B_DelayHAL(AT86RF212B_tTR12, config);
	/* AT86RF212::TRX_OFF */
	ASSERT(AT86RF212B_RegRead(RG_TRX_STATUS) & TRX_OFF);
}

static void AT86RF212B_SetPhyMode(){
	/* AT86RF212::TRX_OFF */
	//TODO: Need to implement the exceptions for the gctxOffset for the OQPSK-RC-(100,200,400) cases (Table 9-15)
	switch(config.phyMode){
	case AT86RF212B_BPSK_20:
		config.useOQPSK = 0;
		config.submode = 0;
		config.OQPSK_Rate = 0;
		config.gctxOffset = 3;
		break;
	case AT86RF212B_BPSK_40:
		config.useOQPSK = 0;
		config.submode = 1;
		config.OQPSK_Rate = 0;
		config.gctxOffset = 3;
		break;
	case AT86RF212B_O_QPSK_100:
		config.useOQPSK = 1;
		config.submode = 0;
		config.OQPSK_Rate = 0;
		config.gctxOffset = 2;
		break;
	case AT86RF212B_O_QPSK_200:
		config.useOQPSK = 1;
		config.submode = 0;
		config.OQPSK_Rate = 1;
		config.gctxOffset = 2;
		break;
	case AT86RF212B_O_QPSK_250:
		config.useOQPSK = 1;
		config.submode = 0;
		config.OQPSK_Rate = 2;
		config.gctxOffset = 2;
		break;
	case AT86RF212B_O_QPSK_400:
		config.useOQPSK = 1;
		config.submode = 1;
		config.OQPSK_Rate = 0;
		config.gctxOffset = 2;
		break;
	case AT86RF212B_O_QPSK_500:
		config.useOQPSK = 1;
		config.submode = 1;
		config.OQPSK_Rate = 1;
		config.gctxOffset = 2;
		break;
	case AT86RF212B_O_QPSK_1000:
		config.useOQPSK = 1;
		config.submode = 1;
		config.OQPSK_Rate = 2;
		config.gctxOffset = 2;
		break;
	default:
		ASSERT(0);
		if(logging){
			LOG(LOG_LVL_ERROR, "Unknown Phy Configuration\r\n");
		}
		return;
	}

	AT86RF212B_BitWrite(SR_BPSK_OQPSK, config.useOQPSK);
	AT86RF212B_BitWrite(SR_SUB_MODE, config.submode);
	AT86RF212B_BitWrite(SR_OQPSK_DATA_RATE, config.OQPSK_Rate);
	AT86RF212B_BitWrite(SR_OQPSK_SCRAM_EN, config.scramen);
	AT86RF212B_BitWrite(SR_OQPSK_SUB1_RC_EN, config.rcen);
	AT86RF212B_BitWrite(SR_GC_TX_OFFS, config.gctxOffset);
	AT86RF212B_RegWrite(RG_PHY_TX_PWR, config.txPower);
	AT86RF212B_BitWrite(SR_RX_PDT_LEVEL, config.rxSensLvl);
	AT86RF212B_BitWrite(SR_TX_AUTO_CRC_ON, config.txCrc);
	AT86RF212B_RegWrite(RG_PAN_ID_0, config.panId_7_0);
	AT86RF212B_RegWrite(RG_PAN_ID_1, config.panId_15_8);
	AT86RF212B_RegWrite(RG_SHORT_ADDR_0, config.shortAddr_7_0);
	AT86RF212B_RegWrite(RG_SHORT_ADDR_1, config.shortAddr_15_8);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_0, config.extAddr_7_0);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_1, config.extAddr_15_8);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_2, config.extAddr_23_16);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_3, config.extAddr_31_24);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_4, config.extAddr_39_32);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_5, config.extAddr_47_40);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_6, config.extAddr_55_48);
	AT86RF212B_RegWrite(RG_IEEE_ADDR_7, config.extAddr_63_56);
	return;
}

//---------------------------------Custom Functions---------------------------------//

//TODO: Remove this it is just for testing
void AT86RF212B_TestSleep(){
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_HIGH);
	Delayms(1);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);

	AT86RF212B_WaitForIRQ(TRX_IRQ_AWAKE_END);
}

void AT86RF212B_WaitForIRQ(uint8_t expectedIRQ){
	//Max time in ms to wait for an IRQ before timing out
	//TODO: Change this delay, it is set to 1 sec so the USB can connect and display logging in time
	uint32_t maxTime = 1000;

	//TODO: What happens if the timer rolls
	uint32_t timeout = AT86RF212B_SysTickMsHAL()+maxTime;
	while(!interupt){
		if(AT86RF212B_SysTickMsHAL() > timeout){
			ASSERT(0);
			if(logging){
				LOG(LOG_LVL_ERROR, "Timeout while waiting for IRQ\r\n");
			}
			return;
		}
	}
	//Clear the interrupt flag
	interupt = 0;

	uint8_t irqState = AT86RF212B_RegRead(RG_IRQ_STATUS);

	if(!(irqState & expectedIRQ)){
		ASSERT(0);
		if(logging){
			LOG(LOG_LVL_ERROR, "Something very strange happened\r\n");
		}
	}
	else if(logging){
		LOG(LOG_LVL_DEBUG, "Expected IRQ Received!\r\n");
	}
}



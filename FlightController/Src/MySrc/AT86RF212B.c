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
//static void AT86RF212B_PowerOnReset();
//static void AT86RF212B_TRX_Reset();
static void AT86RF212B_StateMachineReset();
static void AT86RF212B_BitWrite(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t value);

//-----------External Variables--------------------//
extern uint8_t logging;


//------------Private Global Variables----------------//
static AT86RF212B_Config config;

//------------Public Functions----------------//

void AT86RF212B_Open(){
	AT86RF212B_OpenHAL(1000);

	//TODO: Turn this back on
	//AT86RF212B_PowerOnReset();
}

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

static void AT86RF212B_BitWrite(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t value){
	uint8_t pRxData[2] = {0, 0};
	uint8_t pTxData[2] = {0, 0};

	uint8_t currentValue = AT86RF212B_RegRead(reg);

	//Set the MSB and MSB-1 of the 8 bit register to a 1 1 for write access
	reg |= 1 << 7;
	reg |= 1 << 6;
	pTxData[0] = reg;

	pTxData[1] = (currentValue & (~mask)) | (value << shift);
	AT86RF212B_RegReadAndWriteHAL(pTxData, pRxData, 2);

	return;
}

//------------Private Functions----------------//

//TODO: Change this back to static
void AT86RF212B_PowerOnReset(){
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
	//      These dont look like they work for      //
	//      the P_ON -> TRX_OFF transition          //
	//      but work after this transition          //
	//**********************************************//
	//Set IRQ Polarity to active high
	AT86RF212B_BitWrite(SR_IRQ_POLARITY, 0);
	//Enable Awake IRQ
	AT86RF212B_RegWrite(RG_IRQ_MASK, TRX_IRQ_AWAKE_END);
	//Only show enabled interrupts in the IRQ register
	AT86RF212B_BitWrite(SR_IRQ_MASK_MODE, 0);

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
	AT86RF212B_RegWrite(RG_IRQ_MASK, TRX_IRQ_AWAKE_END);
	AT86RF212B_DelayHAL(AT86RF212B_tTR13, config);
	/* AT86RF212::TRX_OFF */
	ASSERT(AT86RF212B_RegRead(RG_TRX_STATUS) & TRX_OFF);
}

static void AT86RF212B_StateMachineReset(){
	/* AT86RF212::[ACTIVE] */
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_BitWrite(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
	AT86RF212B_DelayHAL(AT86RF212B_tTR12, config);
	/* AT86RF212::TRX_OFF */
	ASSERT(AT86RF212B_RegRead(RG_TRX_STATUS) & TRX_OFF);
}

//TODO: Remove this it is just for testing
void AT86RF212B_TestSleep(){
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_HIGH);
	Delayms(1);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);

	while(!AT86RF212B_ReadPinHAL(AT86RF212B_PIN_IRQ));

	uint8_t irqState = AT86RF212B_RegRead(RG_IRQ_STATUS);

	if(!(irqState & TRX_IRQ_AWAKE_END)){
		ASSERT(0);
		if(logging){
			LOG(LOG_LVL_ERROR, "Something very strange happened\r\n");
		}
	}
	else if(logging){
		LOG(LOG_LVL_DEBUG, "AT86RF212B Successfully Woke Up!\r\n");
	}
}

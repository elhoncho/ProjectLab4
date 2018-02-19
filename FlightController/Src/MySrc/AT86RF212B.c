#include <stdint.h>
#include <stdio.h>
#include "MyInc/AT86RF212B_HAL.h"
#include "MyInc/AT86RF212B.h"
#include "MyInc/terminal.h"
#include "MyInc/AT86RF212B_Regesters.h"
#include "MyInc/AT86RF212B_Constants.h"
#include "MyInc/generalHAL.h"
#include "MyInc/errors_and_logging.h"

//------------Private Function Prototypes----------------//
static void AT86RF212B_ID(AT86RF212B_Config cfg);
static void AT86RF212B_PowerOnReset();
static void AT86RF212B_TRX_Reset();

//------------Private Global Variables----------------//
static AT86RF212B_Config config;

//------------Public Functions----------------//

void AT86RF212B_Open(){
	AT86RF212B_OpenHAL(1000);

	//Power on Reset
	AT86RF212B_PowerOnReset();

	//Get ID information
	AT86RF212B_ID(config);
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

//------------Private Functions----------------//

static void AT86RF212B_PowerOnReset(){
	//The following programming sequence should be executed after power-on to
	//completely reset the radio transceiver. The MCU can not count on CLKM
	//before finalization of this sequence.

	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_HIGH);
	Delayus(400);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_DelayHAL(AT86RF212B_t10);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_HIGH);
	//Turn off CLKM clock (available as a clock reference if needed)
	AT86RF212B_RegWrite(RG_TRX_CTRL_0, 0x18);
	//Change to TRX_OFF state
	AT86RF212B_RegWrite(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
	AT86RF212B_DelayHAL(AT86RF212B_tTR13);
	//Make sure the TRX_STATE == TRX_OFF
	ASSERT(AT86RF212B_RegRead(RG_TRX_STATUS) & TRX_OFF);
}

static void AT86RF212B_ID(AT86RF212B_Config cfg){
	cfg.partid = AT86RF212B_RegRead(RG_PART_NUM);
	cfg.version = AT86RF212B_RegRead(RG_VERSION_NUM);
	cfg.manid0 = AT86RF212B_RegRead(RG_MAN_ID_0);
	cfg.manid1 = AT86RF212B_RegRead(RG_MAN_ID_1);
}

static void AT86RF212B_TRX_Reset(){
	//This routine will bring the radio transceiver into a known state,
	//e.g. in case of a fatal error. The use case assumes, that the
	//radio transceiver is in one of the [ACTIVE] states (any state except P_ON and SLEEP)
	//and will do a reset, so that all registers get initialized
	//with their default values.

	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_SLP_TR, AT86RF212B_PIN_STATE_LOW);
	AT86RF212B_DelayHAL(AT86RF212B_t10);
	AT86RF212B_WritePinHAL(AT86RF212B_PIN_RST, AT86RF212B_PIN_STATE_HIGH);
	AT86RF212B_RegWrite(RG_IRQ_MASK, TRX_IRQ_AWAKE_END);
	AT86RF212B_DelayHAL(AT86RF212B_tTR13);
	//Make sure the TRX_STATE == TRX_OFF
	ASSERT(AT86RF212B_RegRead(RG_TRX_STATUS) & TRX_OFF);
}

//

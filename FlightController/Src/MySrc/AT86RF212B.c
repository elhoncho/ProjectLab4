#include <stdint.h>
#include <stdio.h>
#include "MyInc/AT86RF212B_HAL.h"
#include "MyInc/AT86RF212B.h"
#include "MyInc/terminal.h"
#include "MyInc/AT86RF212B_Regesters.h"
#include "MyInc/AT86RF212B_Constants.h"
#include "MyInc/generalHAL.h"
//t10 should be 625ns but 1us is good enough
#define T10 1

void AT86RF212B_Open(){
	AT86RF212B_OpenHAL(1000);

	//Reset the AT86RF212B on startup
	//TODO: These timings can be optimized by referencing the manual
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

	//TODO: Implement error if the state is not TRX_OFF
	///* AT86RF212::TRX_OFF */
    //state = trx_bit_read(SR_TRX_STATUS);
    //ASSERT(state==TRX_OFF);
}


void AT86RF212B_RegRead(uint8_t reg){
	uint8_t pRxData[2] = {0, 0};
	uint8_t pTxData[2] = {0, 0};
	char text[20];

	//Set the MSB and MSB-1 of the 8 bit register to a 1 0 for read access
	reg |= 1 << 7;
	reg &= ~(1 << 6);
	pTxData[0] = reg;
	AT86RF212B_RegReadAndWriteHAL(pTxData, pRxData, 2);
	sprintf(text, "0x%02X 0x%02X\r\n", pRxData[0], pRxData[1]);
	terminalWrite(text);
}

void AT86RF212B_RegWrite(uint8_t reg, uint8_t value){
	uint8_t pRxData[2] = {0, 0};
	uint8_t pTxData[2] = {0, 0};
	char text[20];

	//Set the MSB and MSB-1 of the 8 bit register to a 1 1 for write access
	reg |= 1 << 7;
	reg |= 1 << 6;
	pTxData[0] = reg;
	pTxData[1] = value;
	AT86RF212B_RegReadAndWriteHAL(pTxData, pRxData, 2);
	sprintf(text, "0x%02X 0x%02X\r\n", pRxData[0], pRxData[1]);
	terminalWrite(text);
}

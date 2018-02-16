#include <stdint.h>
#include <stdio.h>
#include "MyInc\AT86RF212B_HAL.h"
#include "MyInc/terminal.h"

void AT86RF212B_Open(){
	AT86RF212B_OpenHAL(1000);
}


void AT86RF212B_RegRead(uint8_t reg){
	uint8_t pRxData[2] = {0, 0};
	uint8_t pTxData[2] = {0, 0};
	char text[20];

	//Set the MSB and MSB-1 of the 8 bit register to a 1 0 for read access
	reg |= 1 << 7;
	reg &= ~(1 << 6);
	pTxData[0] = reg;
	AT86RF212B_ReadAndWriteHAL(pTxData, pRxData, 2);
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
	AT86RF212B_ReadAndWriteHAL(pTxData, pRxData, 2);
	sprintf(text, "0x%02X 0x%02X\r\n", pRxData[0], pRxData[1]);
	terminalWrite(text);
}

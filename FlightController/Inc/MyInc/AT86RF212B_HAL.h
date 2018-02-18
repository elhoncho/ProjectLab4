/*
 * AT86RF212B_HAL.h
 *
 *  Created on: Feb 15, 2018
 *      Author: owner
 */

#ifndef MYINC_AT86RF212B_HAL_H_
#define MYINC_AT86RF212B_HAL_H_

void AT86RF212B_OpenHAL(uint32_t time_out);
void AT86RF212B_CloseHAL();
void AT86RF212B_RegReadAndWriteHAL(uint8_t * pTxData, uint8_t * pRxValue, uint16_t size);
void AT86RF212B_WritePinHAL(uint8_t pin, uint8_t state);
void AT86RF212B_DelayHAL(uint8_t time);

#endif /* MYINC_AT86RF212B_HAL_H_ */

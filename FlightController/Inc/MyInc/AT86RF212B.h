/*
 * AT86RF212B.h
 *
 *  Created on: Feb 15, 2018
 *      Author: owner
 */

#ifndef MYINC_AT86RF212B_H_
#define MYINC_AT86RF212B_H_

void AT86RF212B_Open();
void AT86RF212B_RegRead(uint8_t reg);
void AT86RF212B_RegWrite(uint8_t reg, uint8_t value);

#endif /* MYINC_AT86RF212B_H_ */

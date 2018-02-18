/*
 * generalHAL.c
 *
 *  Created on: Feb 17, 2018
 *      Author: owner
 */

#include "stm32f4xx_hal.h"

void Delayms(uint32_t timeMs){
	HAL_Delay(timeMs);
}

void Delayus(uint32_t timeUs){
	//Clear the counter
	DWT->CYCCNT = 0;
	uint32_t stopTime = timeUs*(HAL_RCC_GetHCLKFreq()/1000000);
	//Start the counter
	DWT->CTRL |= 1;
	while(DWT->CYCCNT < stopTime);
	//Stop the counter
	DWT->CTRL |= 0;
	return;
}

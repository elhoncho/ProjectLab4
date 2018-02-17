/*
 * generalHAL.c
 *
 *  Created on: Feb 17, 2018
 *      Author: owner
 */

#include "stm32f4xx_hal.h"

void generalHAL_Open(){
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delayms(uint32_t timeMs){
	HAL_Delay(timeMs);
}

void delayus(uint32_t timeUs){
	uint32_t stopTime = DWT->CYCCNT+timeUs*(HAL_RCC_GetHCLKFreq()/1000000);
	while(DWT->CYCCNT < stopTime);
	return;
}

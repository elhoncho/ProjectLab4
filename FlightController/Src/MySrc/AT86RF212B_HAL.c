/*
 * AT86RF212B_HAL.c
 *
 *  Created on: Feb 15, 2018
 *      Author: owner
 */

#include "stm32f4xx_hal.h"

#define SPI_NSS_PORT GPIOG
#define SPI_NSS_PIN GPIO_PIN_15

#define CLKM_PORT GPIOG
#define CLKM_PIN GPIO_PIN_14

#define IRQ_PORT GPIOG
#define IRQ_PIN GPIO_PIN_13

#define SLP_TR_PORT GPIOG
#define SLP_TR_PIN GPIO_PIN_12

#define RST_PORT GPIOG
#define RST_PIN GPIO_PIN_11

#define DIG2_PORT GPIOG
#define DIG2_PIN GPIO_PIN_10

uint32_t timeout = 1000;
extern SPI_HandleTypeDef hspi3;
SPI_HandleTypeDef hspi;

//TODO: Change the returns from void to an indicator, that means the functions need to validate a successful operation or not
void AT86RF212B_OpenHAL(uint32_t time_out){
	hspi = hspi3;
	timeout = time_out;
	HAL_GPIO_WritePin(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SLP_TR_PORT, SLP_TR_PIN, GPIO_PIN_RESET);

	//Reset the AT86RF212B on startup
	//TODO: These timings can be optimized by referencing the manual
	HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);
	HAL_Delay(1);
}

void AT86RF212B_CloseHAL(){

}


//Register = register to read
//pTxData = pointer to the data to send
//pRxValue = pointer to rx data array
//size = amount of data to be sent and received
void AT86RF212B_ReadHAL(){

}

void AT86RF212B_WriteHAL(){

}

void AT86RF212B_ReadAndWriteHAL(uint8_t * pTxData, uint8_t * pRxData, uint16_t size){
	HAL_GPIO_WritePin(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi , pTxData, pRxData, size, timeout);
	//TODO: This probably needs to be changed, could lock up here.
	while(hspi.State == HAL_SPI_STATE_BUSY);
	HAL_GPIO_WritePin(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_PIN_SET);
}

/*
 * ADCDMADriver_STM32L0x3.h		v.1.0.
 *
 *  Created on: Oct 26, 2023
 *      Author: Balazs Farkas
 */

#ifndef INC_ADCDMADRIVER_STM32L0X3_H_
#define INC_ADCDMADRIVER_STM32L0X3_H_

#include "stdint.h"
#include "stm32l053xx.h"								//device specific header file for registers

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE

//FUNCTION PROTOTYPES
void ADCDMAInit(void);
void ADCDMATempRead(DMA_Channel_TypeDef* dma_adc_channel_selector);

#endif /* INC_ADCDMADRIVER_STM32L0X3_H_ */

/*
 *  Created on: Oct 26, 2023
 *  Author: BalazsFarkas
 *  Project: STM32_DMADriver
 *  Processor: STM32L053R8
 *  HEader version: 1.0
 *  File: ADCwDMA_STM32L0x3.h
 *  Modified from: STM32_ADC-DACDriver/ADCDriver_STM32L0x3.h
 */

#ifndef INC_ADCWDMA_STM32L0X3_H_
#define INC_ADCWDMA_STM32L0X3_H_

#include "stdint.h"
#include "stm32l053xx.h"								//device specific header file for registers

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE

//FUNCTION PROTOTYPES
void ADCDMAInit(void);
void ADCDMATempRead(DMA_Channel_TypeDef* dma_adc_channel_selector);

#endif /* INC_ADCWDMA_STM32L0X3_H_ */

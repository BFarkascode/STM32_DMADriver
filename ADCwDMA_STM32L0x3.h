/*
 *  Created on: Oct 26, 2023
 *  Project: STM32_DMADriver
 *  File: ADCwDMA_STM32L0x3.h
 *  Author: BalazsFarkas
 *  Processor: STM32L053R8
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  Program description: N/A
 *  Hardware description/pin distribution: N/A
 *  Modified from: N/A
 *  Change history: N/A
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

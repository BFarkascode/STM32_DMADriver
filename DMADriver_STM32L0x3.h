/*
 *
 *  Created on: Oct 26, 2023
 *  Project: STM32_DMADriver
 *  File: DMADriver_STM32L0x3.h
 *  Author: BalazsFarkas
 *  Processor: STM32L053R8
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  Program description: N/A
 *  Hardware description/pin distribution: N/A
 *  Modified from: N/A
 *  Change history: N/A
 */

#ifndef INC_DMADRIVER_STM32L0X3_H_
#define INC_DMADRIVER_STM32L0X3_H_

#include "stdint.h"
#include "stm32l053xx.h"

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE
extern uint32_t TC_executed;

//FUNCTION PROTOTYPES
void DMAADCInit(void);
void DMAADCConfig(uint32_t mem_addr, uint16_t transfer_width);
void DMAADCIRQPriorEnable(void);

#endif /* INC_ADCDRIVER_CUSTOM_H_ */

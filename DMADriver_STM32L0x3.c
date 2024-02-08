/*
 *  Created on: Oct 26, 2023
 *  Author: BalazsFarkas
 *  Project: STM32_DMADriver
 *  Processor: STM32L053R8
 *  Program version: 1.0
 *  File: DMADriver_STM32L0x3.c
 *  Change history:
 *
 * v.1.0
 * Below is a custom DMA driver.
 * It initialises the seven DMA channels according to page 266 of the refman (Table 51).
 * Currently runs only the ADC on Channel1.
 *
 * Note: even though Channel2 also should be able to run the ADC, it doesn't seem to be too reliable at doing so. Certain DMA channels might be better for use than others.
 *
 *
 */

#include "DMADriver_STM32L0x3.h"
#include "stm32l053xx.h"														//device specific header file for registers

//1)We set up the basic driver for the DMA
void DMAADCInit(void){
	/* Initialize DMA
	 *
	 * What happens here? Well not much, we simly:
	 *
	 * 1)Enable clocking
	 * 2)Remap channels to be used
	 *
	 * Remapping is done by writing a specific sequence of bits to the CSELR register in the DMA. For which sequence defined what, check Table 51.
	 *
	 * Note: it is possible to use channel2 for ADC, albeit it doesn't seem to reliably work for some unknown reason. Selection of channels for DMA might not be completely flexible.
	 *
	 * */

	//1)
	RCC->AHBENR |= (1<<0);														//enable DMA clocking
																				//clocking is directly through the AHB

	//2)
	DMA1_CSELR->CSELR &= ~(15<<0);												//DMA1_Channel1: will be requested at 4'b0000 for ADC
}


//2)We set up a channel for the ADC
void DMAADCConfig(uint32_t mem_addr, uint16_t transfer_width){
	/* Configure the 1st DMA channel as ADC
	 *
	 * 1)Ensure that selected adc dma channel is disabled
	 * 2)Configure channel parameters: transfer length, transfer direction, address increment, transfer type (peri-to-mem)
	 * 3)Provide peri and mem addresses
	 * 4)Provide transfer width
	 *
	 * Note: the DMA is agnostic to whatever is on source register and will just keep on reading out the register whenever a request is received from the peripheral.
	 * 			As such, the DMA is fully controlled by the peripheral - here, the ADC.
	 * 			Once a DMA channel is enabled, it runs completely separate from the main loop.
	 * 			DMA channel priority is set by the DMA channel config (see below). Data flow priority (what comes voer first) is governed by the peripheral though.
	 * */
	//1)
	DMA1_Channel1->CCR &= ~(1<<0);												//we disable the DMA channel

	//2)
	DMA1_Channel1->CCR |= (1<<1);												//we enable the IRQ on the transfer complete
	DMA1_Channel3->CCR |= (1<<3);												//we enable the error interrupt within the DMA channel
	DMA1_Channel1->CCR &= ~(1<<4);												//we read form the peripheral
	DMA1_Channel1->CCR |= (1<<5);												//circular mode is on
																				//peripheral increment is not used
	DMA1_Channel1->CCR |= (1<<7);												//memory increment is is used
	DMA1_Channel1->CCR |= (1<<8);												//peri side data length is 16 bits
	DMA1_Channel1->CCR |= (1<<10);												//mem side data length is 16 bits
																				//priority level is kept at LOW
																				//mem-to-mem is not used
	//3)
	DMA1_Channel1->CPAR = (uint32_t) (&(ADC1->DR));								//we want the data to be extracted from the ADC's DR register

	DMA1_Channel1->CMAR = mem_addr;												//this is the address (!) of the memory buffer we want to funnel data into

	//4)
	DMA1_Channel1->CNDTR |= (transfer_width<<0);								//we want to have an element burst of "transfer_width"
																				//Note: circular mode will automatically reset this register
}

//3)We define what should happen upon DMA IRQ
void DMA1_Channel1_IRQHandler (void){
	/*
	 * Unlike other DMA IRQs, the DMA1 channel 1 IRQ can only be activated by channel 1.
	 * The IRQ is activated on DMA full transmission and error.
	 *
	 * 1)We check, what activated the IRQ.
	 * 2)We pull the appropriate flag according to the activation.
	 * 3)We reset the IRQ.
	 *
	 * Note: we want an indifferent FLASH loader, not one that is not controlled differently depending on if we are at the halfway or end point.
	 *
	 * */

	//1)
	if ((DMA1->ISR & (1<<1)) == (1<<1)) {										//if we had the full transmission triggered on channel 1

		TC_executed++;															//we count the transfer complete actions

	} else if ((DMA1->ISR & (1<<3)) == (1<<3)){									//if we had an error trigger on channel 1

		printf("DMA transmission error!");
		while(1);

	} else {
		//do nothing
	}

	DMA1->IFCR |= (1<<0);														//we remove all the interrupt flags from channel 1

}

//4)DMA IRQ priority
void DMAADCIRQPriorEnable(void) {
	/*
	 * We call the two special CMSIS functions to set up/enable the IRQ.
	 *
	 * */
	NVIC_SetPriority(DMA1_Channel1_IRQn, 1);									//IRQ priority for channel 1
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);											//IRQ enable for channel 1
}

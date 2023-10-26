/*
 * ADCDMADriver_STM32L0x3.c			v.1.0
 *
 *  Created on: Oct 26, 2023
 *      Author: Balazs Farkas
 *
 * v.1.0
 * Below is a custom ADC driver with the DMA activated.
 * For explanation on how to set up the ADC, consult the STM32_ADC_DACDriver project.
 *
 */

#include "ADCDMADriver_STM32L0x3.h"
#include "stm32l053xx.h"								//device specific header file for registers

//4)We set up the DMA driver for the ADC
void ADCDMAInit(void){
	/*
	 * Difference with regular ADC init is that we enable the DMA, we enable circular mdoe for the DMA and set up continous conversion for the ADC. Rest is the same.
	 * Note: this function is agnostic to which DMA channel we are using. That selection is done on the DMA's side.
	 *
	 * */
	//1)Clocking
	RCC->APB2ENR |= (1<<9);										//enable the ADC clocking


	//2)Setup
	if (ADC1->CR & (1<<2) !=0) {								//if the ADC is measuring
		ADC1->CR |= (1<<4);										//we stop it
		while((ADC1->CR & (1<<4)) == 1);						//we wait until the ADC is stopped and the ADSTP bit goes LOW
	} else {
		//do nothing
	}

	if (ADC1->CR & (1<<0) !=0) {								//if the ADC is running already
		ADC1->CR |= (1<<1);										//we shut off the ADC
		while((ADC1->CR & (1<<1)) == 1);						//we wait until the ADC is shut off and the ADDIS bit goes LOW
	} else {
		//do nothing
	}

	ADC1->CFGR1 = 0x0;											//AUTOFF, DMA, EXTEN off (if it was on) so it won't restart
	ADC1->CFGR2 = 0x0;											//oversampling, clocking reset
	ADC1->CR = 0x0;												//we wipe the control register of the ADC

	ADC1->CR |= (1<<31);										//we start calibration
	while((ADC1->ISR & (1<<11)) == 0);							//we wait until the EOCAL flag goes HIGH and thus ADC becomes calibrated
	ADC1->ISR |= (1<<11);										//we remove the EOCAL flag by writing a 1 to it

	//3)Configure
	ADC1->CFGR1 |= (1<<0);										//DMA enabled
	ADC1->CFGR1 |= (1<<1);										//DMA circular mode for the ADC

	ADC1->CFGR1 |= (1<<15);										//AUTOOFF on
																//Note: the AUTOOFF removes the ADRDY control flag. Turning off becomes automatic.

	ADC1->CFGR1 |= (1<<14);										//WAIT on
	ADC1->CFGR1 |= (1<<13);										//CONT on
																//RES kept at 12 bit, no external triggers, no DISCEN, SCANDIR is upward

	ADC1->CFGR2 = 0x0;											//clock source is direct HSI16 without AHB or APB prescalers
																//by picking the CKMODE as 2'b0, we pick the HSI16 as the clock for the ADC. HSI16 is enabled in the clock config.

	ADC1->SMPR = 0x0;											//we put 1.5 ADC cycles for sample time (must be adjusted to whatever the sample rate is for the device we measure)
																//for the internal temperature sensor, this needs to be (7<<1) since we need 10 us sampling time
	ADC->CCR = 0x0;												//we don't enable any in-built peripherals
																//we use the ADC with higher frequency than 3.5 MHz
																//also, no prescaler! We run the ADC at full speed.
}


//5)External analog reading from a single channel - using DMA
void ADCDMATempRead(DMA_Channel_TypeDef* dma_adc_channel_selector) {
	/*
	 * Below we are reading a single channel ADC value using DMA.
	 * Please note that the readout is done by the DMA after a request is sent by the ADC after each conversion.
	 * Thus, no need to read out the DR register and remove the EOC flag. DMA does that for us.
	 * Also, the DMA channel needs to be activated here.
	 * Don't forget to activate the temperature sensor as well!
	 *
	 * */

	ADC1->CR |= 0x0;										//we clean the ADC control register
															//Note: we need to use ADC1, not ADC, to comply to the general naming of the register (despite only having one ADC).

	ADC1->CHSELR |= (1<<18);								//we enable channel 18, which is the internal temperature sensor
															//Note: we select only one channel!
	ADC1->SMPR |= (7<<0);									//we keep as slow sampling time as possible
	ADC->CCR |= (1<<23);									//we enable the internal temperature sensor
	dma_adc_channel_selector->CCR |= (1<<0);				//we enable the DMA channel
	ADC1->CR |= (1<<2);										//we start conversion
															//Note: we don't need to manage the EOP flag like we did in the STM32_ADC-DACDriver project
}

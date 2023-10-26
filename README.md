# STM32_DMADriver

## General description
Originally I wasn’t planning to do a separate project on direct memory access (DMA) since there isn’t a lot of complicated coding into it, not to mention, the explanations on the internet over how DMA works are far and wide. But then I realised that the concept of DMA itself is useful enough to merit a small project, even if it won’t be unravelling any great mysteries. It will also allow me to talk a bit again about interrupts (IRQs) which is also a topic that is pretty useful to get a hang of but has little in themselves by themselves to be interesting.

In general, DMAs and interrupts (IRQs) are very similar in how they behave since neither of them are, strictly speaking, but of the main programming loop we, well, program. They are both controlled by hardware instead of software and thus either take priority in execution compared to other code segments (as it is with IRQ) or can run IN PARALLEL to the main loop and do their thing.

So then, why use IRQs? Wouldn’t it be just as easy to simply wait for the code to find out that something important has changed and then react to it? Well, no, not always. For simple projects where timing is not important, one can of course wait until the program loop finds out that a variable has changed or that a GPIO input has been pulled high, but that won’t be good enough for an efficient execution. After all, it takes some time to the loop to actually check the variable or the GPIO input values (called “polling”), meaning that we might figure out that a value is different than before milliseconds (or more) after it has changed. This is not good enough if we need an immediate reaction to a triggering signal. The other reason to use IRQs is simply practical: some processes – like NVM - and peripherals – like DMA - run on them and use them to provide feedback to the main loop over their functions and state. Without IRQs, they don’t have a way to signal to the main loop (DMA, for instance, is literally parallel running to the main loop so it can’t just “wait” for the main to be convenient. If it is not dealt with, it shuts off and won’t run again unless specifically told to.)

And why use DMA? Because it allows parallel actions. You see, some rather tedious processes such as communication protocol drivers either have to wait and sit still until a communication comes in – effectively blocking all other actions – or they risk losing data. Of course one way to get around that problem is have an IRQ triggered whenever the event is coming in…or to use DMA which will just capture the incoming data into a memory buffer for the mcu, meaning that the main loop would only need to check the buffer whenever it has the time, the entire data package will be there. This latter is particularly useful when we have a lot of data coming into the mcu that we would need to process in a bulk – by, for instance, doing software filtering. The DMA will prepare for us a data array we can have out main loop process. Or we can just build something called a ping-pong buffer: a buffer that is loaded by the DMA while at the same time, being processed by the main loop.

In order to have a nice project to show the capabilities of the DMA, we will be attaching a DMA to the ADC that provides the internal temperature value for the mcu and then show that even at a time when we aren’t telling the main loop to read out values from the ADC, the DMA will do the job for us. As such, we will rely on the STM32_ADC-DACDriver project as the source with the ADC driver modified to run DMA.

### To read
We will be using the ADC defined in the STM32_ADC-DACDriver project as the input source for the DMA. As such, familiarity with that project is necessary. We should also check, how the ADC is connected to the DMA:
-	14.5.5 Managing converted data using the DMA: describes how to set up the DMA on the ADC’s side.

Additionally, for DMA must reads are:
-	11.3.2 DMA Request Mapping: Table 51 in this section is what tells us, how to configure the DMA channels to connect them to particular peripherals. As the table suggests, only specific channels can be attached to specific peripherals.
-	11.4.2 DMA transfers: describes how the DMA transfer works
-	11.4.3 DMA arbitration: channel priority assignment
-	11.4.4 DMA channels: channel configuration
-	11.4.6 DMA error management: when and how the DMA can stop
-	11.5 DMA interrupts
-	11.6 DMA registers

Lastly, for the IRQs, there isn’t really a good part in the refman to explain them specifically. The “EXTI” and the “NVIC” sections can give a hint on how they work, but information is somewhat sketchy. I suggest playing with a button IRQ – see the NVMDriver project on how to use the blue button on the nucleo for this purpose – and figure it out by doing.

## Particularities
### With the peripheral (in this project, the ADC)

Since DMA is separate from the main loop, it is not controlled by it. Instead, it is the peripherals – I2C, SPI, UART, TIM, ADC, DAC – that drive the DMA directly using “requests” sent to the DMA. These requests must be enabled on the peripheral’s side, otherwise it won’t be sending them out.

Once the requests are enabled – and the ADC is calibrated like it normally is – everything starts to run automatically. Literally: once you tell the ADC to send the requests (and the DMA is properly set), the data will start to flow, even if we haven’t reached the main loop yet with our execution and won’t stop unless the DMA is stopped or the requests stop coming over. The DMA will handle/reset all the flags within the peripheral. Once we enabled the peripheral with DMA, we won’t need to drive it anymore: the DMA will do it for us (which, in return will be driven by the peripheral using the requests, thus the closed loop between the two).

### With DMA
We assign/connect the DMA channels to peripherals by writing a specific sequence to the designated channel in the CSELR register. These sequences will be what the peripheral will send to the DMA as a request, the DMA recognises these and then activates itself accordingly.

As it goes though, the same request sequence can be used for different channels to be activated by different peripherals (see Table 51). A good example is the request “4’b1000” which can mean 6 (!) different things, depending on which peripheral is sending it to which channel. This means that the DMA may be expecting the same exact sequence/request to activate different channels. This is a problem if the DMA is requested by same request sequence at the same time, but not if the requests are not overlapping. If different request sequences are coming in at the same time, the DMA will simply schedule them after each other without a problem, depending on priority and the channels number (the lower number takes priority over he higher one). As such, one has to be sure that:
1) peripherals drive the DMA using different requests (that is, if possible, chose the channel that is uniquely driven)
2) or if that is not possible, the peripherals with the same request sequence are not activated at the same time

DMA can also increment memory so it can fill a memory buffer gradually. It can also circularly load the same buffer, effectively providing an up-to-date data dump for the main loop to use.

Lastly, DMA is pretty sensitive and can shut off pretty easily. The circumstances have to be always perfect to make it work and even a slightest hick-up can make it stop. It is recommended to not run it all the time but reset it regularly. This needs to be a full reset with reconfiguration since the DMA does not support “halting”. Also, pretty much any type of changing of the DMA registers without the DMA being shut off will freeze the DMA.

### With the DMA IRQ
All IRQs must be enabled and assigned a priority to properly work. when assigning the IRQ priority, I had problems assigning highest priority (that is, priority 0) to IRQs. The code often crashed. I recommend using priority 1 as the highest and then go from there instead! 

IRQs run on flags and these flags need to be managed by the “handler” of the IRQ – the function that is called when the IRQ is triggered. If these flags are not removed, the IRQ will keep on being called after being activated, hijacking the execution and putting it into an infinite loop.

Usually an IRQ has multiple ways to be triggered – here for instance, we will trigger it when the DMA had a “transfer complete” or when we had an error in the DMA – and there is no inherent difference between how these versions are triggered. One has to check the flags of the peripheral within the IRQ to understand, what state the peripheral is at the moment of interrupt. Also, the same IRQ might be connected to multiple channels (DMA channels 4,5,6,7 call the same IRQ handler called “DMA1_Channel4_5_6_7_IRQHandler”, for instance). Handler functions are defined within the startup and won’t need to be prototyped. (Just an FYI: if you check the NVIC vector table, you will actually find all the handlers there…DMA channel 1 ate position 9, for instance.)

## User guide
There isn’t much to say here.  The main loop simply reads out the data buffer every second to show that the buffer is being updated regularly without the main loop demanding the change. If you wish to have temperature values, just use the conversion of the ADC data to temperature values from the STM32_ADC-DACDriver project.

Of note, if simply touching the mcu does not show a noticeable change, put the mcu under a heat source (heat guns work the best).

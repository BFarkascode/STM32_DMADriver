# STM32_DMADriver

A project to implement DMA on an STM32L0x3 using bare metal programming.

## General description
Originally I wasn’t planning to do a separate project on direct memory access (DMA) since there isn’t a lot of complicated coding into it, not to mention, the explanations on the internet over how DMA works are far and wide. But then I realised that the concept of DMA itself is useful enough to merit a small project, even if it won’t be unravelling any great mysteries. It will allow me to talk a bit again about interrupts (IRQs) which is also a topic that is pretty useful to get a hang of but has little to offer by themselves.

In general, DMAs and IRQs are very similar in how they behave. Neither of them are, strictly speaking, part of the main programming loop we "program". Instead they are both controlled by hardware and thus either take priority in execution compared to other code segments (as it is with IRQ) or can run IN PARALLEL to the main loop and do their own thing (like DMA).

In order to have a nice project to show the capabilities of the DMA, we will be attaching it to the ADC that provides the internal temperature value for the mcu. What we want is to show that even at a time when we aren’t telling the main loop to read out values from the ADC, the DMA will do the job for us.

### Why use IRQs?
This question used to be on my mind a lot. Wouldn’t it be just as easy to simply wait for the code to find out that something important has changed and then react to it?

Well, no, not always. For simple projects where timing is not important (like most Arduino-based hobby projects), one can certainly wait until the program loop finds out that a variable has changed or that a GPIO input has been pulled high, but that won’t be good enough when efficient execution is key. It takes some time for the main loop to actually check the variable or the GPIO input value (called “polling”), meaning that we might realise that a value is different milliseconds (or more) after it has changed. This is not good enough if we need an immediate reaction to a triggering signal.

The other reason to use IRQs is simply practical: some processes – like NVM - and peripherals – like DMA - run on them and use them to provide feedback to the main loop over their state. Since both of the processes mentioned above need to run outside the main loop, without IRQs, they won’t have a way to signal to the code execution. DMA, for instance, runs parallel to the main loop so it can’t just “wait” for the main to react.

### Why use DMA?
Because it allows parallel actions. Some rather tedious processes such as communication protocol drivers often have to wait until a communication comes in – effectively blocking all other actions – or they risk losing data. One way to get around that problem is to use DMA which will just capture the incoming data whenever it comes in. This is particularly useful if we have a lot of data coming in. DMA allows the mcu to process them in a bulk by loading the data into an data array.

Another gate usecase for DMA is the ping-pong buffer: a buffer that is one half loaded while the other half is being processed by the main loop. (We use a ping-pong buffer in the Bootloader project.)

## To read
We will be using the ADC defined in the STM32_ADC-DACDriver project as the input source for the DMA. As such, familiarity with that project is necessary.

We should also check, how the ADC is connected to the DMA:
-	14.5.5 Managing converted data using the DMA: describes how to set up the DMA on the ADC’s side.

Additionally, for the DMA, the "must reads" are:
-	11.3.2 DMA Request Mapping: Table 51 in this section is what tells us, how to configure the DMA channels to connect them to particular peripherals. As the table suggests, only specific channels can be attached to specific peripherals.
-	11.4.2 DMA transfers: describes how the DMA transfer works
-	11.4.3 DMA arbitration: channel priority assignment
-	11.4.4 DMA channels: channel configuration
-	11.4.6 DMA error management: when and how the DMA can stop
-	11.5 DMA interrupts
-	11.6 DMA registers

For the IRQs, there isn’t really a good part in the reference  manual (refman) to explain them specifically. The Section 13 “EXTI” and the Section 12 “NVIC” sections can give a hint on how they work, but I have found the information to be somewhat sketchy. I suggest playing with a button IRQ – see the NVMDriver project on how to use the blue button on the nucleo for this purpose – and figure it out IRQs by doing if the explanations below are not adequate.

## Previous relevant projects
The following projects should be checked:
- STM32_ADC-DACDriver

## Particularities
### With the peripheral (in this project, the ADC)

Since DMA is separate from the main loop, it is not controlled by it. Instead, it is the peripherals – here, the ADC – that drive the DMA directly using “requests”. These requests must be specifically enabled on the peripheral’s side, otherwise it won’t be sending them out.

Once the requests are enabled – with the ADC calibrated normally – everything starts to run automatically. Literally: once you tell the ADC to send the requests (and the DMA is properly set), the data will start to flow into the memory buffer atatched to the DMA, even if we haven’t reached the main loop yet with our execution. The flow won’t stop unless the DMA is stopped or the requests stop coming in.

The DMA will handle/reset all the flags within the peripheral. Once we enabled the peripheral with DMA, we won’t need to drive it anymore: the DMA will do it for us (which, in return will be driven by the peripheral using the requests, thus the closed loop between the two).

### With DMA
We assign/connect the DMA channels to peripherals by writing a specific sequence to the designated channel in the CSELR register. These sequences will be what the peripheral will send to the DMA as a request, the DMA recognises these sequences and then activates itself accordingly.

The same request sequence can be used for different channels to be activated by different peripherals though (see Table 51). A good example is the request “4’b1000” which can mean 6 (!) different things, depending on which peripheral is sending it to which channel. This means that the DMA may be expecting the same exact sequence/request to activate different channels. This is a problem if the DMA is activated by same request sequence at the same time, but not if the requests are not actually overlapping. Additionally, if different request sequences are coming in at the same time, the DMA will simply schedule them after each other, teh schedule depending on priority and the channel's numbering (the lower number takes priority over he higher one). As such, one has to be sure that:
1) peripherals driving the DMA use different requests (that is, if possible, chose channels that can be uniquely driven)
2) or, if that is not possible, the peripherals with the same request sequence MUSt NEVER be activated at the same time. Doing so crashes the DMA.

DMA can increment memory so it can fill a memory buffer gradually. It can also circularly load the same buffer, effectively providing an up-to-date data dump for the main loop to use.

Lastly, it is recommended to not run DMA all the time but reset it regularly. This needs to be a full reset with reconfiguration since the DMA does not support “halting”. A full reset must also accompany any type of changing of the DMA registers to avoid crashing.

### With the DMA IRQ
All IRQs must be enabled and assigned a priority to properly work. when assigning the IRQ priority, I had problems assigning highest priority (that is, priority 0) to IRQs. The code often crashed. I recommend using priority 1 as the highest and then go from there instead! 

IRQs run on flags and these flags need to be managed by the “handler” of the IRQ – the function that is called when the IRQ is triggered. If these flags are not removed, the IRQ will keep on being called after being activated, hijacking the execution and putting it into an infinite loop.

Usually an IRQ has multiple ways to be triggered – in this project here, for instance, we will trigger it when the DMA had a “transfer complete” or when we had an error in the DMA – and there is no inherent difference between how these versions are triggered. One has to check the flags of the peripheral within the IRQ to understand, what state the peripheral is at the moment of interrupt. Also, the same IRQ might be connected to multiple channels (DMA channels 4,5,6,7 call the same IRQ handler called “DMA1_Channel4_5_6_7_IRQHandler”). Handler functions are defined within the startup and won’t need to be prototyped. (Just an FYI: if you check the NVIC vector table, you will actually find all the handlers there…DMA channel 1 is at position 9, for instance.)

## User guide
There isn’t much to say here.  The main loop simply reads out the data buffer every second to show that the buffer is being updated regularly without the main loop demanding an update. Of note, it is the raw ADC value that is being stored in the buffer, so if you wish to have temperature values, just use the conversion of the ADC data to temperature values from the STM32_ADC-DACDriver project.

If simply touching the mcu does not show a noticeable change, put the mcu under a heat source (heat guns work the best).

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "../include/stm32f407xx.h"


// Use PB 3,4,5 (SCK, MISO, MOSI)
void SPI_init(void) {
	// INIT PB

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN; // enable GPIOB
    while ((RCC->AHB2ENR&RCC_AHB2ENR_GPIOBEN)==0){}; // wait for GPIOB to be setup

    GPIOB->MODER &= ~(GPIO_MODER_MODE3|GPIO_MODER_MODE4|GPIO_MODER_MODE5); // clear PB345 bits
	GPIOB->MODER |= (GPIO_MODER_MODER3_1|GPIO_MODER_MODER4_1|GPIO_MODER_MODER5_1); // set MODER to 10 for AF
    
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3|GPIO_AFRL_AFSEL4|GPIO_AFRL_AFSEL5); // clear AF forB345
	GPIOB->AFR[0] = ((GPIO_AFRL_AFSEL3_0|GPIO_AFRL_AFSEL3_2)|(GPIO_AFRL_AFSEL4_0|GPIO_AFRL_AFSEL4_2)|(GPIO_AFRL_AFSEL5_0|GPIO_AFRL_AFSEL5_2)); // set AF5 (0101), TIM2 CH1


	// INIT SPI

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // enable SPI1
    while ((RCC->APB2ENR&RCC_APB2ENR_SPI1EN)==0){}; // wait for SPI1 to be setup

    SPI->CR1 &= ~(SPI_CR1_CPHA|SPI_CR1_CPOL|SPI_CR1_LSBFIRST); // set CK to 0 when idle and capture first edge on first CK transition, send MSB first
    SPI->CR1 |= (SPI_CR1_BR_2|SPI_CR1_DFF|SPI_CR1_SSI|SPI_CR1_SSM|SPI_CR1_BIDIMODE); // set baud rate divider to 32, datafram is 16bit, set SSI/SSM for software handling, enable bidirectional data line

    SPI->CR2 &= ~(SPI_CR2_FRF); // select motorola mode (not TI)
    SPI->CR1 |= (SPI_CR1_MSTR); // set master mode

    /*
    Transmit sequence
    The transmit sequence begins when a byte is written in the Tx Buffer.
    The data byte is parallel-loaded into the shift register (from the internal bus) during the first
    bit transmission and then shifted out serially to the MOSI pin MSB first or LSB first
    depending on the LSBFIRST bit in the SPI_CR1 register. The TXE flag is set on the transfer
    of data from the Tx Buffer to the shift register and an interrupt is generated if the TXEIE bit in
    the SPI_CR2 register is set.

    Receive sequence
    For the receiver, when data transfer is complete:
    • The data in the shift register is transferred to the RX Buffer and the RXNE flag is set
    • An interrupt is generated if the RXNEIE bit is set in the SPI_CR2 register
    At the last sampling clock edge the RXNE bit is set, a copy of the data byte received in the
    shift register is moved to the Rx buffer. When the SPI_DR register is read, the SPI
    peripheral returns this buffered value.
    Clearing the RXNE bit is performed by reading the SPI_DR register.
    A continuous transmit stream can be maintained if the next data to be transmitted is put in
    the Tx buffer once the transmission is started. Note that TXE flag should be ‘1 before any
    attempt to write the Tx buffer is made.
    Note: When a master is communicating with SPI slaves which need to be de-selected between
    transmissions, the NSS 
    */
}

/*
TRANSFER PROCEDURE: TBA

1. Enable the SPI by setting the SPE bit to 1.
2. Write the first data item to be transmitted into the SPI_DR register (this clears the TXE
flag).
3. Wait until TXE=1 and write the second data item to be transmitted. Then wait until
RXNE=1 and read the SPI_DR to get the first received data item (this clears the RXNE
bit). Repeat this operation for each data item to be transmitted/received until the n–1
received data.
4. Wait until RXNE=1 and read the last received data.
5. Wait until TXE=1 and then wait until BSY=0 before disabling the SPI
*/
#include "I2C.h"
#include "stm32f407xx.h"
#include <stdbool.h>
#define I2C_TIMEOUT 200000u

/* 
 * helper function to check status of I2C
 * Only useable in this file
*/
static I2C_STATUS wait_sr1(uint32_t set_mask) {
    uint32_t n = I2C_TIMEOUT;
    while (n--) {
        uint32_t curr_status = I2C1->SR1; // Read I2C1 status register 1
                                          //  This checks whether coomunication events can start

        if (curr_status & I2C_SR1_AF) {  // Check if device didn't acknowledge signal/expected byte not recieved
            I2C1->SR1 &= ~I2C_SR1_AF;   // AF = Acknowledge Failure
            return I2C_NACK;          // Returns No ACKnowledge
        }

        /*
         * BERR = Bus error; ARLO = Arbitration Lost; OVR = overrun/underrun
         * overrun means that we didn't read the data before it got overwritten
        */
        if (curr_status & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR)) 
            return I2C_BUS; // notifies system of bus issue

        // Ready to move ahead in state machine
        // Recieved signal matches what needed
        if ((curr_status & set_mask) == set_mask) 
            return I2C_OK;
    }
    return I2C_TIMEOUT; // timeout, took too long
}

/*
 * Helper functions; useable only in this file
 * Terminate transmission
*/
static void abort_bus(void) {
    I2C1->CR1 |= I2C_CR1_STOP; // Stops I2C
    I2C1->SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR); // Clears all error flags
}

/* 
 * Helper function; useable only in this file
 * Starts transaction and sends address
 * addr7 = 7-bit I2C address
 * read = read or write operation
*/
static I2C_STATUS start_address(uint8_t addr7, bool read) {
    uint32_t n = I2C_TIMEOUT; // Just to make sure things don't hang

    // Status Register 2 reports state of bus (busy, which is master, transmitting/recieving)
    while ((I2C1->SR2 & I2C_SR2_BUSY) && n--) {} // waits until I2C1 in status register 2 isn't busy

    // If polling limit is reached, and bus is still busy, then report busy
    if (I2C1->SR2 & I2C_SR2_BUSY) 
        return I2C_BUSY;

    // I2C start
    I2C1->CR1 |= I2C_CR1_START;

    // Checks whether I2C is ready to start transmitting (SB = Starting Bit active or not)
    I2C_STATUS e = wait_sr1(I2C_SR1_SB);
    if (e != I2C_OK) // I2C has a failure (setup wrong or smth)
        return e;

    // Sends address to I2C to tell it where we want to send our data
    // Also tells I2C if we want to read or write
    I2C1->DR = (uint8_t)((addr7 << 1) | (read ? 1u : 0u));

    // Will either return a fail or success in transmitting the address
    e = wait_sr1(I2C_SR1_ADDR);

    return e;
}

/*
 * address = address of device to communicate to
 * *p = data to send out
 * len is length of msg
 * Writes to device
*/
I2C_STATUS I2C1_write(uint8_t address, const uint8_t *p, unsigned len) {

    // Fails immediately if bus is left hanging or can't even start transmission
    I2C_STATUS e = start_address(address, false);
    if (e != I2C_OK) { 
        abort_bus(); 
        return e; 
    }

    // clears addr for SR1 and SR2 to enter data phase
    (void)I2C1->SR1; 
    (void)I2C1->SR2; 

    // Transmit data
    for (unsigned i=0; i<len; i++) {
        e = wait_sr1(I2C_SR1_TXE); // Checks Transmit Data Register Empty; if yes, ready for next byte

        if (e != I2C_OK) {  // if it encounters an error, it'll stop
            abort_bus(); // basically clears bus and stops transmission
            return e; 
        }
        I2C1->DR = p[i]; // sends next data bit to I2C
    }
    // waits for Byte Transfer Finish flag
    // Which basically tells us that last byte sent has settled
    e = wait_sr1(I2C_SR1_BTF);

    // clears bus for next use
    abort_bus();
    return e;
}

/*
 * Reads SINGULAR BYTE from device 
 * WILL STOP INTERRUPTS; DO NOT CALL OFTEN
*/
I2C_STATUS I2C1_read_byte(uint8_t address, uint8_t *out) {

    // Make sure pointer actually exists
    if (!out) 
        return I2C_BAD_ARG;

    // Only want ONE byte, so stop reading after 1 byte
    I2C1->CR1 &= ~I2C_CR1_ACK;

    // Start read mode and make sure bus is ready deliver
    I2C_STATUS e = start_address(address, true);
    if (e != I2C_OK) { 
        abort_bus(); 
        return e; 
    }

    // START OF ATOMIC TRANSACTION
    // Stores interrupt state
    uint32_t primask = __get_PRIMASK();
    // disables interrupt to ensure that next steps are done without interruption
    // ensures that timing criteria for following steps are met
    __disable_irq();

    // Reads from both registers to clear them
    (void)I2C1->SR1; 
    (void)I2C1->SR2; 

    // stops I2C at specific bit
    I2C1->CR1 |= I2C_CR1_STOP;

    // restores previous interrupt state
    __set_PRIMASK(primask);

    // Waits until recieved byte is available to be read
    e = wait_sr1(I2C_SR1_RXNE);
    
    // If there exists a byte to read and bus is ready:
    // then it stores the byte into the pointer
    if (e == I2C_OK) 
        *out = (uint8_t)I2C1->DR;
    else 
        abort_bus(); // clears bus otherwise
    
    return e;
}

// Initializing I2C
// pclk1_hz is 16MHz
I2C_STATUS I2C1_init(uint32_t pclk1_hz) {
    // set up PB8 (SCL) and PB9 (SDA)

    // Set up clock
    if (pclk1_hz < 2000000u || pclk1_hz > 42000000u) 
        return I2C_BAD_ARG;

    // Enable GPIOB clock
    // *work note: AHB1ENR = AHB1 Peripheral Clock Enable Register
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // Enable I2C clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    
    // Read register after enabling clocks; ensures bus write has progressed
    (void)RCC->APB1ENR;

    // **PB8, PB9 AF4, external pullups required **

    // CONFIGURE GPIO PINS
    // MODER
    // 00 = input; 01 = general purpose output; 10 = alternate funct; 11 = analog
    // PB8 = 17:16, PB9 = 19:18
    GPIOB->MODER    = (GPIOB->MODER & ~((3u<<16)|(3u<<18))); // Clears MODER
    GPIOB->MODER   |= (2u<<16)|(2u<<18); // Sets MODER for PB8 and PB9

    // OTYPER
    // open-drain output mode: I2C protocol to let other devices pull SDA/SCL low
    // External pull-up resistors necessary to pull bus lines high
    GPIOB->OTYPER  |= (1u<<8)|(1u<<9);

    // OSPEEDR
    // Max. GPIO output speed for PB8 and PB9
    // Means that when outputting through the pin, it changes quickly instead of gradually
    // Creates more switching noise; want faster for 100MHz clock usually but 0x11 might be too much
    GPIOB->OSPEEDR |= (3u<<16)|(3u<<18);

    // PUPDR
    // Disable internal pull up and pull down resistors; MUST USE EXTERNAL
    // Want external because internap is weak
    GPIOB->PUPDR   &= ~((3u<<16)|(3u<<18));

    // AFR
    // Controls alternative funct for certain pins
    // For AFR[1], it's pins 8-15
    // 0x44 selects AF4 for PB8 and PB9. Connects them to I2C1
    GPIOB->AFR[1]   = (GPIOB->AFR[1] & ~0xFFu) | 0x44u;


    // CONFIGURE I2C
    // Clear Peripheral Enable (Disables I2C1)
    I2C1->CR1      &= ~I2C_CR1_PE;

    // Software reset I2C
    I2C1->CR1      |= I2C_CR1_SWRST;

    // Releases reset before setting clock to ensure precision
    // Doesn't touch MUX or camera
    I2C1->CR1      &= ~I2C_CR1_SWRST;

    // Convert APB1 clock from hz to mhz
    uint32_t mhz    = pclk1_hz / 1000000u; // it'll be  16
    uint32_t ccr    = pclk1_hz / 200000u;  // Clock divider for standard I2C; 80

    // If clock divider is less than minimum, then set it at minimum
    if (ccr < 4u) 
        ccr = 4u; // 4u = genuinely 4, bruh 

    // Program I2C1 peripheral clock to whatever mhz ended up being
    // Keeps bottom lower bits (0x3F)
    I2C1->CR2   = mhz & 0x3Fu;

    // Program clock control register
    I2C1->CCR   = ccr;

    // Program maximum-rise time parameter
    // Allows for 17 cycles for signal to go from low to high
    // Not a hard rule, but I2C needs to raise within 17 or will need to change electrical design
    I2C1->TRISE = mhz + 1u;

    // Enables I2C Peripheral
    I2C1->CR1   = I2C_CR1_PE;

    // Init successful
    return I2C_OK;
}
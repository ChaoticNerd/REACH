#include "ADC.h"
#include "stm32f407xx.h"

#define ADC_TIMEOUT_COUNT 100000u

ADC_STATUS ADC1_init(void){
    // Enable GPIOA and ADC1 clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    (void)RCC->APB2ENR;

    // PA1: analog mode
    GPIOA->MODER |= (3u << 2);

    // Disable pull-up and pull-down on PA1
    GPIOA->PUPDR &= ~(3u << 2);

    // Disable ADC before configuring
    ADC1->CR2 &= ~ADC_CR2_ADON;

    // 12-bit resolution, single channel
    ADC1->CR1 = 0;

    // Single conversion, software triggered,
    // right-aligned, no DMA
    ADC1->CR2 = 0;

    // ADC clock prescaler: PCLK2 / 4
    ADC->CCR = (ADC->CCR & ~ADC_CCR_ADCPRE) | ADC_CCR_ADCPRE_0;

    // Long sample time for FSR voltage divider:
    // ADC channel 1 = 480 cycles
    ADC1->SMPR2 = (ADC1->SMPR2 & ~(7u << 3)) | (7u << 3);

    // One conversion in regular sequence
    ADC1->SQR1 &= ~ADC_SQR1_L;

    // First conversion uses channel 1
    ADC1->SQR3 = (ADC1->SQR3 & ~0x1Fu) | 1u;

    // Enable ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    // Allow ADC to stabilize
    for (volatile uint32_t i = 0; i < 1000; ++i) {
        __NOP();
    }

    return ADC_OK;
}

ADC_STATUS ADC1_read(uint16_t *value){
    if (value == 0)
        return ADC_BAD_ARG;

    // Clear previous end-of-conversion flag
    ADC1->SR &= ~ADC_SR_EOC;

    // Begin regular conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;

    uint32_t timeout = ADC_TIMEOUT_COUNT;

    // Wait for conversion to finish
    while (!(ADC1->SR & ADC_SR_EOC)) {
        if (timeout-- == 0u)
            return ADC_TIMEOUT;
    }

    // Read the 12-bit conversion result
    *value = (uint16_t)(ADC1->DR & 0x0FFFu);

    return ADC_OK;
}

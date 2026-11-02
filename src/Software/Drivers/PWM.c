#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "../include/stm32f407xx.h"


// TIM2 Interrupt: Position 28, Prio 35 (may include in a future update)
// CONNECTED TO PA0, AF1
void PWM_init(void) {
	// INIT PA0

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // enable GPIOA
	GPIOA->MODER &= ~GPIO_MODER_MODE0; // clear PA0 bits
	GPIOA->MODER |= GPIO_MODER_MODER0_1; // set MODER to 10 for AF
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL0; // clear AF for PA0
	GPIOA->AFR[0] = GPIO_AFRL_AFSEL0_0; // set AF1 (0001), TIM2 CH1


	// INIT TIM2

	// note: base clock speed is 16 Mhz
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // activate TIM2
	while ((RCC->APB1ENR&RCC_APB1ENR_TIM2EN)==0){}; // wait for TIM2 to be setup

	// disable timer, one pulse mode, center align, preload, and clock div. allow update events, set counting to upcounter
	TIM2->CR1 &= ~(TIM_CR1_CEN|TIM_CR1_DIR|TIM_CR1_UDIS|TIM_CR1_OPM|TIM_CR1_CMS|TIM_CR1_ARPE|TIM_CR1_CKD);
	TIM2->CR1 |= (TIM_CR1_URS); // set update source to counter overflow only
	
	// use interrupts? maybe to set mode
	TIM2->CNT = 7999; // set counter (0.1 seconds)
	TIM2->PSC = 199; // set prescaler to 199
	TIM2->ARR = 7999; // set equal to counter: will auto-reload the counter
	// don't need arr if upcounting? check later
	TIM2->RCR = 0; // set repetition number to 4+1 (use PWM 5/5), 50% (this will make 0.5 sec until overload)


	TIM2->CCER &= ~(TIM_CCER_CC1E|TIM_CCER_CC1P) // clear capture polarity and enable
	TIM2->CCER |= TIM_CCER_CC1E; // enable channel 1 capture
	TIM2->CCMR1 |= (TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1PE); // set OC1M to 110 (PWM mode 1, CH1) & enable preload
	TIM2->CCR1 = 3999; // set compare value to 50% PWM (8000/2)

	TIM2->EGR |= TIM_EGR_UG; // update all registers
}

void enableTIM1(void) {
	TIM2->CR1 |= TIM_CR1_CEN;
}

void disableTIM1(void) {
	TIM2->CR1 &= ~TIM_CR1_CEN;
}

// input integer PWM percent from 0-100, multiply reload by PWM/100 to get desired value
void updatePWM(int PWM) {
	TIM2->CCR1 = (7999*PWM)/100; // set compare value to PWM% (PWM/100)
}
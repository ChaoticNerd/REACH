#ifndef PWM_H
#define PWM_H
#include <stdint.h>

/*
AVAILABLE PWM:
- PA0 : TIM2 CH1
*/
void PWM_init(void);
void enableTIM1(void);
void disableTIM1(void);
void updatePWM(int PWM);
#endif
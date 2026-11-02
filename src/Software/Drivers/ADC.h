#ifndef REACH_ADC_H
#define REACH_ADC_H

#include <stdint.h>

typedef enum {
    ADC_OK = 0,
    ADC_BAD_ARG = -1,
    ADC_TIMEOUT = -2
} ADC_STATUS;

ADC_STATUS ADC1_init(void);
ADC_STATUS ADC1_read(uint16_t *value);

#endif

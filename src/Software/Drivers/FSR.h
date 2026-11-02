/*
Each general-purpose I/O port has four 32-bit configuration registers (GPIOx_MODER, GPIOx_OTYPER, GPIOx_OSPEEDR and GPIOx_PUPDR)
two 32-bit data registers (GPIOx_IDR and GPIOx_ODR)
a 32-bit set/reset register (GPIOx_BSRR)
a 32-bit locking register (GPIOx_LCKR),
two 32-bit alternate function selection register (GPIOx_AFRH and GPIOx_AFRL).
*/
#ifndef FSR_H
#define FSR_H

#include <stdint.h>
#include <stdbool.h>

#define FSR_CONTACT_ON  600u
#define FSR_CONTACT_OFF 400u

void FSR_init(void);
bool FSR_contact(void);

#endif
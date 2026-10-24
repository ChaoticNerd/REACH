#ifndef I2C_H
#define I2C_H
#include <stdint.h>
#include "i2c_status.h"

/* I2C1: PB8=SCL, PB9=SDA, AF4; standard mode 100 kHz.
 * Pass actual APB1 peripheral clock in Hz; requires external pullups.
 * Blocking master-only API, not interrupt/reentrant safe. 
 */
I2C_STATUS I2C1_init(uint32_t pclk1_hz);
I2C_STATUS I2C1_write(uint8_t address7, const uint8_t *bytes, unsigned length);
I2C_STATUS I2C1_read_byte(uint8_t address7, uint8_t *value);
#endif
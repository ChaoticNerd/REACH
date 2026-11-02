/* 
 * FSR logic only needs to:
 * 1. Read the ADC value
 * 2. Decide whether that value indicates contact
 * 3. Return the result
 */
#include "FSR.h"
#include "ADC.h"

/* Remember the previous contact state */
static bool contact = false;

void FSR_init(void){
    ADC1_init();
    contact = false;
}

bool FSR_contact(void){
    uint16_t adc_value = 0u;

    // Read the sensor
    if (ADC1_read(&adc_value) != ADC_OK) {
        return false;
    }

    // Detect contact
    if (!contact && adc_value >= FSR_CONTACT_ON) {
        contact = true;
    }

    // Detect release
    else if (contact && adc_value <= FSR_CONTACT_OFF) {
        contact = false;
    }

    return contact;
}
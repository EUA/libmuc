#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>
#include <stdbool.h>

#include "adc_conf.h"

void adc_init(void);


volatile uint16_t adc_values[ADC_CHANNEL_CT];
volatile bool adc_new_data;

#endif

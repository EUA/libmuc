/*
//Follow the damn line
char Author [] ="Cody Schafer";
*/

#include "defines.h"
#include "usart.h"
#include "adc.h"
#include "motor.h"
#include "timers.h"
#include <avr/power.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

void clock_init(void) {
	
	#if   F_CPU == 1000000
		clock_prescale_set(clock_div_8);	
	#elif F_CPU == 2000000
		clock_prescale_set(clock_div_4);	
	#elif F_CPU == 4000000
		clock_prescale_set(clock_div_2);	
	#elif F_CPU == 8000000
		clock_prescale_set(clock_div_1);	
	#else
		#error "F_CPU Unrecognized"
	#endif
	
	//OSCAL set by the bootloader.
}

void  print_bin(uint8_t inp) {
	for(int8_t j=7; j>=0; --j) {
	   	printf("%c",((inp&(1<<j))>>j)+'0');
	}
}

void joy_init(void) {
	// Set pins as inputs.
	DDRB&=(uint8_t)~((1<<4)|(1<<6)|(1<<7));
	DDRE&=(uint8_t)~((1<<2)|(1<<3));
	// Enable pullup resistors.
	PORTB|=((1<<4)|(1<<6)|(1<<7));
	PORTE|=((1<<2)|(1<<3));
	
	//DOWN	= PINB&(1<<7)
	//LEFT	= PINE&(1<<2)
	//RIGHT = PINE&(1<<3)
	//UP	= PINB&(1<<6)
	//IN	= PINB&(1<<4)
	
	// Enable Pin Change interupts. Disable INT0
	//EIMSK|=((1<<PCIE1)|(1<<PCIE0));
	//EIMSK&=(uint8_t)~(1<<INT0);
	EIMSK=(1<<PCIE1)|(1<<PCIE0);
	PCMSK1=(1<<PCINT15)|(1<<PCINT14)|(1<<PCINT12);
	PCMSK0=(1<<PCINT3)|(1<<PCINT2);	
	
}



ISR(PCINT0_vect) {
	//PE2,3
	uint8_t iPINE = (uint8_t)~PINE;
	if (iPINE&((1<<2)|(1<<3))) {
		if (iPINE&(1<<2))
			printf_P(PSTR("\n[debug] Left"));
		if (iPINE&(1<<3))
			printf_P(PSTR("\n[debug] Right"));
	}
	else
		printf_P(PSTR("\n[debug] PE? Released"));
}

ISR(PCINT1_vect) {
	//PB7,4,6
	uint8_t iPINB = (uint8_t)~PINB;
	if (iPINB&((1<<7)|(1<<6)|(1<<4))) {
		if (iPINB&(1<<7)) {
			printf_P(PSTR("\n[debug] Down "));
			adc_calibrate_update();
			print_adc_calibration();
			print_adc_values();
		}
		if (iPINB&(1<<4)) {
			printf_P(PSTR("\n[debug] In"));
			print_adc_calibration();
			print_adc_values();
		}
		if (iPINB&(1<<6)) {
			printf_P(PSTR("\n[debug] Up"));
			adc_calibrate_clear();
			print_adc_calibration();
			print_adc_values();
		}
	}
	else
		printf_P(PSTR("\n[debug] PB? Released"));
}

void pcint_init(void) {
	
}

void init(void) {
	cli();
	power_lcd_disable();
	power_spi_disable();
	clock_init();
	joy_init();
	usart_init();
	adc_init();
	timers_init();	MOTOR_CTL_DDR|=((1<<M_AIN1)|(1<<M_AIN2)|(1<<M_BIN1)|(1<<M_BIN2));
	motor_mode(MOTOR_L_FWD,LEFT);
	motor_mode(MOTOR_R_FWD,RIGHT);
	sei();
	printf_P(PSTR("\nInit: Done\n\n"));
}

int main(void) {
	init();
	motor_set_speed(0,LEFT);
	motor_set_speed(0,RIGHT);
		
	char input;
	for(;;) {
		printf_P(PSTR("\nWhat ([T]est/[F]ollow): "));
		scanf("%c",&input);
		if (input=='F') {
			for (;;) {
				uint16_t c_speed [2] = {motor_get_speed(LEFT),motor_get_speed(RIGHT)};
				printf("\nML: %X",c_speed[0]);
				printf("\nMR: %X",c_speed[1]);
				print_adc_values();
		
				uint16_t adc_val_mixed [2] = {	adc_get_val(0) + adc_get_val(1) * LF_ADC_MIX_WIEGHT,	\
												adc_get_val(3) + adc_get_val(2) * LF_ADC_MIX_WIEGHT	};

				if (adc_val_mixed[0]>adc_val_mixed[1])
					lf_turn_left_inc(LF_INC);
				else if (adc_val_mixed[1]>adc_val_mixed[0])
					lf_turn_right_inc(LF_INC);
				else
					lf_full_speed();

				_delay_ms(700);
				// do at every adc calc or pwm vector.
			}
		}
		else if(input=='T') {
			motor_mode(MOTOR_L_FWD,LEFT);
			motor_mode(MOTOR_R_FWD,RIGHT);	
			for(;;) {
				motor_set_speed(0x0F00,LEFT);
				motor_set_speed(0x00F0,RIGHT);
				printf_P(PSTR("\n       76543210"));
				printf_P(PSTR("\nPORTB: "));print_bin(PORTB);
				printf_P(PSTR("\nPORTD: "));print_bin(PORTD);
				printf_P(PSTR("\nPINB : "));print_bin(PINB);
				printf_P(PSTR("\nPINE : "));print_bin(PINE);
				_delay_ms(700);
				motor_set_speed(0xFFFF,LEFT);
				motor_set_speed(0xFFFF,RIGHT);
				printf_P(PSTR("\n       76543210"));
				printf_P(PSTR("\nPORTB: "));print_bin(PORTB);
				printf_P(PSTR("\nPORTD: "));print_bin(PORTD);
				printf_P(PSTR("\nPINB : "));print_bin(PINB);
				printf_P(PSTR("\nPINE : "));print_bin(PINE);
				_delay_ms(700);
			}
		}
		else {
			printf_P(PSTR("\nInvalid Mode."));
		}
	}	
} 
		
ISR(BADISR_vect) {
	printf_P(PSTR("\n\nInvalid Interupt Enabled\n"));
}


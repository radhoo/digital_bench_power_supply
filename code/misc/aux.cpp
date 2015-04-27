#include "aux.h"

// Reset the microcontroller
void aux_softwarereset() {
	//asm ("JMP 0");
	// http://forum.arduino.cc/index.php?topic=78020.0
}

// Proper reset using the watchdog: untested
void aux_wdreset() {
  wdt_enable(WDTO_15MS);
  while(1);
}

// Reads the ADC port specified by i
uint16_t aux_ADCRead(uint8_t i) {
	ADMUX = i;
	_delay_ms(1); //very important!
	ADCSRA = (1<<ADEN) | (1<<ADSC) |
			(1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // set prescaler to 128 (16M/128=125kHz, above 200KHz 10bit results are not reliable)
	_delay_ms(1); //very important! allows ADEN to turn on ADC
	while (bit_is_set(ADCSRA,ADSC));
	uint16_t v = ADCL;
	v |= ADCH << 8;
	return v;
}

void aux_ADCReadIntr(uint8_t i) {
	ADMUX = i;
	/* enable analog to digital conversion in free run mode
	*  without noise canceler function. See datasheet of atmega8 page 195
	* ADEN: Analog Digital Converter Enable
	* ADIE: ADC Interrupt Enable
	* ADIF: ADC Interrupt Flag
	* ADFR: ADC Free Running Mode
	* ADCSR: ADC Control and Status Register
	* ADPS2..ADPS0: ADC Prescaler Select Bits */
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADFR) | (1<<ADIF) | (1<<ADSC) |
			(1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // set prescaler to 128 (16M/128=125kHz, above 200KHz 10bit results are not reliable)

}

// return ADC port voltage computed against given VRef, with resistive divider
float aux_readDivVoltage(float vref, uint16_t divider_R1, uint16_t divider_R2, uint8_t i) {
	return aux_ADCRead(i) / 1024.0 * (divider_R1 + divider_R2) / divider_R2 * vref;
}


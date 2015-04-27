/*
 File:       main.cpp
 Version:    1.0 - first version
 Date:       April 7, 2015
 License:	GPL v2

 Digital Bench Power supply
 http://www.pocketmagic.net/digital-bench-power-supply/

 ****************************************************************************
 Copyright (C) 2015 Radu Motisan  <radu.motisan@gmail.com>

 http://www.pocketmagic.net

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ****************************************************************************
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdlib.h>
#include "timeout.h"
#include "io/DigitalIn.h"
#include "io/DigitalOut.h"
#include "sensors/ds18b20.h"
#include "lcd/hd44780.h"
#include "misc/aux.h"
#include "misc/pwm.h"


/************************************************************************************************************************************************/
/* Main entry point                                                    																			*/
/************************************************************************************************************************************************/
HD44780 	lcd;
DS18B20 	ds18b20;
PWM 		pwm;
DigitalOut 	relay,
			speaker,
			fan;
DigitalIn 	but1, // v+
			but2, // v-
			but3, // i+
			but4; // i-
float tempval = 0;
float vref = 5.0;

/* power control modes */
#define MODE_MEASURE_CURRENT 0
#define MODE_MEASURE_VOLTAGE 1
volatile bool adc_mode = MODE_MEASURE_CURRENT;
volatile float current = 0, voltage = 0;

/* my power supply after rectifier shows 16.1V on the center tap and 33.8V on full secondary
 * opamp will introduce some upper voltage limitations. Currently using TL082 and we can reach close to 26V maximum
 */
#define VOLTAGE_LIMIT	26
#define CURRENT_LIMIT	5
#define TEMPERATURE_STARTFAN 50 // if 40degrees celsius are reached, we will start the fan
#define TEMPERATURE_SHUTDOWN 70 // if 80degrees celsius are reached, we will reduce the power until the unit cools down


/* set default values: later save/load them to/from EEPROM */
volatile float targetVol = 5.0, targetCur = 1;

volatile bool alarm = false;
uint16_t cycles = 0;


// Interrupt service routine for the ADC completion
// we measure the current (PC5) and the voltage (PC4) alternatively
ISR(ADC_vect){
	uint16_t analogVal = ADCL | (ADCH << 8);
	if (adc_mode == MODE_MEASURE_CURRENT) {
		current = (analogVal / 1024.0 * vref) / 0.186666; // the output power resistor measures 0.55Ohm
		// alternate mode
		adc_mode = MODE_MEASURE_VOLTAGE;
		ADMUX = PC4; // next time we will do ADC on PC4, to get the voltage
	} else if (adc_mode == MODE_MEASURE_VOLTAGE) {
		// the actual R13 and R14 resistors can be measured using a multimeter for better accuracy
		float R14 = 9.87, /*10k*/ R13 = 98.2; /*100K */
		voltage = analogVal / 1024.0 * (R13 + R14) / R14 * vref;
		// alternate mode
		adc_mode = MODE_MEASURE_CURRENT;
		ADMUX = PC5; // next time we will do ADC on PC5, to get the current
	}

	// If free-running mode is enabled (ADFR), we don't need to restart conversion
	// But we can't use ADFR as we want to monitor two separate ADC channels alternatively

	// start another ADC conversion
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADIF) /* | (1<<ADFR) */ | (1<<ADSC) |
				(1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // set prescaler to 128 (16M/128=125kHz, above 200KHz 10bit results are not reliable)

	// control operations, in order

	// 1. act on short circuit
	// decrease quickly on current spike
	if (current > targetCur + targetCur / 2) {
		pwm.setDuty(pwm.getDuty() / 2);
		alarm = true; // will sound alarm in main thread
	}
	// 2.act on over current
	// decrease slowly on higher current
	else if (current > targetCur) {
		pwm.setDuty(pwm.getDuty() - 1);
	}
	// 3.act on over temperature
	else if (tempval > TEMPERATURE_SHUTDOWN) {
		pwm.setDuty(pwm.getDuty() - 1);
		alarm = true;
	}
	// 4. act on over voltage
	// take care of voltage with lower priority
	else if (voltage > targetVol) pwm.setDuty(pwm.getDuty() - 1);
	// 5. act on under voltage
	else if (voltage < targetVol) pwm.setDuty(pwm.getDuty() + 1);
}

int main(void) {
	// setup everything
	relay.init(&PORTD, PD5);
	fan.init(&PORTD, PD6);
	speaker.init(&PORTB, PB0);
	pwm.initPWM();

	but1.init(&PORTC, PC0);
	but2.init(&PORTC, PC1);
	but3.init(&PORTC, PC2);
	but4.init(&PORTC, PC3);


	ds18b20.init(&PORTD, PD7);
	lcd.init(&PORTD, PD4, &PORTB, PB3, &PORTD, PD3, &PORTB, PB4, &PORTD, PD2, &PORTB, PB5); //RS, E, D4, D5, D6, D7

	// setup ADC to measure as interrupt: go for current first
	adc_mode = MODE_MEASURE_CURRENT;
	ADMUX = PC5;
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADIF) | (1<<ADSC) |
			(1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // set prescaler to 128 (16M/128=125kHz, above 200KHz 10bit results are not reliable)
	sei();

	// show welcome screen
	lcd.sendstring("********************\n"\
				   "*Digital Pwr Supply*\n"\
				   "*pocketmagic.net'15*\n"\
				   "********************\n");

	// keep screen then clear
	//_delay_ms(500);

	// main loop
	while (1) {

		// engage relay if we're at 12V or more. use measured voltage or configured voltage?
		relay.set(targetVol > 12);
		// engage fan is temperature is above threshold
		fan.set(tempval > TEMPERATURE_STARTFAN);

		// limit the frequency for some ops including display and temperature reading
		if (cycles == 0) {
			float temp = 0;
			cli();
			if (ds18b20.readTemperature(&temp) == THERM_OK)
				tempval = temp;
			sei();

			lcd.cursorhome();
			lcd.sendstringformat(
				" +SET: %5.2fV %5.2fA\n"\
				"%5.1fC %5.2fV %5.2fA\n"\
				"%s       PWM:%4d\n"
				"2015 pocketmagic.net",
				targetVol, targetCur,
				tempval, voltage, current,
				fan.get()?" +FAN":"     ",
				pwm.getDuty()
			);
		}
		cycles ++;
		if (cycles > 10000) cycles = 0;


		// sound alarm
		if (alarm) {
			speaker.set(1);
			_delay_ms(30);
			speaker.set(0);
			// stop alarm
			alarm = false;
		}

		// act on buttons
		if (but1.get() == 0) {
			if (targetVol < VOLTAGE_LIMIT) targetVol += 0.1;
			cycles = 0; // force lcd refresh
		}
		if (but2.get() == 0) {
			if (targetVol > 0.1) targetVol -= 0.1;
			cycles = 0; // force lcd refresh
		}
		if (but3.get() == 0) {
			if (targetCur < CURRENT_LIMIT) targetCur += 0.1;
			cycles = 0; // force lcd refresh
		}
		if (but4.get() == 0) {
			if (targetCur > 0.1) targetCur -= 0.1;
			cycles = 0; // force lcd refresh
		}



	}

}



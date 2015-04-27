/*
    File:       pwm.cpp
    Version:    0.1.0
    Date:       July 10, 2013
	License:	GPL v2
    
	handle the PWM generator the easy way
    
    ****************************************************************************
    Copyright (C) 2013 Radu Motisan  <radu.motisan@gmail.com>
	
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

#include "../timeout.h"
#include "pwm.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void PWM::setDuty(uint16_t d) {
	if (d >= INVERTER_DUTY_MIN && d <= INVERTER_DUTY_MAX) {
		duty = d;
		OCR1A = (uint16_t)( (float)ICR1 * (float) duty  / 1000.0);
	}
}

void PWM::initPWM() {
	TCCR1A = 0;     // disable all PWM on Timer1 whilst we set it up
	DDRB |= _BV(PB1); // Set PB1 as output (PB1 is OC1A)
	ICR1 = F_CPU / INVERTER_FREQUENCY; // set the frequency FCPU/(ICR1 * PRESCALLING) Hz . Prescalling set to 1X
	setDuty(INVERTER_DEFAULT_DUTY);
	TCCR1B = (1 << WGM13) | (1<<WGM12) | (1 << CS10); //Fast PWM mode via ICR1, with no prescaling (CS11 = 8x, CS10 = 1x)
	TCCR1A |= (1 << COM1A1) | (1<< CS11); // set none-inverting mode and start PWM
}

uint16_t PWM::getDuty() {
	return duty;
}

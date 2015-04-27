/*
    File:       inverter.h
    Version:    0.1.0
    Date:       July 10, 2013
	License:	GPL v2
        
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

#pragma once

// This class assumes the inverter signal is connected to pin PB1 on an ATmega uC

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/

#define INVERTER_DEFAULT_DUTY	167			// start with low duty to allow powering the unit from higher voltages as well
#define INVERTER_DUTY_MIN		1			// 0.1%
#define INVERTER_DUTY_MAX		1000		// 100%
#define INVERTER_FREQUENCY		100000UL	// inverter frequency in Hertz 

class PWM {
	uint16_t duty;							// used to set the duty cycle to adjust the output voltage
	
	public:
	
	void initPWM();
	
	void setDuty(uint16_t d);

	uint16_t getDuty();
};

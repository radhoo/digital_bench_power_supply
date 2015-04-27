/*
 * Copyright (C) 2012 - 2015, Radu Motisan , radu.motisan@gmail.com
 *
 * http://www.pocketmagic.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * 
 * @purpose DS18B20 Temperature Sensor Minimal interface for Atmega microcontrollers
 * http://www.pocketmagic.net/
 * The accuracy of the DS18B20 is �0.5�C over the range of -10�C to +85�C
 */

/*********************************************
 * CONFIGURE THE WORKING PIN
 *********************************************/

#pragma once

#include <avr/io.h> 
#include <stdio.h>
#include "../timeout.h"

/* list of these commands translated into C defines:*/
#define CMD_CONVERTTEMP 0x44
#define CMD_RSCRATCHPAD 0xbe
#define CMD_WSCRATCHPAD 0x4e
#define CMD_CPYSCRATCHPAD 0x48
#define CMD_RECEEPROM 0xb8
#define CMD_RPWRSUPPLY 0xb4
#define CMD_SEARCHROM 0xf0
#define CMD_READROM 0x33
#define CMD_MATCHROM 0x55
#define CMD_SKIPROM 0xcc
#define CMD_ALARMSEARCH 0xec

#define THERM_OK 0
#define THERM_ERR 1

/* constants */
#define DECIMAL_STEPS_12BIT 625 //.0625

class DS18B20 {
	volatile uint8_t *m_pddr, *m_ppin, *m_pport;
	uint8_t m_dq;
	bool powerup;
	
	volatile uint8_t* Port2DDR(volatile uint8_t *port) {
		return port - 1;
	}
	volatile uint8_t* Port2PIN(volatile uint8_t *port) {
		return port - 2;
	}
	
	inline void pinInput();
	inline uint8_t pinGet();
	inline void pinOutput();
	inline void pinSet(uint8_t state);


	int setConfig();

	uint8_t reset();
	
	void writeBit(uint8_t bit);
	uint8_t readBit(void);
	uint8_t readByte(void);
	uint16_t readWord(void);
	void writeByte(uint8_t byte);

public:
	void init( volatile uint8_t  *port, uint8_t  dq) ;
	int readTemperature(float *temp) ;
};	

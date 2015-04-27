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
 */

#include "ds18b20.h"


void DS18B20::init(volatile uint8_t *port, uint8_t dq) {
	// save globals
	m_pport = port;
	m_dq = dq;
	m_pddr = Port2DDR(m_pport);
	m_ppin = Port2PIN(m_pport);

	powerup = true;

	setConfig();
}

void DS18B20::pinInput() {
	*m_pddr &= ~(1 << m_dq); //set for input
}

uint8_t DS18B20::pinGet() {
	return (*m_ppin & (1 << m_dq));
}

void DS18B20::pinOutput() {
	*m_pddr |= (1 << m_dq); // set for output
}

void DS18B20::pinSet(uint8_t state) {
	if (state)
		*m_pport |= (1 << m_dq);
	else
		*m_pport &= ~(1 << m_dq);
}


uint8_t DS18B20::reset() {
	/*pinSet(0); 	//1.
	pinOutput();//2.
	_delay_us(480);//480
	pinInput();
	_delay_us(60);//60
	uint8_t i = pinGet();
	_delay_us(480);//480
	//Return the value read from the presence pulse (0=OK, 1=WRONG)
	return i; // DS18B20 responds with presence pulse.
	*/
	pinOutput();//2.
	pinSet(0); 	//1.

		_delay_us(480);//480
		pinInput();
		_delay_us(70);
		uint8_t i = pinGet();
		_delay_us(410);//60
		return i;
}

void DS18B20::writeBit(uint8_t bit) {
	pinSet(0);  //1.
	pinOutput();//2. these two lines are reversed on purpose!
	_delay_us(1);
	if (bit) pinInput();
	_delay_us(60);
	pinInput();
}

uint8_t DS18B20::readBit(void) {
	uint8_t bit = 0;
	pinSet(0);  //1.
	pinOutput();//2. these two lines are reversed on purpose!
	_delay_us(1);
	pinInput();
	_delay_us(14);
	if (pinGet()) bit = 1;
	_delay_us(45);
	return bit;
}


uint8_t DS18B20::readByte(void) {
	uint8_t byte = 0;
	for (unsigned bit = 0; bit < 8; ++bit) {
		byte |= (readBit() << bit);    // Reads lsb to msb
	}
	return byte;
}

uint16_t DS18B20::readWord(void) {
	uint16_t word = 0;
	for (unsigned bit = 0; bit < 16; ++bit) {
		word |= (readBit() << bit);    // Reads lsb to msb
	}
	return word;
}

void DS18B20::writeByte(uint8_t byte) {
	for (unsigned bit = 0; bit < 8; ++bit) {
		writeBit(byte & 0x01); // lsb to msb
		byte >>= 1;    // right shift by 1-bit
	}
}

int DS18B20::setConfig() {
	//Reset, skip ROM and start temperature conversion
	if (THERM_OK != reset()) return THERM_ERR;
	writeByte(CMD_SKIPROM);
	writeByte(CMD_WSCRATCHPAD);
	// send Th, Tl, config
	writeByte(0x0); // Th / alarm not used
	writeByte(0x0); // Tl / alarm not used
	//writeByte(0x1F); // config: 9bit resolution: 0.5degrees steps
	writeByte(0x3F); // config: 10bit resolution: 0.25degrees steps
	//writeByte(0x5F); // config: 11bit resolution: 0.125degrees steps
	//writeByte(0x7F); // config: 12bit resolution: 0.0625degrees steps

	if (THERM_OK != reset()) return THERM_ERR;

	return THERM_OK;
}

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
uint8_t crc8(const uint8_t *buffer, uint8_t length) {
	uint8_t crc = 0x00;
	for (int i=0;i<length;i++) {
		crc = crc ^ *(buffer++);
		for (int j=0;j<8;j++) {
			if (crc & 1)
					crc = (crc >> 1) ^ 0x8C;
			else
					crc = crc >> 1;
		}
	}
	return crc; // not-complemented!
}

/* 16bits answer, temperature range -55 .. +125 *C */
int DS18B20::readTemperature(float *temp) {
	//Reset, skip ROM and start temperature conversion
	if (THERM_OK != reset()) return THERM_ERR;
	writeByte(CMD_SKIPROM);
	writeByte(CMD_CONVERTTEMP);

	//Reset, skip ROM and send command to read Scratchpad
	if (THERM_OK != reset()) return THERM_ERR;
	writeByte(CMD_SKIPROM);
	writeByte(CMD_RSCRATCHPAD);

	// Read 9 bytes memory
	uint8_t mem[9] = {0};
	for (unsigned i = 0; i<9; i++) {
		mem[i] = readByte();
	}

	// compute CRC and fail if CRC is not valid
	if (crc8(mem, 8) != mem[8]) return THERM_ERR;

	// compute temperature readings
	int16_t sword = (mem[1] << 8) + mem[0];
	*temp = ((float)sword) * 0.0625;

	// drop initial 85 powerup readings
	if (powerup && *temp == 85) return THERM_ERR;

	//if (THERM_OK != reset()) return THERM_ERR;

	powerup = false;

	return THERM_OK;

}

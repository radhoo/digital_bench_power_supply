/*
    File:       HD44780.cpp
    Version:    0.1.0
    Date:       Feb. 21, 2013
	License:	GPL v2
    
	HD44780 LCD 4-bit IO mode Driver
    
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

#include <avr/io.h> 
#include "hd44780.h"

// index 0 is pin rs
void HD44780::lcd_rs_high(){
	*m_pPorts[0] |= (1<<m_pDqs[0]);
}

void HD44780::lcd_rs_low(){
	*m_pPorts[0] &= ~(1<<m_pDqs[0]);
}

// index 1 is pin e
void HD44780::lcd_e_high(){
	*m_pPorts[1] |= (1<<m_pDqs[1]); 
}

void HD44780::lcd_e_low(){
	*m_pPorts[1] &= ~(1<<m_pDqs[1]);
}

//  flush channel E
void HD44780::lcd_toggle_e(void)
{
    lcd_e_high();
    _delay_us(5);
    lcd_e_low();
}

int intpow(uint8_t x, uint8_t e) {
	int p  =1;
	for (int i = 0;i<e;i++) p = p * x;
	return p;
}
//  send a character or an instruction to the LCD
void HD44780::lcd_write(uint8_t data, uint8_t rs) 
{
	// we cannot check LCD status (no read available) , so we will assume a default delay to wait for lcd to be ready
	_delay_us(400);
	//check write type
	if (rs)
	lcd_rs_high(); //write data
		else
	lcd_rs_low();  //write instruction
		
	// output high nibble first
	for (int i=2;i<6;i++) {
		*(m_pPorts[i]) &= ~(1<< m_pDqs[i]);
		uint8_t mask = 0x10 * intpow(2, i-2); //0x10, 0x20, 0x40, 0x80
		
		if (data & mask ) *(m_pPorts[i]) |= (1<< m_pDqs[i]);
	}
	_delay_us(100);
	lcd_toggle_e();
		
		
	// output low nibble
	for (int i=2;i<6;i++) {
		*(m_pPorts[i]) &= ~(1<< m_pDqs[i]);
		uint8_t mask = 0x01 * intpow(2, i-2); //0x01, 0x02, 0x04, 0x08
		if (data & mask ) *(m_pPorts[i]) |= (1<< m_pDqs[i]);
	}
	_delay_us(100);
	lcd_toggle_e();
}


//   send an instruction to the LCD
void HD44780::lcd_instr(uint8_t instr)
{
	lcd_write(instr,0);
}

//   Initialize LCD to 4 bit I/O mode
void HD44780::init(volatile uint8_t *port_rs, volatile uint8_t dq_rs,
						volatile uint8_t *port_e, volatile uint8_t dq_e,
						volatile uint8_t *port_d4, volatile uint8_t dq_d4,
						volatile uint8_t *port_d5, volatile uint8_t dq_d5,
						volatile uint8_t *port_d6, volatile uint8_t dq_d6,
						volatile uint8_t *port_d7, volatile uint8_t dq_d7)
{
	// save globals
	m_pPorts[0] = port_rs; m_pDqs[0] = dq_rs;
	m_pPorts[1] = port_e ; m_pDqs[1] = dq_e;
	m_pPorts[2] = port_d4; m_pDqs[2] = dq_d4;
	m_pPorts[3] = port_d5; m_pDqs[3] = dq_d5;
	m_pPorts[4] = port_d6; m_pDqs[4] = dq_d6;
	m_pPorts[5] = port_d7; m_pDqs[5] = dq_d7;
   // configure all port bits as output (LCD data and control lines on different ports 
	for (int i=0;i<6;i++) {
		*Port2DDR(m_pPorts[i]) |= (1<< m_pDqs[i]);
	}
			
	// 30ms delay while LCD powers on */
	_delay_ms(30);
	 
	// Write 0x30 to LCD and wait 5 mS for the instruction to complete */
	lcd_instr( 0x30);
	lcd_toggle_e();
	_delay_ms(5);
	 
	// Write 0x30 to LCD and wait 160 uS for instruction to complete */
	lcd_instr( 0x30);
	lcd_toggle_e();
	_delay_us(120);
	 
	// Write 0x30 AGAIN to LCD and wait 160 uS */
	lcd_instr( 0x30);
	lcd_toggle_e();
	_delay_us(120);
	 
	// Set function and wait 40uS */
	lcd_instr(LCD_FUNCTION_4BIT_2LINE); //function
	lcd_toggle_e();
	 
	// Turn off the display and wait 40uS */
	lcd_instr(LCD_CMD_OFF);
	lcd_toggle_e();
	 
	// Clear display and wait 1.64mS */
	lcd_instr(LCD_CMD_CLEAR);
	lcd_toggle_e();
	_delay_ms(2);
	 
	/* Set entry mode and wait 40uS */
	lcd_instr(LCD_CMD_ENTRY_INC);    //entry mode
	lcd_toggle_e();
	 
	/* Turn display back on and wait 40uS */
	lcd_instr(LCD_CMD_ON);
	lcd_toggle_e();

}

//   send a character to the LCD
void HD44780::sendchar(uint8_t data)
{
	if (data=='\n') 
	{
		if (g_nCurrentLine >= LCD_LINES - 1)
			setline(0);
		else
			setline(g_nCurrentLine+1);
	}
	else
	lcd_write(data,1);
}




//   send a null terminated string to the LCD eg. char x[10]="hello!";
void HD44780::sendstring(char *text)
{
	char c;
    while ( (c = *text++) )  sendchar(c);
}

void HD44780::sendstringformat(char *szFormat, ...)
{	
    char szBuffer[256]; //in this buffer we form the message
    int NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    int LASTCHAR = NUMCHARS - 1;
    va_list pArgs;
    va_start(pArgs, szFormat);
    vsnprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);
	
	sendstring(szBuffer);
}
//   Set cursor to specified position
//           Input:    x  horizontal position  (0: left most position)
//                     y  vertical position    (0: first line)
void HD44780::gotoxy(uint8_t x, uint8_t y)
{
	if (LCD_LINES==1) {
		lcd_instr((1<<LCD_DDRAM)+LCD_START_LINE1+x);
	} else if (LCD_LINES == 2) {
		switch (y)
		{
			case 0:lcd_instr((1<<LCD_DDRAM)+LCD_START_LINE1+x);break;
			case 1:lcd_instr((1<<LCD_DDRAM)+LCD_START_LINE2+x);break;
			default: break;
		}
	} else if (LCD_LINES == 4) {
		switch (y)
		{
			case 0:lcd_instr((1<<LCD_DDRAM)+LCD_START_LINE1+x);break;
			case 1:lcd_instr((1<<LCD_DDRAM)+LCD_START_LINE2+x);break;
			case 2:lcd_instr((1<<LCD_DDRAM)+LCD_START_LINE3+x);break;
			case 3:lcd_instr((1<<LCD_DDRAM)+LCD_START_LINE4+x);break;
			default: break;
		}
	}
}

//   Move cursor on specified line
void HD44780::setline(uint8_t line)
{
    uint8_t addressCounter = 0;
	switch(line)
	{
		case 0: addressCounter = LCD_START_LINE1; break;
		case 1: addressCounter = LCD_START_LINE2; break;
		case 2: addressCounter = LCD_START_LINE3; break;
		case 3: addressCounter = LCD_START_LINE4; break;
		default:addressCounter = LCD_START_LINE1; break;
	}
	g_nCurrentLine = line;

    lcd_instr((1<<LCD_DDRAM)+addressCounter);
}

//   Clear display and set cursor to home position
void HD44780::clrscr(void)
{
	lcd_instr(LCD_CMD_CLEAR);
	_delay_us(500);
	lcd_instr(LCD_CMD_HOME);
	setline(0);
}

//   Set cursor to home position
void HD44780::cursorhome(void)
{
	lcd_instr(LCD_CMD_HOME);
	setline(0);
}

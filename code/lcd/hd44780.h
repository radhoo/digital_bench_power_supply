/*
    File:       HD44780.h
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

#pragma once
#include <stdio.h>
#include "../timeout.h"




class  HD44780 {
	private: 
		static const int LCD_LINES				 = 4;	 // number of visible lines of the display
		static const int LCD_DISP_LENGTH		 = 20;   // visibles characters per line of the display
		static const int LCD_START_LINE1  		 = 0x00; // DDRAM address of first char of line 1
		static const int LCD_START_LINE2  		 = 0x40; // DDRAM address of first char of line 2
		static const int LCD_START_LINE3  		 = 0x14; // DDRAM address of first char of line 3
		static const int LCD_START_LINE4  		 = 0x54; // DDRAM address of first char of line 4
		static const int LCD_CGRAM				 = 0x6;  // DB6: set CG RAM address
		static const int LCD_DDRAM             	 = 0x7;   // DB7: set DD RAM address
		
		// instruction register bit positions, see HD44780U data sheet
		static const int LCD_CMD_HOME            = 0x02; // cursor home
		static const int LCD_CMD_ENTRY_DEC       = 0x04; // decrement, no shift
		static const int LCD_CMD_ENTRY_DEC_SHIFT = 0x05; // decrement with shift
		static const int LCD_CMD_ENTRY_INC       = 0x06; // increment, no shift
		static const int LCD_CMD_ENTRY_INC_SHIFT = 0x07; // increment with shift
		static const int LCD_CMD_CLEAR           = 0x01; // clear, cursor home
		static const int LCD_CMD_OFF             = 0x08; // display off
		static const int LCD_CMD_ON              = 0x0C; // display on, cursor off
		static const int LCD_CMD_ON_BLINK        = 0x0D; // display on, cursor off, blink char
		static const int LCD_CMD_ON_CURSOR       = 0x0E; // display on, cursor on
		static const int LCD_CMD_ON_CURSOR_BLINK = 0x0F; // display on, cursor on, blink char
		static const int LCD_CMD_CURSOR_LEFT     = 0x10; // decrement cursor (left)
		static const int LCD_CMD_CURSOR_RIGHT    = 0x14; // increment cursor (right)
		static const int LCD_CMD_SHIFT_LEFT      = 0x18; // shift (left)
		static const int LCD_CMD_SHIFT_RIGHT     = 0x1C; // shift (right)
		// function set: set interface data length and number of display lines
		static const int LCD_FUNCTION_4BIT_1LINE = 0x20; // 4-bit interface, single line, 5x7 dots
		static const int LCD_FUNCTION_4BIT_2LINE = 0x28; // 4-bit interface, dual line,   5x7 dots
		static const int LCD_FUNCTION_8BIT_1LINE = 0x30; // 8-bit interface, single line, 5x7 dots
		static const int LCD_FUNCTION_8BIT_2LINE = 0x38; // 8-bit interface, dual line,   5x7 dots
		


		int g_nCurrentLine;
		volatile uint8_t		*m_pPorts[6],//RS, E, D4, D5, D6, D7
								m_pDqs[6];
		
		void lcd_e_high();
		void lcd_e_low();
		void lcd_rs_high();
		void lcd_rs_low();
		
		// flush channel E
		void lcd_toggle_e(void);
		//  send a character or an instruction to the LCD
		void lcd_write(uint8_t data,uint8_t rs) ;			
		//   send an instruction to the LCD
		void lcd_instr(uint8_t instr);
		// 
		volatile uint8_t* Port2DDR(volatile uint8_t *port) {
			return port - 1;
		}
	public:
		//   Initialize LCD to 4 bit I/O mode, using the given connections, in order
		//   RS, E, D4, D5, D6, D7
		void init(volatile uint8_t *port_rs, volatile uint8_t dq_rs,
					  volatile uint8_t *port_e, volatile uint8_t dq_e,
					  volatile uint8_t *port_d4, volatile uint8_t dq_d4,
					  volatile uint8_t *port_d5, volatile uint8_t dq_d5,
					  volatile uint8_t *port_d6, volatile uint8_t dq_d6,
					  volatile uint8_t *port_d7, volatile uint8_t dq_d7);
		//   send a character to the LCD
		void sendchar(uint8_t data);
		//   send a null terminated string to the LCD eg. char x[10]="hello!";
		void sendstring(char *text);	
		void sendstringformat(char *szFormat, ...);
		//   Set cursor to specified position
		//           Input:    x  horizontal position  (0: left most position)
		//                     y  vertical position    (0: first line)
		void gotoxy(uint8_t x, uint8_t y);
		//   Move cursor on specified line
		void setline(uint8_t line);
		//   Clear display and set cursor to home position
		void clrscr(void);
		// FUNCTION: lcd_home
		//   Set cursor to home position
		void cursorhome(void);
};


#!/bin/sh
echo "*** writing fuses for 8MHz(dc+df)/16MHz(de+df) external crystal"
avrdude -p atmega8 -c usbasp -U lfuse:w:0xde:m 	-U hfuse:w:0xdf:m 
echo "*** writting flash"
avrdude -p atmega8 -c usbasp -U flash:w:DigitalPowerSupply-1.hex:i

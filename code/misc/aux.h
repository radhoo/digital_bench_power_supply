#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "../timeout.h"

//#define ADFR 0x5 // adc free mode

// Reset the microcontroller
void aux_softwarereset();

// Proper reset using the watchdog: untested
void aux_wdreset() ;

// Reads the ADC port specified by i
uint16_t aux_ADCRead(uint8_t i);

void aux_ADCReadIntr(uint8_t i);

// return ADC port voltage computed against given VRef, with resistive divider
float aux_readDivVoltage(float vref, uint16_t divider_R1, uint16_t divider_R2, uint8_t i);


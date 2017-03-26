/****************************************************************************
*
 *   Module: virtual DACS ( DAC = digital analog converter )
 *
 *   Description:
 *     virtual DAC on PLATFORM_LPC1114 ==> PWM output
 *     virtual DAC on PC ==> wav.raw and wav.dat files
 *
 *****************************************************************************/

/**************** playing raw data from linux command line ********************
	#!/bin/bash
	echo playing file
	aplay -q -c 2 -t raw -r 48000 -f  s16 wav.raw
	echo finished
******************************************************************************/

#include "dac.h"

#ifdef PLATFORM_PC
	#include <stdio.h>
	FILE * rawFile; // sound ouput file
	FILE * asciiFile; // some ascii debugging file
	FILE * DebugAsciiFile; // another debugging file
#endif

/******************************************************************************
** Function name:		initialize
**
** Descriptions:		initialize and start DAC. In PC-mode, open files
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

#define EMC0	4
#define EMC1	6
#define EMC2	8
#define EMC3	10

#define MATCH0	(1<<0)
#define MATCH1	(1<<1)
#define MATCH2	(1<<2)
#define MATCH3	(1<<3)
void initDac()
{
#ifdef PLATFORM_STMDUINO_STM32F103

#endif
#ifdef PLATFORM_PC
  rawFile = fopen("wav.raw","w"); // raw int16_t data to be played
  asciiFile = fopen("wav.dat","w"); // integer values for gnuplot
  DebugAsciiFile = fopen("debug.dat","w"); // integer values for gnuplot
#endif
#ifdef PLATFORM_LPC1114
	// Enable AHB clock to the GPIO domain.
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	uint8_t match_enable=MATCH0; // pwm using match0

	// disable timer 0
	LPC_TMR16B0->TCR = 0;

	// enable AHB clock for TMR16B0
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);

	// Setup the external match register
	LPC_TMR16B0->EMR = (1<<EMC3)|(1<<EMC2)|(1<<EMC1)|(2<<EMC0)|(1<<3)|(match_enable);

	// If match1 is enabled, set the output
	//LPC_IOCON->PIO1_10          &= ~0x07;
	//LPC_IOCON->PIO1_10          |= 0x02;		// Timer1_16 MAT1
	LPC_IOCON->PIO0_8          &= ~0x07;
	LPC_IOCON->PIO0_8          |= 0x02;		// Timer0_16 MAT0

	// Enable the selected PWM and enable Match3
	LPC_TMR16B0->PWMC = (1<<3)|(MATCH0);

	LPC_TMR16B0->MR3 	= PWM_RESULUTION;
	// set pwm value to 50% duty cycle ( or any other value )
	LPC_TMR16B0->MR0	= PWM_RESULUTION/2;

	// Set match control register
	// MR1I: interrupt when TC matches MR1	// MR1I: bit 3
	//LPC_TMR16B0->MCR = 1<<10 | 1<<3; // reset on MR3 and interrupt on MR1
	// MR0I: interrupt when TC matches MR0	// MR0I: bit 0
	LPC_TMR16B0->MCR = 1<<10 | 1<<0; // reset on MR3 and interrupt on MR0
	// enable interrupt
	NVIC_EnableIRQ(TIMER_16_0_IRQn);

	// enable timer 1
	LPC_TMR16B0->TCR = 1;
#endif
}




/******************************************************************************
** Function name:		stopDac
**
** Descriptions:		In PC-mode all files will be closed
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void stopDac()
{
#ifdef PLATFORM_STMDUINO_STM32F103
  //  audioDAC1_STM32F103(value);
#endif
#ifdef PLATFORM_PC
	fclose(rawFile);
	fclose(asciiFile);
	fclose(DebugAsciiFile);
	printf("file closed\n\r");
#endif
}

/******************************************************************************
** Function name:		audioDAC1
**
** Descriptions:		scaling function for audio output
** 						This routine is meant to be adapted to various DACs
**
** parameters:			value: 0..65535
** Returned value:		None
**
******************************************************************************/
#define MAXSAMPLESINFILE 256
void audioDAC(uint16_t value)
{
  #ifdef PLATFORM_STMDUINO_STM32F103
    audioDAC1_STM32F103(value);
  #endif
	#ifdef PLATFORM_LPC1114
		uint32_t wert;
		// value is in the range of 0..65535
		wert=((uint32_t )value*PWM_RESULUTION)>>16; // the resolution may be only 10 bit
		// write match register 0 to control pwm period
		LPC_TMR16B0->MR0 = wert;
	#endif

	#ifdef PLATFORM_PC
		static int n=0;
		int16_t y;
		int32_t temp;
		temp=value; 			// extend to 32 bit
		temp=temp-0x08000; 		// subtract DC offset
		y=temp; 				// convert to int16_t

		// write raw file data
		fputc(y&0xFF,rawFile);
		fputc((y>>8)&0xFF,rawFile);

		if(n%SAMPLEFREQUENCY==0)
		{
			printf("%d seconds written\n\r",n/SAMPLEFREQUENCY);
			//printf("%d\n",y);
			//fprintf(asciiFile,"%d\n",y);
		}
		n++;
	#endif
}
void DebugPrint(int32_t wert)
{
#ifdef PLATFORM_PC
	fprintf(DebugAsciiFile,"%d\n",wert);
#endif
}

/****************************************************************************
 *
 *   (c) 2013 Christoph Haberer, christoph(at)roboterclub-freiburg.de
 *
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License,                *
 *   If you extend the program please maintain the list of authors.        *
 *   If you want to use this software for commercial purposes and you      *
 *   don't want to make it open source, please contact the authors for     *
 *   licensing.                                                            *
 ***************************************************************************/

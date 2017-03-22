/*

  sigma delta modulator experiments

  sin wave generator controlled by analog input ( poti )


  ************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  ********************* list of outhors **********************************

  v0.1   2.6.2013  C. -H-A-B-E-R-E-R-  initial version
  v0.2  22.3.2017  C. -H-A-B-E-R-E-R-  addapted and improved for STM32F103 ( Blue Pill )

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something

*/

#include "integer_sin.h"
#include "GPIOport.h"

// pin for sound out
PB_12 FASTSOUNDPIN;

void setup() 
{
  // initialize the digital pin as an output.
  // pinMode(LOUDSPEAKER, OUTPUT);
  FASTSOUNDPIN.pinMode(OUTPUT);

}

#define MAXVALUE 255
#define SONGLEN 256

// pitch in Hz
// duration in ms
void sound(uint16_t pitch, uint16_t duration)
{
  uint32_t phaseDelta = 1070UL * 10UL;
  int16_t  integrator = 0;
  uint16_t  sollwert = 64;
  uint8_t  oldValue = 0;
  static uint32_t phase = 0;
  uint16_t timer;
  uint16_t index = 0;
  uint8_t n;

  timer = duration * 25 * 4;

  phaseDelta = 36UL * pitch;
  const uint8_t *p;

  FASTSOUNDPIN.pinMode(OUTPUT);
  while (timer != 0)
  {
    // DDS ( direct digital synthesis )
    index = phase >> 16;
    sollwert=sin_int8(index&0xFF)+128;
    phase += phaseDelta;
    timer--;

    // sigma delta DAC, hold the DAC value for n-steps constant
    //FASTSOUNDPIN.pinMode(OUTPUT);
    for (n = 0; n < 40; n++)
    {
      integrator += sollwert - oldValue;
      if (integrator > 0)
      {
        oldValue = MAXVALUE;
        fastWrite(FASTSOUNDPIN, 1);
      }
      else
      {
        oldValue = 0;
        fastWrite(FASTSOUNDPIN, 0);
      }
    }
    //FASTSOUNDPIN.pinMode(INPUT); // turn off pull up
  }
  FASTSOUNDPIN.pinMode(INPUT); // turn off pull up
}


void loop() 
{
  uint16_t f = analogRead(0);
  sound( f*4, 100);
}


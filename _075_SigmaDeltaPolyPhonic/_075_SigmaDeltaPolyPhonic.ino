/*

  sigma delta modulator experiments

  different waveforms controlled by analog input ( poti )

  https://github.com/ChrisMicro/BluePillSound
  
  ************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  ********************* list of outhors **********************************

  v0.2  25.3.2017  C. -H-A-B-E-R-E-R-  addapted and improved for STM32F103 ( Blue Pill )

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something

*/
#include "integer_sin.h"
#include "GPIOport.h"


// pin for sound out
PB_12 FASTSOUNDPIN;

void sigmaDeltaOscillatorBegin()
{
  FASTSOUNDPIN.pinMode(OUTPUT);
}

#define MAXDACVALUE 0xFFFF
#define SONGLEN 256

uint16_t WaveFormBuffer[SONGLEN];

#define SINUS 0
#define TRIANGLE 1
#define SAWTOOTH 2
#define RECTANGLE 3
#define NOISE 4


void initWaveForm(uint8_t wavetype)
{
  uint16_t n;
  if (wavetype == SINUS)
  {
    for(n=0;n<SONGLEN;n++) WaveFormBuffer[n] = ( (int32_t) sin_int8(n&0xFF)+128 )*MAXDACVALUE/2/128;
    //for (n = 0; n < SONGLEN; n++) WaveFormBuffer[n] = (sin(2 * PI * 1 / SONGLEN * n) + 1) * MAXDACVALUE / 2;
  }
  if (wavetype == TRIANGLE)
  {
    for (n = 0; n < SONGLEN; n++)
    {
      if (n<128)WaveFormBuffer[n] = n<<9;
      else WaveFormBuffer[n] = MAXDACVALUE-(n<<9) ;
    }
  }

  if (wavetype == RECTANGLE)
  {
     for (n = 0; n < SONGLEN; n++)
     {
       if (n<128)WaveFormBuffer[n] = 0;
       else WaveFormBuffer[n] = MAXDACVALUE ;
     }
  }
  if (wavetype == SAWTOOTH)
  {
    for (n = 0; n < SONGLEN; n++) WaveFormBuffer[n] = n << 8;
  }
  if (wavetype == NOISE)
  {
    for (n = 0; n < SONGLEN; n++) WaveFormBuffer[n] = random(0xFFFF);
  }


}
// pitch in Hz
// duration in ms
void oscillator(uint16_t pitch, uint16_t pitch1, uint16_t duration)
{
  static uint32_t  phase = 0;
  uint32_t         phaseDelta = 1070UL * 10UL;

  static uint32_t  phase1 = 0;
  uint32_t         phaseDelta1 = 1070UL * 10UL;
  
  int_fast32_t          integrator = 0;
  uint_fast32_t         sollwert = 128;
  uint_fast32_t         oldValue = 0;
  uint_fast32_t         timer;
  uint_fast32_t         n;

  timer = duration * 25 * 4;

  phaseDelta = 36UL * pitch;
  phaseDelta1 = 36UL * pitch1;
  const uint8_t *p;

  FASTSOUNDPIN.pinMode(OUTPUT);
  while (timer != 0)
  {
    // DDS ( direct digital synthesis )
    
    sollwert = WaveFormBuffer[(phase >> 16) & 0xFF]+WaveFormBuffer[(phase1 >> 16) & 0xFF];
    sollwert = sollwert>>1;
    phase    += phaseDelta;
    phase1   += phaseDelta1;

    timer--;

    // sigma delta DAC, hold the DAC value for n-steps constant
    for (n = 0; n < 40; n++)
    {
      integrator += sollwert - oldValue;

      if (integrator > 0)
      {
        oldValue = MAXDACVALUE;
        fastWrite(FASTSOUNDPIN, 1);
      }
      else
      {
        oldValue = 0;
        fastWrite(FASTSOUNDPIN, 0);
      }
    }

  }
  FASTSOUNDPIN.pinMode(INPUT); // turn off pull up
}

void setup()
{
  initWaveForm(SINUS);
  //initWaveForm(SAWTOOTH);
  //initWaveForm(NOISE);
  //initWaveForm(TRIANGLE);
}

uint32_t Counter=0;
uint8_t WForm=0;
void loop()
{
  uint16_t f = analogRead(0);
  oscillator( f*4, f*4 + 100, 100);
  Counter++;
  if(Counter>30)
  {
    WForm++;
    if(WForm>NOISE) WForm=0;
    initWaveForm(WForm);
  
    Counter=0;
  }  
  
}


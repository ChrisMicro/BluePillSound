/*

  sigma delta modulator experiments

  playind sound from a buffer.


  ************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  ********************* list of outhors **********************************

  v0.1   2.6.2013  C. -H-A-B-E-R-E-R-  initial version
  v0.2  24.3.2017  C. -H-A-B-E-R-E-R-  sampling frequency adjustment added

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something

*/

#include "integer_sin.h"
#include "speakerpin.h"

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
    sollwert = sin_int8(index & 0xFF) + 128;
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

#define WAVEBUFFERLENGTH 10000
uint8_t WaveBuffer[WAVEBUFFERLENGTH];
uint32_t CorrectionFactorSamplingRate = 400; // CPU execution speed dependent

// sigma delta buffer sound player
void playSound(uint8_t *samplesBuffer, uint32_t bufferLength)
{
  int16_t  integrator =  0;
  uint16_t sollwert   = 128;
  uint8_t  oldValue   =  0;

  uint32_t n, k;

  FASTSOUNDPIN.pinMode(OUTPUT);
  for (n = 0; n < bufferLength; n++)
  {
    sollwert = WaveBuffer[n];

    // sigma delta DAC, hold the DAC value for n-steps constant
    for (k = 0; k < CorrectionFactorSamplingRate; k++)
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
  }
  FASTSOUNDPIN.pinMode(INPUT); // turn off pull up
}

#define INITTESTLENGTH 1000 // must not be < 4000 to prevent uint32 overflow

uint32_t SamplingFrequency_Hz;

// calibrate player speed related to CPU speed
void initSoundPlayer(uint32_t samplingFrequency_Hz)
{
  int32_t n;
  SamplingFrequency_Hz = samplingFrequency_Hz;

  for (n = 0; n < INITTESTLENGTH; n++) WaveBuffer[n] = 0; // clear buffer
  
  delay(100);
  
  uint32_t startTime = micros();
  playSound(WaveBuffer, INITTESTLENGTH);
  uint32_t stopTime = micros();

  uint32_t duration_us             = stopTime - startTime;
  uint32_t measuredSamplingRate_Hz = INITTESTLENGTH * 1000000UL / duration_us;

  uint32_t adustmentRelation_x1000 = measuredSamplingRate_Hz * 1000 / samplingFrequency_Hz;

  // adjust factor
  CorrectionFactorSamplingRate = CorrectionFactorSamplingRate * adustmentRelation_x1000 / 1000;

}

#define SAMPLINGFREQUENCY_HZ 22100

void calcSound()
{
  int32_t n;
  float f_Hz = 440;
  for (n = 0; n < WAVEBUFFERLENGTH; n++)
  {
    WaveBuffer[n] = ( sin(2 * PI * f_Hz / SamplingFrequency_Hz * n) + 1) * 127;
  }

}

void setup()
{
  FASTSOUNDPIN.pinMode(OUTPUT);
  initSoundPlayer(22050);
}

void loop()
{

  calcSound();
  playSound(WaveBuffer, WAVEBUFFERLENGTH);

  //sound(1000,1000);
}


/*

  sigma delta modulator experiments

  using the speech synthesizer from Peter Knight to say numbers

  ************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  ********************* list of outhors **********************************

  v0.1   2.6.2013  C. -H-A-B-E-R-E-R-  initial version
  v0.2  24.3.2017  C. -H-A-B-E-R-E-R-  using speech snythesizer 
 

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something

*/

#include "speakerpin.h"
#include "talkie.h"
#include "voltmeter.h"

#define MAXVALUE 255

#define WAVEBUFFERLENGTH 15000

uint8_t WaveBuffer[WAVEBUFFERLENGTH];
uint32_t CorrectionFactorSamplingRate=400; // CPU execution speed dependent

// sigma delta buffer sound player
void playSound(uint8_t *samplesBuffer, uint32_t bufferLength)
{
  int16_t  integrator =  0;
  uint16_t sollwert   = 128;
  uint8_t  oldValue   =  0;
 
  uint32_t n,k;
  
  pinMode(FASTSOUNDPIN,OUTPUT);
  for(n=0;n<bufferLength;n++)
  {
    sollwert=WaveBuffer[n];
    
    // sigma delta DAC, hold the DAC value for n-steps constant
    for (k = 0; k < CorrectionFactorSamplingRate; k++)
    {
      integrator += sollwert - oldValue;
      if (integrator > 0)
      {
        oldValue = MAXVALUE;
        SPEAKERPINHIGH;
      }
      else
      {
        oldValue = 0;
        SPEAKERPINLOW;
      }
    }
  }
  pinMode(FASTSOUNDPIN,INPUT);
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
  //Serial.print("measuredSamplingRate_Hz "); Serial.println(measuredSamplingRate_Hz);

  uint32_t adustmentRelation_x1000 = measuredSamplingRate_Hz * 1000 / samplingFrequency_Hz;

  // adjust factor
  CorrectionFactorSamplingRate = CorrectionFactorSamplingRate * adustmentRelation_x1000 / 1000;
}


void calcSineWave()
{
  int32_t n;
  float f_Hz=440;
  for(n=0;n<WAVEBUFFERLENGTH;n++)
  {
    WaveBuffer[n]=( sin(2*PI*f_Hz/SamplingFrequency_Hz*n)+1)*80;
  }
}

void setup() 
{
  pinMode(FASTSOUNDPIN,OUTPUT);
  initSoundPlayer(FS); // FS=8000Hz
  setWaveBuffer(WaveBuffer,WAVEBUFFERLENGTH);
}

int Counter=0;
void loop() 
{
  uint32_t n;
  calcSineWave();
  playSound(WaveBuffer,WAVEBUFFERLENGTH);
  delay(2000);

  // clear buffer
  for(n=0;n<WAVEBUFFERLENGTH;n++)WaveBuffer[n]=0;
  sayNumber(Counter++);
  if(Counter>11)Counter=0;
  
  playSound(WaveBuffer,WAVEBUFFERLENGTH);
  delay(2000);

}


/*
  Testing the Deskripator Synth Library written for a STM32F4 on the BluePill

  The STM32F4 is much faster than the BluePill and has floating point support.
  Therefore the BluePill is to slow for this library but it can show some
  basic sounds and effects( with minor quality ).

  The Dekrispoatro_V2 was found here:
  https://github.com/MrBlueXav/Dekrispator_v2

  Sigma Delta DAC example using DMA and double buffering

  Generate a sine wave, fill a buffer and play the samples in a timer driven interrupt routine.

  Hardware: 
    Pitch Poti on analog input 0
    Phaser Poti on analog input 1
    Speaker see below

  Library dependency: you need the libmaple hardware timer
  http://docs.leaflabs.com/docs.leaflabs.com/index.html

  This is just an experiment using a buffer filled with the sigmal delta principle
  and output it by the DMA to port PB0.

  The DMA is programmed bare metal without any wrapper functions.

  https://github.com/ChrisMicro/BluePillSound

  ************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  ********************* list of outhors **********************************

  v0.1  4.4.2017  C. -H-A-B-E-R-E-R-  initial version
  v0.2  5.4.2017  C. -H-A-B-E-R-E-R-  higher sampling rate due to 32bit table
  v0.3  6.4.2017  C. -H-A-B-E-R-E-R-  interrupt driven sine wave oscillator
  v0.4 18.4.2017  C. -H-A-B-E-R-E-R-  buffered player
  v0.4 18.4.2017  C. -H-A-B-E-R-E-R-  Dekrispator_v2 parts added
  

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something

*/

#include "stm32f103.h"

#include "oscillators.h"
#include "phaser.h"
#include "resonantFilter.h"

#define DACSAMPLINGRATE_HZ    SAMPLERATE
#define TIMER_INTTERUPT_US    1000000UL / DACSAMPLINGRATE_HZ

HardwareTimer timer(2);

#define DMABUFFERLENGTH 100

volatile uint32_t DmaBuffer[DMABUFFERLENGTH];

// good DMA article:
// https://vjordan.info/log/fpga/stm32-bare-metal-start-up-and-real-bit-banging-speed.html
void startDMA()
{
  RCC->AHBENR |= RCC_AHBENR_DMA1EN; // enable DMA

  // channel 1: mem:8bit -> peri:8bit
  DMA1_Channel1->CNDTR = DMABUFFERLENGTH;
  DMA1_Channel1->CMAR = (uint32_t)DmaBuffer;
  DMA1_Channel1->CPAR = (uint32_t) & (GPIOB->ODR);
  DMA1_Channel1->CCR = 0;
  DMA1_Channel1->CCR = DMA_CCR1_MEM2MEM | DMA_CCR1_PL | DMA_CCR1_MINC | DMA_CCR1_CIRC | DMA_CCR1_DIR | DMA_CCR1_EN;
}

// maple timer
// http://docs.leaflabs.com/docs.leaflabs.com/index.html
void startTimer()
{
  // Pause the timer while we're configuring it
  timer.pause();

  // Set up period
  timer.setPeriod(TIMER_INTTERUPT_US); // in microseconds

  // Set up an interrupt on channel 1
  timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
  timer.attachCompare1Interrupt(DACsampleRateHandler);

  // Refresh the timer's count, prescale, and overflow
  timer.refresh();

  // Start the timer counting
  timer.resume();
}

uint16_t SineTable[256];

#define SPEAKERPIN PB0            // speaker pin, be sure that it corresponds to the DMA pin setup in the next line
#define PIN_PB 0                  // speaker pin, you can choose 0..7, 0 means PB0, 1 means PB1 ...
#define PORTB_PINMASK (1<<PIN_PB)

uint32_t CoarseSigmaDeltaTable[] = {0x00000000 * PORTB_PINMASK, 0x01000000 * PORTB_PINMASK, 0x01000100 * PORTB_PINMASK, 0x01010100 * PORTB_PINMASK, 0x01010101 * PORTB_PINMASK};

// Sigma Delta DAC
// input value 10 bit 0..1023
void writeDac( uint32_t sollwert )
{
  uint32_t  integrator = 0;
  static uint32_t  oldValue = 0;

  uint32_t n;

  // sigma delta DAC
  for (n = 0; n < DMABUFFERLENGTH / 4; n++)
  {
    integrator += sollwert - oldValue;
    oldValue = integrator & 0b11100000000;
    DmaBuffer[n] = CoarseSigmaDeltaTable[ oldValue >> 8 ];
  }
}

typedef uint16_t buf_t;
#define SOUNDBUFFERLENGTH 256
volatile buf_t SoundBuffer0[SOUNDBUFFERLENGTH];
volatile buf_t SoundBuffer1[SOUNDBUFFERLENGTH];

volatile buf_t * volatile SoundBufferCurrentlyPlaying;

void DACsampleRateHandler()
{
  static uint32_t bufferIndex = 0;

  writeDac(SoundBufferCurrentlyPlaying[bufferIndex++]);

  if ( bufferIndex >= SOUNDBUFFERLENGTH )
  {
    bufferIndex = 0;
    if ( SoundBufferCurrentlyPlaying != SoundBuffer1 )SoundBufferCurrentlyPlaying = SoundBuffer1;
    else                                    SoundBufferCurrentlyPlaying = SoundBuffer0;
  }

}
#define LEDPIN     PC13 // Blue Pill LED
#define INITLED    pinMode(LEDPIN, OUTPUT)

#define LEDON      digitalWrite(LEDPIN, HIGH)
#define LEDOFF     digitalWrite(LEDPIN, LOW)

void toggleLed()
{
  static uint8_t flag = 0;

  if (flag) LEDON;
  else     LEDOFF;

  flag ^= 1;
}

volatile buf_t * volatile SoundBuffer;

volatile float Oscillator1_Frequency_Hz = 440;

extern Oscillator_t op1 ;
extern Oscillator_t op2 ;
extern Oscillator_t op3 ;
extern Oscillator_t op4 ;

extern ResonantFilter   SVFilter;
extern ResonantFilter   SVFilter2;
extern float      filterFreq  ;
extern float      filterFreq2  ;

void setup()
{
  uint32_t n;

  pinMode(PC13, OUTPUT);
  pinMode(SPEAKERPIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("SPI SIGMA DELTA SOUND calculation performance");

  delay(100);
  SoundBufferCurrentlyPlaying = SoundBuffer0;
  SoundBuffer                 = SoundBuffer1;
  startDMA();

  // ** just check the write speed **
  uint32_t startTime = micros();
  for (n = 0; n < 1000; n++) writeDac(0); // dummy write silence
  uint32_t stopTime = micros();

  uint32_t DacSamplingFrequency;
  Serial.print("DAC write time for 1000 samples in us: "); Serial.println(stopTime - startTime);
  DacSamplingFrequency = 1000000UL * 1000 / (stopTime - startTime);
  Serial.print("maximum possible DAC sampling frequency [Hz]: "); Serial.println(DacSamplingFrequency);

  // *********************************

  for (n = 0; n < 256; n++) SineTable[n] = (sin(2 * PI * n / 255) + 1) * 500;

  startTimer();

  osc_init(&op1, 0.8f, 587.f);
  osc_init(&op2, 0.8f, 587.f);
  osc_init(&op3, 0.8f, 587.f);
  osc_init(&op4, 0.8f, 587.f);

  OpSetFreq(&op1, 440.0);

  PhaserInit();
  PhaserRate(0.5);

  //filterFreq = 0.25f;
  //filterFreq2 = 0.25f;

  //SVF_directSetFilterValue(&SVFilter, Ts * 600.f * powf(5000.f / 600.f, frand_a_b(0 , 1)));
  
}

void updateSound()
{
  uint32_t amplitude = 500;

  for (uint32_t n = 0; n < SOUNDBUFFERLENGTH; n++)
  {
    //SoundBuffer[n]=(OpSampleCompute0(&op1)+1)*500; // sine
    //SoundBuffer[n]=(OpSampleCompute3(&op1)+1)*500; // power sine
    //SoundBuffer[n]=(MorphingSaw_SampleCompute(&op1)+1)*500; // morph saw

    //MorphingSaw_SampleCompute(&op1)
    //SoundBuffer[n] = (waveCompute(ACC_SINE,440)+1)*500;
    //SoundBuffer[n] = (waveCompute(MORPH_SAW,Oscillator1_Frequency_Hz)+1)*500;
    //SoundBuffer[n] = (waveCompute(NOISE,Oscillator1_Frequency_Hz)+1)*500;
    //SoundBuffer[n] = (waveCompute(CHORD15, Oscillator1_Frequency_Hz) + 1) * 500;
    //SoundBuffer[n] = (waveCompute(CHORD135, Oscillator1_Frequency_Hz) + 1) * 500;
    //SoundBuffer[n] = (waveCompute(CHORD13min5, Oscillator1_Frequency_Hz) + 1) * 500;
    //SoundBuffer[n] = (waveCompute(SPLIT, Oscillator1_Frequency_Hz) + 1) * 500;
    float value;
    //value=waveCompute(ACC_SINE,Oscillator1_Frequency_Hz);
    //value=OpSampleCompute0(&op1); // sine wave
    value=MorphingSaw_SampleCompute(&op1);
    //value=waveCompute(NOISE,Oscillator1_Frequency_Hz);

    //value=waveCompute(CHORD135, Oscillator1_Frequency_Hz);
    //value=SVF_calcSample(&SVFilter, value);
    value=Phaser_compute(value);
    SoundBuffer[n]=(value+1)*500; 

// !!! if you hava outliers, decrease the SAMPLERATE constant in "CONSTANTS.h" to reduce calculation load !!!! 
  }
}

void loop()
{
  SoundBuffer = SoundBufferCurrentlyPlaying;
  while (SoundBuffer == SoundBufferCurrentlyPlaying); // wait until buffer is emtpy

  LEDON; // debug led for processing time measuremt with oscilloscope
  updateSound();
  LEDOFF;

  Oscillator1_Frequency_Hz = analogRead(0);
  OpSetFreq(&op1, Oscillator1_Frequency_Hz);
  PhaserRate(analogRead(1)/1000.0);
  //OpSetFreq(&op2, Oscillator1_Frequency_Hz*0.2);
  //OpSetFreq(&op2, Oscillator1_Frequency_Hz*0.1+10);
}

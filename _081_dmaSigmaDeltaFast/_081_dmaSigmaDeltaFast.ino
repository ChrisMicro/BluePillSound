/*

  Sigma Delta DAC example using DMA.

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

  v0.1 4.4.2017  C. -H-A-B-E-R-E-R-  initial version
  v0.2 5.4.2017  C. -H-A-B-E-R-E-R-  higher sampling rate due to 32bit table

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something

*/

#include "stm32f103.h"

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


#define PIN_PB 0 // speaker pin, you can choose 0..7, don't forgett to adjust the pin initializaion in setup when you change this value
#define PORTB_PINMASK (1<<PIN_PB) 

uint32_t CoarseSigmaDeltaTable[]={0x00000000*PORTB_PINMASK,0x01000000*PORTB_PINMASK,0x01000100*PORTB_PINMASK,0x01010100*PORTB_PINMASK,0x01010101*PORTB_PINMASK};

// Sigma Delta DAC
// input value 10 bit 0..1023
void writeDac( uint32_t sollwert )
{
  uint32_t  integrator = 0;
  static uint32_t  oldValue = 0;

  uint32_t n;
  
  // sigma delta DAC
  for (n = 0; n < DMABUFFERLENGTH/4; n++)
  {
    integrator += sollwert - oldValue;
    oldValue = integrator & 0b11100000000;
    DmaBuffer[n] = CoarseSigmaDeltaTable[ oldValue>>8 ];
  }
}

uint32_t DacSamplingFrequency;

uint16_t SineTable[256];

void setup()
{
  pinMode(PC13, OUTPUT);
  pinMode(PB0, OUTPUT); // speaker pin
  Serial.begin(115200);
  Serial.println("hello");

  delay(100);

  startDMA();

  uint32_t n;

  uint32_t startTime=micros();
  for (n = 0; n < 1000; n++) writeDac(0);
  uint32_t stopTime=micros();
  
  Serial.print("DAC write time for 1000 samples in us: ");Serial.println(stopTime-startTime);
  DacSamplingFrequency = 1000000UL*1000 / (stopTime-startTime);
  Serial.print("DAC sampling frequency [Hz]: ");Serial.println(DacSamplingFrequency);

  for(n=0;n<256;n++) SineTable[n]= (sin(2*PI*n/255)+1)*500;

}

uint16_t Phase;

void loop()
{
  uint32_t f_Hz;
  f_Hz=440;
  
  //f_Hz=analogRead(0);

  Phase+=256*256*f_Hz/DacSamplingFrequency;
  writeDac(SineTable[Phase>>8]);

}

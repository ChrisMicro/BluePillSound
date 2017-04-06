/*

  Sigma Delta DAC example using DMA.

  Generate a sine wave in a timer driven interrupt routine.

  Hardware: Poti on analog input 0
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

  v0.1 4.4.2017  C. -H-A-B-E-R-E-R-  initial version
  v0.2 5.4.2017  C. -H-A-B-E-R-E-R-  higher sampling rate due to 32bit table
  v0.3 6.4.2017  C. -H-A-B-E-R-E-R-  interrupt driven sine wave oscillator

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something

*/

#include "stm32f103.h"

#define DACSAMPLINGRATE_HZ    20000
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

volatile uint32_t Oscillator1_Frequency_Hz;

void DACsampleRateHandler()
{
  static uint16_t Phase;
  Phase += 256 * 256 * Oscillator1_Frequency_Hz / DACSAMPLINGRATE_HZ;
  writeDac(SineTable[Phase >> 8]);
}

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

void setup()
{
  uint32_t n;
  
  pinMode(PC13, OUTPUT);
  pinMode(SPEAKERPIN, OUTPUT); 
  Serial.begin(115200);
  Serial.println("hello");

  delay(100);

  startDMA();

  // ** just check the write speed **
  uint32_t startTime = micros();
  for (n = 0; n < 1000; n++) writeDac(0);
  uint32_t stopTime = micros();

  uint32_t DacSamplingFrequency;
  Serial.print("DAC write time for 1000 samples in us: "); Serial.println(stopTime - startTime);
  DacSamplingFrequency = 1000000UL * 1000 / (stopTime - startTime);
  Serial.print("maximum possible DAC sampling frequency [Hz]: "); Serial.println(DacSamplingFrequency);

  // *********************************

  for (n = 0; n < 256; n++) SineTable[n] = (sin(2 * PI * n / 255) + 1) * 500;

  startTimer();
}

void loop()
{
  Oscillator1_Frequency_Hz = analogRead(0);
  Serial.println(Oscillator1_Frequency_Hz);
  delay(10);
}

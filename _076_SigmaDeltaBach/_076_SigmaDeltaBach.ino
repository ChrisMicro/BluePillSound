// !! to setup a specific platform, take a look into the file "platform.h" !!
// at the moment very experimental for the STM32F103
/****************************************************************************
*
 *   Project: Audio Sound Generation
 *
 *   Description:
 *     This file generates the sounds of two bells with the help of 5
 *     5 DDS sin wave generators
 *     This program has two possible configuration options:
 *
 *     - PLATFORM_PC    ( PC simulation )
 *     or
 *     - PLATFORM_LPC1114 ( ARM LPC1114 )
 *     or
 *     - STM32F103 ( Blue Pill Arduino ) 
 *     At the moment this one uses an sigma delta dac. 
 *     Unfortunately this introduces clicks due to processing time needed.
 *
 *     which can be set up in "platform.h"
 *
 *     The PLATFORM_PC is meant for developing purposes. You can develop the
 *     sound on the PC. The debugging is faster on the PC than on the ARM
 *     system. The program for PC outputs two files:
 *
 *     - wav.raw  ( int16 raw data )
 *     - wav.dat
 *     ( ascii data for graphical representation, only the first 512 values )
 *
 ****************************************************************************
 *
 *   1. ARM-Hardware LPC1114Xpresso ( ARM M0 ) or LPC1114FN/102 28-Pin-Dip
 *   Compiler: GCC
 *   SysClk: 48Mhz
 *
 *   Pins
 *   ====
 *   pin 1.10 : PWM output MATCH0
 *   pin 0.7  : blinking LED
 *   
 *   ==========================================================================
 *   2. ARM-Hardware Arduino STM32F103 Blue Pill
 *   Compiler: GCC
 *   SysClk: 72Mhz
 *
 *   Pins
 *   ====
 *   PB12 sound output
 *
 *****************************************************************************/
/*****************************************************************************

  date  authors         version   comment
  ======  ======================  =======   ==============================
  2013
  Jan.13 - Feb.17  C. -H-A-B-E-R-E-R- initial versions  
  26.3.2017        C. -H-A-B-E-R-E-R- port to STM32F103
 
  Versions Jan13-Feb17:

  V1.0
  - sin wav generator
  - MsTimer
  V1.1
  - 5 channel DDS
  V1.2
  - simulation on PC: sound can be output as raw-file
    change the #define PLATFOMR_PC in platform.h
  V1.3
  - attack, decay, sustain, realease waveform added
  - playtune sequencer added
  V1.4
  - square and triangle waveform added ( thanks to Ale )

  V1.5
  This version is made for the 28-PIN-DIP chip ( LPC1114FN102 )
  - pwm output change to pin 0.8
    timer 16b0 and match 0 used for PWM instead of match1
    reason: the LPC1114FN DIP has no P1.10 like the LPCXpresso
  - control LED inplemented

****************************************************************************/

#include "sequencer.h"


#ifdef PLATFORM_PC
  #include <stdlib.h>
  #include <stdio.h>
#endif


#ifdef PLATFORM_STMDUINO_STM32F103

#include "GPIOport.h"

// pin for sound out
PB_12 FASTSOUNDPIN;

//***************************************  sigma delta player ****************************************
#define MAXDACVALUE 0xFF

#define WAVEBUFFERLENGTH 16000
uint8_t WaveBuffer[WAVEBUFFERLENGTH];
uint32_t CorrectionFactorSamplingRate = 308; // CPU execution speed dependent

// sigma delta buffer sound player
void playSound(uint8_t *samplesBuffer, uint32_t bufferLength)
{
  int32_t          integrator = 0;
  uint32_t         sollwert = 128;
  uint32_t         oldValue = 0;
  uint32_t         index = 0;
  uint32_t         n;

  FASTSOUNDPIN.pinMode(OUTPUT);
  while (index<bufferLength)
  {

    sollwert = WaveBuffer[index++];

    // sigma delta DAC, hold the DAC value for n-steps constant
    for (n = 0; n < CorrectionFactorSamplingRate; n++)
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
//*************************************** end sigma delta player ****************************************
void setup()
{
  Serial.begin(115200);
  Serial.println("sound generator");   
  initWaveforms();
  initDac();


  MsTimerForSound=2000;
}

void loop()
{
  int n;

  double duration = PC_SIMULATION_DURATION_IN_SECONDS; // duration in seconds
  for(n=0;n<(int)SAMPLEFREQUENCY*duration;n++) TIMER16_1_IRQHandler();

  stopDac();
  Serial.println("all output stopped");  
  while(1);
}
#endif

/******************************************************************************
** Function name:   TIMER16_1_IRQHandler
**
** Descriptions:    Timer/Counter 1 interrupt handler
**
** parameters:      None
** Returned value:    None
**
******************************************************************************/
#ifdef  PLATFORM_STMDUINO_STM32F103
void TIMER16_1_IRQHandler(void)
{
  static uint16_t counter=0;

  counter++;
  if(counter>SAMPLEFREQUENCY/1000)
  {
    ms_loop(); // call is PLATFORM_PC specific because there are no interrupts
    counter=0;
    MsTimerForSound++; // increase millisecond program timer
  }
  waveGenerators();
  //Serial.println(MsTimerForSound);
  //delay(10);
}


uint32_t count=0;
void playNow()
{
    playSound(WaveBuffer,count);
    count=0;  
}

void audioDAC1_STM32F103(uint16_t value)
{
  uint16_t temp=value/192;
  WaveBuffer[count++]=temp;
  //Serial.println(temp);
  //delay(1);
  int num=WAVEBUFFERLENGTH;
  if(count>num) playNow();
   
}
#endif

#ifdef PLATFORM_PC 
void TIMER16_1_IRQHandler(void)
{
  static uint16_t counter=0;

  counter++;
  if(counter>SAMPLEFREQUENCY/1000)
  {
    ms_loop(); // call is PLATFORM_PC specific because there are no interrupts
    counter=0;
    MsTimer++; // increase millisecond program timer
  }
  waveGenerators();
}
#endif
#ifdef PLATFORM_LPC1114
void TIMER16_0_IRQHandler(void)
{

  static uint16_t counter=0;
  // match channel 0 interrupt
  if ( LPC_TMR16B0->IR & (1<<0) )
  {
  counter++;
  if(counter>SAMPLEFREQUENCY/1000)
  {
    counter=0;
    MsTimer++; // increase millisecond prgram timer
  }
  // clear interrupt flag of match channel 0
  LPC_TMR16B0->IR = 0x1<<0;

  waveGenerators();
  }
  return;
}
#endif


/******************************************************************************
** Function name:   main
**
** Descriptions:    main starting point
**
** parameters:      None
** Returned value:    None
**
******************************************************************************/
#ifdef PLATFORM_PC

int main(void)
{
  printf("sound generator"); /* prints !!!Hello World!!! */
  initWaveforms();
  initDac();

  int n;
  MsTimer=2000;
  double duration=PC_SIMULATION_DURATION_IN_SECONDS; // duration in seconds
  for(n=0;n<(int)SAMPLEFREQUENCY*duration;n++) TIMER16_1_IRQHandler();

  stopDac();

  return EXIT_SUCCESS;
}
#endif
#ifdef PLATFORM_LPC1114
// definition of LPC1114Xpresso's indicator LED
#define LED_INIT {GPIOSetDir( 0, 7, 1 );}
#define LED_ON {GPIOSetValue( 0, 7, 1 );}
#define LED_OFF {GPIOSetValue( 0, 7, 0 );}
void toogleLed()
{
  static uint8_t flag=1;
  flag^=1;
  if(flag)
  {
    LED_ON;
  }
  else
  {
    LED_OFF;
  }
}
int main(void)
{
  int msCounter=500;
  initWaveforms();
  initDac();
  LED_INIT;

  uint32_t temp;
  MsTimer=2000;

  while(1)
  {
    ms_loop();  // ms loop in wavegeneration module
    while(temp==MsTimer); // wait for one milli second
    temp=MsTimer;
    msCounter--;
    if(msCounter==0)
    {
        toogleLed();
        msCounter=500; // 500ms
    }

  }

  stopDac();
}
#endif
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


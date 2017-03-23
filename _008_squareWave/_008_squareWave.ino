/*
 * 
 * simple square wave by toggling speaker pin
 * 
 *  6.3.2017 ChrisMicro initial version
 * 23.3.2017 ChrisMicro addapted for BluePill
 * 
 */
#define USEFASTPINFUNCTIONS

#ifdef USEFASTPINFUNCTIONS
 #include "speakerpin.h"

#else  // slow Arduino functions
  
  #define SPEAKERPIN          PB12 // Blue Pill SPEAKER
  #define INITSPEAKERPIN      pinMode(SPEAKERPIN, OUTPUT)
  
  #define SPEAKERPINHIGH      digitalWrite(SPEAKERPIN, HIGH)
  #define SPEAKERPINLOW       digitalWrite(SPEAKERPIN, LOW)
 
#endif

void toggleSpeakerPin()
{
  static uint8_t flag=0;

  if(flag) SPEAKERPINHIGH;
  else     SPEAKERPINLOW;

  flag^=1;
}

#define TOGGLESPEAKERPIN toggleSpeakerPin()
  
void playSound(long freq_Hz, long duration_ms)
{
  uint16_t n;
  uint32_t delayTime_us;
  uint32_t counts;

  delayTime_us = 1000000UL / freq_Hz / 2;
  counts = duration_ms * 1000 / delayTime_us;

  for (n = 0; n < counts; n++)
  {
    TOGGLESPEAKERPIN;
    delayMicroseconds(delayTime_us);
  }
}

void setup() 
{
  INITSPEAKERPIN;
}

void loop() 
{
  playSound(440,1000);
  playSound(880,1000);
  
  delay(1000); // wait for a second
}


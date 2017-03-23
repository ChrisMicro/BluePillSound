/*
  Arpeggio
  from http://fonte.me/arduino/#Arpeggio

  Plays arpeggios of chosen chords

  Requires an 8-ohm speaker on digital pin 8

  Copyright (c) 2012 Jeremy Fonte
  This code is released under the MIT license
  http://opensource.org/licenses/MIT
  23.7.2017 ChrisMicro code addapted for Blue Pill 
*/
#include "pitches.h"
#include "speakerpin.h"


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


/*
  uint8_t getButton()

  return value:
  1: left button pressed
  2: right button pressed
  3: both buttons pressed
  ADC values
  no button:512, left:341, right:248, both:200
*/
uint8_t getButton()
{
  uint8_t button = 1;
  // not implemented
  delay (3000);
  return button;
}


void playArp(int notes[]);

// available chord to arpeggiate
//C major
int c[] = {
  NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5
};

//D minor
int d[] = {
  NOTE_D4, NOTE_F4, NOTE_A4, NOTE_D5, NOTE_F5
};

//E minor
int e[] = {
  NOTE_E4, NOTE_G4, NOTE_B4, NOTE_E5, NOTE_G5
};

//F major
int f[] = {
  NOTE_F4, NOTE_A4, NOTE_C5, NOTE_F5, NOTE_A5
};

//G major
int g[] = {
  NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_B5
};

//A minor
int a[] = {
  NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A5, NOTE_C6
};

//B diminished
int b[] = {
  NOTE_B4, NOTE_D5, NOTE_F5, NOTE_B5, NOTE_D6
};

int pauseBetweenNotes = 80;
int noteDuration_ms = 250;

void playArp(int notes[])
{

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote <= 3; thisNote++) {


    //displayBinrayValue(notes[thisNote],pixels.Color(0,15,0));
    //pixels.show(); // This sends the updated pixel color to the hardware.
    playSound( notes[thisNote], noteDuration_ms);
    // setColorAllPixel(0); // pixels off
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:

    delay(pauseBetweenNotes);
    // stop the tone playing:
    //noTone(8);
  }
  for (int thisNote = 4; thisNote >= 1; thisNote--) {
    

    //displayBinrayValue(notes[thisNote]&0x7,pixels.Color(15,0,0));
    //pixels.show(); // This sends the updated pixel color to the hardware.

    playSound( notes[thisNote], noteDuration_ms);
    //setColorAllPixel(0); // pixels off
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    delay(pauseBetweenNotes);

  }
}
// analog read scaled for the 8BitMixTape voltage divider
uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  if (value > 511) value = 511;
  return value * 2;
}

void loop()
{
  uint16_t speedPoti;
  //uint16_t speedPoti = analogReadScaled(POTI_LEFT);
  //speedPoti = analogReadScaled(POTI_RIGHT);
  //pauseBetweenNotes=20;
  //noteDuration_ms = analogReadScaled+20;

  //list any chord sequence here by calling playArp(chord)
  //Only plays chords in the key of C Major/A Minor
  //for example, C Major is playArp(c) and A Minor is playArp(a)
  playArp(c);
  playArp(b);
  playArp(a);
  playArp(g);
// wait for button
    while( getButton()==0);

  /*
    uint16_t p1 = analogRead(POTI_LEFT);
    uint16_t p2 = analogRead(POTI_RIGHT);

    setColorAllPixel(0); // pixels off

    displayBinrayValue( p1 / 4, pixels.Color(0, 15, 0));
    displayBinrayValue( p2 / 4, pixels.Color(0, 0, 15));

    //pixels.setPixelColor(0, 0); // off
    //pixels.setPixelColor(1, 0); // off

    uint8_t x = getButton();
    if (x & 1) pixels.setPixelColor(0, pixels.Color(15, 0, 0));
    if (x & 2) pixels.setPixelColor(1, pixels.Color(15, 0, 0));
    pixels.show(); // This sends the updated pixel color to the hardware.

    if (x == 3)playSound(p1 + 300, 100);*/

}

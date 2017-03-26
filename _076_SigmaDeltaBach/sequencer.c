/****************************************************************************

    Module: sequencer

    Description:

  		This module implements sequencers to play notes.
		The first version uses the platune format from Len Shustek
		( see playtune.c )

*****************************************************************************/

#include "sequencer.h"
#include "playtune.h"

/******************************************************************************
** Function name:		ms_loop
**
** Descriptions:		should be called every ms for update processes
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

#define USE_PLAYTUNE
//#define USE_SIMPLE_TEST_SEQUENCER

#define MAXTIME 200

void ms_loop(void)
{
  static uint8_t bellcount = 0;

#ifdef USE_PLAYTUNE
  uint32_t waitTime = 100;

  if (MsTimerForSound >= waitTime)
  {
    while ((waitTime = sequencer()) == 0); // process sequences until waitTime is not zero
    MsTimerForSound = 0;
  }
#endif

#ifdef USE_SIMPLE_TEST_SEQUENCER
  if (MsTimerForSound >= MAXTIME) // reset bell
  {
    MsTimerForSound = 0;
    //noteOn(0,score1[bellcount],100);
    //if(bellcount==0) ring_bell(freq_bell_2,time_bell_2);
    if (bellcount == 0) noteOn(0, 69, 100);
    if (bellcount == 1) noteOff(0, 69);
    if (bellcount == 4) noteOn(0, 63, 100);
    if (bellcount == 8) noteOff(0, 63);

    //if(bellcount==1) bellOff();
    //if(bellcount==1) ring_bell(freq_bell_2,time_bell_2);
    //organ(440+bellcount*440/8);
    //organ(note2frequency(bellcount));
    //organ(note2frequency(69));

    //if(bellcount==2) ring_bell(freq_bell_1,time_bell_1);
    bellcount++;
    //if(bellcount==10) bellcount=0;
  }
#endif
  updateEnvelope();
  //voiceOff();
}

/******************************************************************************
** Function name:		sequencer
**
** Descriptions:		playtune sequencer
** 						for a detailed description see "playtune.c"
**
** parameters:			None
** Returned value:		time to next tone to be played in milli seconds
**
******************************************************************************/

//#define MIDIOCTAVE 12
enum state_e { COMMAND, KK, VV, STOP};

uint16_t sequencer(void)
{
  static uint32_t midi_fp = 0;
  uint8_t value;
  enum state_e state = COMMAND;
  uint8_t status, n, kk = 0, vv; // n=channel, kk=note, vv=velocity
  uint16_t time;

  vv = 100;
  time = 0;
  value = score1[midi_fp++]; // read byte from playtune file ( midi like )
  //printf("value: %x ",value);
  if (state == COMMAND)
  {
    if ((value & 0x80) == 0)
    {
      time = (uint16_t)(value & 0xF7) << 8;
      time += score1[midi_fp++];
      //printf("wait time [ms]%d\n\r",time);
    }

    status = value & 0xF0;

    n = value & 0x0F;

    switch (status)
    {
      case 0x80:// note off
        {
          noteOff(n, kk);
          //printf("noteOff %d\n\r",n);
        }; break;
      case 0x90: // note on
        {
          kk = score1[midi_fp++];
          noteOn(n, kk, vv);
          //printf("noteOn %d , note:%d\n\r",n,kk);
        }; break;
      case 0xF0:// stop playing
        {
          state = STOP;
          //printf("STOP");
        }; break;
      case 0xE0: // start playing again
        {
          midi_fp = 0;
          //printf("play again");
        }; break;
    }
  }
  //getchar();
  return time;
}
/******************************************************************************
** Function name:		noteON
**
** Descriptions:		turn on an oscillator
**
** parameters:			channel, note, velocity ( see midi documentation )
** Returned value:		None
**
******************************************************************************/
// tbd: do not ignore velocity
void noteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{


  //WaveGenerator *wave;

  // set sin wave generators
  if (channel < NUMBER_OF_OSCILLATORS)
  {
    //Wave[channel]=&Wave[channel][channel];

    Wave[channel].freq_coefficient = 0x01000000L / SAMPLEFREQUENCY * 256 * note2frequency(note);

    Wave[channel].attack_ms = 50;
    Wave[channel].decay_ms = 60;
    Wave[channel].release_ms = 400; // 400 ms
    Wave[channel].sustain_level = MAXLEVEL / 4 * 3;
    Wave[channel].attackdecay_flag = TRUE;

    if (velocity > 0)	Wave[channel].noteOn = TRUE;
    else Wave[channel].noteOn = FALSE;
  }
#ifdef PLATFORM_STMDUINO_STM32F103
  playNow();
#endif

}
// Kammerton A:midiNote 69 ( A4)
uint32_t note2frequency(uint8_t midiNote)
{
  if (midiNote > 127)midiNote = 0; // error
  return tune_frequencies2_PGM[midiNote] / 2;
}
/******************************************************************************
** Function name:		noteOff
**
** Descriptions:		turn off an oscillator
**
** parameters:			channel, note ( see midi documentation )
** Returned value:		None
**
******************************************************************************/
// tbd: do not ignore note
void noteOff(uint8_t channel, uint8_t note)
{


  //WaveGenerator *wave;

  // set sin wave generators
  if (channel < NUMBER_OF_OSCILLATORS)
  {
    //wave=&Wave[channel];

    Wave[channel].noteOn = FALSE;
  }
}
/****************************************************************************
 *
 *   (c) 2013 ChrisMicro
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

/****************************************************************************
*
*   Module: waveform generator
*
*   Description:
*
*     This module contains the waveform generator and the envelope generator
*     and some instruments and sound which use more than one oscillator
*
*
*****************************************************************************/
#include "platform.h"
#include "wavegenerator.h"
#include "sequencer.h"

volatile WaveGenerator Wave[NUMBER_OF_OSCILLATORS];
volatile uint32_t MsTimerForSound; // millisecond timer, increased in interrupt routine
  
/******************************************************************************
** Function name:		ring_bell
**
** Descriptions:		Initializes the frequency and time coefficients of the
** 						DDS oscillators.
** 						With this routine you can start a bell sound
**
** parameters:			pointers to the frequency and time array
** Returned value:		None
**
******************************************************************************/
// deep bell
uint16_t freq_bell_1[]={125,130,250,375,525,675}; // frequency in Hz
uint16_t time_bell_1[]={3000*2,3000*2,2376*2,2178*2,1980*2,1307*2}; // time in ms
// tibetanische Gebetsglocke
uint16_t freq_bell_2[]={1314, 2362, 3306, 3928, 4993, 5994, 6619, 7671, 8413, 9305, 9912}; // frequency in Hz
uint16_t time_bell_2[]={7000, 4500, 1940, 570, 580, 570, 140, 50, 250, 170, 740}; // time in ms

void ring_bell(uint16_t *freq,uint16_t *time)
{
	uint8_t n;

	//WaveGenerator *wave;

	//wave=Wave;

	// set sin wave generators
	for(n=0;n<NUMBER_OF_OSCILLATORS;n++)
	{
		Wave[n].phase=0;
		Wave[n].envelope=0;
		Wave[n].freq_coefficient=0x01000000L/SAMPLEFREQUENCY*256*freq[n];

	    Wave[n].attack_ms=1;
	    Wave[n].decay_ms=20;
	    Wave[n].release_ms=time[n]/4;
	    Wave[n].sustain_level=MAXLEVEL/4*3;
	    Wave[n].attackdecay_flag=TRUE;

	    Wave[n].amplitude=0;

		Wave[n].noteOn=TRUE;
		//Wave++;

	}
}

/******************************************************************************
** Function name:		organ
**
** Descriptions:		organ waves
** 						The coefficients for this instrument where found at
**
** https://github.com/Joeboy/pixperiments/tree/master/pitracker/plugins/organ
**
** parameters:			frequency in Hz
** Returned value:		None
**
******************************************************************************/

#define FIFTH_MULTIPLIER (1498/1000)
void organ(uint16_t frequency)
{
	uint8_t n;

	//WaveGenerator *wave;

	//wave=Wave;

	// set sin wave generators
	for(n=0;n<NUMBER_OF_OSCILLATORS;n++)
	{
		Wave[n].phase=0;
		Wave[n].envelope=0;
		if(n==0)
			{
				Wave[n].freq_coefficient=0x01000000L/SAMPLEFREQUENCY*256*frequency;
				Wave[n].sustain_level=MAXLEVEL;
			}
		if(n==1)
			{
				Wave[n].freq_coefficient=0x01000000L/SAMPLEFREQUENCY*256*(frequency*FIFTH_MULTIPLIER);
				Wave[n].sustain_level=MAXLEVEL/20;
			}
		if(n==2)
			{
				Wave[n].freq_coefficient=0x01000000L/SAMPLEFREQUENCY*256*frequency*4;
				Wave[n].sustain_level=MAXLEVEL/3;
			}

	    Wave[n].attack_ms=1;
	    Wave[n].decay_ms=20;
	    Wave[n].release_ms=300;
	    Wave[n].attackdecay_flag=TRUE;

	    Wave[n].amplitude=0;

		Wave[n].noteOn=TRUE;
		//Wave++;

	}
}

/******************************************************************************
** Function name:		voiceOff
**
** Descriptions:		turn off all voices ( oscillators )
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void voiceOff(void)
{
	//WaveGenerator *wave;

	//wave=Wave;
	uint8_t n;

	for(n=0;n<NUMBER_OF_OSCILLATORS;n++)
	{
		Wave[n].noteOn=FALSE;

		//wave++;
	}
}
/******************************************************************************
** Function name:		void initWaveforms(void)
**
** Descriptions:		The wave form structures are not initialized at
**                      at program start. Call this routine before starting.
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void initWaveforms(void)
{
	uint8_t n;

	for(n=0;n<NUMBER_OF_OSCILLATORS;n++)
	{
		Wave[n].waveform=triangletab;
		if(n<4) Wave[n].waveform=squaretab;
		if(n==6) Wave[n].waveform=sintab;
		Wave[n].phase=0;
		Wave[n].envelope=0;
		Wave[n].freq_coefficient=0;

	    Wave[n].attack_ms=500;
	    Wave[n].decay_ms=500;
	    Wave[n].release_ms=1000;
	    Wave[n].sustain_level=MAXLEVEL/4*3;
	    Wave[n].attackdecay_flag=TRUE;

	    Wave[n].amplitude=0;

	    Wave[n].noteOn=FALSE;
	}
}
/******************************************************************************
** Function name:		updateEnvelope
**
** Descriptions:		calculate the envelopes of the wave generators
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void updateEnvelope(void)
{
	int32_t attack;
	int32_t decay;
	int32_t release;

	int n;
	for(n=0;n<NUMBER_OF_OSCILLATORS;n++) 	// decrease Envelope
	{
		// calculate linear slopes
		attack=MAXLEVEL/Wave[n].attack_ms;
		decay=MAXLEVEL/Wave[n].decay_ms;
		release=MAXLEVEL/Wave[n].release_ms;

		if(Wave[n].noteOn) // if gate set then attack,decay,sustain
		{
			if(Wave[n].attackdecay_flag)
			{	// if attack cycle
				Wave[n].amplitude+=attack;
				if(Wave[n].amplitude>MAXLEVEL) // check if decay phase should be entered
				{
					Wave[n].amplitude=MAXLEVEL;
					Wave[n].attackdecay_flag=FALSE; // if level reached, then switch to decay
				}
			}
			else // decay cycle
			{
				if(Wave[n].amplitude>Wave[n].sustain_level)
				{
					Wave[n].amplitude -= decay;
					if(Wave[n].amplitude<Wave[n].sustain_level) Wave[n].amplitude=Wave[n].sustain_level;
				}
			}
		}
		else // if gate flag is not set then release
		{
			Wave[n].attackdecay_flag=TRUE; // at next attack/decay cycle start wiht attack
			if(Wave[n].amplitude>0)
			{
				Wave[n].amplitude -= release;
				if(Wave[n].amplitude<0) Wave[n].amplitude=0;
			}
		}
		Wave[n].envelope=Wave[n].amplitude>>8;
		if(n==0)DebugPrint(Wave[n].envelope);

		//wave++;
	}
}
/******************************************************************************
** Function name:		waveGenerators
**
** Descriptions:		In this routine the waveform generation is done
**
** parameters:			value: value is in the range of 0..65536
** Returned value:		None
**
******************************************************************************/
const int8_t sintab[256]={
0 ,3 ,6 ,9 ,12 ,15 ,18 ,21 ,24 ,27 ,30 ,33 ,36 ,39 ,42 ,45 ,48 ,51 ,54 ,57 ,59 ,62 ,65 ,67 ,70 ,73 ,75 ,78 ,80 ,82 ,85 ,87 ,
89 ,91 ,94 ,96 ,98 ,100 ,102 ,103 ,105 ,107 ,108 ,110 ,112 ,113 ,114 ,116 ,117 ,118 ,119 ,120 ,121 ,122 ,123 ,123 ,124 ,125 ,125 ,126 ,126 ,126 ,126 ,126 ,
127 ,126 ,126 ,126 ,126 ,126 ,125 ,125 ,124 ,123 ,123 ,122 ,121 ,120 ,119 ,118 ,117 ,116 ,114 ,113 ,112 ,110 ,108 ,107 ,105 ,103 ,102 ,100 ,98 ,96 ,94 ,91 ,
89 ,87 ,85 ,82 ,80 ,78 ,75 ,73 ,70 ,67 ,65 ,62 ,59 ,57 ,54 ,51 ,48 ,45 ,42 ,39 ,36 ,33 ,30 ,27 ,24 ,21 ,18 ,15 ,12 ,9 ,6 ,3 ,
0 ,-3 ,-6 ,-9 ,-12 ,-15 ,-18 ,-21 ,-24 ,-27 ,-30 ,-33 ,-36 ,-39 ,-42 ,-45 ,-48 ,-51 ,-54 ,-57 ,-59 ,-62 ,-65 ,-67 ,-70 ,-73 ,-75 ,-78 ,-80 ,-82 ,-85 ,-87 ,
-89 ,-91 ,-94 ,-96 ,-98 ,-100 ,-102 ,-103 ,-105 ,-107 ,-108 ,-110 ,-112 ,-113 ,-114 ,-116 ,-117 ,-118 ,-119 ,-120 ,-121 ,-122 ,-123 ,-123 ,-124 ,-125 ,-125 ,-126 ,-126 ,-126 ,-126 ,-126 ,
-127 ,-126 ,-126 ,-126 ,-126 ,-126 ,-125 ,-125 ,-124 ,-123 ,-123 ,-122 ,-121 ,-120 ,-119 ,-118 ,-117 ,-116 ,-114 ,-113 ,-112 ,-110 ,-108 ,-107 ,-105 ,-103 ,-102 ,-100 ,-98 ,-96 ,-94 ,-91 ,
-89 ,-87 ,-85 ,-82 ,-80 ,-78 ,-75 ,-73 ,-70 ,-67 ,-65 ,-62 ,-59 ,-57 ,-54 ,-51 ,-48 ,-45 ,-42 ,-39 ,-36 ,-33 ,-30 ,-27 ,-24 ,-21 ,-18 ,-15 ,-12 ,-9 ,-6 ,-3
};
const int8_t squaretab[256]={
0 ,9 ,18 ,27 ,36 ,45 ,53 ,61 ,68 ,75 ,82 ,88 ,94 ,99 ,103 ,107 ,110 ,113 ,115 ,116 ,117 ,117 ,117 ,117 ,116 ,115 ,113 ,112 ,110 ,108 ,106 ,103 ,
101 ,99 ,97 ,95 ,94 ,92 ,91 ,90 ,89 ,88 ,88 ,88 ,88 ,89 ,89 ,90 ,91 ,92 ,94 ,95 ,97 ,98 ,100 ,102 ,103 ,104 ,106 ,107 ,108 ,109 ,109 ,109 ,
110 ,109 ,109 ,109 ,108 ,107 ,106 ,104 ,103 ,102 ,100 ,98 ,97 ,95 ,94 ,92 ,91 ,90 ,89 ,89 ,88 ,88 ,88 ,88 ,89 ,90 ,91 ,92 ,94 ,95 ,97 ,99 ,
101 ,103 ,106 ,108 ,110 ,112 ,113 ,115 ,116 ,117 ,117 ,117 ,117 ,116 ,115 ,113 ,110 ,107 ,103 ,99 ,94 ,88 ,82 ,75 ,68 ,61 ,53 ,45 ,36 ,27 ,18 ,9 ,
0 ,-9 ,-18 ,-27 ,-36 ,-45 ,-53 ,-61 ,-68 ,-75 ,-82 ,-88 ,-94 ,-99 ,-103 ,-107 ,-110 ,-113 ,-115 ,-116 ,-117 ,-117 ,-117 ,-117 ,-116 ,-115 ,-113 ,-112 ,-110 ,-108 ,-106 ,-103 ,
-101 ,-99 ,-97 ,-95 ,-94 ,-92 ,-91 ,-90 ,-89 ,-88 ,-88 ,-88 ,-88 ,-89 ,-89 ,-90 ,-91 ,-92 ,-94 ,-95 ,-97 ,-98 ,-100 ,-102 ,-103 ,-104 ,-106 ,-107 ,-108 ,-109 ,-109 ,-109 ,
-110 ,-109 ,-109 ,-109 ,-108 ,-107 ,-106 ,-104 ,-103 ,-102 ,-100 ,-98 ,-97 ,-95 ,-94 ,-92 ,-91 ,-90 ,-89 ,-89 ,-88 ,-88 ,-88 ,-88 ,-89 ,-90 ,-91 ,-92 ,-94 ,-95 ,-97 ,-99 ,
-101 ,-103 ,-106 ,-108 ,-110 ,-112 ,-113 ,-115 ,-116 ,-117 ,-117 ,-117 ,-117 ,-116 ,-115 ,-113 ,-110 ,-107 ,-103 ,-99 ,-94 ,-88 ,-82 ,-75 ,-68 ,-61 ,-53 ,-45 ,-36 ,-27 ,-18 ,-3
};

const int8_t triangletab[256]={
0 ,1 ,3 ,5 ,7 ,9 ,10 ,12 ,13 ,15 ,16 ,17 ,18 ,19 ,20 ,20 ,21 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,23 ,
23 ,24 ,25 ,26 ,27 ,28 ,29 ,31 ,33 ,34 ,36 ,38 ,40 ,42 ,45 ,47 ,49 ,51 ,53 ,55 ,57 ,59 ,61 ,63 ,64 ,65 ,66 ,67 ,68 ,69 ,69 ,69 ,
70 ,69 ,69 ,69 ,68 ,68 ,67 ,67 ,66 ,66 ,65 ,65 ,65 ,65 ,65 ,65 ,66 ,66 ,67 ,68 ,70 ,72 ,74 ,76 ,78 ,81 ,84 ,87 ,90 ,93 ,96 ,100 ,
103 ,106 ,110 ,113 ,115 ,118 ,120 ,122 ,124 ,125 ,125 ,125 ,125 ,124 ,122 ,120 ,117 ,114 ,110 ,105 ,99 ,93 ,87 ,80 ,72 ,64 ,56 ,47 ,38 ,29 ,19 ,9 ,
0 ,-9 ,-19 ,-29 ,-38 ,-47 ,-56 ,-64 ,-72 ,-80 ,-87 ,-93 ,-99 ,-105 ,-110 ,-114 ,-117 ,-120 ,-122 ,-124 ,-125 ,-125 ,-125 ,-125 ,-124 ,-122 ,-120 ,-118 ,-115 ,-113 ,-110 ,-106 ,
-103 ,-100 ,-96 ,-93 ,-90 ,-87 ,-84 ,-81 ,-78 ,-76 ,-74 ,-72 ,-70 ,-68 ,-67 ,-66 ,-66 ,-65 ,-65 ,-65 ,-65 ,-65 ,-65 ,-66 ,-66 ,-67 ,-67 ,-68 ,-68 ,-69 ,-69 ,-69 ,
-70 ,-69 ,-69 ,-69 ,-68 ,-67 ,-66 ,-65 ,-64 ,-63 ,-61 ,-59 ,-57 ,-55 ,-53 ,-51 ,-49 ,-47 ,-45 ,-42 ,-40 ,-38 ,-36 ,-34 ,-33 ,-31 ,-29 ,-28 ,-27 ,-26 ,-25 ,-24 ,
-23 ,-23 ,-22 ,-22 ,-22 ,-22 ,-21 ,-21 ,-21 ,-22 ,-22 ,-22 ,-22 ,-21 ,-21 ,-21 ,-21 ,-20 ,-20 ,-19 ,-18 ,-17 ,-16 ,-15 ,-13 ,-12 ,-10 ,-9 ,-7 ,-5 ,-3 ,-1
};
__inline void waveGenerators(void)
{
	static int32_t temp;
	uint16_t outValue;
	uint8_t n;

	temp=0;
	for(n=0;n<NUMBER_OF_OSCILLATORS;n++)
	{
		Wave[n].phase+=Wave[n].freq_coefficient;
		temp+=Wave[n].envelope*Wave[n].waveform[Wave[n].phase>>24];
	}

	outValue=temp+0x8000; // add DC offset
	audioDAC(outValue);
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

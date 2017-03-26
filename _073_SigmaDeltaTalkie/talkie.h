/*********************************************************************************************************
// https://github.com/going-digital/Talkie
// Talkie library
// Copyright 2011 Peter Knight
// This code is released under GPLv2 license.
********************************************** list of outhors *******************************************

  V0.0       2011 Peter Knight  initial version for ARDUINO UNO
  v0.1  24.3.2017 ChrisMicro    modified to be used on any MC by writting a wavebuffer

*********************************************************************************************************/


#ifndef _Talkie_h_
#define _Talkie_h_

#include <inttypes.h>
//#include <avr/io.h>

#define FS 8000 // Speech engine sample rate

class Talkie
{
  public:
    Talkie(void);
    void say(uint8_t* address);
    void setWaveBuffer(uint8_t *waveBuffer,uint32_t bufferLength);
    void setFunction_playSound(void (*functionPoitner)(uint32_t numberOfSamples));
    uint8_t nextSample();
    uint8_t* ptrAddr;
    uint8_t ptrBit;

    
  private:
    // Setup
    uint8_t setup;

    uint8_t *talkieWaveBuffer;
    uint32_t talkieWaveBufferLength;
    uint32_t talkieWaveBufferIndex;
    // Bitstream parser
    void setPtr(uint8_t* addr);
    uint8_t rev(uint8_t a);
    uint8_t getBits(uint8_t bits);
    uint8_t pgm_read_byte1(uint8_t *address) ;
    void  (*CallBackFunctionPointer_playSound)(uint32_t numberOfSamples);

};


#endif

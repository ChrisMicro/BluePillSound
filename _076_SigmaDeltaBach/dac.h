#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __DAC_H_
#define __DAC_H_

  #include "platform.h"
  #include <stdint.h>
  
  #define MAXLEVEL (0x7FFFL/NUMBER_OF_OSCILLATORS) // maximum amplitude/voice
  
  void initDac();
  void stopDac();
  void audioDAC(uint16_t value);
  //__inline void audioDAC(uint16_t value);
  void DebugPrint(int32_t wert);
  void audioDAC1_STM32F103(uint16_t value);
  void playNow();
  
#endif //__DAC_H_

#ifdef __cplusplus
}
#endif

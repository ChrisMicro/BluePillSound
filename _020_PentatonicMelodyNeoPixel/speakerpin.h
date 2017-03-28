  #include "GPIOport.h"

  PB_0 FASTSOUNDPIN;
  #define INITSPEAKERPIN FASTSOUNDPIN.pinMode(OUTPUT)
  #define SPEAKERPINHIGH      fastWrite(FASTSOUNDPIN, 1)
  #define SPEAKERPINLOW       fastWrite(FASTSOUNDPIN, 0)

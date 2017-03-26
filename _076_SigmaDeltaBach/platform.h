	#define PC_SIMULATION_DURATION_IN_SECONDS 240

//#define PLATFORM_PC
//#define PLATFORM_LPC1114
#define PLATFORM_STMDUINO_STM32F103

#define NUMBER_OF_OSCILLATORS 5

#ifdef PLATFORM_STMDUINO_STM32F103

  #define SAMPLEFREQUENCY 8000 // sampling frequency

#endif


#ifdef PLATFORM_PC

	#define SAMPLEFREQUENCY 48000 // sampling frequency

#endif

#define  FALSE	0
#define  TRUE	1

/****************************************************************************
*
 *   Project: generation AUDIO sounds
 *
 *   Description:
 *     This file generates the sounds of two bells with the help of 5
 *     5 DDS sin wave generators
 *
 *   Hardware used: LPC1114Xpresso ( ARM M0 ) or LPC1114FN102 ( 28 Pin-Dip )
 *   Compiler: GCC
 *   SysClk: 48Mhz
 *
 *	 Pins
 *	 ====
 *	 pin 0.8  : PWM output MATCH0 with RC filter ( 470 Ohm, 100nF )
 *	 pin 0.7  : blinking LED
 *
 *	 Attention: if you use the LPC1114FN102 without external crystal
 *	 you have to configure the file "system_LPC11xx.h" to use the internal
 *	 RC-oscillator:
 *
 *	 #define SYSPLLCLKSEL_Val      0x00000000	// IRC oscillator
 *
 *
 ****************************************************************************/
#ifdef PLATFORM_LPC1114

	#include "driver_config.h"
	#include "target_config.h"
	#include "gpio.h"

	#define PWM_RESULUTION 1024 //46,875Khz PWM frequency @48MHz SysClk
	#define SAMPLEFREQUENCY 46875 // sampling frequency

#endif

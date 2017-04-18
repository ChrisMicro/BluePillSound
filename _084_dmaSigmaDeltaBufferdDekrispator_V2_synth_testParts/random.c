/**
 ******************************************************************************
   File Name          : random.c
   Author			  : Xavier Halgand
   Date               :
   Description        :
 ******************************************************************************
*/
//-------------------------------------------------------------------------------------------------------

#include "random.h"

//-------------------------------------------------------------------------------------------------------

/* RNG handler declaration */
//RNG_HandleTypeDef RngHandle;

//-------------------------------------------------------------------------------------------------------
/*
  void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
  {
  //RNG Peripheral clock enable
  //__RNG_CLK_ENABLE();
  //__HAL_RNG_ENABLE(&RngHandle);
  }
*/
//-------------------------------------------------------------------------------------------------------

/**
    @brief RNG MSP De-Initialization
           This function freeze the hardware resources used in this example:
             - Disable the Peripheral's clock
    @param hrng: RNG handle pointer
    @retval None
*/
/*
  void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
  {
  // Enable RNG reset state
  //__RNG_FORCE_RESET();

  // Release RNG from reset state
  //__RNG_RELEASE_RESET();
  }
*/
//---------------------------------------------------------------------------
void randomGen_init(void)
{
  /* Configure the RNG peripheral #######################################*/
  /*
  	  RngHandle.Instance = RNG;

  	  if(HAL_RNG_Init(&RngHandle) != HAL_OK)
  	  {
  	    // Initialization Error
  	    Error_Handler();
  	  }
  	srand(HAL_RNG_GetRandomNumber(&RngHandle));
  */
}
//---------------------------------------------------------------------------
/**************
   returns a random float between a and b
 *****************/
float_t frand_a_b(float_t a, float_t b)
{
  return ( rand() / (float_t)RAND_MAX ) * (b - a) + a ;
}


//---------------------------------------------------------------------------
/**************
   returns a random float between 0 and 1
   // changed by ChrisMicro 2017
 *****************/
float_t randomNum(void)
{
  static uint16_t phase1, oldphase;
  static int16_t temp, temp2;
  static uint16_t noise = 0xAA;
  float_t random = 1.0f;
  temp = noise;
  noise = noise << 1;
  temp ^= noise;
  if ( ( temp & 0x4000 ) == 0x4000 )noise |= 1;

  if ((oldphase ^ phase1) & 0x4000)
  {
    temp2 = (noise >> 6) & 0xFF;
  }
  temp = temp2;
  random = (float)temp / 256;
  //random = (float_t) (HAL_RNG_GetRandomNumber(&RngHandle) / 4294967294.0f);
  return random;
}

/*-----------------------------------------------------------------------------*/
/**************
   returns a random integer between 0 and MIDI_MAX
 *****************/
uint8_t MIDIrandVal(void)
{
  return (uint8_t)lrintf(frand_a_b(0 , MIDI_MAX));
}

//-------------------------------------------------------------------------------------------------------

/*
 play "sid_star.wav" from SD-card using a sigmal delta DAC

 How To:
 You have to copy the wav-file "sid_star.wav" from this folder onto the SD-card first.

 Hardware: 
 Board: Generic STM32F103C BluePill, Arduino 1.6.9,  
 sound output PB0

 May 2017 ChrisMicro
 
*/

// SD-card pins
//  SCK   <-->  PA5 
//  MISO  <-->  PA6 
//  MOSI  <-->  PA7 
#define SD_CS PA8

#include "stm32f103.h"
#include <SPI.h>
#include <SD.h>

Sd2Card card;
File sFile;

char * fileNameText="sid_star.wav";

#define DAC_DC_VALUE      0x4000
#define DAC_MAX_AMPLITUDE 0x2000 // experimental maximum on my speakers without distortion

#define SAMPLINGFREQUENCY 800

#define WAVEBUFFERLENGTH  SAMPLINGFREQUENCY * 1 // 1 seconds, time is limited due to RAM capacity

int16_t wave_i16[WAVEBUFFERLENGTH];

#define DACSAMPLINGRATE_HZ    8000
#define TIMER_INTTERUPT_US    1000000UL / DACSAMPLINGRATE_HZ    

HardwareTimer timer(2);

#define DMABUFFERLENGTH 100

volatile uint32_t DmaBuffer[DMABUFFERLENGTH];

// good DMA article:
// https://vjordan.info/log/fpga/stm32-bare-metal-start-up-and-real-bit-banging-speed.html
void startDMA()
{
  RCC->AHBENR |= RCC_AHBENR_DMA1EN; // enable DMA

  // channel 1: mem:8bit -> peri:8bit
  DMA1_Channel1->CNDTR = DMABUFFERLENGTH;
  DMA1_Channel1->CMAR = (uint32_t)DmaBuffer;
  DMA1_Channel1->CPAR = (uint32_t) & (GPIOB->ODR);
  DMA1_Channel1->CCR = 0;
  DMA1_Channel1->CCR = DMA_CCR1_MEM2MEM | DMA_CCR1_PL | DMA_CCR1_MINC | DMA_CCR1_CIRC | DMA_CCR1_DIR | DMA_CCR1_EN;
}

// maple timer
// http://docs.leaflabs.com/docs.leaflabs.com/index.html
void startTimer()
{
  // Pause the timer while we're configuring it
  timer.pause();

  // Set up period
  timer.setPeriod       ( TIMER_INTTERUPT_US ); // in microseconds

  // Set up an interrupt on channel 1
  timer.setChannel1Mode ( TIMER_OUTPUT_COMPARE);
  timer.setCompare      ( TIMER_CH1,         1);  // Interrupt 1 count after each update
  timer.attachCompare1Interrupt( DACsampleRateHandler );

  // Refresh the timer's count, prescale, and overflow
  timer.refresh();

  // Start the timer counting
  timer.resume();
}

uint16_t SineTable[256];

#define SPEAKERPIN PB0            // speaker pin, be sure that it corresponds to the DMA pin setup in the next line
#define PIN_PB 0                  // speaker pin, you can choose 0..7, 0 means PB0, 1 means PB1 ...
#define PORTB_PINMASK (1<<PIN_PB)

uint32_t CoarseSigmaDeltaTable[] = {0x00000000 * PORTB_PINMASK, 0x01000000 * PORTB_PINMASK, 0x01000100 * PORTB_PINMASK, 0x01010100 * PORTB_PINMASK, 0x01010101 * PORTB_PINMASK};

// Sigma Delta DAC
// input value 10 bit 0..1023
void writeDac( uint32_t sollwert )
{
  uint32_t  integrator = 0;
  static uint32_t  oldValue = 0;

  uint32_t n;

  // sigma delta DAC
  for (n = 0; n < DMABUFFERLENGTH / 4; n++)
  {
    integrator += sollwert - oldValue;
    oldValue = integrator & 0b11100000000;
    DmaBuffer[n] = CoarseSigmaDeltaTable[ oldValue >> 8 ];
  }
}

typedef uint16_t buf_t;
#define SOUNDBUFFERLENGTH 256
volatile buf_t SoundBuffer0[SOUNDBUFFERLENGTH];
volatile buf_t SoundBuffer1[SOUNDBUFFERLENGTH];

volatile buf_t * volatile SoundBufferCurrentlyPlaying;

void DACsampleRateHandler()
{
  static uint32_t bufferIndex=0;
  
  writeDac(SoundBufferCurrentlyPlaying[bufferIndex++]);
  
  if( bufferIndex >= SOUNDBUFFERLENGTH )
  {
    bufferIndex=0;
    if( SoundBufferCurrentlyPlaying != SoundBuffer1 )SoundBufferCurrentlyPlaying = SoundBuffer1;
    else                                    SoundBufferCurrentlyPlaying = SoundBuffer0;
  }
 
}
#define LEDPIN     PC13 // Blue Pill LED
#define INITLED    pinMode(LEDPIN, OUTPUT)

#define LEDON      digitalWrite(LEDPIN, HIGH)
#define LEDOFF     digitalWrite(LEDPIN, LOW)

void toggleLed()
{
  static uint8_t flag=0;

  if(flag) LEDON;
  else     LEDOFF;

  flag^=1;
}

volatile buf_t * volatile SoundBuffer;

void setup() {
  Serial.begin(115200);
  for(int n=0;n<5;n++)
  {
    delay(1000);
    Serial.print(".");
  }
 
  Serial.print("Initializing SD card...");
  pinMode(SD_CS, OUTPUT);

  pinMode(SD_CS, OUTPUT);
  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    return;
  }
  //******************************** DAC init ***************************
   uint32_t n;
  
  pinMode(PC13, OUTPUT);
  pinMode(SPEAKERPIN, OUTPUT); 
  Serial.begin(115200);
  Serial.println("SPI SIGMA DELTA SOUND calculation performance");

  delay(100);
  SoundBufferCurrentlyPlaying = SoundBuffer0;
  SoundBuffer                 = SoundBuffer1;
  startDMA();

  // ** just check the write speed **
  uint32_t startTime = micros();
  for (n = 0; n < 1000; n++) writeDac(0); // dummy write silence
  uint32_t stopTime = micros();

  uint32_t DacSamplingFrequency;
  Serial.print("DAC write time for 1000 samples in us: "); Serial.println(stopTime - startTime);
  DacSamplingFrequency = 1000000UL * 1000 / (stopTime - startTime);
  Serial.print("maximum possible DAC sampling frequency [Hz]: "); Serial.println(DacSamplingFrequency);

  // *********************************

  for (n = 0; n < 256; n++) SineTable[n] = (sin(2 * PI * n / 255) + 1) * 500;

  startTimer();

}

volatile uint32_t Oscillator1_Frequency_Hz=440;

void updateSound1()
{
  uint32_t amplitude=500;
  // float sine wave calculation is very slow on STM32f103@72MHz for real time calculation
  // 16ms/256 samples @ fab=10kHz
  // ~63us per sample ( sigma delta dac with spi DMA running in parallel )
  static float phase=0;
  float deltaPhase;
  deltaPhase=2*PI*Oscillator1_Frequency_Hz / DACSAMPLINGRATE_HZ;
    
  for(uint32_t n=0;n<SOUNDBUFFERLENGTH;n++)
  { 
    phase+=deltaPhase;
    if( phase > 2 *PI ) phase -= 2 * PI;  
    SoundBuffer[n]=(sin(phase)+1)*amplitude;
  }
}

uint8_t fileReadByte()
{
  uint8_t value=0;
  if(sFile.available())  value=sFile.read();
  else sFile.seek(44);
  return value;
}

void updateSoundSdCard()
{
  static boolean fileOpenFlag=false;
  
  if(!fileOpenFlag)
  {
    
    sFile = SD.open(fileNameText);
    if(sFile)
    {
      fileOpenFlag=true;
      sFile.seek(44); // start of data block, see waf RIFF format spec
    }
    else
    {
      Serial.println("file not found");
      delay(1000);
    }
  }
    
  if(fileOpenFlag)
  {
    int startTime=micros();
    for (int n = 0; n < SOUNDBUFFERLENGTH; n++)
    {
      int16_t value = 0;
  
      // left channel
      value = fileReadByte();
      value += fileReadByte() << 8;
      SoundBuffer[n] = (int32_t) value * 500/0x10000+250;
  
      // right channel has to be read but will not be stored into the mono buffer
      fileReadByte();
      fileReadByte();
    }
    int stopTime=micros();
    //Serial.println(stopTime-startTime);
  }

}

void loop()
{
  SoundBuffer = SoundBufferCurrentlyPlaying; 

  LEDON; // debug led for processing time measuremt with oscilloscope

  updateSoundSdCard();
  
  LEDOFF;
  while(SoundBuffer == SoundBufferCurrentlyPlaying); // wait until buffer is emtpy

}

/*
  sigma delta modulator experiments

  Generate a wavetable song sound.

  The wave is recorded 'A' ( 440Hz )

  Uses the melodies of  "Weihnachtsklingel" from M.Baudisch ( take a look at the comments of the file lied.h )

  Hardware: Arduino Uno
  Output pin: Pin 2 on Arduino ( Sigma Delta DAC ) connect to a RC-Low pass ( 470 Ohm, 100nF )

  2.6.2013 ch

*/

#include "lied.h"
#include "wavetables.h"

//********************************************************************************
// fast pin access from Rick Kimball
// https://gist.github.com/RickKimball/7d9a5bd278c8ea2e4d8e
// more fun with c++ classes and templates
class GPIOPort :
  public gpio_reg_map {
  public:
    void high(const uint32_t pin) {
      BSRR = 1 << pin;
    }
    void low(const uint32_t pin) {
      BRR = 1 << pin;
    }
    void pinMode(const uint32_t pin, gpio_pin_mode mode) {
      volatile uint32_t *cr = &CRL + (pin >> 3);
      uint32_t shift = (pin & 0x7) * 4;
      uint32_t tmp = *cr;
      tmp &= ~(0xF << shift);
      tmp |= (mode == GPIO_INPUT_PU ? GPIO_INPUT_PD : mode) << shift;
      *cr = tmp;

      if (mode == GPIO_INPUT_PD) {
        ODR &= ~(1u << pin);
      } else if (mode == GPIO_INPUT_PU) {
        ODR |= (1u << pin);
      }
    }
    int value(const uint32_t pin) {
      return (IDR & (1u << pin) ? 1 : 0);
    }

};

#define GPIOPORT_REF(a) *((GPIOPort * const)(a))

static GPIOPort & gPortA = GPIOPORT_REF(0x40010800);
static GPIOPort & gPortB = GPIOPORT_REF(0x40010C00);
static GPIOPort & gPortC = GPIOPORT_REF(0x40011000);

template<const uint32_t PIN>
class GPIOPortBPin {
  public:
    void high() {
      gPortB.high(PIN);
    }
    void low() {
      gPortB.low(PIN);
    }
    void operator=(const int value) {
      if ( value )
        gPortB.high(PIN);
      else
        gPortB.low(PIN);
    }

    operator int() {
      return gPortB.value(PIN);
    }

    void pinMode(WiringPinMode mode) {
      gpio_pin_mode gpio_mode;
      bool pwm = false;

      switch (mode) {
        case OUTPUT:
          gpio_mode = GPIO_OUTPUT_PP;
          break;
        case OUTPUT_OPEN_DRAIN:
          gpio_mode = GPIO_OUTPUT_OD;
          break;
        case INPUT:
        case INPUT_FLOATING:
          gpio_mode = GPIO_INPUT_FLOATING;
          break;
        case INPUT_ANALOG:
          gpio_mode = GPIO_INPUT_ANALOG;
          break;
        case INPUT_PULLUP:
          gpio_mode = GPIO_INPUT_PU;
          break;
        case INPUT_PULLDOWN:
          gpio_mode = GPIO_INPUT_PD;
          break;
        case PWM:
          gpio_mode = GPIO_AF_OUTPUT_PP;
          pwm = true;
          break;
        case PWM_OPEN_DRAIN:
          gpio_mode = GPIO_AF_OUTPUT_OD;
          pwm = true;
          break;
        default:
          return;
      }

      gPortB.pinMode(PIN, gpio_mode);
      (void)pwm; // TODO: implement timer start/stop
    }
};

typedef GPIOPortBPin<1> PB_1;
typedef GPIOPortBPin<2> PB_2;
typedef GPIOPortBPin<12> PB_12;
// ... and on and on
typedef GPIOPortBPin<11> PB_15;
//********************************************************************************


// pin for sound out
PB_12 FASTSOUNDPIN;

#define fastWrite(pin,value) do { (value) ? pin.high() : pin.low(); } while(0)

//#define LOUDSPEAKER PB12

void setup() {
  // initialize the digital pin as an output.
  //pinMode(LOUDSPEAKER, OUTPUT);
  FASTSOUNDPIN.pinMode(OUTPUT);

}

//uint8_t n;

// pitch in Hz
// duration in ms
void sound(uint16_t pitch, uint16_t duration)
{
  uint32_t phaseDelta = 1070UL * 10UL;
  int16_t  integrator = 0;
  uint8_t  sollwert = 64;
  uint8_t  oldValue = 0;
  uint32_t phase = 0;
  uint16_t timer;
  uint16_t index = 0;
  uint8_t n;

  timer = duration * 25 * 4;
  //timer=duration*50;
  //phaseDelta = 42UL * pitch;
  phaseDelta = 36UL * pitch;
  const uint8_t *p;
  if(pitch>300)  p=sabine;
  else p=piano;
  FASTSOUNDPIN.pinMode(OUTPUT);
  while (timer != 0)
  {
    // DDS ( direct digital synthesis )
    index = phase >> 16;
    if(index<SONGLEN) sollwert=p[index];
    else sollwert = 128;
    phase += phaseDelta;
    timer--;

    // sigma delta DAC, hold the DAC value for n-steps constant
    //FASTSOUNDPIN.pinMode(OUTPUT);
    for (n = 0; n < 40; n++)
    {
      integrator += sollwert - oldValue;
      if (integrator > 0)
      {
        oldValue = MAXVALUE;
        //digitalWrite(LOUDSPEAKER,1);
        fastWrite(FASTSOUNDPIN, 1);
      }
      else
      {
        oldValue = 0;
        //digitalWrite(LOUDSPEAKER,0);
        fastWrite(FASTSOUNDPIN, 0);
      }
    }
    //pinMode(LOUDSPEAKER,INPUT);
    //FASTSOUNDPIN.pinMode(INPUT); // turn off pull up
  }
  FASTSOUNDPIN.pinMode(INPUT); // turn off pull up
}

const uint16_t tonhoehe[] =
{
  262,//262 c    Timerwerte fuer die enspr. Tonhoehen in Hz
  277,//277 cis
  294,//294 d
  311,//311 dis
  330,//330 e
  349,//349 f
  370,//370 fis
  392,//392 g
  416,//416 gis
  440,//440 a
  466,//466 b
  495,//495 h
  524,//524 c'
  554,//554 cis'
  588,//588 d'
  622,//622 dis'
  660,//660 e'
  698,//698 f'
  740,//740 fis'
  784,//784 g'
  831,//831 gis'
  880,//880 a'
  929,//929 b'
  992,//992 h'
  1050,//1050 c''
  1106,//1106 cis''
  1179,//1179 d''
  1244,//1244 dis''
  1316,//1316 e'
  1397,//1397 f''
  1562, //1562 g'' auf position von fis'' verschoben
  0
};

void playLied(uint8_t liedNr)
{
  uint16_t liedIndex, endIndex;
  uint32_t duration;
  uint8_t dauer, playSpeed;
  uint16_t frequency;
  uint16_t n;

  liedIndex = liedofs[liedNr];
  playSpeed = lied[liedIndex];
  endIndex = liedofs[liedNr + 1];
  for (n = liedIndex; liedIndex < endIndex; n++)
  {
    dauer = pgm_read_byte(&lied[n]) >> 5;
    duration = (uint32_t) zeit[dauer] * 30 * playSpeed; // duration in ms
    frequency = tonhoehe[(pgm_read_byte(&lied[n]) & 0x1F)]; // frequenz in Hz
    sound(frequency, duration);

    liedIndex++;
  }
  sound(0, 1000); // delay 1000ms
}

void loop() 
{
  uint8_t n;
  /*

    for(n=0;n<0x20;n++)
    {

      sound(440,200);
    }
  */
  for (n = 0; n < ANZ; n++) playLied(n);
}


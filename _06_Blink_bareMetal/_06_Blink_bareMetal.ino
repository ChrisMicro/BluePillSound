/*

  real bare metall pin access for maximum speed

  
*/

// STM32F103 IO register
#define GPIOPort_B_ADDRESS 0x40010C00
#define GPIOPort_C_ADDRESS 0x40011000
#define BSRR_REGISTER_OFFSET 0x10 // port bit set/reset register
#define BRR_REGISTER_OFFSET  0x14 // port bit reset register


void setup()
{
  pinMode(PC13, OUTPUT);
}

// measurement result: ~21Mhz toggling frequency in unrolled part
void maxSpeedTogglePinTest()
{
  while (1)
  {
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BSRR_REGISTER_OFFSET) = 1 << 13; // LEDON: write direct into GPIO memory
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BRR_REGISTER_OFFSET)  = 1 << 13; // LEDOFF
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BSRR_REGISTER_OFFSET) = 1 << 13; // LEDON: write direct into GPIO memory
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BRR_REGISTER_OFFSET)  = 1 << 13; // LEDOFF
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BSRR_REGISTER_OFFSET) = 1 << 13; // LEDON: write direct into GPIO memory
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BRR_REGISTER_OFFSET)  = 1 << 13; // LEDOFF
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BSRR_REGISTER_OFFSET) = 1 << 13; // LEDON: write direct into GPIO memory
    *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BRR_REGISTER_OFFSET)  = 1 << 13; // LEDOFF
  }
}
void blink()
{
  *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BSRR_REGISTER_OFFSET) = 1 << 13; // LEDON: write direct into GPIO memory
  delay(1000);
  *(volatile uint16_t *) (GPIOPort_C_ADDRESS + BRR_REGISTER_OFFSET)  = 1 << 13; // LEDOFF
  delay(1000);
}

void loop()
{
  maxSpeedTogglePinTest();
  //blink();
}

// memory access examples:
// uint16_t x = *(uint16_t *) 0x40010C00; // read from memory address
// *(uint16_t *) 0x40010C00 = 10; // write into memory address

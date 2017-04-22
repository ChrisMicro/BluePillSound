#define FASTSOUNDPIN PB0
#define SPEAKERPINNUMBER 0 // at PB

// bare metal IO pin access

// STM32F103 IO register
#define GPIOPort_B_ADDRESS   0x40010C00
#define GPIOPort_C_ADDRESS   0x40011000
#define BSRR_REGISTER_OFFSET 0x10 // port bit set/reset register
#define BRR_REGISTER_OFFSET  0x14 // port bit reset register

#define SPEAKERPINHIGH { *(volatile uint16_t *) (GPIOPort_B_ADDRESS + BSRR_REGISTER_OFFSET) = 1 << SPEAKERPINNUMBER ; } // LEDON: write direct into GPIO memory
#define SPEAKERPINLOW  { *(volatile uint16_t *) (GPIOPort_B_ADDRESS + BRR_REGISTER_OFFSET)  = 1 << SPEAKERPINNUMBER ; } // LEDOFF





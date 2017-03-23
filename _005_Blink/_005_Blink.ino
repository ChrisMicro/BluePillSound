/*
 * 
 * simple blink program with macros
 * 
 *  6.3.2017 ChrisMicro
 * 23.3.2017 addapted for BluePill
 * 
 */
 
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

#define TOGGLELED toggleLed()

void setup() 
{
  INITLED;
}

void loop() 
{
  TOGGLELED;
  delay(1000); // wait for a second
}


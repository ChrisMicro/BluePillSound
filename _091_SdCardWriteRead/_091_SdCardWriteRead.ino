/*
 SD-Card file write/read test

 How To:
 You have to copy the wav-file "sid_star.wav" from this folder onto the SD-card first.

 Hardware: 
  Board: Generic STM32F103C BluePill, Arduino 1.6.9,  
 
*/

// SD-card pins
//  SCK   <-->  PA5 
//  MISO  <-->  PA6 
//  MOSI  <-->  PA7 
const int chipSelect = PA8;

#include <SPI.h>
#include <SD.h>

void setup() {
  Serial.begin(115200);
  for(int n=0;n<5;n++)
  {
    delay(1000);
    Serial.print(".");
  }
 
  Serial.print("Initializing SD card...");
  pinMode(chipSelect, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  delay(2000);
 
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File myFile = SD.open("test.txt", FILE_WRITE);
 
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("Helo writing and reading the SD-card works");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  Serial.println("reading test.txt");
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
   
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop()
{
// nothing happens after setup
}

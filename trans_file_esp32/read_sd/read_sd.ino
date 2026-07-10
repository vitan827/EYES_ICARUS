#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; } 

  if (!SD.begin(chipSelect)) {
    Serial.println("Loi: Khong the khoi tao the SD.");
    return;
  }
  
  File myFile = SD.open("test.txt");
  if (myFile) {
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
  } else {
    Serial.println("Loi: Khong the mo file test.txt.");
  }
}

void loop() {
  // Trống
}
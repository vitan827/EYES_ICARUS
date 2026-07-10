#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

SoftwareSerial espSerial(10, 2); // RX = 10, TX = 2 (để trống D11 cho thẻ SD)
const int chipSelect = 4;        // Chân CS của module thẻ SD nối với D4
File dataFile;

void setup() {
  Serial.begin(115200);
  espSerial.begin(9600);

  Serial.print("Khoi tao the SD...");
  if (!SD.begin(chipSelect)) {
    Serial.println(" That bai!");
    return;
  }
  Serial.println(" Thanh cong.");
}

void loop() {
  if (espSerial.available()) {
    String data = espSerial.readStringUntil('\x03');
    data.trim(); 

    if (data == "ON") {
      Serial.println("Lenh: Kich hoat");
    } 
    else if (data.startsWith("START:")) {
      String fileName = data.substring(6);
      Serial.println("Bat dau ghi file: " + fileName);
      
      // Đảm bảo tên file đúng định dạng FAT16 8.3 (VD: "data.txt")
      if (!fileName.startsWith("/")) {
        fileName = "/" + fileName; 
      }
      if (SD.exists(fileName)) {
        SD.remove(fileName); // Xóa file cũ
      }
      dataFile = SD.open(fileName, FILE_WRITE);
    } 
    else if (data.startsWith("DATA:")) {
      if (dataFile) {
        dataFile.print(data.substring(5)); // Ghi nội dung vào thẻ
      }
    } 
    else if (data == "END") {
      if (dataFile) {
        dataFile.close();
        Serial.println("Hoan tat luu file.");
      }
    }
  }
}
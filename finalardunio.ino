#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_PIN 11
#define TOTAL_CELLS 9

// MOSFET pins
const int mosfetPins[TOTAL_CELLS] = {
  2,3,4,
  5,6,7,
  8,9,10
};

bool matrixState[TOTAL_CELLS] = {
  0,0,0,
  0,0,0,
  0,0,0
};

float temps[TOTAL_CELLS];

String serialBuffer = "";

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

DeviceAddress sensorAddr[TOTAL_CELLS];

unsigned long lastTempRead = 0;

void setup() {

  Serial.begin(115200);

  // MOSFET init
  for(int i=0;i<TOTAL_CELLS;i++) {

    pinMode(
      mosfetPins[i],
      OUTPUT
    );

    digitalWrite(
      mosfetPins[i],
      LOW
    );
  }

  sensors.begin();

  // tìm tối đa 9 sensor
  for(int i=0;i<TOTAL_CELLS;i++) {

    if(
      sensors.getAddress(
        sensorAddr[i],
        i
      )
    ) {

      sensors.setResolution(
        sensorAddr[i],
        12
      );
    }
  }

  Serial.println("UNO READY");
}

void loop() {

  readUART();

  readTemps();
}

void readUART() {

  while(
    Serial.available()
  ) {

    char c =
      Serial.read();

    if(c == '\n') {

      serialBuffer.trim();

      if(
        serialBuffer.length()
        >= 9
      ) {

        processMatrix(
          serialBuffer
        );
      }

      serialBuffer = "";
    }
    else {

      if(
        c != '\r'
        &&
        c != ' '
      ) {

        serialBuffer += c;
      }
    }
  }
}

void processMatrix(
  String data
) {

  for(
    int i=0;
    i<TOTAL_CELLS;
    i++
  ) {

    if(
      data[i] == '1'
    ) {

      matrixState[i] = true;

      digitalWrite(
        mosfetPins[i],
        HIGH
      );
    }
    else {

      matrixState[i] = false;

      digitalWrite(
        mosfetPins[i],
        LOW
      );
    }
  }
}

void readTemps() {

  if(
    millis()
    - lastTempRead
    < 1000
  ) return;

  lastTempRead =
    millis();

  sensors.requestTemperatures();

  for(
    int i=0;
    i<TOTAL_CELLS;
    i++
  ) {

    temps[i] =
      sensors.getTempC(
        sensorAddr[i]
      );

    // bảo vệ nhiệt
    if(
      temps[i] >= 80
    ) {

      digitalWrite(
        mosfetPins[i],
        LOW
      );

      matrixState[i] =
        false;
    }
  }

  sendTempsToESP();
}

void sendTempsToESP() {

  Serial.print("T:");

  for(
    int i=0;
    i<TOTAL_CELLS;
    i++
  ) {

    Serial.print(
      temps[i],
      1
    );

    if(
      i < TOTAL_CELLS-1
    ) {

      Serial.print(",");
    }
  }

  Serial.println();
}

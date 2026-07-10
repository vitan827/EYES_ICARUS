#include "EspWebManager.h"

EspWebManager webManager;

void setup() {
    Serial.begin(115200);
    webManager.begin();
}

void loop() {
    webManager.loop();
}
#include "EspWebManager.h"
#include "index.h"

EspWebManager::EspWebManager() : server(80), shouldConnect(false), connectStatus(0), shouldTurnOffAP(false), turnOffTime(0) {}

void EspWebManager::begin() {
    Serial2.begin(9600, SERIAL_8N1, 16, 17); // RX2 = GPIO 16, TX2 = GPIO 17

    if (!LittleFS.begin(true)) {
        Serial.println("Lỗi LittleFS");
        return;
    }

    WiFi.mode(WIFI_AP_STA);
    scanWiFi();
    WiFi.softAP("ESP32_Setup");
    Serial.print("Đã phát WiFi. Truy cập IP để cấu hình: ");
    Serial.println(WiFi.softAPIP());

    setupRoutes();
    server.begin();
}

void EspWebManager::scanWiFi() {
    int n = WiFi.scanNetworks();
    wifiOptions = "";
    for (int i = 0; i < n; ++i) {
        wifiOptions += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
    }
}

void EspWebManager::setupRoutes() {
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (WiFi.status() == WL_CONNECTED && !shouldTurnOffAP) {
            request->send(200, "text/html", fileManagerHTML);
        } else {
            String html = "<h2>Cau hinh WiFi ket noi</h2>"
                          "<form action='/connect' method='POST'>"
                          "Chon WiFi: <select name='ssid'>" + wifiOptions + "</select><br><br>"
                          "Mat khau: <input type='password' name='pass'><br><br>"
                          "<input type='submit' value='Xac nhan'>"
                          "</form>";
            request->send(200, "text/html", html);
        }
    });

    server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true)) {
            reqSSID = request->getParam("ssid", true)->value();
            reqPass = request->hasParam("pass", true) ? request->getParam("pass", true)->value() : "";
            shouldConnect = true;
            connectStatus = 0; 
            request->send(200, "text/html", "<meta http-equiv='refresh' content='3; url=/result'><h2>Dang ket noi vao " + reqSSID + "...</h2>");
        }
    });

    server.on("/result", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (connectStatus == 0) {
            request->send(200, "text/html", "<meta http-equiv='refresh' content='2; url=/result'><h2>Van dang xu ly...</h2>");
        } else if (connectStatus == -1) {
            request->send(200, "text/html", "<h2>Ket noi that bai!</h2><a href='/'>Thu lai</a>");
        } else if (connectStatus == 1) {
            String html = "<h2>Thanh cong! Dang chuyen trang...</h2>"
                          "<script>setTimeout(function(){ window.location.href = 'http://" + newIP + "'; }, 5000);</script>";
            request->send(200, "text/html", html);
            shouldTurnOffAP = true;
            turnOffTime = millis();
        }
    });

    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<meta http-equiv='refresh' content='2; url=/'>Tai len thanh cong! Dang tai lai...");
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        static File file;
        if (!index) file = LittleFS.open("/" + filename, FILE_WRITE);
        if (file) file.write(data, len);
        if (final && file) file.close();
    });

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("file")) request->send(LittleFS, "/" + request->getParam("file")->value(), "text/plain", true); 
    });

    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "[";
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        bool isFirst = true;
        
        while(file){
            if(!isFirst) json += ",";
            String fileName = String(file.name());
            if(fileName.startsWith("/")) fileName = fileName.substring(1); 
            
            json += "{\"name\":\"" + fileName + "\",\"size\":" + String(file.size()) + "}";
            isFirst = false;
            file = root.openNextFile();
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    server.on("/delete", HTTP_DELETE, [](AsyncWebServerRequest *request){
        if (request->hasParam("file")) {
            String fileName = request->getParam("file")->value();
            if (LittleFS.remove("/" + fileName)) {
                request->send(200, "text/plain", "Da xoa");
            } else {
                request->send(500, "text/plain", "Loi khi xoa file");
            }
        } else {
            request->send(400, "text/plain", "Thieu ten file");
        }
    });

    server.on("/send-nano", HTTP_POST, [](AsyncWebServerRequest *request){
        Serial2.println("ON"); // Gửi tín hiệu hoặc chuỗi dữ liệu sang Nano ở đây
        request->send(200, "text/plain", "Đã truyền dữ liệu");
    });
}

void EspWebManager::loop() {
    if (shouldConnect) {
        shouldConnect = false;

        WiFi.disconnect();
        delay(200);
        WiFi.begin(reqSSID.c_str(), reqPass.c_str());
        int timeout = 0;
        
        while (WiFi.status() != WL_CONNECTED && timeout < 30) { 
            delay(500);
            timeout++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            newIP = WiFi.localIP().toString();
            connectStatus = 1; 
        } else {
            connectStatus = -1;
        }
    }

    if (shouldTurnOffAP && (millis() - turnOffTime > 3000)) {
        shouldTurnOffAP = false;
        WiFi.softAPdisconnect(true);
    }
}

void EspWebManager::sendToArduino(String data) {
    if (Serial2) {
        Serial2.println(data);
    }
}
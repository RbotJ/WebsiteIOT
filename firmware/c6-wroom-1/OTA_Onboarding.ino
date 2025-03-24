// pull the first update and get the controller into your OTA system.
// If upload error, start download (compile) hold boot, press reset, release boot after upload starts

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// Replace with your WiFi credentials
const char* ssid = "Wifi";
const char* password = "Pass";

// Direct .bin firmware file
const char* firmwareURL = "https://raw.githubusercontent.com/"; // raw url to .bin file

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  performOTA();
}

void performOTA() {
  Serial.println("Starting OTA update...");
  HTTPClient http;
  http.begin(firmwareURL);
  int httpCode = http.GET();

  if (httpCode == 200) {
    int contentLength = http.getSize();
    WiFiClient* stream = http.getStreamPtr();

    if (!Update.begin(contentLength)) {
      Serial.println("Not enough space for OTA");
      return;
    }

    size_t written = Update.writeStream(*stream);
    if (written == contentLength && Update.end() && Update.isFinished()) {
      Serial.println("OTA Success! Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      Serial.println("OTA Failed");
      if (!Update.end()) {
        Serial.printf("Update Error #: %d\n", Update.getError());
      }
    }
  } else {
    Serial.printf("HTTP error code: %d\n", httpCode);
  }

  http.end();
}

void loop() {
  // Nothing to do here
}

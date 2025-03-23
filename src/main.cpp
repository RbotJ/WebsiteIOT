#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Update.h>

Preferences prefs;
WebServer server(80);

const char* ap_ssid = "ESP32-Setup";
const char* ap_password = "setup123";


String inputSSID, inputPASS;
bool shouldReboot = false;
String remoteVersion = "";
bool updateAvailable = false;

// GitHub raw firmware URL
const char* firmwareUrl = "https://github.com/RbotJ/WebsiteIOT/releases/latest/download/version.txt
";
const char* versionUrl  = "https://raw.githubusercontent.com/RbotJ/WebsiteIOT/main/firmware/c6-wroom-1/version.txt";

void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Started AP mode");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", R"rawliteral(
      <form action="/save" method="POST">
        SSID: <input name="ssid"><br>
        Password: <input name="pass" type="password"><br>
        <input type="submit" value="Save">
      </form>
    )rawliteral");
  });

  server.on("/save", HTTP_POST, []() {
    inputSSID = server.arg("ssid");
    inputPASS = server.arg("pass");

    prefs.begin("wifi", false);
    prefs.putString("ssid", inputSSID);
    prefs.putString("pass", inputPASS);
    prefs.end();

    server.send(200, "text/html", "Saved. Rebooting...");
    shouldReboot = true;
  });

  server.begin();
}

bool connectToWiFi() {
  prefs.begin("wifi", true);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");
  prefs.end();

  if (ssid == "") return false;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.print("Connecting to WiFi");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("\nFailed to connect.");
    return false;
  }
}

void checkRemoteVersion() {
  HTTPClient http;
  http.begin(versionUrl);
  int httpCode = http.GET();
  if (httpCode == 200) {
    remoteVersion = http.getString();
    remoteVersion.trim();
    Serial.print("Remote version: ");
    Serial.println(remoteVersion);
    if (remoteVersion != currentVersion) {
      updateAvailable = true;
    }
  } else {
    Serial.printf("Failed to check version: %d\n", httpCode);
  }
  http.end();
}

void checkForUpdates() {
  Serial.println("Checking for OTA update...");

  HTTPClient http;
  http.begin(firmwareUrl);
  int httpCode = http.GET();

  if (httpCode == 200) {
    int contentLength = http.getSize();
    WiFiClient* stream = http.getStreamPtr();

    if (!Update.begin(contentLength)) {
      Serial.println("Update begin failed!");
      return;
    }

    size_t written = Update.writeStream(*stream);
    if (written == contentLength && Update.end() && Update.isFinished()) {
      Serial.println("Update successful! Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      Serial.println("Update failed.");
    }
  } else {
    Serial.printf("HTTP error code: %d\n", httpCode);
  }

  http.end();
}

void startUpdatePromptServer() {
  server.on("/", HTTP_GET, []() {
    String html = "<h1>IOT Controller</h1>";
    html += "<p><strong>Firmware Version:</strong> " + String(currentVersion) + "</p>";
    html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";

    if (updateAvailable) {
      html += "<p style='color:orange'><strong>Update Available:</strong> " + remoteVersion + "</p>";
      html += "<a href='/update'><button>Update Now</button></a>";
    } else {
      html += "<p style='color:green'>Your firmware is up to date.</p>";
    }

    html += "<p><a href='/info'>System Info</a></p>";
    server.send(200, "text/html", html);
  });

  server.on("/update", HTTP_GET, []() {
    if (updateAvailable) {
      server.send(200, "text/html", R"html(
        <h2>Firmware Update Available!</h2>
        <p>New version: )html" + remoteVersion + R"html(</p>
        <form action="/confirm_update" method="POST">
          <input type="submit" value="Install Update">
        </form>
      )html");
    } else {
      server.send(200, "text/html", "<p>Your firmware is up to date. No updates available.</p><p><a href='/'>Back to Home</a></p>");
    }
  });

  server.on("/confirm_update", HTTP_POST, []() {
    server.send(200, "text/html", "<p>Updating firmware, please wait...</p>");
    delay(1000);
    checkForUpdates(); // Pull and apply .bin
  });

  server.on("/info", HTTP_GET, []() {
    String info = "IOT Controller\n";
    info += "Current Firmware Version: " + String(currentVersion) + "\n";
    info += "Remote Version: " + (remoteVersion != "" ? remoteVersion : "Unknown") + "\n";
    info += "IP Address: " + WiFi.localIP().toString() + "\n";
    info += "Update Available: " + String(updateAvailable ? "Yes" : "No") + "\n";
    server.send(200, "text/plain", info);
  });

  server.begin();
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!connectToWiFi()) {
    startAccessPoint();
  } else {
    checkRemoteVersion();
    startUpdatePromptServer();  // <-- Always start the server
    if (updateAvailable) {
      Serial.println("Update available. Host webpage at /update");
    } else {
      Serial.println("No update available.");
    }
  }
}


void loop() {
  server.handleClient();  // Always handle server

  if (shouldReboot) {
    delay(2000);
    ESP.restart();
  }
}


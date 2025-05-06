#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // Include the WiFiManager library

// SD card CS pin
#define SD_CS_PIN D8

// Web server
ESP8266WebServer server(80);
WiFiManager wifiManager; // Create a WiFiManager instance

File myFile;

// ---------- Web Handlers ----------

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>ESP8266 File Upload</title>
      <style>
        body {
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          flex-direction: column;
          font-family: Arial, sans-serif;
          margin: 0;
          padding: 20px;
          box-sizing: border-box;
          background-color: #f9f9f9;
        }
        input[type='file'], input[type='submit'], button {
          margin: 10px;
          padding: 10px 20px;
          font-size: 16px;
          border: none;
          border-radius: 5px;
          cursor: pointer;
        }
        input[type='submit'], button {
          background-color: #4caf50;
          color: white;
        }
        #progress {
          width: 100%;
          max-width: 400px;
          background-color: #f3f3f3;
          border: 1px solid #ccc;
          margin-top: 10px;
          border-radius: 5px;
        }
        #bar {
          width: 0%;
          height: 30px;
          background-color: #4caf50;
          border-radius: 5px;
        }
      </style>
      <script>
        function uploadFile() {
          const fileInput = document.getElementById('fileInput');
          const file = fileInput.files[0];
          const formData = new FormData();
          formData.append('data', file);

          const xhr = new XMLHttpRequest();
          xhr.open('POST', '/upload', true);

          xhr.upload.onprogress = function(e) {
            if (e.lengthComputable) {
              const percentComplete = (e.loaded / e.total) * 100;
              document.getElementById('bar').style.width = percentComplete + '%';
            }
          };

          xhr.onload = function() {
            if (xhr.status === 200) {
              alert('File uploaded successfully!');
              document.getElementById('bar').style.width = '0%';
              fileInput.value = ""; // Reset file input
            } else {
              alert('Upload failed.');
            }
          };

          xhr.send(formData);
        }

        function resetWiFi() {
          if (confirm("Are you sure you want to reset WiFi settings?")) {
            fetch("/reset", { method: "POST" })
              .then(response => response.text())
              .then(data => alert(data))
              .catch(err => alert("Reset failed."));
          }
        }
      </script>
    </head>
    <body>
      <h2>ESP8266 File Upload</h2>
      <form onsubmit="event.preventDefault(); uploadFile();">
        <input type="file" id="fileInput" name="data" required><br>
        <input type="submit" value="Upload">
      </form>
      <div id="progress"><div id="bar"></div></div>
      <br>
      <button onclick="resetWiFi()">Reset WiFi Settings</button>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

unsigned long uploadStartTime = 0;
size_t totalBytesWritten = 0;

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  String filename = "/" + upload.filename;

  if (upload.status == UPLOAD_FILE_START) {
    Serial.print("Uploading: "); Serial.println(filename);
    if (SD.exists(filename)) SD.remove(filename);
    myFile = SD.open(filename, FILE_WRITE);
    if (!myFile) {
      Serial.println("Failed to open file");
      return;
    }
    uploadStartTime = millis();
    totalBytesWritten = 0;
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (myFile) {
      myFile.write(upload.buf, upload.currentSize);
      totalBytesWritten += upload.currentSize;
    }
    Serial.print("Written: "); Serial.println(upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (myFile) {
      myFile.close();
    }
    unsigned long uploadDuration = millis() - uploadStartTime;
    float speedKBps = (float)totalBytesWritten / (float)uploadDuration; // KB/s

    Serial.println("Upload complete.");
    Serial.print("Total bytes written: "); Serial.println(totalBytesWritten);
    Serial.print("Time taken (ms): "); Serial.println(uploadDuration);
    Serial.print("Write speed: "); Serial.print(speedKBps); Serial.println(" KB/s");

    server.send(200, "text/plain", "File uploaded successfully");
  } else {
    Serial.println("Upload failed");
  }
}


void handleReset() {
  server.send(200, "text/html", "<h2>Resetting WiFi settings...</h2><p>ESP will restart.</p>");
  delay(2000);
  wifiManager.resetSettings();
  ESP.restart();
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

// ---------- Setup and Loop ----------

void setup() {
  Serial.begin(115200);
  delay(1000);
  wifiManager.setHostname("espcard");
  // Attempt auto connect or open config portal if no known WiFi
  wifiManager.autoConnect("ESPcard-Setup");
  // Start mDNS
  if (!MDNS.begin("espcard")) {
    Serial.println("Error starting mDNS");
  } else {
    Serial.println("mDNS started: http://espcard.local");
    MDNS.addService("http", "tcp", 80);
  }

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Setup routes
  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, []() {}, handleFileUpload);
  server.on("/reset", HTTP_POST, handleReset);
  server.onNotFound(handleNotFound);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  MDNS.update();
}

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#define USE_SERIAL Serial

const char* ssid = "iPhone";  // WiFi SSID
const char* password = "012345678";      // WiFi Password

WebServer server(80);  // Create web server on port 80

// Handle root URL request
void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from ESP32!</h1>");
}

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.println("\n\nConnecting to WiFi...");

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    USE_SERIAL.print(".");
  }
  
  USE_SERIAL.println("\nWiFi connected!");
  USE_SERIAL.print("IP Address: ");
  USE_SERIAL.println(WiFi.localIP());  // Print ESP32's IP address

  server.on("/", handleRoot);  // Define route for the root URL
  server.begin();  // Start web server
  USE_SERIAL.println("Web server started!");
}

void loop() {
  server.handleClient();  // Listen for incoming client requests
}

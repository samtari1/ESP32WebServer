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
  // Read the current LED state
  int ledState = digitalRead(LED_BUILTIN);
  String switchState = (ledState == LOW) ? "off" : "on";  // Determine switch state based on LED
  
  // HTML content with a toggle switch
  String html = "<h1>Hello from ESP32!</h1>";
  html += "<p>Click the toggle switch to turn the LED on or off.</p>";
  html += "<label class=\"switch\">";
  html += "<input type=\"checkbox\" id=\"toggleSwitch\" ";
  html += (switchState == "on" ? "checked" : ""); // Correct concatenation here
  html += ">"; // Close the input element
  html += "<span class=\"slider\"></span>";
  html += "</label>";
  html += "<script>";
  html += "document.getElementById('toggleSwitch').addEventListener('change', function() {";
  html += "  fetch('/toggleLED');";  // This triggers the route to toggle the LED
  html += "});";
  html += "</script>";
  
  html += "<style>";
  html += "/* Toggle switch CSS */";
  html += ".switch {";
  html += "  position: relative;";
  html += "  display: inline-block;";
  html += "  width: 60px;";
  html += "  height: 34px;";
  html += "}";
  html += ".switch input {";
  html += "  opacity: 0;";
  html += "  width: 0;";
  html += "  height: 0;";
  html += "}";
  html += ".slider {";
  html += "  position: absolute;";
  html += "  cursor: pointer;";
  html += "  top: 0;";
  html += "  left: 0;";
  html += "  right: 0;";
  html += "  bottom: 0;";
  html += "  background-color: #ccc;";
  html += "  transition: .4s;";
  html += "  border-radius: 34px;";
  html += "}";
  html += ".slider:before {";
  html += "  position: absolute;";
  html += "  content: \"\";";
  html += "  height: 26px;";
  html += "  width: 26px;";
  html += "  border-radius: 50%;";
  html += "  left: 4px;";
  html += "  bottom: 4px;";
  html += "  background-color: white;";
  html += "  transition: .4s;";
  html += "}";
  html += "input:checked + .slider {";
  html += "  background-color: #4CAF50;";
  html += "}";
  html += "input:checked + .slider:before {";
  html += "  transform: translateX(26px);";
  html += "}";
  html += "</style>";
  
  server.send(200, "text/html", html);
}

// Handle LED toggle request
void handleToggleLED() {
  int ledState = digitalRead(LED_BUILTIN);  // Read current LED state
  if (ledState == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn on the LED
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
  }
  server.send(200, "text/plain", "LED Toggled");  // Respond to the request
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

  pinMode(LED_BUILTIN, OUTPUT);  // Set the built-in LED pin as output

  server.on("/", handleRoot);  // Define route for the root URL
  server.on("/toggleLED", handleToggleLED);  // Define route for toggling LED
  server.begin();  // Start web server
  USE_SERIAL.println("Web server started!");
}

void loop() {
  server.handleClient();  // Listen for incoming client requests
}

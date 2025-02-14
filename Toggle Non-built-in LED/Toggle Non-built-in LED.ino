#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#define USE_SERIAL Serial

const char* ssid = "iPhone";  // WiFi SSID
const char* password = "012345678";      // WiFi Password

// Define LED pins (can change these values as needed)
int ledPinBuiltIn = LED_BUILTIN;  // Built-in LED pin
int ledPin14 = 14;  // Pin 14 for the second LED

WebServer server(80);  // Create web server on port 80

// Handle root URL request
void handleRoot() {
  // Read the current state of both LEDs
  int ledStateBuiltIn = digitalRead(ledPinBuiltIn);
  int ledState14 = digitalRead(ledPin14);
  
  // Determine the switch state based on the LED state
  String switchStateBuiltIn = (ledStateBuiltIn == LOW) ? "off" : "on";
  String switchState14 = (ledState14 == LOW) ? "off" : "on";
  
  // HTML content with two toggle switches
  String html = "<h1>Hello from ESP32!</h1>";
  html += "<p>Click the toggle switches to turn the LEDs on or off.</p>";
  
  // Built-in LED toggle switch
  html += "<label class=\"switch\">";
  html += "<input type=\"checkbox\" id=\"toggleSwitchBuiltIn\" ";
  html += (switchStateBuiltIn == "on" ? "checked" : ""); // Check the switch if LED is on
  html += ">";
  html += "<span class=\"slider\"></span>";
  html += "</label>";
  html += "<p>Built-in LED</p>";

  // Pin 14 LED toggle switch
  html += "<label class=\"switch\">";
  html += "<input type=\"checkbox\" id=\"toggleSwitch14\" ";
  html += (switchState14 == "on" ? "checked" : ""); // Check the switch if LED is on
  html += ">";
  html += "<span class=\"slider\"></span>";
  html += "</label>";
  html += "<p>Pin 14 LED</p>";

  html += "<script>";
  html += "document.getElementById('toggleSwitchBuiltIn').addEventListener('change', function() {";
  html += "  fetch('/toggleLED?pin=" + String(ledPinBuiltIn) + "');";  // This triggers the route to toggle the built-in LED
  html += "});";
  html += "document.getElementById('toggleSwitch14').addEventListener('change', function() {";
  html += "  fetch('/toggleLED?pin=" + String(ledPin14) + "');";  // This triggers the route to toggle Pin 14 LED
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
  String pinParam = server.arg("pin");  // Get the pin parameter from the URL
  int pin = pinParam.toInt();  // Convert the pin parameter to an integer

  int ledState = digitalRead(pin);  // Read the current LED state for the specified pin
  if (ledState == LOW) {
    digitalWrite(pin, HIGH);  // Turn on the LED
  } else {
    digitalWrite(pin, LOW);  // Turn off the LED
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

  pinMode(ledPinBuiltIn, OUTPUT);  // Set the built-in LED pin as output
  pinMode(ledPin14, OUTPUT);  // Set Pin 14 as output

  server.on("/", handleRoot);  // Define route for the root URL
  server.on("/toggleLED", handleToggleLED);  // Define route for toggling LED
  server.begin();  // Start web server
  USE_SERIAL.println("Web server started!");
}

void loop() {
  server.handleClient();  // Listen for incoming client requests
}

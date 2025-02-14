#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define USE_SERIAL Serial

const char* ssid = "iPhone";  // WiFi SSID
const char* password = "012345678";      // WiFi Password
const char* openai_api_key = "your_OpenAI_API_Key_here";  // Replace with your OpenAI API key
const char* openai_endpoint = "https://api.openai.com/v1/chat/completions";

// Define LED pins (can change these values as needed)
int ledPinBuiltIn = LED_BUILTIN;  // Built-in LED pin
int ledPin14 = 14;  // Pin 14 for the second LED

WebServer server(80);  // Create web server on port 80

// Function to control LED
void controlLED(const char* led_name, bool state) {
  int pin = (String(led_name) == "builtin") ? ledPinBuiltIn : ledPin14;
  digitalWrite(pin, state ? HIGH : LOW);
}

String handleChatRequest(String message) {
  USE_SERIAL.println("\nUser message: " + message);
  
  HTTPClient http;
  http.begin(openai_endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + openai_api_key);

  StaticJsonDocument<2048> doc;
  doc["model"] = "gpt-4o-mini";  // Changed from gpt-4o-mini to gpt-4
  
  JsonArray messages = doc.createNestedArray("messages");
  
  JsonObject systemMessage = messages.createNestedObject();
  systemMessage["role"] = "system";
  systemMessage["content"] = "You are an assistant that controls LEDs. Use the control_led function to manage the LED states.";
  
  JsonObject userMessage = messages.createNestedObject();
  userMessage["role"] = "user";
  userMessage["content"] = message;

  // Add function calling configuration
  JsonArray tools = doc.createNestedArray("tools");
  JsonObject tool = tools.createNestedObject();
  tool["type"] = "function";
  
  JsonObject function = tool["function"].to<JsonObject>();
  function["name"] = "control_led";
  function["description"] = "Control the state of an LED";
  
  JsonObject parameters = function["parameters"].to<JsonObject>();
  parameters["type"] = "object";
  
  JsonObject properties = parameters["properties"].to<JsonObject>();
  
  JsonObject led_name = properties["led_name"].to<JsonObject>();
  led_name["type"] = "string";
  led_name["enum"][0] = "builtin";
  led_name["enum"][1] = "pin14";
  led_name["description"] = "The LED to control (builtin or pin14)";
  
  JsonObject state = properties["state"].to<JsonObject>();
  state["type"] = "boolean";
  state["description"] = "true to turn on, false to turn off";
  
  JsonArray required = parameters["required"].to<JsonArray>();
  required.add("led_name");
  required.add("state");

  String jsonString;
  serializeJson(doc, jsonString);
  USE_SERIAL.println("Sending request: " + jsonString);  // Debug print

  int httpResponseCode = http.POST(jsonString);
  String response = "Error communicating with OpenAI";
  
  if (httpResponseCode > 0) {
    response = http.getString();
    USE_SERIAL.println("Raw response: " + response);  // Debug print
    
    StaticJsonDocument<2048> responseDoc;  // Increased buffer size
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (error) {
      USE_SERIAL.println("JSON parsing failed: " + String(error.c_str()));
      return "Error parsing response";
    }

    // Check for function calls in the response
    if (responseDoc["choices"][0]["message"].containsKey("tool_calls")) {
      JsonArray toolCalls = responseDoc["choices"][0]["message"]["tool_calls"];
      for (JsonVariant tool_call : toolCalls) {
        if (tool_call["function"]["name"] == "control_led") {
          StaticJsonDocument<200> argsDoc;
          String args = tool_call["function"]["arguments"].as<const char*>();
          USE_SERIAL.println("Function arguments: " + args);  // Debug print
          
          DeserializationError argError = deserializeJson(argsDoc, args);
          if (argError) {
            USE_SERIAL.println("Args parsing failed: " + String(argError.c_str()));
            continue;
          }
          
          const char* led_name = argsDoc["led_name"];
          bool state = argsDoc["state"];
          
          controlLED(led_name, state);
          
          return String("LED ") + led_name + " turned " + (state ? "on" : "off");
        }
      }
    }
    
    // If no function call was found, return the normal message
    if (responseDoc["choices"][0]["message"].containsKey("content")) {
      return responseDoc["choices"][0]["message"]["content"].as<String>();
    }
    
    return "Received response but no valid message or function call found";
  } else {
    USE_SERIAL.println("HTTP Error: " + String(httpResponseCode));
    return "Error " + String(httpResponseCode) + ": " + http.getString();
  }
}

void handleChat() {
  String message = server.arg("message");
  USE_SERIAL.println("\n--- New Chat Message ---");  // Print conversation separator
  String response = handleChatRequest(message);
  server.send(200, "text/plain", response);
}

// Handle root URL request
void handleRoot() {
  // Read the current state of both LEDs
  int ledStateBuiltIn = digitalRead(ledPinBuiltIn);
  int ledState14 = digitalRead(ledPin14);
  
  // Determine the switch state based on the LED state
  String switchStateBuiltIn = (ledStateBuiltIn == LOW) ? "off" : "on";
  String switchState14 = (ledState14 == LOW) ? "off" : "on";
  
  // HTML content with two toggle switches
  String html = "<h1>ESP32 Control Panel</h1>";
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

  // Add chat interface
  html += "<div style='margin: 20px; padding: 20px; border: 1px solid #ccc;'>";
  html += "<h2>Chat with AI</h2>";
  html += "<div id='chatHistory' style='height: 200px; overflow-y: scroll; margin-bottom: 10px; border: 1px solid #eee; padding: 10px;'></div>";
  html += "<input type='text' id='chatInput' style='width: 80%; padding: 5px;'>";
  html += "<button onclick='sendMessage()' style='margin-left: 10px; padding: 5px 10px;'>Send</button>";
  html += "</div>";

  html += "<script>";
  html += "document.getElementById('toggleSwitchBuiltIn').addEventListener('change', function() {";
  html += "  fetch('/toggleLED?pin=" + String(ledPinBuiltIn) + "');";  // This triggers the route to toggle the built-in LED
  html += "});";
  html += "document.getElementById('toggleSwitch14').addEventListener('change', function() {";
  html += "  fetch('/toggleLED?pin=" + String(ledPin14) + "');";  // This triggers the route to toggle Pin 14 LED
  html += "});";
  html += "function sendMessage() {";
  html += "  var input = document.getElementById('chatInput');";
  html += "  var message = input.value;";
  html += "  if(message.trim() === '') return;";
  html += "  appendMessage('You: ' + message);";
  html += "  input.value = '';";
  html += "  fetch('/chat?message=' + encodeURIComponent(message))";
  html += "    .then(response => response.text())";
  html += "    .then(response => {";
  html += "      appendMessage('AI: ' + response);";
  html += "    });";
  html += "}";
  html += "function appendMessage(message) {";
  html += "  var history = document.getElementById('chatHistory');";
  html += "  history.innerHTML += '<div>' + message + '</div>';";
  html += "  history.scrollTop = history.scrollHeight;";
  html += "}";
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
  server.on("/chat", handleChat);  // Add chat endpoint
  server.begin();  // Start web server
  USE_SERIAL.println("Web server started!");
}

void loop() {
  server.handleClient();  // Listen for incoming client requests
}

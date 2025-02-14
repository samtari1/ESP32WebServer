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
  
  String switchStateBuiltIn = (ledStateBuiltIn == LOW) ? "off" : "on";
  String switchState14 = (ledState14 == LOW) ? "off" : "on";
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 Control Panel</title>";
  
  // Updated CSS
  html += "<style>";
  html += "* { box-sizing: border-box; margin: 0; padding: 0; }";
  html += "body { font-family: Arial, sans-serif; padding: 15px; max-width: 600px; margin: 0 auto; }";
  html += "h1 { font-size: 24px; margin-bottom: 15px; }";
  html += "h2 { font-size: 20px; margin-bottom: 10px; }";
  html += ".control-panel { margin-bottom: 20px; }";
  html += ".led-control { display: flex; align-items: center; margin: 15px 0; }";
  html += ".led-label { margin-left: 10px; font-size: 16px; }";
  
  // Updated switch styles
  html += ".switch { position: relative; display: inline-block; width: 52px; height: 28px; }";
  html += ".switch input { opacity: 0; width: 0; height: 0; }";
  html += ".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; ";
  html += "background-color: #ccc; transition: .4s; border-radius: 28px; }";
  html += ".slider:before { position: absolute; content: \"\"; height: 20px; width: 20px; ";
  html += "left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }";
  html += "input:checked + .slider { background-color: #4CAF50; }";
  html += "input:checked + .slider:before { transform: translateX(24px); }";
  
  // Chat interface styles
  html += ".chat-container { border: 1px solid #ddd; border-radius: 8px; padding: 15px; margin-top: 20px; }";
  html += "#chatHistory { height: 300px; overflow-y: auto; border: 1px solid #eee; border-radius: 4px; ";
  html += "padding: 10px; margin-bottom: 15px; background: #f9f9f9; }";
  html += ".chat-input-container { display: flex; gap: 8px; }";
  html += "#chatInput { flex: 1; padding: 10px; border: 1px solid #ddd; border-radius: 4px; font-size: 16px; }";
  html += ".send-button { background: #4CAF50; color: white; border: none; padding: 10px 20px; ";
  html += "border-radius: 4px; cursor: pointer; font-size: 16px; }";
  html += ".message { margin: 8px 0; padding: 8px; border-radius: 4px; }";
  html += ".user-message { background: #e3f2fd; }";
  html += ".ai-message { background: #f5f5f5; }";
  html += "</style></head><body>";

  // Main content
  html += "<h1>ESP32 Control Panel</h1>";
  
  html += "<div class='control-panel'>";
  // Built-in LED control
  html += "<div class='led-control'>";
  html += "<label class='switch'>";
  html += "<input type='checkbox' id='toggleSwitchBuiltIn' " + String(switchStateBuiltIn == "on" ? "checked" : "") + ">";
  html += "<span class='slider'></span>";
  html += "</label>";
  html += "<span class='led-label'>Built-in LED</span>";
  html += "</div>";
  
  // Pin 14 LED control
  html += "<div class='led-control'>";
  html += "<label class='switch'>";
  html += "<input type='checkbox' id='toggleSwitch14' " + String(switchState14 == "on" ? "checked" : "") + ">";
  html += "<span class='slider'></span>";
  html += "</label>";
  html += "<span class='led-label'>Pin 14 LED</span>";
  html += "</div>";
  html += "</div>";

  // Chat interface
  html += "<div class='chat-container'>";
  html += "<h2>Chat with AI</h2>";
  html += "<div id='chatHistory'></div>";
  html += "<div class='chat-input-container'>";
  html += "<input type='text' id='chatInput' placeholder='Type your message...'>";
  html += "<button class='send-button' onclick='sendMessage()'>Send</button>";
  html += "</div></div>";

  // JavaScript
  html += "<script>";
  // Add status update function
  html += "function updateLEDStates() {";
  html += "  fetch('/ledstates')";
  html += "    .then(response => response.json())";
  html += "    .then(states => {";
  html += "      document.getElementById('toggleSwitchBuiltIn').checked = states.builtin;";
  html += "      document.getElementById('toggleSwitch14').checked = states.pin14;";
  html += "    });";
  html += "}";
  
  // Add periodic update
  html += "setInterval(updateLEDStates, 1000);"; // Check every second
  
  // Modify existing toggle event listeners
  html += "document.getElementById('toggleSwitchBuiltIn').addEventListener('change', function() {";
  html += "  fetch('/toggleLED?pin=" + String(ledPinBuiltIn) + "')";
  html += "    .then(() => updateLEDStates());"; // Update status after toggle
  html += "});";
  html += "document.getElementById('toggleSwitch14').addEventListener('change', function() {";
  html += "  fetch('/toggleLED?pin=" + String(ledPin14) + "')";
  html += "    .then(() => updateLEDStates());"; // Update status after toggle
  html += "});";
  
  // Rest of the existing JavaScript
  html += "function sendMessage() {";
  html += "  var input = document.getElementById('chatInput');";
  html += "  var message = input.value;";
  html += "  if(message.trim() === '') return;";
  html += "  appendMessage('You: ' + message, 'user-message');";
  html += "  input.value = '';";
  html += "  fetch('/chat?message=' + encodeURIComponent(message))";
  html += "    .then(response => response.text())";
  html += "    .then(response => {";
  html += "      appendMessage('AI: ' + response, 'ai-message');";
  html += "    });";
  html += "}";
  html += "function appendMessage(message, className) {";
  html += "  var history = document.getElementById('chatHistory');";
  html += "  var div = document.createElement('div');";
  html += "  div.className = 'message ' + className;";
  html += "  div.textContent = message;";
  html += "  history.appendChild(div);";
  html += "  history.scrollTop = history.scrollHeight;";
  html += "}";
  html += "document.getElementById('chatInput').addEventListener('keypress', function(e) {";
  html += "  if (e.key === 'Enter') sendMessage();";
  html += "});";
  html += "</script></body></html>";
  
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

// Add this new handler function before setup():
void handleGetLEDStates() {
  StaticJsonDocument<200> doc;
  doc["builtin"] = digitalRead(ledPinBuiltIn);
  doc["pin14"] = digitalRead(ledPin14);
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
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
  server.on("/ledstates", handleGetLEDStates);  // Add this line
  server.begin();  // Start web server
  USE_SERIAL.println("Web server started!");
}

void loop() {
  server.handleClient();  // Listen for incoming client requests
}

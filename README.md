# ESP32 Web Server Projects

This repository contains a collection of ESP32 projects that demonstrate various web server functionalities, from basic LED control to AI-powered chatbots.

## Setup Instructions

### 1. Installing Arduino IDE
1. Download Arduino IDE from [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)
2. Install the software following the instructions for your operating system

### 2. Installing ESP32 Board Support
1. Open Arduino IDE
2. Go to `File > Preferences`
3. In "Additional Boards Manager URLs" add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to `Tools > Board > Boards Manager`
5. Search for "esp32"
6. Install "ESP32 by Espressif Systems"

### 3. Configuring Board Settings
1. Select `Tools > Board > ESP32 Arduino > uPesy ESP32 Wroom DevKit`
2. Select the correct COM port under `Tools > Port`
   - On Windows: It will appear as "COM" followed by a number
   - On macOS: It will be "/dev/cu.usbserial-*"
   - On Linux: It will be "/dev/ttyUSB*"
3. Set Upload Speed to "115200"

### 4. Required Libraries
Install these libraries from `Tools > Manage Libraries`:
- `WiFi`
- `WebServer`
- `ArduinoJson` (for chatbot projects)
- `HTTPClient` (for chatbot projects)

## Projects (From Basic to Advanced)

### 1. Hello World (Hello_ESP32.ino)
- Basic web server that displays "Hello from ESP32!"
- Perfect for testing your ESP32 setup
- Demonstrates basic WiFi connection and web server creation

### 2. Toggle Built-in LED (Button.ino)
- Control the ESP32's built-in LED through a web interface
- Features a stylish toggle switch
- Introduces basic GPIO control and HTML/CSS styling

### 3. Toggle Non-built-in LED (Toggle Non-built-in LED.ino)
- Controls both built-in LED and an external LED on Pin 14
- Multiple toggle switches in the web interface
- Demonstrates handling multiple GPIO pins and URL parameters

### 4. Basic Chatbot (Chatbot.ino)
- Implements a basic ChatGPT integration
- Web interface with chat history
- Requires OpenAI API key
- Demonstrates HTTP client usage and JSON handling

### 5. ChatbotControlLED (ChatbotControlLED.ino)
- Combines LED control with AI chat capabilities
- Use natural language to control LEDs
- Advanced function calling with OpenAI API
- Demonstrates integration of multiple features

### 6. ChatbotControlLED with Better UI (ChatbotControlLED_BetterUI.ino)
- Enhanced version of ChatbotControlLED
- Responsive design with mobile support
- Real-time LED state updates
- Improved chat interface
- Best example of full-featured web application

## Usage Instructions

1. Open desired .ino file in Arduino IDE
2. Update WiFi credentials:
   ```cpp
   const char* ssid = "Your_WiFi_SSID";
   const char* password = "Your_WiFi_Password";
   ```
3. For chatbot projects, add your OpenAI API key:
   ```cpp
   const char* openai_api_key = "your_OpenAI_API_Key_here";
   ```
4. Upload the code to your ESP32
5. Open Serial Monitor to get the IP address
6. Access the web interface through a browser using the IP address

## Hardware Setup

### Basic LED Projects
- Built-in LED: No additional hardware needed
- External LED:
  - LED connected to Pin 14
  - 220Ω resistor in series with LED
  - Ground connection

### Circuit Diagram
```
ESP32 Pin 14 ---> 220Ω Resistor ---> LED ---> GND
```

## Troubleshooting

1. Can't upload code?
   - Make sure the correct board and port are selected
   - Press and hold BOOT button while uploading
   - Check USB cable connection

2. WiFi won't connect?
   - Verify WiFi credentials
   - Ensure ESP32 is within range of WiFi network
   - Check Serial Monitor for connection status

3. Web page not loading?
   - Verify IP address in Serial Monitor
   - Ensure device is on same network as ESP32
   - Try refreshing the page

## Contributing

Feel free to submit issues and enhancement requests!

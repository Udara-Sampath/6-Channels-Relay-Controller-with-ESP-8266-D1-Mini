#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

// Replace with your WiFi network credentials
const char* ssid     = "SSID";
const char* password = "Password";

// Create a web server object that listens on port 80
ESP8266WebServer server(80);

// Define the 6 GPIO pins to control (using safe pins for the D1 mini)
const int pinCount = 6;
const int pins[pinCount] = {D0, D1, D2, D5, D6, D7};

// For active low relay modules: LOW means ON and HIGH means OFF.
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// Global array to hold the current state for each relay
int relayStates[pinCount];

// File path for saving state in LittleFS
const char* stateFile = "/state.txt";

// Helper function to save relay states to LittleFS
void saveStates() {
  File file = LittleFS.open(stateFile, "w");
  if (!file) {
    Serial.println("Error opening state file for writing");
    return;
  }
  
  // Write the states as a comma-separated list (e.g., "1,1,0,1,1,0")
  for (int i = 0; i < pinCount; i++) {
    file.print(relayStates[i]);
    if (i < pinCount - 1) {
      file.print(",");
    }
  }
  file.close();
  Serial.println("States saved to LittleFS.");
}

// Helper function to load relay states from LittleFS
void loadStates() {
  File file = LittleFS.open(stateFile, "r");
  if (!file) {
    Serial.println("State file not found, using default states (OFF).");
    // Default: all relays off (HIGH)
    for (int i = 0; i < pinCount; i++) {
      relayStates[i] = RELAY_OFF;
    }
    return;
  }
  
  String data = file.readString();
  file.close();
  data.trim();
  
  // Parse comma-separated values
  int index = 0;
  int start = 0;
  int commaIndex;
  while (index < pinCount && (commaIndex = data.indexOf(',', start)) != -1) {
    String token = data.substring(start, commaIndex);
    relayStates[index] = token.toInt();
    start = commaIndex + 1;
    index++;
  }
  // Get last value
  if (index < pinCount) {
    String token = data.substring(start);
    relayStates[index] = token.toInt();
    index++;
  }
  
  // Validate each value; if not valid, default to OFF (HIGH)
  for (int i = 0; i < pinCount; i++) {
    if (relayStates[i] != RELAY_ON && relayStates[i] != RELAY_OFF) {
      relayStates[i] = RELAY_OFF;
    }
  }
  
  Serial.print("Loaded states: ");
  for (int i = 0; i < pinCount; i++) {
    Serial.print((relayStates[i] == RELAY_ON) ? "ON " : "OFF ");
  }
  Serial.println();
}

// Blink the built-in LED for visual feedback (active LOW on most ESP8266 boards)
void blinkLed() {
  digitalWrite(LED_BUILTIN, LOW);  // Turn LED on (active LOW)
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED off
}

// Handler for /ping to respond with a simple "pong"
void handlePing() {
  server.send(200, "text/plain", "pong");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  // Load saved relay states (or use defaults if none exist)
  loadStates();

  // Setup each relay pin as OUTPUT and apply the stored state
  for (int i = 0; i < pinCount; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], relayStates[i]);
    Serial.print("Pin ");
    Serial.print(i);
    Serial.print(" (GPIO ");
    Serial.print(pins[i]);
    Serial.print(") initialised to ");
    Serial.println((relayStates[i] == RELAY_ON) ? "ON" : "OFF");
  }
  
  // Setup the built-in LED (usually on D4) for output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Ensure LED is off (active LOW)

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Define web server routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/ping", handlePing);

  // Start the web server
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}

// Serves a modern, responsive HTML page with 6 buttons for relay control
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Room Light Controls</title>";
  html += "<style>";
  html += "body { margin:0; padding:0; font-family: Arial, sans-serif; background: #f4f4f4; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; }";
  html += "h1 { color: #333; margin-bottom: 20px; }";
  html += ".button-container { display: flex; flex-wrap: wrap; justify-content: center; gap: 15px; }";
  html += "button { background: #007BFF; border: none; color: #fff; padding: 20px; font-size: 18px; border-radius: 8px; width: 150px; cursor: pointer; transition: background 0.3s ease; }";
  html += "button:hover { background: #0056b3; }";
  html += "@media (max-width: 600px) { button { width: 100px; padding: 15px; font-size: 16px; } }";
  html += "</style>";
  html += "<script>";
  html += "function togglePin(pin){";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/toggle?pin=' + pin, true);";
  html += "  xhr.send();";
  html += "}";
  // Connectivity check: ping the server every 5 seconds; if no response, reload the page
  html += "function checkConnectivity() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.timeout = 3000;";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState === XMLHttpRequest.DONE) {";
  html += "      if (xhr.status !== 200) {";
  html += "        location.reload();";
  html += "      }";
  html += "    }";
  html += "  };";
  html += "  xhr.onerror = function() { location.reload(); };";
  html += "  xhr.ontimeout = function() { location.reload(); };";
  html += "  xhr.open('GET', '/ping', true);";
  html += "  xhr.send();";
  html += "}";
  html += "setInterval(checkConnectivity, 5000);";
  html += "</script></head><body>";
  html += "<h1>Room Light Controls</h1>";
  html += "<div class='button-container'>";
  for (int i = 0; i < pinCount; i++) {
    html += "<button onclick='togglePin(" + String(i) + ")'>Light " + String(i+1) + "</button>";
  }
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Toggle the state of the specified pin, update LittleFS, log debug messages, and blink the built-in LED
void handleToggle() {
  Serial.println("Received toggle request.");
  
  if (!server.hasArg("pin")) {
    Serial.println("Error: Missing pin parameter.");
    server.send(400, "text/plain", "Missing pin parameter");
    return;
  }
  
  int index = server.arg("pin").toInt();
  if (index < 0 || index >= pinCount) {
    Serial.print("Error: Invalid pin index: ");
    Serial.println(index);
    server.send(400, "text/plain", "Invalid pin index");
    return;
  }
  
  int currentState = digitalRead(pins[index]);
  Serial.print("Toggling Pin ");
  Serial.print(index);
  Serial.print(" from ");
  Serial.print((currentState == RELAY_OFF) ? "OFF" : "ON");
  Serial.println("...");
  
  // Toggle the state: if currently off (HIGH), turn it on (LOW); if on (LOW), turn it off (HIGH)
  int newState = (currentState == RELAY_OFF) ? RELAY_ON : RELAY_OFF;
  digitalWrite(pins[index], newState);
  
  // Update our state array and save to LittleFS
  relayStates[index] = newState;
  saveStates();
  
  Serial.print("Pin ");
  Serial.print(index);
  Serial.print(" is now ");
  Serial.println((newState == RELAY_OFF) ? "OFF" : "ON");
  
  blinkLed();
  
  String response = "Light " + String(index+1) + " toggled to " + ((newState == RELAY_ON) ? "ON" : "OFF");
  server.send(200, "text/plain", response);
}

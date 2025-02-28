# 6 Channels Relay Controller with ESP 8266 D1 Mini

This project provides firmware for controlling a 6-channel relay module using an ESP8266 D1 mini. It features a modern, responsive web UI with auto-reload functionality if the connection is lost, and relay state persistence using LittleFS so that the last known states are restored on power-up.

## Features

- **6 Relay Channels:** Control six lights (or other loads) via relays.
- **Modern Responsive UI:** A full-screen, modern web interface with clearly labeled buttons ("Light 1" to "Light 6") and a title "Room Light Controls".
- **Auto-Reload on Disconnection:** The UI automatically pings the server and reloads the page if connectivity is lost.
- **State Persistence:** Relay states are saved to the ESP8266’s flash using LittleFS. On power loss and subsequent restart, the last saved state is restored.
- **Debug Output:** Serial logging for connection status, relay toggles, and state changes for easier debugging.
- **Active-Low Relay Logic:** Configured for relay modules that activate when their control input is driven LOW.

## Hardware Details

- **Microcontroller:** ESP8266 D1 mini.
- **Relay Module:** 6-channel relay module (designed for active low operation).  
  **Note:** Ensure your relay module is compatible with 3.3V logic or use appropriate level shifting.
- **Power Supply:** 5V, 6A power source (adequate for powering both the ESP8266 and the relay module).
- **Connections:**

  The following safe pins are used on the D1 mini to avoid interfering with the boot process:

  | Light Number | ESP8266 Pin | Arduino Label | GPIO Number |
  | ------------ | ----------- | ------------- | ----------- |
  | Light 1      | D0          | D0            | GPIO16      |
  | Light 2      | D1          | D1            | GPIO5       |
  | Light 3      | D2          | D2            | GPIO4       |
  | Light 4      | D5          | D5            | GPIO14      |
  | Light 5      | D6          | D6            | GPIO12      |
  | Light 6      | D7          | D7            | GPIO13      |

  - **Built-in LED:** Typically on D4, used for visual feedback (blinks on every relay toggle).

## Software Details

- **Development Environment:** Arduino IDE with ESP8266 board support.
- **Libraries Used:**
  - `ESP8266WiFi` – For WiFi connectivity.
  - `ESP8266WebServer` – To run the HTTP server.
  - `LittleFS` – For file system support to persist relay states.

## How It Works

1. **Web Interface:**

   - The ESP8266 hosts a web server on port 80.
   - A modern, responsive web page is served which displays six buttons labeled "Light 1" to "Light 6".
   - Buttons send HTTP GET requests to toggle the respective relay channels.

2. **Relay Control:**

   - Relays use active low logic: setting a GPIO LOW turns the relay on, and HIGH turns it off.
   - Each toggle command updates the physical relay and saves the new state.

3. **State Persistence:**

   - Relay states are stored in `/state.txt` on LittleFS as a comma-separated list.
   - On startup, the stored states are loaded and applied to each relay.

4. **Auto-Reload Mechanism:**

   - The web page pings the `/ping` endpoint every 5 seconds.
   - If the ping request fails (e.g. due to a disconnection), the page automatically reloads to re-establish the connection.

5. **Debugging:**
   - Serial output is used to log connection status, relay toggles, and state changes, which can be viewed via the Serial Monitor in the Arduino IDE.

## Setup Instructions

1. **Install Arduino IDE** and add the ESP8266 board package.
2. **Install Required Libraries:**  
   Make sure the following libraries are installed:
   - `ESP8266WiFi`
   - `ESP8266WebServer`
   - `LittleFS`
3. **Configure WiFi Credentials:**  
   Edit the sketch to update your WiFi SSID and password.
4. **Upload the Sketch:**  
   Connect your ESP8266 D1 mini and upload the code using the Arduino IDE.
5. **Access the Web Interface:**  
   Open the Serial Monitor to note the assigned IP address, then open that IP in your web browser.

## Source Code

The complete code is included in this repository and incorporates:

- Modern, responsive UI with auto-reload on connectivity loss.
- Relay control using safe GPIO pins to avoid boot issues.
- State persistence using LittleFS.
- Detailed Serial debug messages.

## Developer

**Udara Sampath**

Contributions, issues, and suggestions are welcome. Please feel free to fork and modify the project.

## License

This project is open-source and available under the MIT License.

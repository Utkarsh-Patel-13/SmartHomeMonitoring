# Smart Home Monitor

## Description
The Smart Home Monitor is an IoT project that integrates various sensors to monitor environmental conditions such as temperature, humidity, and light levels. It features a user interface on a TFT display and can control a servo motor for temperature regulation. The system can operate in different modes (AUTO, MANUAL, OFF) and communicates with a remote server to send sensor data and receive configuration settings.

## Instructions to Run

### Hardware Setup
1. **Connect the Sensors and Components:**
   - Connect the DHT20 temperature and humidity sensor.
   - Connect the LDR (Light Dependent Resistor) for light level detection.
   - Connect the servo motor for temperature regulation.
   - Connect the capacitive touch sensor for mode selection.
   - Ensure the TFT display is connected properly.

### Software Setup

1. **Clone the Repository:**
   ```bash
   git clone <repository-url>
   cd <repository-folder>
   ```

2. **Install Backend Requirements:**
   Navigate to the backend folder and install the required packages:
   ```bash
   cd backend
   pip install -r requirements.txt
   ```

3. **Update WiFi Credentials:**
   In `src/main.cpp`, change the following lines to update the WiFi credentials:
   ```cpp
   const char *ssid = "YOUR_SSID";
   const char *password = "YOUR_PASSWORD";
   ```

4. **Update API URLs:**
   In `webapp/frontend/constants.js`, update the API URL if necessary:
   ```javascript
   export const API_URL = "http://YOUR_SERVER_IP:5001/api";
   ```

5. **Run the Frontend:**
   Navigate to the frontend folder and install the required packages:
   ```bash
   cd webapp/frontend
   npm install
   ```

   Then, start the development server:
   ```bash
   npm run dev
   ```

6. **Compile and Upload the Code:**
   Use the Arduino IDE or PlatformIO to compile and upload the code to your microcontroller.

7. **Run the Backend Server:**
   Ensure the backend server is running to handle API requests.

### Usage
- The system will initialize and connect to WiFi.
- Use the capacitive touch sensor to switch between operation modes (AUTO, MANUAL, OFF).
- The TFT display will show current sensor readings and allow toggling between current values and threshold settings.

## Notes
- Ensure that the server IP and ports are correctly configured to match your network setup.
- Adjust the sensor thresholds in the code as needed for your specific environment.

# ESP32 Wi-Fi Probe Sniffer

## 1. Overview
An ESP32 project that listens to nearby Wi-Fi traffic for 802.11 probe request frames. It uses the payload from captured packets to extract useful information, including timestamp, Wi-Fi channel, RSSI, device’s average RSSI, estimated distance (cm), MAC address, and SSID. It saves this information in its SPIFFs as a CSV which can be retrieved later from another device running retrieval.py.
- **Disclaimer:** For educational and research use only.

## 2. Hardware Design
- Uses an ESP32 microcontroller from ELEGOO.
- The onboard blue LED flashes each time a relevant packet is captured
- Powered via USB-C and stores logs in internal SPIFFS memory.

## 3. Software Architecture
- **Packet Sniffer Core:** When Powered on the ESP32 goes into Promiscuous mode to listen for packets. When a packet is received it checks to make sure it is the correct type of packet. If it is the right type it takes the information from the payload.
- **Data Logger:** When the data has been extracted from a packet the program queues the information in a buffer. Periodically the buffer is flushed and the data gets moved into the SPIFFS.
- **Retrieval Script:** Very basic python script to send the "GET_CSV" command and then save the given csv.

## 4. Data Format
- CSV schema: `timestamp (ms), channel, rssi (dBm), Avg_rssi (dBm), estimated_distance(cm), mac_address, network_name`
- Estimated distance derived from RSSI using the formula: `distance_cm = 100 * 10^((txPower - rssi) / (10 * n))` where RSSI is the signal strength, txPower is the expected transmission power at 1 meter, and n is the Path-loss exponent which represents how quickly a radio signal weakens as it travels through space.

## 5. Retrieval Workflow
- Make sure you have the Serial library installed.
- Connect ESP32 to laptop.
- Run `retrieval.py` to get the csv.

## 7. Challenges and Design Choices
- Initially, I encountered a large number of corrupted and irrelevant packets, which led to implementing a frame-type filter.
- I originally planned for a Bluetooth-connected companion app to receive live data, but storage constraints on the ESP32 made SPIFFS logging a more practical solution.

## 8. Ethics & Legal Use
- Project for **educational demonstration only**.
- Captures only **broadcast probe frames**, not personal communications.
- Do not use to track or deanonymize individuals.
- Always verify local regulations before operating in public areas. This project is intended for controlled lab or demo environments.

## 10. Demo / Gallery (Optional)
- PHOTOS TO BE ADDED

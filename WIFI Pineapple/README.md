# ESP32 Wi-Fi Pineapple

## 1. Overview
An ESP32 project that creates a Wi-Fi access point with a captive portal login page. It records demo input from connected clients, including the clients IP, email, password and timestamp, and saves it in the SPIFFS as a CSV which can be retrieved later from a web browser via an admin endpoint. Once someone has input their test data once it bans their IP and gives them some fake error message as an excuse. If they try to connect again they are told they are banned for "possible malicious behavior".
- **Disclaimer:** For educational and research use only. Use of this project in public is very illegal. Again, **DO NOT USE THIS FOR ANYTHING MALICIOUS**. This is intended to be used for awareness training.

## 2. Hardware Design
- Uses an ESP32 microcontroller from ELEGOO.
- The onboard blue LED flashes each time demo input is successfully captured.
- Powered via USB-C and stores a csv and a txt in internal SPIFFS memory.
- Wi-Fi Access Point broadcasts SSID: "Google Free WiFi Demo (DONT CLICK)"

## 3. Software Architecture
- **Wi-Fi AP & Captive Portal:** ESP32 starts a soft access point and responds to HTTP requests with a demo login page.
- **DNS Redirection:** Minimal DNS server redirects all client requests to the captive portal page.
- **Form Parsing & Data Logging:** Parses application/x-www-form-urlencoded POST requests and appends sanitized entries to submissions.csv on SPIFFS.
- **IP Restrction:** Clients are banned after submitting to prevent multiple entries.
- **Admin Retreival:** Protected /admin endpoint allows retrieval of the CSV using a token.

## 4. Data Format
- CSV schema: `Client IP, Email, Password, Timestamp`

## 5. Retrieval Workflow
- Connect a laptop or device to the ESP32’s SSID.
- Open a web browser and navigate to: http://192.168.4.1/admin
- Use the Bearer token you configured in the code to authenticate.
- The CSV file will be returned in the browser for download.

## 7. Challenges and Design Choices
- I Chose to make it look like a Google login because it's a pretty well known service.
- I had some issues figuring out how to have the Google "G" photo appear. At first, I tried using a plugin that allowed me to save a png in the SPIFFs but that ended up being outdated and I had to resort to just embedding the photo as a base-64 string.
- A challenge was getting the ESP32 to act like a DNS server because it had to respond to all client requests without causing errors. I solved this by making a small DNS responder that sends the ESP32’s IP for every request, so devices always get sent to the captive portal

## 8. Ethics & Legal Use
- Project for **educational demonstration only**.
- Do not use to track or deanonymize individuals.
- Always inform users and obtain consent before testing in a lab or controlled environment.
- **DO NOT USE IN PUBLIC**

## 10. Demo / Gallery (Optional)
- PHOTOS TO BE ADDED

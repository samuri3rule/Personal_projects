# Paxxel Packet Sniffer

**Work in Progress:** Paxxel is still under development, so some features are incomplete and improvements are ongoing.

## 1. Overview
Paxxel is a Python-based packet sniffing and network analysis tool built using **Scapy**. It captures packets on the local network and prints relevant information such as Ethernet headers, IP addresses, transport layer details, and DNS queries. Paxxel also has utilities for IP info lookup, MAC vendor lookup, and sending TCP reset packets.

**Disclaimer:** For educational and research use only. Packet sniffing and traffic manipulation on networks without permission may be illegal. **Do not use this tool on networks you do not own or have explicit authorization to analyze.**


## 2. System Design
- Built in **Python** using the **Scapy** library.
- Captures live network packets directly from the host machine's network interface.
- Optional logging to a text file (`Packets.txt`) for later analysis.
- Uses external APIs to retrieve metadata about IP addresses (ipinfo.io) and MAC vendors (macvendors.com).
- Runs as a simple **command-line tool**.


## 3. Software Architecture

### Packet Sniffer
- Uses Scapy’s `sniff()` function to capture packets in real time.
- Packets are passed to a processing function that extracts relevant protocol fields.
- Supports optional time-limited captures or continuous sniffing.
- Captures common network layers including Ethernet, IP, TCP, UDP, and DNS.
- Extracts source and destination MAC addresses, IP addresses, and port numbers.

Captured data can be printed to the console or written to a log file.


### TCP Reset Tool
Allows the user to craft and send a **TCP Reset (RST)** packet using Scapy.

Inputs required:
- Source IP
- Destination IP
- Source port
- Destination port
- Sequence number
- Acknowledgment number

This tool demonstrates how TCP connections can be forcefully terminated by injecting crafted packets.


### IP Intelligence Lookup
Uses the **ipinfo.io API** to retrieve metadata about an IP address.

Information returned may include:

- Hostname  
- City  
- Region  
- Country  
- Geographic coordinates  
- Organization/ISP  

Requires a **free** API token from **ipinfo.io**.


### MAC Vendor Lookup
Uses the **macvendors.com API** to identify the manufacturer associated with a MAC address.

Example output: `Vendor: Apple, Inc.`


## 4. Output Format

Each captured packet is printed in a structured format:
```
--- New Packet ---
Ethernet Layer:
-Source MAC Address
-Destination MAC Address

IP Layer:
-Source IP Address
-Destination IP Address

TCP Layer:
-Source Port
-Destination Port
-Payload
-Sequence Number
-Ack Number

UDP Layer:
-Source Port
-Destination Port
-Payload

DNS Layer:
-Query Details
```

If enabled, this data is also sent to: `Packets.txt`


## 5. Usage Workflow

Run the program: `python paxxel.py`

### Main Menu
1 - Pick a tool  
2 - Analyze something  
3 - Quit

### Tools
1 - Packet Sniffer  
2 - TCP Reset

### Analysis Tools
1 - IP Address Information  
2 - MAC Address Vendor Lookup


## 6. Dependencies

Required Python libraries:

scapy
requests

Install with: `pip install scapy requests`

Scapy may require **administrator/root privileges** depending on your operating system because it accesses raw network packets.


## 7. Challenges and Design Choices

- **Finding Free APIs:** One challenge was locating reliable free APIs for IP and MAC address lookups that did not require paid subscriptions. I ultimately used lightweight public APIs to retrieve IP information and MAC vendor data.

- **Handling Different Packet Structures:** Not every captured packet contains the same protocol layers, so the program has to check whether a layer exists before trying to access its fields. This required structuring the packet output logic so it only prints information for protocols that are actually present.


## 8. Ethics & Legal Use

- Intended for **educational purposes, network learning, and lab environments**.
- Only run packet sniffers on networks **you own or have permission to analyze**.
- Do not use this tool to intercept private communications without consent.
- TCP reset functionality should only be used in controlled environments for **demonstration or testing**.

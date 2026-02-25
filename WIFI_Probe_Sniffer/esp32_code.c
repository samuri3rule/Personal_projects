/*
  ESP32 Probe-Request Sniffer (use arduinoIDE)
  - Tracks up to 5 devices
  - Buffers CSV lines in RAM and flushes them to SPIFFS from loop()
  - Blinks blue LED when a probe request is successfully parsed
  - Send "GET_CSV\n" over USB Serial to retrieve /probes.csv
*/

#include <WiFi.h>
#include "esp_wifi.h"
#include <SPIFFS.h>

#define LED_PIN 2

// Tracking limits
#define MAX_DEVICES 5
#define CHANNEL_MIN 1
#define CHANNEL_MAX 13
#define CHANNEL_HOP_MS 200

// Pending CSV buffer 
#define PENDING_CAP 64        // max pending lines in RAM
#define PENDING_LINE_MAX 128  // max bytes per CSV line

struct Device {
  uint8_t mac[6];
  int avgRSSI;
  char ssid[33];
  bool used;
};

Device devices[MAX_DEVICES];
int deviceCount = 0;

int currentChannel = CHANNEL_MIN;
unsigned long lastHop = 0;

// pending circular buffer
char pending[PENDING_CAP][PENDING_LINE_MAX];
volatile int pending_head = 0; // next to read 
volatile int pending_tail = 0; // next to write
volatile int pending_count = 0;

// LED blink state
volatile bool packetDetected = false;
unsigned long ledEndTime = 0;
const unsigned long LED_BLINK_MS = 50;

//convert MAC bytes to upper-case hex 
void macToHexStr(const uint8_t *mac, char *out, size_t outLen) {
 
  const char hex[] = "0123456789ABCDEF";
  int pos = 0;
  for (int i = 0; i < 6 && pos + 3 < (int)outLen; ++i) {
    uint8_t b = mac[i];
    out[pos++] = hex[(b >> 4) & 0xF];
    out[pos++] = hex[b & 0xF];
    if (i < 5) out[pos++] = ':';
  }
  out[pos] = '\0';
}

// Estimate distance in cm using log-distance path loss.
int estimateDistanceCM(int rssi, int txPower = -30, float n = 3.0f) {
  float exp = ((float)(txPower - rssi)) / (10.0f * n);
  float meters = powf(10.0f, exp);
  int cm = (int)(meters * 100.0f + 0.5f);
  if (cm < 0) cm = 0;
  return cm;
}

// Extract SSID
bool extractSSID(const uint8_t *buf, int len, char *ssidOut, int ssidOutLen) {
  int i = 0;
  while (i + 2 <= len) {
    uint8_t tag = buf[i];
    uint8_t slen = buf[i + 1];
    if (i + 2 + slen > len) break;
    if (tag == 0x00) {
      int copyLen = slen;
      if (copyLen > ssidOutLen - 1) copyLen = ssidOutLen - 1;
      if (copyLen > 0) memcpy(ssidOut, buf + i + 2, copyLen);
      ssidOut[copyLen] = '\0';
      return true;
    }
    i += 2 + slen;
  }
  return false;
}

// Find device by MAC or add a new one 
Device* getDeviceOrAdd(const uint8_t *mac, const char *ssid, int rssi) {
  for (int i = 0; i < MAX_DEVICES; ++i) {
    if (devices[i].used) {
      bool match = true;
      for (int j = 0; j < 6; ++j) {
        if (devices[i].mac[j] != mac[j]) { match = false; break; }
      }
      if (match) {
        devices[i].avgRSSI = (devices[i].avgRSSI + rssi) / 2;
        return &devices[i];
      }
    }
  }
  // add new device if we have space
  for (int i = 0; i < MAX_DEVICES; ++i) {
    if (!devices[i].used) {
      devices[i].used = true;
      memcpy(devices[i].mac, mac, 6);
      devices[i].avgRSSI = rssi;
      strncpy(devices[i].ssid, ssid ? ssid : "", 32);
      devices[i].ssid[32] = '\0';
      ++deviceCount;
      return &devices[i];
    }
  }
  return NULL;
}

// queue CSV line into buffer
void enqueueLineFromCallback(const char *line) {
  int nextTail = (pending_tail + 1) % PENDING_CAP;
  if (nextTail == pending_head) {
    pending_head = (pending_head + 1) % PENDING_CAP;
    pending_count = max(0, pending_count - 1);
  }
  strncpy(pending[pending_tail], line, PENDING_LINE_MAX - 1);
  pending[pending_tail][PENDING_LINE_MAX - 1] = '\0';
  pending_tail = nextTail;
  if (pending_count < PENDING_CAP) ++pending_count;
}

// parse probe request, prepare CSV line, and queue
void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return; // if not the right type of packet return
  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t*) buf;
  const uint8_t *payload = ppkt->payload;
  int len = ppkt->rx_ctrl.sig_len;
  int8_t rssi = ppkt->rx_ctrl.rssi;
  uint8_t ch = ppkt->rx_ctrl.channel;

  if (len <= 24) return;

  uint8_t fc0 = payload[0];
  uint8_t subtype = (fc0 >> 4) & 0x0F;
  uint8_t typefield = (payload[0] >> 2) & 0x03;
  if (typefield != 0 || subtype != 4) return; 

  const uint8_t *srcMac = payload + 10;
  const uint8_t *tagged = payload + 24;
  int tagged_len = len - 24;
  if (tagged_len <= 0) return;

  char ssid[33];
  if (!extractSSID(tagged, tagged_len, ssid, sizeof(ssid))) return;

  Device *d = getDeviceOrAdd(srcMac, ssid, rssi);
  if (!d) return;

  int avg = d->avgRSSI;
  int distanceCM = estimateDistanceCM(avg);

  // Build CSV line: timestamp_ms,channel,rssi,avg_rssi,distance_cm,MAC,SSID
  char macStr[18];
  macToHexStr(srcMac, macStr, sizeof(macStr));
  char line[PENDING_LINE_MAX];
  int written = snprintf(line, sizeof(line), "%lu,%u,%d,%d,%d,%s,%s",
    millis(), (unsigned)ch, (int)rssi, (int)avg, distanceCM, macStr, ssid);
  if (written > 0) {
    enqueueLineFromCallback(line);
    packetDetected = true; // blink LED
  }
}

// Flush buffer to SPIFFS (called from loop)
void flushPendingToSPIFFS() {
  if (pending_count <= 0) return;
  File f = SPIFFS.open("/probes.csv", "a");
  if (!f) { //if it doesn't work don't worry
    return;
  }
  while (pending_count > 0) {
    f.println(pending[pending_head]);
    pending_head = (pending_head + 1) % PENDING_CAP;
    --pending_count;
  }
  f.close();
}

void setup() {
  Serial.begin(115200);
  delay(20);
  esp_log_level_set("wifi", ESP_LOG_NONE);

  // set up LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  packetDetected = false;
  ledEndTime = 0;

  // SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
  }

  // initialize wifi for promiscuous sniffing
  WiFi.mode(WIFI_MODE_NULL);
  esp_wifi_stop();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();

  // set into promiscuous mode
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&snifferCallback);

  wifi_promiscuous_filter_t filt;
  filt.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  esp_wifi_set_promiscuous_filter(&filt);

  currentChannel = CHANNEL_MIN;
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  lastHop = millis();

  // ensure CSV exists / header if you want (optional)
  if (SPIFFS.exists("/probes.csv") == false) {
    File f = SPIFFS.open("/probes.csv", "w");
    if (f) {
      f.println("timestamp_ms,channel,rssi,avg_rssi,distance_cm,mac,ssid");
      f.close();
    }
  }

  Serial.println("Sniffer started (max 5 devices). Type GET_CSV and press Enter to download.");
}

unsigned long lastFlushMs = 0;
const unsigned long FLUSH_INTERVAL_MS = 2000; // flush every 2s

void loop() {
  unsigned long now = millis();

  // channel hopping
  if (now - lastHop >= CHANNEL_HOP_MS) {
    lastHop = now;
    currentChannel++;
    if (currentChannel > CHANNEL_MAX) currentChannel = CHANNEL_MIN;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  }

  // Periodically add lines to SPIFFS
  if ((now - lastFlushMs) >= FLUSH_INTERVAL_MS) {
    lastFlushMs = now;
    flushPendingToSPIFFS();
  }

  // Handle USB Serial command GET_CSV
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("GET_CSV")) {
      // flush before sending
      flushPendingToSPIFFS();
      File f = SPIFFS.open("/probes.csv", "r");
      if (f) {
        while (f.available()) {
          Serial.write(f.read());
        }
        f.close();
        Serial.println("\n---END---");
      } else {
        Serial.println("No CSV file found");
      }
    }
  }

  // LED blink when packet detected
  if (packetDetected) {
    digitalWrite(LED_PIN, HIGH);
    ledEndTime = now + LED_BLINK_MS;
    packetDetected = false;
  }
  if (ledEndTime && now > ledEndTime) {
    digitalWrite(LED_PIN, LOW);
    ledEndTime = 0;
  }

  delay(1);
}

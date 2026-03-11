#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_now.h>
#include <ArduinoJson.h>

/* ===== CONSTANTS ===== */
#define UDP_PORT 19018
#define MAX_SLAVES 4

/* ===== TALLY PACKET ===== */
typedef struct {
  uint8_t camId;  // ID Camera (1-4)
  uint8_t state;  // 0=OFF, 1=PVW (Green), 2=PGM (Red)
} TallyPacket;

/* ===== NETWORK CLASS ===== */
class Network {
private:
  WiFiUDP udp;
  uint8_t slaveMacs[MAX_SLAVES][6];
  bool wifiConnected;
  bool udpReady;
  bool espnowReady;
  int slaveCount;

public:
  Network();
  
  // Khởi tạo WiFi và kết nối
  void init(const char* ssid, const char* password);
  
  // Thêm MAC address của Slave
  void addSlave(uint8_t index, const uint8_t* mac);
  
  // Xử lý UDP packet và gọi callback khi nhận được tally update
  void handleUDP(void (*onTallyUpdate)(int pgm, int pvw));
  
  // Gửi tally packet đến một camera cụ thể
  void sendTally(uint8_t cam, uint8_t state);
  
  // Rebuild và gửi tất cả camera states
  void rebuildAndSend(int pgm, int pvw);
  
  // Getters cho status
  bool isWiFiConnected() { return wifiConnected; }
  bool isUDPReady() { return udpReady; }
  bool isESPNowReady() { return espnowReady; }
  int getWiFiChannel() { return WiFi.channel(); }
};

/* ===== IMPLEMENTATION ===== */

Network::Network() {
  wifiConnected = false;
  udpReady = false;
  espnowReady = false;
  slaveCount = 0;
}

void Network::init(const char* ssid, const char* password) {
  Serial.println("[Network] Initializing WiFi...");
  
  // Khởi tạo WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Chờ kết nối WiFi
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 50) {
    delay(200);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\n[Network] WiFi connected!");
    Serial.printf("[Network] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[Network] Channel: %d\n", WiFi.channel());
  } else {
    Serial.println("\n[Network] WiFi connection failed!");
    return;
  }
  
  // Khởi tạo UDP
  if (udp.begin(UDP_PORT)) {
    udpReady = true;
    Serial.printf("[Network] UDP listening on port %d\n", UDP_PORT);
  }
  
  // Khởi tạo ESP-NOW
  if (esp_now_init() == ESP_OK) {
    espnowReady = true;
    Serial.println("[Network] ESP-NOW initialized");
  } else {
    Serial.println("[Network] ESP-NOW init failed!");
  }
}

void Network::addSlave(uint8_t index, const uint8_t* mac) {
  if (index >= MAX_SLAVES) {
    Serial.printf("[Network] Invalid slave index: %d\n", index);
    return;
  }
  
  // Lưu MAC address
  memcpy(slaveMacs[index], mac, 6);
  
  // Thêm peer vào ESP-NOW
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = WiFi.channel();
  peer.encrypt = false;
  
  if (esp_now_add_peer(&peer) == ESP_OK) {
    Serial.printf("[Network] Added slave %d: ", index + 1);
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
    slaveCount++;
  } else {
    Serial.printf("[Network] Failed to add slave %d\n", index + 1);
  }
}

void Network::handleUDP(void (*onTallyUpdate)(int pgm, int pvw)) {
  int packetSize = udp.parsePacket();
  if (packetSize == 0) return;
  
  // Đọc packet
  char buffer[256];
  int len = udp.read(buffer, sizeof(buffer) - 1);
  if (len <= 0) return;
  buffer[len] = '\0';
  
  Serial.printf("[Network] UDP RX: %s\n", buffer);
  
  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, buffer);
  
  if (error) {
    Serial.printf("[Network] JSON parse error: %s\n", error.c_str());
    return;
  }
  
  // Lấy tally data
  const char* id = doc["id"];
  if (!id) return;
  
  JsonArray valueArray = doc["value"];
  if (valueArray.size() == 0) return;
  
  uint8_t cam = valueArray[0];
  
  // Xác định PGM hay PVW
  static int currentPGM = 0;
  static int currentPVW = 0;
  
  if (strcmp(id, "pgmTally") == 0) {
    currentPGM = cam;
    Serial.printf("[Network] PGM updated: CAM %d\n", cam);
  } else if (strcmp(id, "pvwTally") == 0) {
    currentPVW = cam;
    Serial.printf("[Network] PVW updated: CAM %d\n", cam);
  }
  
  // Gọi callback
  if (onTallyUpdate) {
    onTallyUpdate(currentPGM, currentPVW);
  }
}

void Network::sendTally(uint8_t cam, uint8_t state) {
  if (cam < 1 || cam > MAX_SLAVES) return;
  
  TallyPacket packet;
  packet.camId = cam;
  packet.state = state;
  
  esp_err_t result = esp_now_send(slaveMacs[cam - 1], 
                                   (uint8_t*)&packet, 
                                   sizeof(packet));
  
  if (result == ESP_OK) {
    Serial.printf("[Network] Sent to CAM %d: state=%d\n", cam, state);
  } else {
    Serial.printf("[Network] Send failed to CAM %d\n", cam);
  }
}

void Network::rebuildAndSend(int pgm, int pvw) {
  Serial.printf("[Network] Rebuilding states: PGM=%d, PVW=%d\n", pgm, pvw);
  
  for (int cam = 1; cam <= MAX_SLAVES; cam++) {
    uint8_t state = 0;  // OFF by default
    
    if (cam == pgm) {
      state = 2;  // PGM (Red)
    } else if (cam == pvw) {
      state = 1;  // PVW (Green)
    }
    
    sendTally(cam, state);
  }
}

#endif // NETWORK_H

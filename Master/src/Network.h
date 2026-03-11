#ifndef NETWORK_H
#define NETWORK_H

#include <ArduinoJson.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h> // Gọi thư viện SPI để ép cấu hình W5500
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

/* ===== PINS ===== */
#define ETH_CS 15
#define ETH_RST 2
// Các chân SPI dùng chung (Dành riêng cho ESP32 DevKit)
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define UDP_PORT 19018
#define MAX_SLAVES 4

/* ===== TALLY PACKET ===== */
typedef struct {
  uint8_t camId; // ID Camera (1-4)
  uint8_t state; // 0=OFF, 1=PVW (Green), 2=PGM (Red)
} TallyPacket;

/* ===== NETWORK CLASS ===== */
class Network {
private:
  EthernetUDP udp;
  uint8_t slaveMacs[MAX_SLAVES][6];
  bool ethernetConnected;
  bool udpReady;
  bool espnowReady;
  int slaveCount;
  byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // MAC for Ethernet
  IPAddress staticIP;
  bool useStaticIP;

public:
  Network();

  // Khởi tạo Ethernet (DHCP hoặc Static IP)
  void init(const char *ip = nullptr);

  // Thêm MAC address của Slave
  void addSlave(uint8_t index, const uint8_t *mac);

  // Xử lý UDP packet và gọi callback khi nhận được tally update
  void handleUDP(void (*onTallyUpdate)(int pgm, int pvw));

  // Gửi tally packet đến một camera cụ thể
  void sendTally(uint8_t cam, uint8_t state);

  // Rebuild và gửi tất cả camera states
  void rebuildAndSend(int pgm, int pvw);

  // Getters cho status
  bool isEthernetConnected() { return ethernetConnected; }
  bool isUDPReady() { return udpReady; }
  bool isESPNowReady() { return espnowReady; }
  IPAddress getIP() { return Ethernet.localIP(); }
};

/* ===== IMPLEMENTATION ===== */

Network::Network() {
  ethernetConnected = false;
  udpReady = false;
  espnowReady = false;
  slaveCount = 0;
  useStaticIP = false;
}

void Network::init(const char *ip) {
  Serial.println("[Network] Initializing Ethernet...");

  pinMode(ETH_CS, OUTPUT);
  digitalWrite(ETH_CS, HIGH); // Vô hiệu hoá W5500 lúc đầu

  // Kích hoạt giao tiếp chip nối tiếp W5500
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, HIGH);
  delay(10);
  digitalWrite(ETH_RST, LOW);
  delay(10);
  digitalWrite(ETH_RST, HIGH);
  delay(150); // Cấp thêm thời gian cho W5500 khởi động lại hoàn toàn từ Reset

  // Bắt đầu Bus chung SPI trước tiên (Nếu dùng chung)
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, ETH_CS);

  // Gốc thư viện Ethernet trỏ qua chân CS 15
  Ethernet.init(ETH_CS);

  // Khởi động mạng có cấu hình IP/MAC
  if (ip != nullptr) {
    // Static IP mode
    useStaticIP = true;
    staticIP.fromString(ip);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(8, 8, 8, 8);

    Serial.printf("[Network] Try starting Ethernet with static IP: %s\n", ip);
    Ethernet.begin(mac, staticIP, dns, gateway, subnet);
  } else {
    // DHCP mode
    Serial.println("[Network] Starting Ethernet with DHCP...");
    if (Ethernet.begin(mac) == 0) {
      Serial.println("[Network] DHCP failed! Using fallback IP.");
      IPAddress fallbackIP(192, 168, 1, 100);
      IPAddress gateway(192, 168, 1, 1);
      IPAddress subnet(255, 255, 255, 0);
      Ethernet.begin(mac, fallbackIP, gateway, subnet);
    }
  }

  // Chờ W5500 đàm phán Link Vật Lý (Đèn ngõ quang phải sáng!)
  Serial.println("[Network] Waiting for PHY link...");
  bool linked = false;
  for (int i = 0; i < 15; i++) { // Chờ tối đa 3-4 giây
    if (Ethernet.linkStatus() == LinkON) {
      linked = true;
      break;
    }
    delay(250);
  }

  // Check Ethernet hardware tồn tại
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("[Network] W5500 chip not found!");
  } else {
    // Báo kết quả trạng thái kết nối
    if (!linked) {
      Serial.println("[Network] Ethernet cable is NOT connected!");
    } else {
      ethernetConnected = true;
      Serial.println("[Network] Ethernet connected!");
      Serial.printf("[Network] IP: %s\n", Ethernet.localIP().toString().c_str());
      Serial.printf("[Network] Gateway: %s\n",
                    Ethernet.gatewayIP().toString().c_str());
      Serial.printf("[Network] Subnet: %s\n",
                    Ethernet.subnetMask().toString().c_str());
    }

    // Bắt đầu mở port UDP chờ bản tin Tally
    if (udp.begin(UDP_PORT)) {
      udpReady = true;
      Serial.printf("[Network] UDP listening on port %d\n", UDP_PORT);
    }
  }

  // Chuyển WiFi sang mode Station và tắt để tránh nhiễu
  Serial.println("[Network] Initializing ESP-NOW...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Khóa cố định kênh WiFi số 1 và Tắt chế độ tiết kiệm pin (WIFI_PS_NONE)
  // Việc này khắc phục 99% lỗi chập chờn, mất kết nối ESP-NOW dù ở rất gần
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_ps(WIFI_PS_NONE);

  if (esp_now_init() == ESP_OK) {
    espnowReady = true;
    Serial.println("[Network] ESP-NOW initialized");
  } else {
    Serial.println("[Network] ESP-NOW init failed!");
  }
}

void Network::addSlave(uint8_t index, const uint8_t *mac) {
  if (index >= MAX_SLAVES) {
    Serial.printf("[Network] Invalid slave index: %d\n", index);
    return;
  }

  // Lưu MAC address
  memcpy(slaveMacs[index], mac, 6);

  // Thêm peer vào ESP-NOW
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = 1; // Fixed channel (no WiFi router to sync with)
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) == ESP_OK) {
    Serial.printf("[Network] Added slave %d: ", index + 1);
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5)
        Serial.print(":");
    }
    Serial.println();
    slaveCount++;
  } else {
    Serial.printf("[Network] Failed to add slave %d\n", index + 1);
  }
}

void Network::handleUDP(void (*onTallyUpdate)(int pgm, int pvw)) {
  int packetSize = udp.parsePacket();
  if (packetSize == 0)
    return;

  // Đọc packet
  char buffer[256];
  int len = udp.read(buffer, sizeof(buffer) - 1);
  if (len <= 0)
    return;
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
  const char *id = doc["id"];
  if (!id)
    return;

  // Phát hiện xem có mảng dữ liệu mảy quay không (nếu mảng rỗng = tắt cam)
  JsonArray valueArray = doc["value"];
  uint8_t cam = 0; // 0 nghĩa là không có camera nào hoạt động
  if (valueArray.size() > 0) {
    cam = valueArray[0];
  }

  // Xác định PGM hay PVW
  static int currentPGM = 0;
  static int currentPVW = 0;
  bool stateChanged = false;

  if (strcmp(id, "pgmTally") == 0) {
    if (currentPGM != cam) {
      currentPGM = cam;
      stateChanged = true;
      Serial.printf("[Network] PGM updated: CAM %d\n", cam);
    }
  } else if (strcmp(id, "pvwTally") == 0) {
    if (currentPVW != cam) {
      currentPVW = cam;
      stateChanged = true;
      Serial.printf("[Network] PVW updated: CAM %d\n", cam);
    }
  }

  // CHỈ Gọi callback Cập nhật Màn hình & Đèn LED nếu CÓ SỰ THAY ĐỔI
  if (stateChanged && onTallyUpdate) {
    onTallyUpdate(currentPGM, currentPVW);
  }
}

void Network::sendTally(uint8_t cam, uint8_t state) {
  if (cam < 1 || cam > MAX_SLAVES)
    return;

  TallyPacket packet;
  packet.camId = cam;
  packet.state = state;

  esp_err_t result =
      esp_now_send(slaveMacs[cam - 1], (uint8_t *)&packet, sizeof(packet));

  if (result != ESP_OK) {
    Serial.printf("[Network] Send failed to CAM %d\n", cam);
  }
}

void Network::rebuildAndSend(int pgm, int pvw) {
  for (int cam = 1; cam <= MAX_SLAVES; cam++) {
    uint8_t state = 0; // OFF by default

    if (cam == pgm) {
      state = 2; // PGM (Red)
    } else if (cam == pvw) {
      state = 1; // PVW (Green)
    }

    sendTally(cam, state);
  }
}

#endif // NETWORK_H

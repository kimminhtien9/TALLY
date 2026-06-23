#ifndef NETWORK_H
#define NETWORK_H

/* =====================================================
   TALLY V2 - MASTER NETWORK
   Ethernet (W5500) <- UDP từ Osee
   ESP-NOW -> Slaves (Long Range, retry, theo dõi online)
   ===================================================== */

#include "Config.h"
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

/* ===== PINS (W5500) ===== */
#define ETH_CS 15
#define ETH_RST 2
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define UDP_PORT 19018

/* ===== ESP-NOW RELIABILITY ===== */
#define ESPNOW_MAX_RETRY 2     // Số lần gửi lại nếu MAC-layer báo fail
#define SLAVE_ONLINE_TIMEOUT 3000 // ms: quá hạn không có ACK -> coi là offline

/* ===== TALLY PROTOCOL (phải KHỚP với Slave) ===== */
#define TALLY_MAGIC 0xA7
#define TALLY_PROTO_VER 2
typedef struct {
  uint8_t magic;   // 0xA7 - lọc gói rác
  uint8_t version; // 2
  uint8_t camId;   // ID Camera (1..MAX_SLAVES)
  uint8_t state;   // 0=OFF, 1=PVW (Green), 2=PGM (Red)
  uint8_t seq;     // Sequence counter
} TallyPacket;

/* ===== Theo dõi trạng thái từng Slave (cho Dashboard) ===== */
struct SlaveLink {
  bool online = false;          // Có ACK gần đây không
  unsigned long lastAck = 0;    // millis() lần ACK cuối
  uint8_t state = 0;            // Trạng thái đang gửi (0/1/2)
  uint32_t okCount = 0;
  uint32_t failCount = 0;
};

/* ===== Static instance để callback C truy cập được ===== */
class Network; // forward
static Network *g_netInstance = nullptr;

class Network {
private:
  EthernetUDP udp;
  MasterConfig *config;
  bool ethernetConnected;
  bool udpReady;
  bool espnowReady;
  byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // MAC cho W5500
  uint8_t seqCounter = 0;

  // Đối chiếu MAC -> index slave (dùng trong send callback)
  int macToIndex(const uint8_t *m) {
    for (int i = 0; i < config->numSlaves; i++) {
      if (memcmp(config->macs[i], m, 6) == 0)
        return i;
    }
    return -1;
  }

public:
  SlaveLink slaves[MAX_SLAVES];

  Network() {
    ethernetConnected = false;
    udpReady = false;
    espnowReady = false;
    config = nullptr;
    g_netInstance = this;
  }

  // ====== Callback khi ESP-NOW báo kết quả gửi (MAC-layer ACK) ======
  static void onDataSent(const uint8_t *macAddr, esp_now_send_status_t status) {
    if (!g_netInstance || !g_netInstance->config)
      return;
    int idx = g_netInstance->macToIndex(macAddr);
    if (idx < 0)
      return;
    SlaveLink &s = g_netInstance->slaves[idx];
    if (status == ESP_NOW_SEND_SUCCESS) {
      s.online = true;
      s.lastAck = millis();
      s.okCount++;
    } else {
      s.failCount++;
    }
  }

  /* ===== Khởi tạo Ethernet ===== */
  void initEthernet(MasterConfig *c) {
    config = c;
    Serial.println("[Network] Initializing Ethernet (W5500)...");

    pinMode(ETH_CS, OUTPUT);
    digitalWrite(ETH_CS, HIGH);

    // Reset cứng W5500
    pinMode(ETH_RST, OUTPUT);
    digitalWrite(ETH_RST, HIGH);
    delay(10);
    digitalWrite(ETH_RST, LOW);
    delay(10);
    digitalWrite(ETH_RST, HIGH);
    delay(150);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, ETH_CS);
    Ethernet.init(ETH_CS);

    if (config->useDhcp) {
      Serial.println("[Network] Starting Ethernet with DHCP...");
      if (Ethernet.begin(mac) == 0) {
        Serial.println("[Network] DHCP failed! Using fallback static IP.");
        IPAddress fb, gw, sn;
        fb.fromString(config->ip);
        gw.fromString(config->gw);
        sn.fromString(config->sn);
        Ethernet.begin(mac, fb, gw, sn);
      }
    } else {
      IPAddress ip, gw, sn, dns;
      ip.fromString(config->ip);
      gw.fromString(config->gw);
      sn.fromString(config->sn);
      dns.fromString(config->dns);
      Serial.printf("[Network] Static IP: %s\n", config->ip);
      Ethernet.begin(mac, ip, dns, gw, sn);
    }

    // Chờ link vật lý
    Serial.println("[Network] Waiting for PHY link...");
    bool linked = false;
    for (int i = 0; i < 15; i++) {
      if (Ethernet.linkStatus() == LinkON) {
        linked = true;
        break;
      }
      delay(250);
    }

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("[Network] W5500 chip not found!");
    } else {
      if (!linked) {
        Serial.println("[Network] Ethernet cable is NOT connected!");
      } else {
        ethernetConnected = true;
        Serial.printf("[Network] Ethernet connected! IP: %s\n",
                      Ethernet.localIP().toString().c_str());
      }
      if (udp.begin(UDP_PORT)) {
        udpReady = true;
        Serial.printf("[Network] UDP listening on port %d\n", UDP_PORT);
      }
    }
  }

  /* ===== Khởi tạo ESP-NOW (Long Range, khóa kênh, công suất max) ===== */
  void initEspNow() {
    Serial.println("[Network] Initializing ESP-NOW...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Khóa kênh cố định (khớp Slave)
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(config->channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    // Tắt tiết kiệm pin WiFi -> chống chập chờn ESP-NOW
    esp_wifi_set_ps(WIFI_PS_NONE);

    // BẬT LONG RANGE: tăng độ nhạy ~ -129dBm, gấp ~2-4 lần tầm, ổn định hơn
    // Yêu cầu Slave cũng bật LR thì mới giao tiếp được.
    if (config->longRange) {
      esp_err_t r = esp_wifi_set_protocol(WIFI_IF_STA,
                                          WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G |
                                              WIFI_PROTOCOL_11N |
                                              WIFI_PROTOCOL_LR);
      Serial.printf("[Network] Long Range mode: %s\n",
                    r == ESP_OK ? "ON" : "FAILED");
    }

    // Công suất phát tối đa
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    if (esp_now_init() == ESP_OK) {
      espnowReady = true;
      esp_now_register_send_cb(onDataSent);
      Serial.println("[Network] ESP-NOW initialized");
    } else {
      Serial.println("[Network] ESP-NOW init failed!");
      return;
    }

    // Thêm peers từ config
    for (int i = 0; i < config->numSlaves; i++)
      addPeer(i);
  }

  /* ===== Thêm 1 peer ESP-NOW ===== */
  void addPeer(int index) {
    if (index < 0 || index >= MAX_SLAVES)
      return;
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, config->macs[index], 6);
    peer.channel = config->channel;
    peer.encrypt = false;
    if (esp_now_add_peer(&peer) == ESP_OK) {
      Serial.printf("[Network] Peer %d added: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    index + 1, config->macs[index][0], config->macs[index][1],
                    config->macs[index][2], config->macs[index][3],
                    config->macs[index][4], config->macs[index][5]);
    } else {
      Serial.printf("[Network] Failed to add peer %d\n", index + 1);
    }
  }

  /* ===== Xử lý UDP từ Osee ===== */
  void handleUDP(void (*onTallyUpdate)(int pgm, int pvw)) {
    int packetSize = udp.parsePacket();
    if (packetSize == 0)
      return;

    char buffer[256];
    int len = udp.read(buffer, sizeof(buffer) - 1);
    if (len <= 0)
      return;
    buffer[len] = '\0';

    Serial.printf("[Network] UDP RX: %s\n", buffer);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, buffer);
    if (error) {
      Serial.printf("[Network] JSON parse error: %s\n", error.c_str());
      return;
    }

    const char *id = doc["id"];
    if (!id)
      return;

    JsonArray valueArray = doc["value"];
    uint8_t cam = 0; // 0 = không cam nào
    if (valueArray.size() > 0)
      cam = valueArray[0];

    static int currentPGM = 0;
    static int currentPVW = 0;
    bool stateChanged = false;

    if (strcmp(id, "pgmTally") == 0) {
      if (currentPGM != cam) {
        currentPGM = cam;
        stateChanged = true;
        Serial.printf("[Network] PGM -> CAM %d\n", cam);
      }
    } else if (strcmp(id, "pvwTally") == 0) {
      if (currentPVW != cam) {
        currentPVW = cam;
        stateChanged = true;
        Serial.printf("[Network] PVW -> CAM %d\n", cam);
      }
    }

    if (stateChanged && onTallyUpdate)
      onTallyUpdate(currentPGM, currentPVW);
  }

  /* ===== Gửi 1 packet tới 1 camera (có retry) ===== */
  void sendTally(uint8_t cam, uint8_t state) {
    if (cam < 1 || cam > config->numSlaves)
      return;

    TallyPacket packet;
    packet.magic = TALLY_MAGIC;
    packet.version = TALLY_PROTO_VER;
    packet.camId = cam;
    packet.state = state;
    packet.seq = seqCounter++;

    slaves[cam - 1].state = state;

    // Gửi + retry nếu lỗi ngay tại lệnh gọi (queue đầy / lỗi)
    for (int attempt = 0; attempt <= ESPNOW_MAX_RETRY; attempt++) {
      esp_err_t r = esp_now_send(config->macs[cam - 1], (uint8_t *)&packet,
                                 sizeof(packet));
      if (r == ESP_OK)
        break;
      delayMicroseconds(500);
    }
  }

  /* ===== Gửi lại toàn bộ trạng thái cho tất cả Slave ===== */
  void rebuildAndSend(int pgm, int pvw) {
    for (int cam = 1; cam <= config->numSlaves; cam++) {
      uint8_t state = 0;
      if (cam == pgm)
        state = 2;
      else if (cam == pvw)
        state = 1;
      sendTally(cam, state);
    }
  }

  /* ===== Cập nhật cờ online theo timeout ACK ===== */
  void updateOnlineStatus() {
    unsigned long now = millis();
    for (int i = 0; i < config->numSlaves; i++) {
      if (slaves[i].online && (now - slaves[i].lastAck > SLAVE_ONLINE_TIMEOUT))
        slaves[i].online = false;
    }
  }

  /* ===== Getters ===== */
  bool isEthernetConnected() { return ethernetConnected; }
  bool isUDPReady() { return udpReady; }
  bool isESPNowReady() { return espnowReady; }
  IPAddress getIP() { return Ethernet.localIP(); }
  bool linkUp() { return Ethernet.linkStatus() == LinkON; }
};

#endif // NETWORK_H

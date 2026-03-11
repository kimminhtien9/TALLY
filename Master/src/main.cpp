#include "Network.h"
#include <Arduino.h>

/* ===== ETHERNET CONFIGURATION ===== */
// Option 1: DHCP (auto IP) - set to nullptr
// Option 2: Static IP (recommended) - set IP address
const char *STATIC_IP =
    "192.168.1.100"; // Change to your network, or nullptr for DHCP

/* ===== SLAVE MAC ADDRESSES ===== */
// Cấu hình MAC address của các Slave (lấy từ Serial Monitor khi chạy code
// Slave)
uint8_t slave1Mac[6] = {0x80, 0xF1, 0xB2, 0x63, 0x52, 0xE4};
uint8_t slave2Mac[6] = {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x68};
uint8_t slave3Mac[6] = {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x69};
uint8_t slave4Mac[6] = {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x6A};

/* ===== GLOBAL OBJECTS ===== */
Network network;

/* ===== STATE VARIABLES ===== */
int currentPGM = 0;
int currentPVW = 0;

/* ===== CALLBACK: Tally Update ===== */
// Được gọi khi Network module nhận được UDP packet từ Osee
void onTallyUpdate(int pgm, int pvw) {
  Serial.printf("[Main] Tally Update: PGM=%d, PVW=%d\n", pgm, pvw);

  // Cập nhật state
  currentPGM = pgm;
  currentPVW = pvw;

  // Gửi tới tất cả Slave
  network.rebuildAndSend(currentPGM, currentPVW);
}

/* ===== HELPER: In trạng thái ra Serial ===== */
void printSystemStatus() {
  Serial.println("\n==================================");
  Serial.println("       TALLY SYSTEM STATUS        ");
  Serial.println("==================================");
  
  // Ethernet status
  Serial.print("ETH: ");
  Serial.println(network.isEthernetConnected() ? "OK" : "FAIL");
  
  // IP address
  Serial.print("IP:  ");
  Serial.println(network.getIP());
  
  // UDP status
  Serial.print("UDP: ");
  Serial.println(network.isUDPReady() ? "OK" : "WAIT");
  
  // ESP-NOW status
  Serial.print("NOW: ");
  Serial.println(network.isESPNowReady() ? "OK" : "FAIL");

  Serial.println("----------------------------------");
  
  // Cấu hình MAC đang lưu
  Serial.printf("CAM 1 MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", slave1Mac[0], slave1Mac[1], slave1Mac[2], slave1Mac[3], slave1Mac[4], slave1Mac[5]);
  Serial.printf("CAM 2 MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", slave2Mac[0], slave2Mac[1], slave2Mac[2], slave2Mac[3], slave2Mac[4], slave2Mac[5]);
  Serial.printf("CAM 3 MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", slave3Mac[0], slave3Mac[1], slave3Mac[2], slave3Mac[3], slave3Mac[4], slave3Mac[5]);
  Serial.printf("CAM 4 MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", slave4Mac[0], slave4Mac[1], slave4Mac[2], slave4Mac[3], slave4Mac[4], slave4Mac[5]);

  Serial.println("----------------------------------");
  
  // PGM & PVW
  Serial.print("PGM: ");
  if (currentPGM > 0) Serial.println(currentPGM); else Serial.println("-");
  
  Serial.print("PVW: ");
  if (currentPVW > 0) Serial.println(currentPVW); else Serial.println("-");
  
  Serial.println("==================================\n");
}

/* ===== SETUP ===== */
void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== TALLY MASTER (ESP32 DevKit + Ethernet) ===");

  // Khởi tạo Network (Ethernet + UDP + ESP-NOW)
  network.init(STATIC_IP);

  // Thêm các Slave vào ESP-NOW
  network.addSlave(0, slave1Mac);
  network.addSlave(1, slave2Mac);
  network.addSlave(2, slave3Mac);
  network.addSlave(3, slave4Mac);

  Serial.println("[Main] Setup complete!");
  
  // In trạng thái ban đầu ra console giống như lúc vẽ LCD
  printSystemStatus();
}

void loop() {
  // Xử lý UDP packets (non-blocking)
  network.handleUDP(onTallyUpdate);

  // Gửi Tally Heartbeat liên tục mỗi 500ms để Mạch Slave không bị báo FAIL-SAFE do quá hạn chờ timeout
  static unsigned long lastTallySendTime = 0;
  if (millis() - lastTallySendTime > 500) {
    lastTallySendTime = millis();
    network.rebuildAndSend(currentPGM, currentPVW);
  }

  // In lại trạng thái mỗi 10 giây (System Report) để dễ theo dõi
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 10000) {
    lastPrintTime = millis();
    printSystemStatus();
  }
}

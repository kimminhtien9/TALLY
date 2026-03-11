#include "Display.h"
#include "Network.h"
#include <Arduino.h>

/* ===== CONFIGURATION ===== */
const char *WIFI_SSID = "C12-11";
const char *WIFI_PASS = "Alo123123123";

/* ===== SLAVE MAC ADDRESSES ===== */
// Cấu hình MAC address của các Slave (lấy từ Serial Monitor khi chạy code
// Slave)
uint8_t slave1Mac[6] = {0x0C, 0x4E, 0xA0, 0x31, 0xF5, 0xD4};
uint8_t slave2Mac[6] = {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x68};
uint8_t slave3Mac[6] = {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x69};
uint8_t slave4Mac[6] = {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x6A};

/* ===== GLOBAL OBJECTS ===== */
Network network;
Display display;

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

  // Cập nhật màn hình
  display.drawStatus(network.isWiFiConnected(), network.getWiFiChannel(),
                     network.isUDPReady(), network.isESPNowReady(), currentPGM,
                     currentPVW);
}

/* ===== SETUP ===== */
void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== TALLY MASTER (ESP32 DevKit) ===");

  // Khởi tạo Display
  display.init();
  display.drawStatus(false, 0, false, false, 0, 0);

  // Khởi tạo Network (WiFi + UDP + ESP-NOW)
  network.init(WIFI_SSID, WIFI_PASS);

  // Thêm các Slave vào ESP-NOW
  network.addSlave(0, slave1Mac);
  network.addSlave(1, slave2Mac);
  network.addSlave(2, slave3Mac);
  network.addSlave(3, slave4Mac);

  // Hiển thị status ban đầu
  display.drawStatus(network.isWiFiConnected(), network.getWiFiChannel(),
                     network.isUDPReady(), network.isESPNowReady(), currentPGM,
                     currentPVW);

  Serial.println("[Main] Setup complete!");
}

/* ===== LOOP ===== */
void loop() {
  // Xử lý UDP packets (non-blocking)
  network.handleUDP(onTallyUpdate);

  // Không cần delay, handleUDP đã non-blocking
}

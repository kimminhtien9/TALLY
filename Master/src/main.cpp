/* =====================================================
   TALLY V2 - MASTER (ESP32 DevKit + W5500 + ST7789)
   - Cấu hình động qua NVS + Web Portal (giữ nút BOOT)
   - Dashboard LCD: 4 camera + trạng thái mạng
   - ESP-NOW Long Range + retry + theo dõi online
   ===================================================== */

#include "Config.h"
#include "Display.h"
#include "Network.h"
#include "WebPortal.h"
#include <Arduino.h>

#define BOOT_BUTTON 0          // Nút BOOT trên ESP32 DevKit (giữ -> config mode)
#define CONFIG_HOLD_MS 1500    // Giữ nút >= 1.5s khi khởi động để vào setup
#define HEARTBEAT_MS 500       // Gửi lại tally định kỳ (chống fail-safe Slave)
#define DASHBOARD_MS 1000      // Làm tươi dashboard tối đa mỗi 1s (online dots)

/* ===== GLOBAL ===== */
ConfigStore configStore;
Network network;
Display display;
WebPortal portal;

bool configMode = false;
int currentPGM = 0;
int currentPVW = 0;
bool dashboardDirty = true;

/* ===== Kiểm tra giữ nút BOOT để vào config mode ===== */
bool checkConfigButton() {
  pinMode(BOOT_BUTTON, INPUT_PULLUP);
  if (digitalRead(BOOT_BUTTON) != LOW)
    return false;
  // Đang nhấn -> xác nhận giữ đủ lâu
  unsigned long start = millis();
  while (digitalRead(BOOT_BUTTON) == LOW) {
    if (millis() - start >= CONFIG_HOLD_MS)
      return true;
    delay(10);
  }
  return false;
}

/* ===== Callback khi có tally mới ===== */
void onTallyUpdate(int pgm, int pvw) {
  currentPGM = pgm;
  currentPVW = pvw;
  network.rebuildAndSend(currentPGM, currentPVW);
  dashboardDirty = true;
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== TALLY V2 MASTER ===");

  display.init();
  configStore.load();
  configStore.print();

  // Vào config mode nếu giữ nút BOOT lúc khởi động
  if (checkConfigButton()) {
    configMode = true;
    Serial.println("[Main] >>> CONFIG MODE <<<");
    portal.begin(&configStore);
    display.configMode(portal.ssid(), portal.ip().c_str());
    return;
  }

  // ----- Chạy bình thường -----
  display.splash();
  network.initEthernet(&configStore.cfg);
  network.initEspNow();
  delay(400);
  display.drawDashboard(&configStore.cfg, &network, currentPGM, currentPVW);

  Serial.println("[Main] Setup complete (RUN mode).");
}

/* ===== LOOP ===== */
void loop() {
  // --- CONFIG MODE ---
  if (configMode) {
    portal.handle();
    return;
  }

  // --- RUN MODE ---
  network.handleUDP(onTallyUpdate);

  // Heartbeat: gửi lại trạng thái để Slave không vào fail-safe
  static unsigned long lastBeat = 0;
  if (millis() - lastBeat > HEARTBEAT_MS) {
    lastBeat = millis();
    network.rebuildAndSend(currentPGM, currentPVW);
  }

  // Cập nhật cờ online + làm tươi dashboard
  static unsigned long lastDash = 0;
  network.updateOnlineStatus();
  if (dashboardDirty || millis() - lastDash > DASHBOARD_MS) {
    lastDash = millis();
    dashboardDirty = false;
    display.drawDashboard(&configStore.cfg, &network, currentPGM, currentPVW);
  }

  delay(1); // nhường FreeRTOS, giảm nhiệt
}

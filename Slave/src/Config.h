#ifndef CONFIG_H
#define CONFIG_H

/* =====================================================
   TALLY V2 - SLAVE CONFIG (NVS / Preferences)
   Lưu CAM_ID + kênh vào Flash -> đổi camera mà KHÔNG
   cần nạp lại firmware (cấu hình qua Web Portal).
   ===================================================== */

#include <Arduino.h>
#include <Preferences.h>

#define CFG_NAMESPACE "tally"
#define CFG_VERSION 2

struct SlaveConfig {
  uint8_t camId = 1;       // 1..4
  uint8_t channel = 1;     // Kênh ESP-NOW (phải khớp Master)
  bool longRange = true;   // Bật Long Range (phải khớp Master)
  char name[16] = "CAM 1"; // Tên hiển thị (tùy chọn)
};

class ConfigStore {
private:
  Preferences prefs;

public:
  SlaveConfig cfg;

  void load() {
    prefs.begin(CFG_NAMESPACE, false);
    uint8_t ver = prefs.getUChar("ver", 0);
    if (ver != CFG_VERSION) {
      Serial.println("[Config] No valid config. Writing defaults.");
      prefs.end();
      save();
      return;
    }
    cfg.camId = prefs.getUChar("cam", cfg.camId);
    cfg.channel = prefs.getUChar("chan", cfg.channel);
    cfg.longRange = prefs.getBool("lr", cfg.longRange);
    prefs.getString("name", cfg.name, sizeof(cfg.name));
    prefs.end();
    Serial.println("[Config] Loaded config from NVS.");
  }

  void save() {
    prefs.begin(CFG_NAMESPACE, false);
    prefs.putUChar("ver", CFG_VERSION);
    prefs.putUChar("cam", cfg.camId);
    prefs.putUChar("chan", cfg.channel);
    prefs.putBool("lr", cfg.longRange);
    prefs.putString("name", cfg.name);
    prefs.end();
    Serial.println("[Config] Saved config to NVS.");
  }

  void reset() {
    prefs.begin(CFG_NAMESPACE, false);
    prefs.clear();
    prefs.end();
  }

  void print() {
    Serial.println("---------- SLAVE CONFIG ----------");
    Serial.printf("CAM ID  : %d\n", cfg.camId);
    Serial.printf("Name    : %s\n", cfg.name);
    Serial.printf("Channel : %d\n", cfg.channel);
    Serial.printf("LongRange: %s\n", cfg.longRange ? "ON" : "OFF");
    Serial.println("----------------------------------");
  }
};

#endif // CONFIG_H

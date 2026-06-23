#ifndef CONFIG_H
#define CONFIG_H

/* =====================================================
   TALLY V2 - MASTER CONFIG (NVS / Preferences)
   Lưu cấu hình bền vững vào Flash (NVS) để KHÔNG phải
   nạp lại firmware mỗi lần đổi IP / MAC Slave / kênh.
   ===================================================== */

#include <Arduino.h>
#include <Preferences.h>

#define MAX_SLAVES 4
#define CFG_NAMESPACE "tally"     // NVS namespace
#define CFG_VERSION 2             // Tăng số này nếu đổi cấu trúc -> reset config

/* ===== GIÁ TRỊ MẶC ĐỊNH (dùng cho lần boot đầu / sau khi reset) ===== */
// Các MAC mặc định lấy từ cấu hình V1 cũ để hệ thống chạy được ngay.
static const uint8_t DEFAULT_MACS[MAX_SLAVES][6] = {
    {0x80, 0xF1, 0xB2, 0x63, 0x52, 0xE4},
    {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x68},
    {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x69},
    {0x7C, 0xDF, 0xA1, 0x23, 0x45, 0x6A},
};

struct MasterConfig {
  bool useDhcp = false;             // false = static IP
  char ip[16] = "192.168.1.100";    // static IP
  char gw[16] = "192.168.1.1";      // gateway
  char sn[16] = "255.255.255.0";    // subnet mask
  char dns[16] = "8.8.8.8";         // DNS
  uint8_t channel = 1;              // Kênh ESP-NOW (phải khớp Slave)
  uint8_t numSlaves = MAX_SLAVES;   // Số Slave đang dùng (1..MAX_SLAVES)
  uint8_t macs[MAX_SLAVES][6];      // MAC từng Slave
  char names[MAX_SLAVES][16] = {"CAM 1", "CAM 2", "CAM 3", "CAM 4"};
  bool longRange = true;            // Bật ESP-NOW Long Range (tăng tầm/ổn định)
};

class ConfigStore {
private:
  Preferences prefs;

public:
  MasterConfig cfg;

  // Nạp config từ NVS (nếu chưa có thì dùng mặc định và lưu lại)
  void load() {
    // Khởi tạo MAC mặc định trong RAM trước
    for (int i = 0; i < MAX_SLAVES; i++)
      memcpy(cfg.macs[i], DEFAULT_MACS[i], 6);

    prefs.begin(CFG_NAMESPACE, false);
    uint8_t ver = prefs.getUChar("ver", 0);

    if (ver != CFG_VERSION) {
      // Lần đầu hoặc đổi cấu trúc -> ghi mặc định
      Serial.println("[Config] No valid config found. Writing defaults.");
      prefs.end();
      save();
      return;
    }

    cfg.useDhcp = prefs.getBool("dhcp", cfg.useDhcp);
    prefs.getString("ip", cfg.ip, sizeof(cfg.ip));
    prefs.getString("gw", cfg.gw, sizeof(cfg.gw));
    prefs.getString("sn", cfg.sn, sizeof(cfg.sn));
    prefs.getString("dns", cfg.dns, sizeof(cfg.dns));
    cfg.channel = prefs.getUChar("chan", cfg.channel);
    cfg.numSlaves = prefs.getUChar("nslaves", cfg.numSlaves);
    cfg.longRange = prefs.getBool("lr", cfg.longRange);

    prefs.getBytes("macs", cfg.macs, sizeof(cfg.macs));
    for (int i = 0; i < MAX_SLAVES; i++) {
      char key[8];
      snprintf(key, sizeof(key), "name%d", i);
      prefs.getString(key, cfg.names[i], sizeof(cfg.names[i]));
    }

    prefs.end();
    Serial.println("[Config] Loaded config from NVS.");
  }

  // Lưu config hiện tại xuống NVS
  void save() {
    prefs.begin(CFG_NAMESPACE, false);
    prefs.putUChar("ver", CFG_VERSION);
    prefs.putBool("dhcp", cfg.useDhcp);
    prefs.putString("ip", cfg.ip);
    prefs.putString("gw", cfg.gw);
    prefs.putString("sn", cfg.sn);
    prefs.putString("dns", cfg.dns);
    prefs.putUChar("chan", cfg.channel);
    prefs.putUChar("nslaves", cfg.numSlaves);
    prefs.putBool("lr", cfg.longRange);
    prefs.putBytes("macs", cfg.macs, sizeof(cfg.macs));
    for (int i = 0; i < MAX_SLAVES; i++) {
      char key[8];
      snprintf(key, sizeof(key), "name%d", i);
      prefs.putString(key, cfg.names[i]);
    }
    prefs.end();
    Serial.println("[Config] Saved config to NVS.");
  }

  // Xoá toàn bộ config (factory reset)
  void reset() {
    prefs.begin(CFG_NAMESPACE, false);
    prefs.clear();
    prefs.end();
    Serial.println("[Config] Config cleared (factory reset).");
  }

  // In cấu hình ra Serial
  void print() {
    Serial.println("---------- MASTER CONFIG ----------");
    Serial.printf("Mode    : %s\n", cfg.useDhcp ? "DHCP" : "Static IP");
    Serial.printf("IP      : %s\n", cfg.ip);
    Serial.printf("Gateway : %s\n", cfg.gw);
    Serial.printf("Subnet  : %s\n", cfg.sn);
    Serial.printf("Channel : %d\n", cfg.channel);
    Serial.printf("LongRange: %s\n", cfg.longRange ? "ON" : "OFF");
    Serial.printf("Slaves  : %d\n", cfg.numSlaves);
    for (int i = 0; i < cfg.numSlaves; i++) {
      Serial.printf("  [%d] %-8s %02X:%02X:%02X:%02X:%02X:%02X\n", i + 1,
                    cfg.names[i], cfg.macs[i][0], cfg.macs[i][1], cfg.macs[i][2],
                    cfg.macs[i][3], cfg.macs[i][4], cfg.macs[i][5]);
    }
    Serial.println("-----------------------------------");
  }
};

#endif // CONFIG_H

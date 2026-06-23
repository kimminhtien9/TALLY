#ifndef DISPLAY_H
#define DISPLAY_H

/* =====================================================
   TALLY V2 - MASTER DISPLAY (ST7789 240x240)
   Dashboard: trạng thái mạng + lưới 4 camera
   (online / PGM đỏ / PVW xanh / OFF)
   Vẽ event-driven để tránh giật bộ đệm mạng.
   ===================================================== */

#include "Config.h"
#include "Network.h"
#include <TFT_eSPI.h>

class Display {
private:
  TFT_eSPI tft;

  void drawCamTile(int x, int y, int w, int h, const char *name, uint8_t state,
                   bool online) {
    uint16_t bg = TFT_BLACK;
    uint16_t border = TFT_DARKGREY;
    if (state == 2) {
      bg = TFT_RED;
      border = TFT_RED;
    } else if (state == 1) {
      bg = TFT_DARKGREEN;
      border = TFT_GREEN;
    }

    tft.fillRoundRect(x, y, w, h, 6, bg);
    tft.drawRoundRect(x, y, w, h, 6, border);

    // Chấm online (góc trên phải)
    tft.fillCircle(x + w - 12, y + 12, 5, online ? TFT_GREEN : TFT_RED);

    // Tên camera
    tft.setTextColor(TFT_WHITE, bg);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(2);
    tft.drawString(name, x + 8, y + 10);

    // Nhãn trạng thái
    tft.setTextSize(2);
    const char *label = "OFF";
    if (state == 2)
      label = "PGM";
    else if (state == 1)
      label = "PVW";
    tft.setTextColor(TFT_WHITE, bg);
    tft.drawString(label, x + 8, y + h - 24);
  }

public:
  Display() : tft(TFT_eSPI()) {}

  void init() {
    Serial.println("[Display] Initializing TFT LCD...");
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    pinMode(4, OUTPUT);     // Backlight
    digitalWrite(4, HIGH);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    Serial.println("[Display] TFT LCD ready");
  }

  // Màn hình khởi động
  void splash() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(3);
    tft.drawString("TALLY", 120, 100);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("V2 Master", 120, 140);
    tft.setTextDatum(TL_DATUM);
  }

  // Màn hình chế độ cấu hình (Web Portal)
  void configMode(const char *ssid, const char *ip) {
    tft.fillScreen(TFT_NAVY);
    tft.setTextColor(TFT_YELLOW, TFT_NAVY);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SETUP MODE", 10, 10);
    tft.drawLine(0, 36, 240, 36, TFT_YELLOW);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.drawString("WiFi:", 10, 55);
    tft.setTextColor(TFT_GREEN, TFT_NAVY);
    tft.drawString(ssid, 10, 80);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.drawString("Browse:", 10, 120);
    tft.setTextColor(TFT_CYAN, TFT_NAVY);
    tft.drawString(ip, 10, 145);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setTextSize(1);
    tft.drawString("Cau hinh xong se tu khoi dong lai", 10, 200);
  }

  /* ===== Dashboard chính ===== */
  void drawDashboard(MasterConfig *cfg, Network *net, int pgm, int pvw) {
    tft.fillScreen(TFT_BLACK);

    // ----- Header -----
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("TALLY V2", 6, 4);

    // ETH / NOW status nhỏ bên phải header
    tft.setTextSize(1);
    bool eth = net->isEthernetConnected() && net->linkUp();
    tft.setTextColor(eth ? TFT_GREEN : TFT_RED, TFT_BLACK);
    tft.drawString(eth ? "ETH OK" : "ETH--", 165, 4);
    tft.setTextColor(net->isESPNowReady() ? TFT_GREEN : TFT_RED, TFT_BLACK);
    tft.drawString(net->isESPNowReady() ? "NOW OK" : "NOW--", 165, 16);

    tft.drawLine(0, 26, 240, 26, TFT_DARKGREY);

    // ----- Dòng thông tin mạng -----
    tft.setTextSize(1);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    char line[40];
    snprintf(line, sizeof(line), "IP %s  CH%d  %s",
             net->getIP().toString().c_str(), cfg->channel,
             cfg->longRange ? "LR" : "");
    tft.drawString(line, 6, 32);

    // ----- Lưới 4 camera (2x2) -----
    int gridTop = 46;
    int gap = 8;
    int tileW = (240 - gap * 3) / 2; // 2 cột
    int tileH = (240 - gridTop - gap * 3) / 2;

    for (int i = 0; i < 4; i++) {
      int col = i % 2;
      int row = i / 2;
      int x = gap + col * (tileW + gap);
      int y = gridTop + row * (tileH + gap);

      if (i < cfg->numSlaves) {
        uint8_t st = 0;
        int cam = i + 1;
        if (cam == pgm)
          st = 2;
        else if (cam == pvw)
          st = 1;
        drawCamTile(x, y, tileW, tileH, cfg->names[i], st,
                    net->slaves[i].online);
      } else {
        // Camera không dùng -> ô mờ
        tft.drawRoundRect(x, y, tileW, tileH, 6, TFT_DARKGREY);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("---", x + tileW / 2, y + tileH / 2);
        tft.setTextDatum(TL_DATUM);
      }
    }
  }

  void clear() { tft.fillScreen(TFT_BLACK); }
};

#endif // DISPLAY_H

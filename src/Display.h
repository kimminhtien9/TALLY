#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>

/* ===== DISPLAY CLASS ===== */
class Display {
private:
  TFT_eSPI tft;

public:
  Display();

  // Khởi tạo màn hình LCD
  void init();

  // Vẽ màn hình status với tất cả thông tin hệ thống
  void drawStatus(bool wifiOK, int channel, bool udpOK, bool espnowOK, int pgm,
                  int pvw);

  // Xóa màn hình
  void clear();
};

/* ===== IMPLEMENTATION ===== */

Display::Display() : tft(TFT_eSPI()) {}

void Display::init() {
  Serial.println("[Display] Initializing TFT LCD...");

  // Khởi tạo TFT
  tft.init();

  // Xoay màn hình (0, 1, 2, 3 - thử để tìm hướng phù hợp)
  tft.setRotation(0);

  // Xóa màn hình
  tft.fillScreen(TFT_BLACK);

  // Bật backlight (GPIO 4)
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  // Set text color
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  Serial.println("[Display] TFT LCD ready");
}

void Display::drawStatus(bool wifiOK, int channel, bool udpOK, bool espnowOK,
                         int pgm, int pvw) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Header
  tft.println("TALLY MASTER");
  tft.drawLine(0, 20, 240, 20, TFT_WHITE);
  tft.setCursor(0, 30);

  // WiFi Status
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("WiFi: ");
  if (wifiOK) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("OK");
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("FAIL");
  }

  // WiFi Channel
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("CH: ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(channel);

  // UDP Status
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("UDP: ");
  if (udpOK) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("OK");
  } else {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.println("WAIT");
  }

  // ESP-NOW Status
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("NOW: ");
  if (espnowOK) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("OK");
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("FAIL");
  }

  // Separator
  tft.setCursor(0, 140);
  tft.drawLine(0, 140, 240, 140, TFT_WHITE);
  tft.setCursor(0, 150);

  // PGM Status
  tft.setTextSize(3);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.print("PGM:");
  if (pgm > 0) {
    tft.printf(" %d", pgm);
  } else {
    tft.print(" -");
  }
  tft.println();

  // PVW Status
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print("PVW:");
  if (pvw > 0) {
    tft.printf(" %d", pvw);
  } else {
    tft.print(" -");
  }

  Serial.println("[Display] Status updated");
}

void Display::clear() { tft.fillScreen(TFT_BLACK); }

#endif // DISPLAY_H

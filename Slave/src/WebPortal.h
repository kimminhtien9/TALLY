#ifndef WEBPORTAL_H
#define WEBPORTAL_H

/* =====================================================
   TALLY V2 - SLAVE WEB PORTAL (chế độ cấu hình)
   Giữ nút BOOT (GPIO9) khi khởi động -> tạo WiFi AP
   để đặt CAM_ID / kênh mà không cần nạp lại firmware.
   ===================================================== */

#include "Config.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#define AP_PASS "tally1234"
#define AP_IP_OCT 192, 168, 4, 1

class WebPortal {
private:
  WebServer server;
  DNSServer dns;
  ConfigStore *store;
  bool shouldReboot = false;
  unsigned long rebootAt = 0;
  char apSsid[24];

  String buildPage() {
    SlaveConfig &c = store->cfg;
    String h = F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
                 "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                 "<title>TALLY Slave</title><style>"
                 "body{font-family:sans-serif;background:#111;color:#eee;padding:16px}"
                 "h1{color:#0cf;font-size:20px}label{display:block;margin:10px 0 3px;font-size:14px}"
                 "input,select{width:100%;padding:9px;box-sizing:border-box;border-radius:6px;"
                 "border:1px solid #444;background:#222;color:#fff}"
                 "button{margin-top:18px;width:100%;padding:12px;background:#0a8;color:#fff;"
                 "border:0;border-radius:8px;font-size:16px}</style></head>"
                 "<body><h1>TALLY V2 - Cau hinh Slave</h1><form method='POST' action='/save'>");
    h += "<label>Camera ID</label><select name='cam'>";
    for (int i = 1; i <= 4; i++) {
      h += "<option value='" + String(i) + "'";
      if (c.camId == i) h += " selected";
      h += ">CAM " + String(i) + "</option>";
    }
    h += "</select>";
    h += "<label>Ten (tuy chon)</label><input name='name' value='" + String(c.name) + "'>";
    h += "<label>Kenh ESP-NOW (1-13, khop Master)</label>"
         "<input name='chan' type='number' min='1' max='13' value='" + String(c.channel) + "'>";
    h += "<label><input type='checkbox' name='lr' style='width:auto'";
    if (c.longRange) h += " checked";
    h += "> Long Range (khop Master)</label>";
    h += F("<button type='submit'>Luu &amp; Khoi dong lai</button></form></body></html>");
    return h;
  }

  void handleRoot() { server.send(200, "text/html", buildPage()); }

  void handleSave() {
    SlaveConfig &c = store->cfg;
    if (server.hasArg("cam")) {
      int v = server.arg("cam").toInt();
      if (v >= 1 && v <= 4) c.camId = v;
    }
    if (server.hasArg("chan")) {
      int v = server.arg("chan").toInt();
      if (v >= 1 && v <= 13) c.channel = v;
    }
    if (server.hasArg("name"))
      server.arg("name").toCharArray(c.name, sizeof(c.name));
    c.longRange = server.hasArg("lr");
    store->save();

    String ok = F("<html><head><meta charset='utf-8'>"
                  "<meta http-equiv='refresh' content='4'></head>"
                  "<body style='font-family:sans-serif;background:#062;color:#fff;text-align:center;padding-top:60px'>"
                  "<h1>Da luu!</h1><p>Slave dang khoi dong lai...</p></body></html>");
    server.send(200, "text/html", ok);
    shouldReboot = true;
    rebootAt = millis() + 1500;
  }

public:
  WebPortal() : server(80) {}

  void begin(ConfigStore *s) {
    store = s;
    snprintf(apSsid, sizeof(apSsid), "TALLY-CAM%d-Setup", s->cfg.camId);
    Serial.println("[Portal] Starting config AP...");
    WiFi.mode(WIFI_AP);
    IPAddress apIP(AP_IP_OCT);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSsid, AP_PASS);
    delay(200);
    dns.start(53, "*", apIP);
    server.on("/", [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.onNotFound([this]() { handleRoot(); });
    server.begin();
    Serial.printf("[Portal] SSID: %s  PASS: %s\n", apSsid, AP_PASS);
    Serial.printf("[Portal] Open http://%s\n", apIP.toString().c_str());
  }

  const char *ssid() { return apSsid; }
  String ip() { return WiFi.softAPIP().toString(); }

  void handle() {
    dns.processNextRequest();
    server.handleClient();
    if (shouldReboot && millis() > rebootAt) {
      delay(100);
      ESP.restart();
    }
  }
};

#endif // WEBPORTAL_H

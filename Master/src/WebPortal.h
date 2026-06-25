#ifndef WEBPORTAL_H
#define WEBPORTAL_H

/* =====================================================
   TALLY V2 - MASTER WEB PORTAL (chế độ cấu hình)
   Tạo WiFi AP + web form để cấu hình IP / kênh / MAC
   Slave mà KHÔNG cần nạp lại firmware.
   Chỉ chạy ở "Config Mode" (giữ nút BOOT khi khởi động
   hoặc khi chưa có cấu hình). Không bật ESP-NOW song song
   để tránh nhiễu kênh.
   ===================================================== */

#include "Config.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#define AP_SSID "TALLY-Master-Setup"
#define AP_PASS "tally1234" // tối thiểu 8 ký tự
#define AP_IP_OCT 192, 168, 4, 1

class WebPortal {
private:
  WebServer server;
  DNSServer dns;
  ConfigStore *store;
  bool shouldReboot = false;
  unsigned long rebootAt = 0;

  static String macToStr(const uint8_t *m) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2],
             m[3], m[4], m[5]);
    return String(buf);
  }

  // Parse "AA:BB:CC:DD:EE:FF" -> 6 byte. Trả về true nếu hợp lệ.
  static bool parseMac(const String &s, uint8_t *out) {
    int vals[6];
    if (sscanf(s.c_str(), "%x:%x:%x:%x:%x:%x", &vals[0], &vals[1], &vals[2],
               &vals[3], &vals[4], &vals[5]) != 6)
      return false;
    for (int i = 0; i < 6; i++)
      out[i] = (uint8_t)vals[i];
    return true;
  }

  String buildPage() {
    MasterConfig &c = store->cfg;
    String h = F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
                 "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                 "<title>TALLY V2 Setup</title><style>"
                 "body{font-family:sans-serif;background:#111;color:#eee;margin:0;padding:16px}"
                 "h1{color:#0cf;font-size:20px}h2{color:#fc0;font-size:15px;margin-top:18px}"
                 "label{display:block;margin:8px 0 2px;font-size:13px}"
                 "input,select{width:100%;padding:8px;box-sizing:border-box;border-radius:6px;"
                 "border:1px solid #444;background:#222;color:#fff}"
                 "button{margin-top:16px;width:100%;padding:12px;background:#0a8;color:#fff;"
                 "border:0;border-radius:8px;font-size:16px}"
                 ".row{display:flex;gap:8px}.row>div{flex:1}"
                 ".card{background:#1a1a1a;padding:12px;border-radius:8px;margin-top:8px}"
                 "</style></head><body><h1>TALLY V2 - Cau hinh Master</h1><form method='POST' action='/save'>");

    h += F("<h2>Mang (Ethernet)</h2>");
    h += "<label><input type='checkbox' name='dhcp' style='width:auto'";
    if (c.useDhcp) h += " checked";
    h += "> Dung DHCP (tu dong lay IP)</label>";
    h += "<label>IP tinh</label><input name='ip' value='" + String(c.ip) + "'>";
    h += "<div class='row'><div><label>Gateway</label><input name='gw' value='" + String(c.gw) + "'></div>";
    h += "<div><label>Subnet</label><input name='sn' value='" + String(c.sn) + "'></div></div>";
    h += "<label>DNS</label><input name='dns' value='" + String(c.dns) + "'>";

    h += F("<h2>ESP-NOW</h2>");
    h += "<div class='row'><div><label>Kenh (1-13)</label><input name='chan' type='number' min='1' max='13' value='" + String(c.channel) + "'></div>";
    h += "<div><label>So Slave</label><input name='nslaves' type='number' min='1' max='4' value='" + String(c.numSlaves) + "'></div></div>";
    h += "<label><input type='checkbox' name='lr' style='width:auto'";
    if (c.longRange) h += " checked";
    h += "> Long Range (tang tam, ca Slave phai bat)</label>";

    h += F("<h2>Danh sach Slave</h2>");
    for (int i = 0; i < MAX_SLAVES; i++) {
      h += "<div class='card'><b>Slave " + String(i + 1) + "</b>";
      h += "<label>Ten</label><input name='name" + String(i) + "' value='" + String(c.names[i]) + "'>";
      h += "<label>MAC</label><input name='mac" + String(i) + "' value='" + macToStr(c.macs[i]) + "' placeholder='AA:BB:CC:DD:EE:FF'>";
      h += "</div>";
    }

    h += F("<button type='submit'>Luu &amp; Khoi dong lai</button></form>"
           "<p style='font-size:11px;color:#888'>Sau khi luu, Master se reboot va chay binh thuong.</p>"
           "</body></html>");
    return h;
  }

  void handleRoot() { server.send(200, "text/html", buildPage()); }

  void handleSave() {
    MasterConfig &c = store->cfg;
    c.useDhcp = server.hasArg("dhcp");
    if (server.hasArg("ip")) server.arg("ip").toCharArray(c.ip, sizeof(c.ip));
    if (server.hasArg("gw")) server.arg("gw").toCharArray(c.gw, sizeof(c.gw));
    if (server.hasArg("sn")) server.arg("sn").toCharArray(c.sn, sizeof(c.sn));
    if (server.hasArg("dns")) server.arg("dns").toCharArray(c.dns, sizeof(c.dns));
    if (server.hasArg("chan")) {
      int ch = server.arg("chan").toInt();
      if (ch >= 1 && ch <= 13) c.channel = ch;
    }
    if (server.hasArg("nslaves")) {
      int n = server.arg("nslaves").toInt();
      if (n >= 1 && n <= MAX_SLAVES) c.numSlaves = n;
    }
    c.longRange = server.hasArg("lr");

    for (int i = 0; i < MAX_SLAVES; i++) {
      String nk = "name" + String(i);
      String mk = "mac" + String(i);
      if (server.hasArg(nk))
        server.arg(nk).toCharArray(c.names[i], sizeof(c.names[i]));
      if (server.hasArg(mk)) {
        uint8_t tmp[6];
        if (parseMac(server.arg(mk), tmp))
          memcpy(c.macs[i], tmp, 6);
      }
    }

    store->save();

    String ok = F("<html><head><meta charset='utf-8'>"
                  "<meta http-equiv='refresh' content='4'></head>"
                  "<body style='font-family:sans-serif;background:#062;color:#fff;text-align:center;padding-top:60px'>"
                  "<h1>Da luu!</h1><p>Master dang khoi dong lai...</p></body></html>");
    server.send(200, "text/html", ok);

    shouldReboot = true;
    rebootAt = millis() + 1500;
  }

public:
  WebPortal() : server(80) {}

  // Bắt đầu chế độ portal (AP + web server)
  void begin(ConfigStore *s) {
    store = s;
    Serial.println("[Portal] Starting config AP...");
    WiFi.mode(WIFI_AP);
    IPAddress apIP(AP_IP_OCT);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);
    delay(200);

    dns.start(53, "*", apIP); // captive portal: mọi domain -> AP

    server.on("/", [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.onNotFound([this]() { handleRoot(); }); // captive redirect
    server.begin();

    Serial.printf("[Portal] SSID: %s  PASS: %s\n", AP_SSID, AP_PASS);
    Serial.printf("[Portal] Open http://%s\n", apIP.toString().c_str());
  }

  const char *ssid() { return AP_SSID; }
  String ip() { return WiFi.softAPIP().toString(); }

  // Gọi trong loop khi ở config mode
  void handle() {
    dns.processNextRequest();
    server.handleClient();
    if (shouldReboot && millis() > rebootAt) {
      Serial.println("[Portal] Rebooting...");
      delay(100);
      ESP.restart();
    }
  }
};

#endif // WEBPORTAL_H

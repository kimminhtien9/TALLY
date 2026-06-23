/* =====================================================
   TALLY V2 - SLAVE (Seeed XIAO ESP32C3)
   - CAM_ID cấu hình động qua Web Portal (giữ nút BOOT)
   - ESP-NOW Long Range (khớp Master) -> ổn định, xa hơn
   - Fail-safe nháy xanh dương khi mất tín hiệu
   ===================================================== */

#include "Config.h"
#include "WebPortal.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

/* ===== PIN LED (Common Cathode: HIGH = sáng) ===== */
#define LED1_RED 2   // D0
#define LED1_GREEN 3 // D1
#define LED1_BLUE 4  // D2
#define LED2_RED 5   // D3
#define LED2_GREEN 6 // D4
#define LED2_BLUE 7  // D5

/* ===== NÚT CẤU HÌNH ===== */
#define BOOT_BUTTON 9       // Nút BOOT trên XIAO ESP32C3
#define CONFIG_HOLD_MS 1500 // Giữ >= 1.5s lúc khởi động -> config mode

/* ===== FAIL-SAFE ===== */
#define FAILSAFE_TIMEOUT 10000 // ms không tín hiệu -> nháy đèn
#define BLINK_INTERVAL 1000    // chu kỳ nháy

/* ===== TALLY PROTOCOL (KHỚP Master) ===== */
#define TALLY_MAGIC 0xA7
#define TALLY_PROTO_VER 2
typedef struct {
  uint8_t magic;   // 0xA7
  uint8_t version; // 2
  uint8_t camId;   // 1..4
  uint8_t state;   // 0=OFF, 1=PVW, 2=PGM
  uint8_t seq;     // sequence
} TallyPacket;

/* ===== GLOBAL ===== */
ConfigStore configStore;
WebPortal portal;
bool configMode = false;

uint8_t currentState = 0;
unsigned long lastRxTime = 0;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
bool connectionLost = false;

/* ===== ĐIỀU KHIỂN LED ===== */
void allLedOff() {
  digitalWrite(LED1_RED, LOW);
  digitalWrite(LED2_RED, LOW);
  digitalWrite(LED1_GREEN, LOW);
  digitalWrite(LED2_GREEN, LOW);
  digitalWrite(LED1_BLUE, LOW);
  digitalWrite(LED2_BLUE, LOW);
}

void setLED(uint8_t state) {
  allLedOff();
  switch (state) {
  case 0: // OFF
    break;
  case 1: // PVW -> Xanh lá
    digitalWrite(LED1_GREEN, HIGH);
    digitalWrite(LED2_GREEN, HIGH);
    Serial.println("[LED] GREEN (PVW)");
    break;
  case 2: // PGM -> Đỏ
    digitalWrite(LED1_RED, HIGH);
    digitalWrite(LED2_RED, HIGH);
    Serial.println("[LED] RED (PGM)");
    break;
  }
}

void setLedSolidBlue() {
  allLedOff();
  digitalWrite(LED1_BLUE, HIGH);
  digitalWrite(LED2_BLUE, HIGH);
}

/* ===== Nháy xanh dương khi mất kết nối ===== */
void blinkFailsafeLED() {
  unsigned long now = millis();
  if (now - lastBlinkTime >= BLINK_INTERVAL) {
    blinkState = !blinkState;
    allLedOff();
    digitalWrite(LED1_BLUE, blinkState ? HIGH : LOW);
    digitalWrite(LED2_BLUE, blinkState ? HIGH : LOW);
    lastBlinkTime = now;
  }
}

/* ===== Callback ESP-NOW ===== */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
#else
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int len) {
#endif
  if (len != sizeof(TallyPacket))
    return;

  TallyPacket packet;
  memcpy(&packet, data, sizeof(packet));

  // Lọc gói rác / sai phiên bản
  if (packet.magic != TALLY_MAGIC || packet.version != TALLY_PROTO_VER)
    return;

  // Bỏ qua packet không dành cho Slave này
  if (packet.camId != configStore.cfg.camId)
    return;

  if (packet.state != currentState) {
    currentState = packet.state;
    setLED(currentState);
  }

  lastRxTime = millis();
  if (connectionLost) {
    connectionLost = false;
    Serial.println("[RECONNECT] Kết nối đã khôi phục!");
  }
}

/* ===== Cấu hình radio ESP-NOW (khóa kênh, LR, công suất max) ===== */
void initRadio() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(configStore.cfg.channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_ps(WIFI_PS_NONE);

  if (configStore.cfg.longRange) {
    esp_err_t r = esp_wifi_set_protocol(WIFI_IF_STA,
                                        WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G |
                                            WIFI_PROTOCOL_11N |
                                            WIFI_PROTOCOL_LR);
    Serial.printf("[Radio] Long Range: %s\n", r == ESP_OK ? "ON" : "FAILED");
  }
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
}

/* ===== Kiểm tra nút giữ để vào config mode ===== */
bool checkConfigButton() {
  pinMode(BOOT_BUTTON, INPUT_PULLUP);
  if (digitalRead(BOOT_BUTTON) != LOW)
    return false;
  unsigned long start = millis();
  while (digitalRead(BOOT_BUTTON) == LOW) {
    if (millis() - start >= CONFIG_HOLD_MS)
      return true;
    delay(10);
  }
  return false;
}

/* ===== In trạng thái ===== */
void printSystemStatus() {
  Serial.println("\n----------------------------------");
  Serial.printf("   [CAM %d] SLAVE STATUS\n", configStore.cfg.camId);
  Serial.print("MAC:   ");
  Serial.println(WiFi.macAddress());
  Serial.print("CONN:  ");
  Serial.println(connectionLost ? "FAIL" : "OK");
  Serial.print("LED:   ");
  switch (currentState) {
  case 0: Serial.println("OFF"); break;
  case 1: Serial.println("GREEN (PVW)"); break;
  case 2: Serial.println("RED (PGM)"); break;
  }
  Serial.printf("Rx:    %lu ms ago\n", millis() - lastRxTime);
  Serial.println("----------------------------------\n");
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== TALLY V2 SLAVE (XIAO ESP32C3) ===");

  // Khởi tạo LED
  uint8_t pins[] = {LED1_RED, LED1_GREEN, LED1_BLUE,
                    LED2_RED, LED2_GREEN, LED2_BLUE};
  for (uint8_t p : pins) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }

  configStore.load();
  configStore.print();

  // Vào config mode nếu giữ nút BOOT
  if (checkConfigButton()) {
    configMode = true;
    Serial.println("[Main] >>> CONFIG MODE <<<");
    setLedSolidBlue(); // báo hiệu đang ở chế độ cấu hình
    portal.begin(&configStore);
    return;
  }

  // ----- Chạy bình thường -----
  initRadio();

  Serial.println("\n***********************************");
  Serial.print("   MAC SLAVE (copy vao Master neu can):\n   ");
  Serial.println(WiFi.macAddress());
  Serial.println("***********************************\n");

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] Init FAILED!");
    return;
  }
  esp_now_register_recv_cb(onReceive);
  Serial.println("[ESP-NOW] Sẵn sàng nhận tín hiệu");

  lastRxTime = millis();
  lastBlinkTime = millis();
}

/* ===== LOOP ===== */
void loop() {
  if (configMode) {
    portal.handle();
    return;
  }

  unsigned long now = millis();

  // Fail-safe
  if (now - lastRxTime > FAILSAFE_TIMEOUT) {
    if (!connectionLost) {
      connectionLost = true;
      currentState = 0;
      Serial.println("[FAIL-SAFE] Mất kết nối! Nháy xanh dương...");
    }
    blinkFailsafeLED();
  }

  // Báo cáo trạng thái mỗi 10s
  static unsigned long lastPrint = 0;
  if (now - lastPrint > 10000) {
    lastPrint = now;
    printSystemStatus();
  }
}

#include <WiFi.h>
#include <esp_now.h>

#include <esp_wifi.h>

/* ===== CẤU HÌNH ===== */
#define CAM_ID 1 // Đổi thành 1, 2, 3, hoặc 4 cho mỗi Slave

/* =====================================================
   XIAO ESP32C3 - Pin LED
   LED 1 (trước) và LED 2 (sau) hiển thị CÙNG MÀU
   Common Cathode: HIGH = sáng, LOW = tắt
   ===================================================== */

/* LED 1 - Mặt trước */
#define LED1_RED 2   // D0
#define LED1_GREEN 3 // D1
#define LED1_BLUE 4  // D2

/* LED 2 - Mặt sau */
#define LED2_RED 5   // D3
#define LED2_GREEN 6 // D4 (U0TXD - output only)
#define LED2_BLUE 7  // D5

/* ===== FAIL-SAFE ===== */
#define FAILSAFE_TIMEOUT 10000 // ms không tín hiệu → chớp đèn (Tăng lên 10s để tránh báo lỗi giả do rớt sóng wifi)
#define BLINK_INTERVAL 1000   // ms mỗi lần chớp

/* ===== TALLY PACKET ===== */
typedef struct {
  uint8_t camId; // ID Camera (1–4)
  uint8_t state; // 0=OFF, 1=PVW (Xanh), 2=PGM (Đỏ)
} TallyPacket;

/* ===== BIẾN TRẠNG THÁI ===== */
uint8_t currentState = 0;
unsigned long lastRxTime = 0;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
bool connectionLost = false;

/* ===== ĐIỀU KHIỂN LED ===== */
// Cả LED1 và LED2 luôn hiển thị cùng một màu
void setLED(uint8_t state) {
  // Tắt hết trước
  digitalWrite(LED1_RED, LOW);
  digitalWrite(LED2_RED, LOW);
  digitalWrite(LED1_GREEN, LOW);
  digitalWrite(LED2_GREEN, LOW);
  digitalWrite(LED1_BLUE, LOW);
  digitalWrite(LED2_BLUE, LOW);

  switch (state) {
  case 0: // OFF
    break;

  case 1: // PVW → Xanh lá
    digitalWrite(LED1_GREEN, HIGH);
    digitalWrite(LED2_GREEN, HIGH);
    Serial.println("[LED] GREEN (PVW)");
    break;

  case 2: // PGM → Đỏ
    digitalWrite(LED1_RED, HIGH);
    digitalWrite(LED2_RED, HIGH);
    Serial.println("[LED] RED (PGM)");
    break;

  default:
    Serial.printf("[LED] Unknown state: %d\n", state);
    break;
  }
}

/* ===== CHỚP ĐÈN XANH DƯƠNG (Mất kết nối) ===== */
void blinkFailsafeLED() {
  unsigned long now = millis();
  if (now - lastBlinkTime >= BLINK_INTERVAL) {
    blinkState = !blinkState;
    digitalWrite(LED1_RED, LOW);
    digitalWrite(LED2_RED, LOW);
    digitalWrite(LED1_GREEN, LOW);
    digitalWrite(LED2_GREEN, LOW);
    digitalWrite(LED1_BLUE, blinkState ? HIGH : LOW);
    digitalWrite(LED2_BLUE, blinkState ? HIGH : LOW);
    lastBlinkTime = now;
  }
}

/* ===== CALLBACK ESP-NOW ===== */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
#else
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int len) {
#endif
  if (len != sizeof(TallyPacket)) {
    Serial.printf("[RX] Invalid packet size: %d\n", len);
    return;
  }

  TallyPacket packet;
  memcpy(&packet, data, sizeof(packet));

  Serial.printf("[RX] camId=%d, state=%d\n", packet.camId, packet.state);

  // Bỏ qua packet không dành cho Slave này
  if (packet.camId != CAM_ID)
    return;

  if (packet.state != currentState) {
    currentState = packet.state;
    setLED(currentState);
  }

  // Reset fail-safe
  lastRxTime = millis();
  if (connectionLost) {
    connectionLost = false;
    Serial.println("[RECONNECT] Kết nối đã khôi phục!");
  }
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);
  delay(1000); // Chờ USB CDC sẵn sàng

  Serial.println("\n=== TALLY SLAVE (XIAO ESP32C3) ===");
  Serial.printf("CAM ID: %d\n", CAM_ID);

  // Khởi tạo LED pins
  uint8_t pins[] = {LED1_RED, LED1_GREEN, LED1_BLUE,
                    LED2_RED, LED2_GREEN, LED2_BLUE};
  for (uint8_t p : pins) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }

  // WiFi STA (không connect AP) để lấy MAC & dùng ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Khóa cố định kênh WiFi số 1 (Để trùng khớp với cấu hình peer channel = 1 của Master)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_ps(WIFI_PS_NONE); // Vô hiệu hóa chế độ Ngủ Đông của WiFi để không bị rớt mạng Tally!
  
  // Tăng công suất phát sóng lên tối đa (tùy thuộc vào phần cứng hỗ trợ, C3 tối đa có thể lên 20dBm)
  WiFi.setTxPower(WIFI_POWER_19_5dBm); 

  
  // IN ĐỊA CHỈ MAC THẬT NỔI BẬT ĐỂ COPY
  Serial.println("\n***********************************");
  Serial.print("   COPY ĐỊA CHỈ MAC NÀY VÀO MASTER:\n   ");
  Serial.println(WiFi.macAddress());
  Serial.println("***********************************\n");

  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] Init FAILED!");
    return;
  }
  esp_now_register_recv_cb(onReceive);

  Serial.println("[ESP-NOW] Sẵn sàng nhận tín hiệu");
  Serial.println("Chờ lệnh tally...\n");

  lastRxTime = millis();
  lastBlinkTime = millis();
}

/* ===== HELPER: In trạng thái ra Serial ===== */
void printSystemStatus() {
  Serial.println("\n----------------------------------");
  Serial.printf("   [CAM %d] SLAVE STATUS\n", CAM_ID);
  Serial.println("----------------------------------");
  
  // MAC Address
  Serial.print("MAC:   ");
  Serial.println(WiFi.macAddress());

  // Trạng thái kết nối
  Serial.print("CONN:  ");
  Serial.println(connectionLost ? "FAIL" : "OK");

  // Trạng thái đèn LED
  Serial.print("LED:   ");
  switch (currentState) {
    case 0: Serial.println("OFF"); break;
    case 1: Serial.println("GREEN (PVW)"); break;
    case 2: Serial.println("RED (PGM)"); break;
    default: Serial.println("UNKNOWN"); break;
  }
  
  // Tín hiệu cuối cùng
  Serial.print("Rx:    ");
  Serial.print(millis() - lastRxTime);
  Serial.println(" ms ago");
  
  Serial.println("----------------------------------\n");
}

/* ===== LOOP ===== */
void loop() {
  unsigned long now = millis();

  // Kiểm tra fail-safe
  if (now - lastRxTime > FAILSAFE_TIMEOUT) {
    if (!connectionLost) {
      connectionLost = true;
      currentState = 0;
      Serial.println("[FAIL-SAFE] Mất kết nối! Nháy Xanh dương (Blue)...");
    }
    blinkFailsafeLED();
  }
  
  // In lại trạng thái mỗi 10 giây (Heartbeat) để dễ theo dõi
  static unsigned long lastPrintTime = 0;
  if (now - lastPrintTime > 10000) {
    lastPrintTime = now;
    printSystemStatus();
  }
}

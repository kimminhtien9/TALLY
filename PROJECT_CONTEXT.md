# PROJECT ANTIGRAVITY: OSEE WIRELESS TALLY SYSTEM

## 1. TỔNG QUAN

* **Tên dự án:** Antigravity Tally
* **Mục tiêu:** Hệ thống đèn Tally không dây độ trễ thấp cho bàn trộn Osee Go Stream Duet.
* **Kiến trúc:** Hybrid (Lai).
    * **Input:** Nhận UDP từ Osee Switcher qua **Ethernet** (W5500).
    * **Output:** Gửi tín hiệu tới các đèn Slave qua **ESP-NOW** (độ trễ thấp).

---

## 2. PHẦN CỨNG

### Master (Bộ phát)
* **Board:** ESP32 DevKit V1 CP2102 (có cổng U.FL)
* **Màn hình:** ST7789 1.3" SPI TFT LCD (240×240)
* **Mạng:** W5500 Ethernet Module (SPI, kết nối Osee qua cáp LAN)
* **Anten:** Anten rời 2.4GHz – pigtail IPEX → SMA – dùng cho ESP-NOW

### Slave (Đèn Tally)
* **Board:** Seeed Studio XIAO ESP32C3
* **Anten:** Anten rời 2.4GHz (cổng IPEX tích hợp trên board)
* **LED:** 2× RGB LED 4 chân Common Cathode (cùng màu, nhìn được 2 phía)
* **Pin:** LiPo 3.7V 1200mAh, sạc qua USB-C tích hợp trên XIAO

---

## 3. GIAO THỨC & DỮ LIỆU

### A. Ethernet + UDP (Osee → Master)
```
Port: 19018
Format: JSON
Ví dụ: {"id": "pgmTally", "value": [1]}
```

### B. ESP-NOW (Master → Slave)
```cpp
typedef struct {
  uint8_t camId;  // ID Camera (1–4)
  uint8_t state;  // 0=OFF, 1=PVW (Xanh), 2=PGM (Đỏ)
} TallyPacket;
```

---

## 4. TRẠNG THÁI LED SLAVE

| State | LED | Mô tả |
|-------|-----|-------|
| PGM | 🔴 Đỏ | Camera đang On Air |
| PVW | 🟢 Xanh lá | Camera đang Preview |
| OFF | ⬛ Tắt | Camera không hoạt động |
| Mất kết nối | 🔴💡 Chớp đỏ | Không nhận tín hiệu >2 giây |

---

## 5. LỘ TRÌNH PHÁT TRIỂN

### Phase 1: Stability ✅
* [x] Tách code thành module `.h`
* [x] Fail-safe cho Slave (chớp đèn khi mất tín hiệu)
* [x] Kết nối Ethernet thay WiFi cho Master
* [x] Hỗ trợ anten rời cả Master và Slave

### Phase 2: User Experience
* [ ] Web config portal (Captive Portal) trên Master
* [ ] OTA firmware update

### Phase 3: Hardware
* [ ] Thiết kế PCB tích hợp
* [ ] Vỏ in 3D

---

## 6. QUY TẮC CODE

* Arduino Framework (C++)
* **Không dùng `delay()`** – dùng `millis()` cho non-blocking
* Serial debug ở 115200 baud
* Comment rõ ràng phần xử lý mạng
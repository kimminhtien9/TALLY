# TÍNH TOÁN KHOẢNG CÁCH ESP-NOW VỚI ANTEN RỜI

Hướng dẫn tính toán tầm hoạt động của ESP-NOW giữa Master và Slave.

---

## Thông Số Kỹ Thuật

### ESP32 2.4GHz Radio

**ESP32 DevKit V1 (Master):**
- Công suất phát: 20 dBm (100mW) max
- Độ nhạy thu: -97 dBm @ 1 Mbps
- Cổng anten: U.FL (cho anten rời)

**ESP32-C3 SuperMini (Slave):**
- Công suất phát: 21 dBm (125mW) max
- Độ nhạy thu: -98 dBm @ 1 Mbps
- Anten: PCB onboard (không rời)

---

## So Sánh Loại Anten

### 1. Anten PCB Onboard

**Đặc điểm:**
- Gain: ~0 dBi (tham chiếu)
- Kích thước: Nhỏ gọn, tích hợp
- Định hướng: Đa hướng (omnidirectional)
- **Tầm hoạt động:** ~10-30m trong nhà

### 2. Anten Rời 2.4GHz (External)

**Loại Dipole (Phổ biến nhất):**
- Gain: 2-3 dBi
- Định hướng: Đa hướng
- Kích thước: 5-10cm
- **Tầm hoạt động:** ~50-100m trong nhà

**Loại Directional (Định hướng):**
- Gain: 5-9 dBi
- Định hướng: Hướng cố định
- Kích thước: 10-20cm
- **Tầm hoạt động:** ~100-200m (theo hướng)

---

## Công Thức Tính Toán

### Path Loss (Suy hao đường truyền)

**Free Space Path Loss (FSPL):**
```
FSPL (dB) = 20 × log10(d) + 20 × log10(f) + 32.45

Trong đó:
- d = khoảng cách (km)
- f = tần số (MHz, 2400 MHz cho WiFi 2.4GHz)
```

**Ví dụ:**
- d = 0.05 km (50m)
- f = 2400 MHz

```
FSPL = 20 × log10(0.05) + 20 × log10(2400) + 32.45
     = 20 × (-1.3) + 20 × 3.38 + 32.45
     = -26 + 67.6 + 32.45
     = 74 dB
```

---

### Link Budget

**Công thức:**
```
Link Budget = Tx Power + Tx Gain - FSPL + Rx Gain - Rx Sensitivity

Cần: Link Budget > 0 dB (có dự phòng tốt hơn)
```

---

## Tính Toán Thực Tế

### Scenario 1: Master có Anten Rời, Slave Onboard

**Setup:**
- Master TX: 20 dBm
- Anten Master: +3 dBi (dipole)
- Slave RX Sensitivity: -98 dBm
- Anten Slave (PCB): 0 dBi

**Khoảng cách 50m (trong nhà):**
```
FSPL @ 50m = 74 dB (tính ở trên)
Indoor loss = +20 dB (tường, vật cản)
Total loss = 74 + 20 = 94 dB

Link Budget:
= 20 (TX) + 3 (Ant) - 94 (Loss) + 0 (RX Ant) - (-98) (Sens)
= 20 + 3 - 94 + 0 + 98
= 27 dB ✅ TỐT (dự phòng cao)
```

**Kết luận:** Hoạt động tốt, có dự phòng 27 dB.

---

### Scenario 2: Không Anten Rời (Cả 2 Onboard)

**Setup:**
- Master TX: 20 dBm
- Anten Master (PCB): 0 dBi
- Slave RX Sensitivity: -98 dBm
- Anten Slave (PCB): 0 dBi

**Khoảng cách 50m (trong nhà):**
```
Link Budget:
= 20 + 0 - 94 + 0 + 98
= 24 dB ✅ WORK (nhưng dự phòng ít hơn)
```

**Kết luận:** Vẫn hoạt động nhưng kém ổn định hơn.

---

### Scenario 3: Khoảng Cách Tối Đa

**Với anten rời +3 dBi:**
```
Link Budget ≥ 10 dB (dự phòng tối thiểu)

10 = 20 + 3 - FSPL + 0 + 98
FSPL = 111 dB

Từ công thức FSPL:
111 = 20 × log10(d) + 67.6 + 32.45
20 × log10(d) = 10.95
log10(d) = 0.55
d = 3.5 km (lý thuyết, không có vật cản)

Trong nhà (có tường):
→ Giảm ~70%
→ Tầm thực tế: ~1 km (tối đa)
→ Tầm an toàn: ~200-300m
```

---

## Bảng Tham Khảo Tầm Hoạt Động

| Setup | Môi Trường | Tầm Hoạt Động | Độ Ổn Định |
|-------|------------|---------------|------------|
| **Không anten rời** | Trong nhà | 10-30m | ⭐⭐ |
| | Ngoài trời | 30-100m | ⭐⭐⭐ |
| **Master có anten +3dBi** | Trong nhà | 50-100m | ⭐⭐⭐⭐ |
| | Ngoài trời | 100-300m | ⭐⭐⭐⭐⭐ |
| **Cả 2 có anten +3dBi** | Trong nhà | 100-200m | ⭐⭐⭐⭐⭐ |
| | Ngoài trời | 300-500m | ⭐⭐⭐⭐⭐ |

---

## Yếu Tố Ảnh Hưởng

### 1. Vật Cản

| Vật Liệu | Suy Hao |
|----------|---------|
| Không khí (line of sight) | 0 dB |
| Tường thạch cao | 3-5 dB |
| Tường gạch | 8-15 dB |
| Tường beton | 10-20 dB |
| Tường có sắt thép | 20-30 dB |
| Kim loại | 30-50 dB |

**Ví dụ:**
- 2 tường gạch: -20 dB → giảm tầm ~60%
- 3 tường beton: -40 dB → giảm tầm ~80%

---

### 2. Nhiễu 2.4GHz

**Nguồn nhiễu:**
- WiFi routers
- Bluetooth devices
- Microwave ovens
- Camera không dây
- Wireless keyboards/mice

**Ảnh hưởng:**
- Giảm **độ ổn định** hơn là tầm hoạt động
- Có thể gây packet loss
- ESP-NOW tương đối kháng nhiễu do độ trễ thấp

---

### 3. Hướng Anten

**Anten Dipole (đa hướng):**
```
        ↑ Yếu
        │
    ────┼──── Mạnh (vòng tròn)
        │
        ↓ Yếu
```

**Lưu ý:**
- Đặt anten thẳng đứng cho tầm tốt nhất
- Tránh nằm ngang nếu Slave ở xa ngang

---

### 4. Vị Trí Đặt

**Tối ưu:**
- ✅ Đặt cao (trên 2m)
- ✅ Tránh gần kim loại lớn
- ✅ Tránh gần tường beton dày
- ✅ Line of sight nếu được

**Tránh:**
- ❌ Đặt trong hộp kim loại
- ❌ Gần microwave
- ❌ Dưới sàn beton
- ❌ Trong góc phòng

---

## Khuyến Nghị Cho Hệ Thống Tally

### Setup 1: Studio Nhỏ (<30m)

**Không cần anten rời:**
- Master: PCB antenna
- Slave: PCB antenna
- **Chi phí:** 0 VND thêm
- **Độ tin cậy:** ⭐⭐⭐

---

### Setup 2: Studio Trung Bình (30-100m)

**Master có anten rời (Khuyến nghị):**
- Master: Anten dipole +3 dBi
- Slave: PCB antenna (ESP32-C3)
- **Chi phí:** +10-20k (1 anten)
- **Độ tin cậy:** ⭐⭐⭐⭐⭐
- **Best value!**

---

### Setup 3: Studio Lớn (100-200m)

**Cả 2 có anten rời:**
- Master: Anten +3 dBi
- Slave: ESP32 DevKit + Anten +3 dBi (thay ESP32-C3)
- **Chi phí:** +200-250k (so với Setup 2)
- **Độ tin cậy:** ⭐⭐⭐⭐⭐
- **Link Budget:** 30 dB (excellent!)
- Xem [SLAVE_ANTENNA_UPGRADE.md](SLAVE_ANTENNA_UPGRADE.md) để upgrade

**Tính toán @ 100m trong nhà:**
```
Link Budget = 20 + 3 - 100 + 3 + 98 = 24 dB ✅
```

---

### Setup 4: Ngoài Trời / Very Long Range (>200m)

**Directional antennas:**
- Master: Anten directional +5-9 dBi hoặc directional +5-9 dBi
- Slave: Thay ESP32 thường có U.FL, gắn anten +3 dBi hoặc directional
- **Chi phí:** +100-200k
- **Độ tin cậy:** ⭐⭐⭐⭐⭐
- Tầm: Tới 500m (line of sight)

---

## Test Thực Tế

### Bước 1: Test Tầm Cơ Bản

```cpp
// Thêm vào Slave code (loop)
void loop() {
  static unsigned long lastPrint = 0;
  
  if (millis() - lastPrint > 1000) {
    Serial.printf("RSSI: %d dBm, Packets: %d\n", WiFi.RSSI(), packetCount);
    lastPrint = millis();
  }
  
  // ... fail-safe code
}
```

**Cách test:**
1. Đặt Master và Slave cách nhau 10m
2. Quan sát RSSI trên Serial
3. Từ từ di chuyển Slave ra xa
4. Note khoảng cách khi RSSI < -80 dBm (bắt đầu yếu)

---

### Bước 2: Test Packet Loss

```cpp
// Master gửi counter
static uint32_t packetCounter = 0;
packet.counter = packetCounter++;

// Slave check
if (packet.counter != expectedCounter) {
  Serial.printf("LOST %d packets\n", packet.counter - expectedCounter);
}
```

**Metric tốt:**
- RSSI > -70 dBm: Excellent
- RSSI -70 to -80 dBm: Good
- RSSI -80 to -90 dBm: Fair (có thể loss)
- RSSI < -90 dBm: Poor (nhiều loss)

---

### Bước 3: Test Với Vật Cản

1. Test trong line of sight (không vật cản)
2. Test qua 1 tường
3. Test qua 2 tường
4. Test qua 3 tường
5. Note tầm giảm bao nhiêu % mỗi tường

---

## Lựa Chọn Anten

### Anten Dipole 2.4GHz (Khuyến nghị)

**Spec:**
- Frequency: 2.4-2.5 GHz
- Gain: 2-3 dBi
- Connector: IPEX/U.FL
- Impedance: 50Ω
- Giá: 10-20k

**Mua ở đâu:**
- Shopee: tìm "anten 2.4ghz ipex"
- Lazada: "wifi antenna ufl"
- Điện tử Bách Khoa

---

### Anten PCB (Budget)

Nếu không có anten rời:
- Vẫn hoạt động tốt trong ~30m
- Tiết kiệm chi phí
- Đủ cho studio nhỏ

---

## Tối Ưu Phần Mềm

### 1. Tăng Công Suất TX

```cpp
// Trong Master Network.h, sau WiFi.mode()
esp_wifi_set_max_tx_power(84);  // 84 = 21 dBm (max)
```

**Lưu ý:** Tốn điện hơn, nóng hơn.

---

### 2. Retry Mechanism

```cpp
// Gửi 2 lần để chắc chắn
network.sendTally(cam, state);
delay(10);
network.sendTally(cam, state);
```

---

### 3. Heartbeat

Master gửi heartbeat định kỳ (mỗi 500ms) để Slave detect mất kết nối nhanh hơn.

---

## Kết Luận

### Cho Hệ Thống OneTally

**Khuyến nghị:**
1. **Master:** ESP32 DevKit với anten rời +3 dBi (U.FL)
2. **Slave:** ESP32-C3 với anten PCB onboard

**Kết quả:**
- ✅ Tầm: 50-100m trong nhà
- ✅ Chi phí thêm: ~10-20k
- ✅ Độ tin cậy: Rất cao (link budget 27 dB)
- ✅ Đủ cho 99% use case

**Nếu cần >100m:**
- Upgrade Slave sang ESP32 thường có U.FL
- Thêm anten +3 dBi cho Slave
- Chi phí: +40-60k per Slave
- Tầm: Lên tới 200-300m trong nhà

---

## Quick Reference

| Tầm Mong Muốn | Setup | Chi Phí Thêm |
|---------------|-------|--------------|
| <30m | Không cần anten | 0đ |
| 30-100m | Master có anten | 10-20k |
| 100-200m | Cả 2 có anten* | 50-100k |
| >200m | Directional antenna | 100-200k |

*Slave cần ESP32 có U.FL, không dùng ESP32-C3

---

**Tóm lại:** Với Master có anten dipole +3 dBi, hệ thống sẽ hoạt động **rất tốt** trong phạm vi **50-100m** trong nhà, hoàn toàn đủ cho studio livestream!

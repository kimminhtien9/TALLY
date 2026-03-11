# TALLY SLAVE - Seeed XIAO ESP32C3

Thiết bị Slave nhận tín hiệu tally không dây, chạy bằng pin LiPo.

---

## Phần Cứng

| Linh Kiện | Thông Số |
|-----------|---------|
| Board | Seeed Studio XIAO ESP32C3 |
| Anten | Anten rời 2.4GHz (IPEX, đi kèm board) |
| LED | 2x RGB LED 4 chân (Common Cathode) |
| Điện trở | 6x 220Ω |
| Pin | LiPo 3.7V 1200mAh (30×40mm, dày 10mm) |
| Sạc pin | Cổng USB-C tích hợp trên XIAO ESP32C3 |

> **Lưu ý:** XIAO ESP32C3 đã có sẵn mạch sạc LiPo và cổng BAT (JST 1.25mm 2-pin).  
> Chỉ cần cắm pin vào, sạc qua USB-C.

---

## Sơ Đồ Chân XIAO ESP32C3

```
               ┌─────────────────┐
         5V ── │ 5V          GND │ ── GND
        GND ── │ GND         3V3 │ ── 3.3V
               │ [BAT+] [BAT-]   │  ← cắm pin LiPo
    D0/GPIO2 ──│                 │── D10/GPIO10
    D1/GPIO3 ──│    XIAO         │── D9/GPIO9
    D2/GPIO4 ──│   ESP32C3       │── D8/GPIO8
    D3/GPIO5 ──│                 │── D7/GPIO21
    D4/GPIO6 ──│   [IPEX] ◄──── anten rời
    D5/GPIO7 ──│                 │
               └─────────────────┘
```

---

## Cấu Hình Chân LED

| Chức năng | GPIO | Chân XIAO |
|-----------|------|-----------|
| LED1 Đỏ | GPIO 2 | D0 |
| LED1 Xanh lá | GPIO 3 | D1 |
| LED1 Xanh dương | GPIO 4 | D2 |
| LED2 Đỏ | GPIO 5 | D3 |
| LED2 Xanh lá | GPIO 6 | D4 |
| LED2 Xanh dương | GPIO 7 | D5 |

> LED1 (mặt trước) và LED2 (mặt sau) luôn hiển thị **cùng màu**.

---

## Đấu Dây LED

```
XIAO ESP32C3              RGB LED 1 (mặt trước)
                         ┌──────────────────┐
D0 (GPIO2) ──[220Ω]──────┤ R Anode          │
D1 (GPIO3) ──[220Ω]──────┤ G Anode          │
D2 (GPIO4) ──[220Ω]──────┤ B Anode          │
GND ─────────────────────┤ Cathode (chân dài)│
                         └──────────────────┘

                         RGB LED 2 (mặt sau)
                         ┌──────────────────┐
D3 (GPIO5) ──[220Ω]──────┤ R Anode          │
D4 (GPIO6) ──[220Ω]──────┤ G Anode          │
D5 (GPIO7) ──[220Ω]──────┤ B Anode          │
GND ─────────────────────┤ Cathode (chân dài)│
                         └──────────────────┘
```

> **RGB LED 4 chân:** R, G, B (anode), Cathode chung (chân dài nhất).  
> Common Cathode → HIGH = sáng, LOW = tắt.

---

## Kết Nối Pin LiPo & Công Tắc (Nguồn)

Để có thể bật/tắt thiết bị Slave dễ dàng mà không cần phải rút giắc cắm pin, bạn hãy nối tiếp một **công tắc trượt (slide switch) hoặc công tắc nhấn (toggle switch)** vào dây đỏ (+) của pin.

```
Pin LiPo 3.7V 1200mAh
├── Dây đỏ (+) ──[ CÔNG TẮC ]── BAT+ của XIAO (JST 1.25mm)
└── Dây đen (-) ─────────────── BAT- của XIAO (JST 1.25mm)
```

**Cách lắp công tắc:**
1. Cắt đôi dây đỏ (+) của đuôi cắm pin LiPo (hoặc cắt cáp pigtail JST 1.25mm).
2. Hàn 2 đầu dây vừa cắt vào 2 chân của công tắc (với công tắc 3 chân: nối vào chân giữa và 1 chân bên hông).
3. Dùng ống gen co nhiệt bọc kín các mối hàn tránh chập cháy.

**Sạc:** Cắm USB-C vào XIAO → pin tự sạc (LED đỏ bật = đang sạc, tắt = đầy).  
*(Lưu ý: Tùy mạch XIAO, bạn có thể cần bật công tắc sang ON để điện được nối thông và sạc được vào pin)*.

---

## Cấu Hình CAM_ID

Sửa `src/main.cpp` dòng 5:
```cpp
#define CAM_ID 1   // 1=Cam1, 2=Cam2, 3=Cam3, 4=Cam4
```

---

## Tính Năng

| Tính năng | Mô tả |
|-----------|-------|
| ✅ ESP-NOW | Nhận tín hiệu từ Master, độ trễ <10ms |
| ✅ 2x RGB LED | Cùng màu, nhìn được từ 2 phía |
| ✅ PGM | LED **đỏ** – camera đang On Air |
| ✅ PVW | LED **xanh lá** – camera đang Preview |
| ✅ OFF | LED tắt – camera không hoạt động |
| ✅ Fail-safe | **Nháy Đỏ - Xanh lá** 1000ms khi mất kết nối >2s |
| ✅ Pin LiPo | Hoạt động không dây, sạc qua USB-C |
| ✅ Anten rời | IPEX tích hợp, tầm xa hơn PCB |

---

## Trạng Thái LED

```
Bình thường:
  PGM  → ████ ĐỎ (sáng hoàn toàn)
  PVW  → ████ XANH LÁ
  OFF  → ████ TẮT

Mất kết nối (>2 giây):
  → 🔴🟢🔴🟢 Nháy luân phiên Đỏ - Xanh lá (chu kỳ 1000ms)
  → Tự phục hồi khi Master kết nối lại
```

---

## Build & Upload

### VS Code + PlatformIO

1. Mở thư mục `Slave/` trong VS Code
2. Sửa `CAM_ID` trong `src/main.cpp`
3. Cắm XIAO qua USB-C
4. Click **→ Upload**
5. Click **🔌 Serial Monitor** (115200 baud)

### CLI

```bash
cd Slave/
pio run -t upload
pio device monitor -b 115200
```

---

## Serial Output Mong Đợi

```
=== TALLY SLAVE (XIAO ESP32C3) ===
CAM ID: 1
MAC Address: 34:85:18:XX:XX:XX   ← Copy dòng này!
[ESP-NOW] Sẵn sàng nhận tín hiệu
Chờ lệnh tally...

[RX] camId=1, state=2
[LED] RED (PGM)
```

**Mất kết nối:**
```
[FAIL-SAFE] Mất kết nối! Nháy Đỏ - Xanh lá...
```

**Kết nối lại:**
```
[RECONNECT] Kết nối đã khôi phục!
```

---

## Lấy MAC Address

1. Upload xong → mở Serial Monitor
2. **Nhấn RESET** nếu không thấy output
3. Copy dòng: `MAC Address: XX:XX:XX:XX:XX:XX`
4. Nhập vào `Master/src/main.cpp`:

```cpp
uint8_t slave1Mac[6] = {0x34, 0x85, 0x18, 0xXX, 0xXX, 0xXX};
```

---

## Anten Rời

XIAO ESP32C3 có cổng **IPEX (U.FL)** tích hợp trên board.  
Gắn anten 2.4GHz đi kèm hoặc tương thích.

**Tầm hoạt động (cả 2 có anten):**
- Trong nhà: **100–200m**
- Ngoài trời: **300–500m**

---

## Xử Lý Lỗi

| Vấn đề | Giải pháp |
|--------|----------|
| LED không sáng | Kiểm tra chiều LED (cathode = chân dài = GND) |
| LED sáng sai màu | Kiểm tra đúng GPIO và màu |
| Không thấy Serial | Nhấn RESET sau khi mở Serial Monitor |
| Không nhận tín hiệu | Xác nhận MAC trong Master, kiểm tra anten |
| Nháy Đỏ - Xanh lá liên tục | Master mất kết nối Ethernet, tắt nguồn hoặc Slave ở quá xa |
| Pin không sạc | Kiểm tra cáp USB-C và nguồn điện |

---

## Thông Số Pin LiPo

| Thông số | Giá trị |
|----------|---------|
| Điện áp | 3.7V nominal |
| Dung lượng | 1200mAh |
| Kích thước | 30×40×10mm |
| Connector | JST 1.25mm 2-pin |
| Thời gian hoạt động* | ~8–12 giờ |

*Ước tính: XIAO (~20mA) + 2x LED (~30mA khi sáng) ≈ 50mA → 1200mAh / 50mA ≈ 24h (LED tắt), ~8-12h thực tế.

---

## Danh Sách Mua Sắm

| Linh Kiện | Giá ước tính (VND) |
|-----------|-------------------|
| Seeed XIAO ESP32C3 | 80–100k |
| Anten 2.4GHz IPEX | Thường đi kèm board |
| RGB LED 4 chân x2 | 4–10k |
| Điện trở 220Ω x6 | 2k |
| LiPo 3.7V 1200mAh | 50–80k |
| Công tắc mini (Slide/Toggle) | 2–5k |
| **Tổng/Slave** | **~145–205k** |

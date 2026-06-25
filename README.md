# OneTally - Hệ Thống Đèn Tally Không Dây

Hệ thống đèn Tally không dây cho Osee Go Stream Duet.

> 🆕 **TALLY V2** đã ra mắt: cấu hình động qua Web Portal (không cần nạp lại firmware), Dashboard LCD trên Master, ESP-NOW Long Range chống chập chờn. Xem **[V2_GUIDE.md](V2_GUIDE.md)**.

---

## Video Demo

[![Video Demo](https://img.youtube.com/vi/6-U6D97J7uo/maxresdefault.jpg)](https://www.youtube.com/watch?v=6-U6D97J7uo)

---

## Phần Cứng

### Master
| Linh Kiện | Thông Số |
|-----------|---------|
| Board | ESP32 DevKit V1 CP2102 (có cổng U.FL) |
| Màn hình | ST7789 1.3" SPI TFT LCD (240×240) |
| Ethernet | W5500 Module (SPI) |
| Anten | Anten 2.4GHz rời (IPEX → dây pigtail → SMA) |

### Slave
| Linh Kiện | Thông Số |
|-----------|---------|
| Board | Seeed Studio XIAO ESP32C3 |
| Anten | Anten rời 2.4GHz (cổng IPEX trên board) |
| LED | 2x RGB LED 4 chân Common Cathode |
| Điện trở | 6x 220Ω |
| Pin | LiPo 3.7V 1200mAh (30×40×10mm) |
| Sạc | USB-C tích hợp trên XIAO |

---

## Cấu Trúc Dự Án

```
TALLY/
├── Master/
│   ├── src/
│   │   ├── main.cpp       (cấu hình WiFi, MAC Slaves)
│   │   ├── Network.h      (Ethernet + UDP + ESP-NOW)
│   │   └── Display.h      (ST7789 LCD)
│   ├── platformio.ini
│   ├── README.md
│   ├── UPLOAD_GUIDE.md
│   └── W5500_WIRING.md
│
├── Slave/
│   ├── src/
│   │   └── main.cpp       (CAM_ID, LED pins)
│   ├── platformio.ini
│   ├── README.md
│   └── UPLOAD_GUIDE.md
│
├── README.md
├── WIRING.md
├── ANTENNA_RANGE.md
└── PROJECT_CONTEXT.md
```

---

## Bắt Đầu Nhanh

### Bước 1 – Upload Slave (lấy MAC)

```bash
cd Slave/
# Sửa CAM_ID = 1, 2, 3, 4 trong src/main.cpp
pio run -t upload
pio device monitor -b 115200
# Copy: MAC Address: XX:XX:XX:XX:XX:XX
```

Lặp lại cho từng Slave với CAM_ID khác nhau.

### Bước 2 – Upload Master

```bash
cd Master/
# Sửa src/main.cpp:
#   - STATIC_IP (hoặc nullptr để DHCP)
#   - slave1Mac, slave2Mac... (MAC vừa copy)
pio run -t upload
pio device monitor -b 115200
```

### Bước 3 – Test

```bash
# Từ máy tính cùng mạng
echo '{"id":"pgmTally","value":[1]}' | nc -u <MASTER_IP> 19018
```

→ Slave 1: LED đỏ sáng  
→ LCD Master: `PGM: 1`

---

## Kiến Trúc

```
Osee Duet
   │ Cáp Ethernet
   ▼
W5500 ─SPI─ ESP32 (Master)
                │ ESP-NOW (IPEX+pigtail+SMA)
                │ 2.4GHz Radio
         ┌──────┴──────┐
   Slave1 (XIAO+IPEX)  Slave2 ...
   2x RGB LED           2x RGB LED
   LiPo 3.7V            LiPo 3.7V
```

---

## Trạng Thái LED Slave

| State | LED | Ý nghĩa |
|-------|-----|---------|
| PGM | 🔴 Đỏ | Camera đang On Air |
| PVW | 🟢 Xanh lá | Camera đang Preview |
| OFF | ⬛ Tắt | Camera không hoạt động |
| Mất kết nối | 🔴💡 Chớp đỏ | Không nhận tín hiệu >2s |

---

## Hướng Dẫn Chi Tiết

| Tài liệu | Nội dung |
|----------|---------|
| [Master/README.md](Master/README.md) | Hardware, pin config, upload |
| [Slave/README.md](Slave/README.md) | XIAO pinout, LED, pin LiPo |
| [Master/W5500_WIRING.md](Master/W5500_WIRING.md) | Đấu dây W5500 + LCD |
| [Master/UPLOAD_GUIDE.md](Master/UPLOAD_GUIDE.md) | Upload Master chi tiết |
| [Slave/UPLOAD_GUIDE.md](Slave/UPLOAD_GUIDE.md) | Upload Slave chi tiết |
| [ANTENNA_RANGE.md](ANTENNA_RANGE.md) | Tính toán tầm anten |
| [WIRING.md](WIRING.md) | Đấu dây tổng quát |

---

## Tầm ESP-NOW (Cả 2 Có Anten Rời)

| Môi trường | Tầm hoạt động |
|------------|--------------|
| Trong nhà | 100–200m |
| Ngoài trời | 300–500m |
| Link Budget | ~30 dB |

---

## Chi Phí Ước Tính (1 Master + 4 Slaves)

| Item | Giá (VND) |
|------|-----------|
| ESP32 DevKit V1 CP2102 | 70–90k |
| ST7789 LCD 1.3" | 40–50k |
| W5500 Module | 60–80k |
| Anten + pigtail IPEX→SMA | 20–30k |
| **Tổng Master** | **~200k** |
| Seeed XIAO ESP32C3 ×4 | 320–400k |
| RGB LED + điện trở ×4 | 25–50k |
| LiPo 1200mAh ×4 | 200–320k |
| **Tổng 4 Slaves** | **~550–770k** |
| **TỔNG HỆ THỐNG** | **~750k–1 triệu** |

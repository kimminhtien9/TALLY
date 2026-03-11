# TALLY MASTER - ESP32 DevKit V1 CP2102 + ST7789 + W5500

---

## Phần Cứng

| Linh Kiện | Thông Số |
|-----------|---------|
| Board | ESP32 DevKit V1 **CP2102** (có cổng U.FL) |
| Màn hình | ST7789 1.3" SPI TFT LCD (240×240) |
| Ethernet | W5500 Module (SPI) |
| Anten | Anten 2.4GHz rời: dây pigtail **IPEX → SMA**, cắm anten SMA ngoài |

> **CP2102 vs CH340:** Board CP2102 cần driver CP2102, không phải CH340.  
> Download: [Silicon Labs CP2102 Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

---

## Nguồn Điện (Power)

Master Tally thường được đặt cố định cạnh bàn trộn Osee để cắm cáp Ethernet. Do đó, bạn có các cách cấp nguồn sau:

1. **Cổng Micro-USB (Khuyên Dùng):** Cấp nguồn 5V trực tiếp thông qua cổng Micro-USB trên mạch ESP32 DevKit V1. Bạn có thể cắm vào cổng USB của máy tính, sạc dự phòng, củ sạc điện thoại, hoặc cổng USB trên chính bàn trộn Osee.
2. **Chân VIN (hoặc 5V) và GND:** Cấp nguồn 5V DC vào chân `VIN`/`5V` và GND trên board ESP32 (Thích hợp nếu bạn thiết kế hộp có mạch nguồn/pin riêng).

**Lưu ý:** Tuyệt đối KHÔNG cấp nguồn 5V vào chân `3.3V` của ESP32, cũng như LCD và W5500. Module W5500 và màn hình ST7789 sử dụng nguồn 3.3V được hạ áp chuyển từ chính board ESP32 (chân pin `3.3V` out).

---

## Cấu Hình Chân

### ST7789 LCD (SPI)

| LCD | GPIO ESP32 |
|-----|-----------|
| VCC | 3.3V ⚠️ |
| GND | GND |
| SCL | GPIO 18 (shared) |
| SDA | GPIO 23 (shared) |
| RES | GPIO 17 |
| DC | GPIO 16 |
| CS | GPIO 5 |
| BL | GPIO 25 |

### W5500 Ethernet (SPI)

| W5500 | GPIO ESP32 |
|-------|-----------|
| VCC | 3.3V ⚠️ |
| GND | GND |
| MOSI | GPIO 23 (shared) |
| MISO | GPIO 19 |
| SCK | GPIO 18 (shared) |
| CS | GPIO 15 |
| RST | GPIO 2 |

### Anten ESP-NOW

```
U.FL trên ESP32 ──[dây pigtail IPEX→SMA]──► [Cổng SMA] ──► Anten rời
```

---

## Sơ Đồ Đấu Dây

```
ESP32 DevKit V1 (CP2102)    W5500               ST7789 LCD
       │                   ┌──────┐            ┌──────┐
 3.3V ─┼───────────────────┤VCC   │────────────┤VCC   │
 GND  ─┼───────────────────┤GND   │────────────┤GND   │
       │                   │      │            │      │
 G23  ─┼───────────────────┤MOSI  │────────────┤SDA   │
 G19  ─┼───────────────────┤MISO  │            │      │
 G18  ─┼───────────────────┤SCK   │────────────┤SCL   │
 G15  ─┼───────────────────┤CS    │            │      │
 G2   ─┼───────────────────┤RST   │            │      │
       │                   │      │            │      │
 G5   ─┼───────────────────┼──────┘            ┤CS    │
 G16  ─┼───────────────────┼───────────────────┤DC    │
 G17  ─┼───────────────────┼───────────────────┤RES   │
 G25  ─┼───────────────────┼───────────────────┤BL    │
       │                   │ RJ45──[Ethernet]──►Osee  │
 U.FL──┼──[pigtail]──[SMA]─►[Anten rời]         │      │
       │                                        └──────┘
```

---

## Cấu Hình

### Static IP hoặc DHCP

Sửa `src/main.cpp` dòng 7:
```cpp
const char* STATIC_IP = "192.168.1.100"; // hoặc nullptr cho DHCP
```

### MAC Slave

```cpp
uint8_t slave1Mac[6] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
uint8_t slave2Mac[6] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
uint8_t slave3Mac[6] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
uint8_t slave4Mac[6] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
```

---

## Upload

```bash
cd Master/
pio run -t upload
pio device monitor -b 115200
```

Xem [UPLOAD_GUIDE.md](UPLOAD_GUIDE.md) và [W5500_WIRING.md](W5500_WIRING.md) để biết thêm.

---

## LCD Hiển Thị

```
┌──────────────────┐
│ TALLY MASTER     │
│ ────────────── │
│ ETH: OK (xanh)   │
│ IP: 192.168.1.100│
│ UDP: OK          │
│ NOW: OK          │
│                  │
│ PGM: 1   (đỏ)   │
│ PVW: 2 (xanh lá) │
└──────────────────┘
```

---

## Xử Lý Lỗi

| Vấn đề | Giải pháp |
|--------|----------|
| Port không nhận | Cài driver CP2102 |
| W5500 not found | Kiểm tra 3.3V, CS=GPIO15 |
| LCD không sáng | Kiểm tra BL=GPIO25, nguồn 3.3V |
| ESP-NOW fail | Kiểm tra kết nối pigtail + anten |
| DHCP fail | Dùng static IP |

---

## Kiểm Tra

```bash
# Ping Master
ping 192.168.1.100

# Test UDP
echo '{"id":"pgmTally","value":[1]}' | nc -u 192.168.1.100 19018
```

# W5500 WIRING GUIDE - Master với Ethernet

Hướng dẫn đấu dây W5500 Ethernet module cho Master.

---

## Tổng Quan

Master giờ sử dụng **DUY NHẤT 1 module SPI**:
1. **W5500 Ethernet** - Kết nối mạng từ Osee

Màn hình LCD đã được loại bỏ để tập trung xử lý tín hiệu mạng và nhận xuất dữ liệu ESP-NOW nhanh nhất, tránh bị giật màn hình hoặc làm treo bộ đệm mạng.

---

## Danh Sách Linh Kiện

### Phần Cứng
- ✅ ESP32 DevKit V1 (có U.FL)
- ✅ W5500 Ethernet Module
- ✅ Anten 2.4GHz (IPEX/U.FL)
- ✅ Breadboard
- ✅ Dây nối jumper (đủ màu)
- ✅ Cáp Ethernet (RJ45, 1-3m)
- ✅ Cáp USB Micro-USB

---

## Sơ Đồ Chân Đầy Đủ

### ESP32 DevKit V1

```text
┌─────────────────────────────┐
│         ESP32 DevKit V1     │
│                             │
│ 3.3V ─────────────────┐     │
│ GND ──────────────────┼───┐ │
│                       │   │ │
│ SPI Bus:              │   │ │
│ ├─ GPIO 23 (MOSI) ────┼───┼─┼─→ W5500 MOSI
│ ├─ GPIO 19 (MISO) ────┼───┼─┼─→ W5500 MISO
│ ├─ GPIO 18 (SCK)  ────┼───┼─┼─→ W5500 SCK
│                       │   │ │
│ W5500 Dedicated:      │   │ │
│ ├─ GPIO 15 (CS)  ─────┼───┼─┼─→ W5500 CS
│ └─ GPIO 2  (RST) ─────┼───┼─┼─→ W5500 RST
│                       │   │ │
│ U.FL ◄────────────────┼───┼─┼─ Anten 2.4GHz
│                       │   │ │
└───────────────────────┼───┼─┘
                 Power ─┘   │
                 Ground ────┘
```

---

## Bảng Kết Nối Chi Tiết

### W5500 Ethernet

| W5500 Pin | ESP32 GPIO | Màu Dây (gợi ý) | Ghi Chú |
|-----------|------------|-----------------|---------|
| VCC | 3.3V | Đỏ | ⚠️ 3.3V only, KHÔNG cắm 5V |
| GND | GND | Đen | |
| MOSI | GPIO 23 | Xanh lá | SPI MOSI |
| MISO | GPIO 19 | Hồng | SPI MISO |
| SCK | GPIO 18 | Vàng | SPI Clock |
| CS | GPIO 15 | Tím | Chip Select |
| RST | GPIO 2 | Xám | Reset |
| (INT) | - | - | Optional, không sử dụng |

---

## Hướng Dẫn Đấu Dây Từng Bước

### Bước 1: Chuẩn Bị

1. Đặt ESP32 DevKit lên breadboard
2. Đặt W5500 lên breadboard
3. Chuẩn bị dây nối (cắm lỏng là không nhận module)

### Bước 2: Đấu Nguồn

Kết nối **3.3V và GND**:

```text
ESP32 3.3V ────→ W5500 VCC
ESP32 GND ─────→ W5500 GND
```

⚠️ **QUAN TRỌNG:** W5500 dùng nguồn **3.3V**, TUYỆT ĐỐI KHÔNG cắm vào 5V sẽ làm cháy chip!

### Bước 3: Đấu SPI Bus (Dữ liệu)

Kết nối các dây giao tiếp SPI:

```text
ESP32 GPIO 23 (MOSI) ────→ W5500 MOSI
ESP32 GPIO 19 (MISO) ────→ W5500 MISO
ESP32 GPIO 18 (SCK)  ────→ W5500 SCK
```

### Bước 4: Đấu W5500 Dedicated Pins

Kết nối chân điều khiển module mạng:

```text
ESP32 GPIO 15 (CS)   ────→ W5500 CS
ESP32 GPIO 2  (RST)  ────→ W5500 RST
```

### Bước 5: Kết Nối Cáp Mạng Ethernet

```text
W5500 RJ45 port ◄──[Dây cáp mạng]──→ Osee Duet / Switch Mạng
```

### Bước 6: Gắn Anten (Tùy chọn, để phát sóng ESP-NOW)

```text
ESP32 U.FL connector ◄── Anten 2.4GHz
```

---

## Kiểm Tra Kết Nối

### Checklist

- [ ] **Nguồn**: 3.3V và GND đã nối chắn chắn (không nhầm sang cọc 5V hoặc VIN).
- [ ] **SPI Bus**: Kiểm tra kỹ 3 dây MOSI (23), MISO (19), SCK (18) nối đúng với W5500 chưa, MISO nối bị tuột là không kết nối được luôn.
- [ ] **W5500 CS**: Dây GPIO 15 đã cắm chặt chưa.
- [ ] **Ethernet cable**: Đã cắm vào W5500 và cổng mạng Switch/Router/Osee chưa.

### Test Nhanh

#### Test 1: Nguồn
1. Cắm USB để cấp nguồn cho ESP32.
2. Kiểm tra xem đèn LED báo nguồn (nếu có trên module W5500) có phát sáng màu đỏ không. 

#### Test 2: Ethernet Link
1. Đảm bảo Switch / Osee đang được bật sáng.
2. Khi cắm tiếp dây LAN chuẩn bị RJ45 vào, trên cổng cắm của loại module W5500 full sẽ có 2 đèn báo hiệu mạng (xanh lá cây và cam/vàng). Nếu đèn này **không chớp/sáng** thì dây mạng bị đứt, hoặc đầu ra của Switch không có mạng.

#### Test 3: Upload Code và kiểm tra với Serial Monitor trên máy tính

Mở **PlatformIO**, nạp code (Upload) và mở cổng Serial (115200 baudrate) để check xem mạch đã tìm và kết nối được với chip W5500 chưa.
Thông báo thành công sẽ hiển thị ở Serial Monitor như sau:
```text
[Network] Ethernet connected!
[Network] IP: 192.168.1.100
```
*(Ngược lại, nếu báo đỏ `[Network] W5500 chip not found!` thì bạn hãy rút phích cắm điện, vuốt lại hoặc tuôn cắm lại cọc đồng của các dây MISO, MOSI, SCK, CS).*

---

## Ảnh Tham Khảo (Sơ đồ kết nối Data)

### Setup Hoàn Chỉnh

```text
[Osee Duet] ──Ethernet──→ [W5500] ──SPI──→ [ESP32] 
                                              │
                                              └──ESP-NOW(WiFi+Anten)──→ Truyền sóng cho [Các cục Slaves]
```

---

**Chúc bạn đấu dây phần Master mới thành công! 🎉**

# TALLY V2 - Hướng Dẫn Nâng Cấp

Phiên bản 2 tập trung vào 3 nhóm cải tiến lớn so với V1:

1. ⚙️ **Cấu hình động (NVS + Web Portal)** – không còn phải nạp lại firmware để đổi CAM_ID / MAC / IP / kênh.
2. 🖥️ **Dashboard LCD trên Master** – màn hình ST7789 hiển thị 4 camera + trạng thái mạng.
3. 📡 **ESP-NOW ổn định hơn** – bật Long Range, retry, theo dõi online từng Slave.

---

## 1. Cấu Hình Động – Web Portal

Không cần sửa code rồi nạp lại nữa. Mỗi thiết bị có một **Config Mode** bật bằng nút **BOOT**.

### Vào Config Mode

| Thiết bị | Nút | Cách vào |
|----------|-----|----------|
| **Master** | BOOT (GPIO0) | Giữ nút BOOT **trong lúc cấp nguồn/khởi động** ~1.5s |
| **Slave** | BOOT (GPIO9 trên XIAO) | Giữ nút BOOT khi khởi động ~1.5s |

Khi vào Config Mode:
- Thiết bị phát một WiFi:
  - Master: `TALLY-Master-Setup`
  - Slave: `TALLY-CAMx-Setup`
  - Mật khẩu: `tally1234`
- LCD Master hiện màn hình **SETUP MODE** (SSID + địa chỉ).
- LED Slave **sáng xanh dương** liên tục.

### Cấu hình

1. Dùng điện thoại/laptop kết nối WiFi trên (captive portal sẽ tự mở, hoặc vào `http://192.168.4.1`).
2. **Master**: đặt IP tĩnh/DHCP, gateway, subnet, kênh ESP-NOW, số Slave, Long Range, và **MAC + tên từng Slave**.
3. **Slave**: chọn **Camera ID (1–4)**, tên, kênh, Long Range.
4. Bấm **Lưu** → thiết bị tự khởi động lại và chạy với cấu hình mới.

> Cấu hình lưu trong **NVS (Flash)** nên không mất khi tắt nguồn.

⚠️ **Lưu ý quan trọng:** kênh (`channel`) và **Long Range** của Master và *tất cả* Slave **phải giống nhau** thì mới giao tiếp được. Mặc định: kênh `1`, Long Range `ON`.

---

## 2. Dashboard LCD (Master)

Màn hình ST7789 240×240 được khôi phục và nâng cấp thành dashboard:

```
┌──────────────────────────────┐
│ TALLY V2          ETH OK      │
│                   NOW OK      │
│ IP 192.168.1.100 CH1 LR       │
├──────────────┬───────────────┤
│ CAM 1   ●    │ CAM 2    ●     │  ● xanh = online
│        PGM   │        OFF     │  ● đỏ   = offline
├──────────────┼───────────────┤
│ CAM 3   ●    │ CAM 4    ●     │  ô đỏ  = PGM
│        PVW   │        OFF     │  ô lục = PVW
└──────────────┴───────────────┘
```

- **Chấm online** lấy từ ACK tầng MAC của ESP-NOW (không cần Slave gửi telemetry).
- Vẽ **event-driven** (chỉ khi có thay đổi) + làm tươi tối đa 1s để tránh giật bộ đệm mạng.

**Đấu dây LCD** dùng chung bus SPI với W5500 (khác chân CS):

| LCD | GPIO | | W5500 | GPIO |
|-----|------|-|-------|------|
| SCLK | 18 | (chung) | SCK | 18 |
| MOSI | 23 | (chung) | MOSI | 23 |
| CS | 5 | | CS | 15 |
| DC | 16 | | RST | 2 |
| RST | 17 | | MISO | 19 |
| BL | 4 | | | |

---

## 3. ESP-NOW Ổn Định Hơn

| Cải tiến | Mô tả |
|----------|-------|
| **Long Range (LR)** | `esp_wifi_set_protocol(... \| WIFI_PROTOCOL_LR)` trên cả 2 đầu → độ nhạy tốt hơn, tầm xa & xuyên vật cản tốt hơn nhiều. |
| **Retry khi gửi** | Master gửi lại tối đa 2 lần nếu hàng đợi ESP-NOW báo lỗi. |
| **Send callback** | Theo dõi ACK tầng MAC → biết Slave nào online (hiện trên dashboard). |
| **Khóa kênh + WIFI_PS_NONE** | Giữ từ V1 – tắt tiết kiệm pin WiFi, khóa kênh cố định chống chập chờn. |
| **Công suất phát max** | `WIFI_POWER_19_5dBm`. |
| **Packet có magic + version + seq** | Lọc gói rác, sẵn sàng mở rộng giao thức. |

> Long Range là chuẩn riêng của Espressif → chỉ hoạt động giữa các chip ESP. Vì Master dùng **Ethernet** cho dữ liệu Osee (không phải WiFi router) nên bật LR không ảnh hưởng gì.

### Packet V2 (5 byte)

```cpp
typedef struct {
  uint8_t magic;   // 0xA7
  uint8_t version; // 2
  uint8_t camId;   // 1..4
  uint8_t state;   // 0=OFF, 1=PVW, 2=PGM
  uint8_t seq;     // sequence
} TallyPacket;
```

---

## Quy Trình Triển Khai V2

1. Nạp firmware **Slave** cho từng XIAO (giống nhau, không cần sửa CAM_ID nữa).
2. Với mỗi Slave: giữ BOOT khi bật → vào portal → đặt **Camera ID** → Lưu.
3. Lấy **MAC** của từng Slave (in ra Serial khi chạy, hoặc dán nhãn).
4. Nạp firmware **Master**.
5. Giữ BOOT trên Master khi bật → vào portal → nhập **IP + MAC các Slave** → Lưu.
6. Master khởi động lại, LCD hiện dashboard. Test:

```bash
echo '{"id":"pgmTally","value":[1]}' | nc -u <MASTER_IP> 19018
```

→ CAM 1 đỏ trên dashboard + LED Slave 1 đỏ.

---

## Khôi Phục Cấu Hình Gốc (Factory Reset)

Gọi `configStore.reset()` (hoặc nạp lại firmware sẽ tự ghi mặc định nếu đổi `CFG_VERSION`). Mặc định Master/Slave: kênh 1, Long Range ON.

---

## Ghi Chú Tương Thích

- Thư mục gốc `src/` (bản WiFi + LCD) là **V1 legacy**. Bản chính thức V2 là `Master/` (Ethernet) và `Slave/`.
- Packet V2 **không tương thích** packet V1 (đã thêm magic/version). Hãy nạp V2 cho **cả** Master và Slave.

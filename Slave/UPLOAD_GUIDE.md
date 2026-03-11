# HƯỚNG DẪN UPLOAD CODE LÊN XIAO ESP32C3 - SLAVE

Guide chi tiết để upload code Slave lên board Seeed Studio XIAO ESP32C3.

---

## Chuẩn Bị

### Phần Cứng
- ✅ Seeed Studio XIAO ESP32C3
- ✅ Bảng mạch / hàn sẵn dây RGB LED pin
- ✅ Cáp USB-C loại truyền data (không dùng cáp chỉ sạc)
- ✅ Máy tính (Windows/macOS/Linux)

### Phần Mềm
- VS Code + PlatformIO **HOẶC**
- PlatformIO CLI

---

## ⚠️ LƯU Ý QUAN TRỌNG VỀ XIAO ESP32C3

XIAO ESP32C3 có hệ thống kết nối USB riêng biệt (Native USB CDC):

### 1. Cáp Data
Hãy đảm bảo cáp USB-C của bạn là cáp có thể truyền dữ liệu. Thử cắm điện thoại vào máy tính xem có nhận ổ cứng không. Rất nhiều lỗi tới từ cáp chỉ có chức năng sạc.

### 2. Vào Chế Độ Nạp (Bootloader Mode)
Dòng mạch XIAO rất bé không có nút bấm nhưng có lỗ "BOOT". Trong một vài trường hợp máy không nhận COM port, bạn cần đưa mạch vào chế độ nạp thủ công:
1. Đảm bảo mạch đang nối bằng USB-C vào PC.
2. Giữ nút **BOOT** (Cạnh khe cắm ăng ten, hoặc chấm đồng nhỏ)
3. Rút cáp USB ra và cắm lại trong lúc vẫn giữ BOOT.
4. Nhả nút BOOT ra. Máy sẽ nhận cổng COM mới báo đã bật sẵn sàng nạp.

### 3. Cổng Serial Port
- **Windows:** Báo `USB Serial Device (COM...)`
- **Mac:** Báo `/dev/cu.usbmodem...`

---

## Phương Pháp 1: VS Code + PlatformIO

### Bước 1: Mở Project

1. Mở VS Code
2. File → Open Folder → Chọn `Slave/`
3. Đợi PlatformIO load

### Bước 2: Connect ESP32-C3

1. Cắm ESP32-C3 vào USB-C
2. Đợi 3-5 giây (Windows cài driver)
3. Kiểm tra port:

**Windows:**
```
Mở Device Manager → Ports (COM & LPT)
Tìm: USB Serial Device (COMxx)
```

**macOS:**
```bash
ls /dev/cu.usbmodem*
# Kết quả: /dev/cu.usbmodem14201
```

**Linux:**
```bash
ls /dev/ttyACM*
# Kết quả: /dev/ttyACM0
```

### Bước 3: Cấu Hình CAM_ID

Edit `src/main.cpp` line 7:
```cpp
#define CAM_ID 1  // Đổi 1, 2, 3, hoặc 4 cho mỗi Slave
```

### Bước 4: Upload

1. Click **→** (Upload) ở thanh dưới VS Code
2. Nếu stuck "Connecting...":
   - Giữ nút **BOOT** trên board
   - Click Upload lại
   - Thả BOOT sau khi thấy "Writing at 0x..."

### Bước 5: Lấy MAC Address

1. Click **🔌** (Serial Monitor)
2. Baudrate: **115200**
3. **QUAN TRỌNG:** Nếu không thấy gì, **nhấn nút RESET** trên board
4. Copy MAC address:

```
=== TALLY SLAVE (ESP32-C3) ===
CAM ID: 1
MAC Address: 34:85:18:12:34:56  ← Copy dòng này!
```

---

## Phương Pháp 2: PlatformIO CLI

### Upload

```bash
cd Slave/

# Upload
pio run -t upload

# Nếu cần chỉ định port
pio run -t upload --upload-port /dev/cu.usbmodem14201

# Monitor
pio device monitor -b 115200
```

## Phương Pháp 3: Arduino IDE (Dành cho người quen dùng Arduino)

Nếu bạn không muốn sử dụng VS Code + PlatformIO, bạn có thể nạp code thông qua phần mềm Arduino IDE kinh điển.

### Bước 1: Cài đặt Board ESP32 cho Arduino IDE
1. Tải ứng dụng Arduino IDE.
2. Mở Arduino IDE -> File -> Preferences.
3. Dán đường link này vào phần **"Additional Boards Manager URLs"**:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   *(Nếu đã có link khác trước đó, thêm dấu phẩy `,` ngăn cách)*
4. Mở Tools -> Board -> Boards Manager...
5. Gõ tìm kiếm `esp32` và bấm **Install** cài đặt gói "esp32 by Espressif Systems".

### Bước 2: Setup Môi Trường Nạp
1. Mở thư mục code `Slave/src` và kéo phím thả file `main.cpp` vào ứng dụng Arduino IDE.
2. Lưu file với đuôi `.ino` theo cửa sổ bật lên (Ví dụ `Slave.ino`).
3. Trên thanh công cụ, chọn Tools -> Board -> **ESP32 Arduino** -> Chọn **"XIAO_ESP32C3"**.
4. Chọn đúng cổng COM USB: Tools -> Port -> *(Chọn cổng có thiết bị XIAO)*.
5. **ĐẶC BIẾT QUAN TRỌNG:** Chọn Tools -> **USB CDC On Boot** -> Đổi sang **Enabled**. *(Nếu không bật tính năng này, Serial Monitor sẽ không hiện được địa chỉ MAC lúc khởi động)*

### Bước 3: Nạp Code và Lấy MAC Address
1. Cấu hình dòng `#define CAM_ID 1` ngay đầu file `Slave.ino` nếu cần đặt con Slave này làm số bao nhiêu (từ 1 tới 4).
2. Nhấn nút Mũi tên xanh (Upload) góc trên cùng.
3. Chờ phần mềm báo **Done uploading**.
4. Mở cửa sổ Tools -> **Serial Monitor** trên góc trên bên phải, và cài thông số baudrate ở góc dưới là **115200 baud**.
5. Nhấn nút reset hoặc rút cáp USB cắm lại, Serial Monitor sẽ phải báo như sau:
```
=== TALLY SLAVE (XIAO ESP32C3) ===
CAM ID: 1
[ESP-NOW] Trạm Slave Khởi Động tại MAC: 34:85:18:XX:XX:XX  ← BẠN SẼ COPY DÒNG MAC NÀY VÀO MASTER
```

---

## Xử Lý Lỗi ESP32-C3

### Lỗi: "Port not found"

**macOS:**
```bash
# Kiểm tra port
ls /dev/cu.usb*

# Nếu thấy usbmodem → đúng!
# Nếu thấy usbserial → sai board (không phải C3)
```

**Linux:**
```bash
# Cấp quyền
sudo chmod 666 /dev/ttyACM0

# Hoặc add user vào group
sudo usermod -a -G dialout $USER
# Log out và log in lại
```

**Windows:**
- Vào Device Manager
- Tìm "USB Serial Device"
- Note số COM

### Lỗi: "Connecting..." mãi

ESP32-C3 cần vào **boot mode**:

**Cách 1: Dùng nút BOOT**
1. Rút USB
2. Giữ nút **BOOT**
3. Cắm USB (vẫn giữ BOOT)
4. Click Upload
5. Thả BOOT khi thấy "Writing..."

**Cách 2: platformio.ini**

Thêm vào `platformio.ini`:
```ini
upload_speed = 115200  ; Giảm tốc độ
monitor_rts = 0
monitor_dtr = 0
```

### Lỗi: Serial Monitor trống

ESP32-C3 cần **delay sau Serial.begin()**:

Code đã có:
```cpp
Serial.begin(115200);
delay(1000);  // ← Quan trọng cho C3!
```

Nếu vẫn trống:
1. **Nhấn nút RESET** trên board
2. Hoặc rút cắm lại USB

### Lỗi: "Too many errors"

Build lại từ scratch:
```bash
pio run -t clean
rm -rf .pio/
pio run
```

---

## Kiểm Tra Sau Upload

### 1. Serial Output

```
=== TALLY SLAVE (ESP32-C3) ===
CAM ID: 1
MAC Address: 34:85:18:XX:XX:XX
[ESP-NOW] Ready to receive
Waiting for tally commands...
```

### 2. Test LED

Thêm code test vào `setup()`:
```cpp
// Test LED
digitalWrite(LED_RED, HIGH);
delay(500);
digitalWrite(LED_RED, LOW);
digitalWrite(LED_GREEN, HIGH);
delay(500);
digitalWrite(LED_GREEN, LOW);
```

Upload lại → LED sẽ nhấp nháy Đỏ → Xanh

### 3. Test với Master

Sau khi upload Master + add MAC address:
- Gửi UDP tới Master
- Slave LED sẽ đổi màu

---

## Upload Nhiều Slave

### Cách 1: Upload từng cái

```bash
# Slave 1
# Edit CAM_ID = 1
pio run -t upload
# Copy MAC

# Rút ra, cắm Slave 2
# Edit CAM_ID = 2
pio run -t upload
# Copy MAC
```

### Cách 2: Tạo nhiều env

Edit `platformio.ini`:
```ini
[env:slave1]
board = esp32-c3-devkitm-1
build_flags = -DCAM_ID=1

[env:slave2]
board = esp32-c3-devkitm-1
build_flags = -DCAM_ID=2
```

Upload:
```bash
pio run -e slave1 -t upload
pio run -e slave2 -t upload
```

---

## Lưu MAC Address

Tạo file `MAC_ADDRESSES.txt`:
```
Slave 1 (CAM_ID=1): 34:85:18:12:34:56
Slave 2 (CAM_ID=2): 34:85:18:AB:CD:EF
Slave 3 (CAM_ID=3): 34:85:18:11:22:33
Slave 4 (CAM_ID=4): 34:85:18:44:55:66
```

---

## Workflow Hoàn Chỉnh

### Lần Đầu

1. Đấu dây LED (xem WIRING.md)
2. Cắm ESP32-C3
3. Edit CAM_ID
4. Upload code
5. Copy MAC từ Serial
6. Paste vào Master code

### Upload Slave Tiếp Theo

1. Rút Slave hiện tại
2. Cắm Slave mới
3. Đổi CAM_ID (2, 3, 4...)
4. Upload
5. Copy MAC

---

## Checklist

### Trước Upload
- [ ] ESP32-C3 có dây USB-C tốt
- [ ] Đã edit CAM_ID
- [ ] Đã đấu LED (hoặc để sau)

### Sau Upload
- [ ] Serial Monitor hiện output
- [ ] Copy được MAC address
- [ ] LED test được (nếu đã đấu)
- [ ] Lưu MAC vào file/Master code

---

## Tips

### Auto-reset sau Serial.begin()

```cpp
// Thêm vào setup()
#ifdef ARDUINO_USB_CDC_ON_BOOT
  delay(1000); // Đợi USB CDC sẵn sàng
#endif
```

### Upload không cần giữ BOOT

Thêm vào `platformio.ini`:
```ini
upload_speed = 115200
monitor_dtr = 0
monitor_rts = 0
```

### Xem log chi tiết

```bash
pio run -t upload -v  # Verbose mode
```

---

## Các Lệnh Hữu Ích

```bash
# Chỉ build
pio run

# Upload + monitor liền
pio run -t upload && pio device monitor

# List devices
pio device list

# Clean build
pio run -t clean

# Erase flash
esptool.py --chip esp32c3 erase_flash
```

---

## So Sánh Port

| Board | macOS | Windows | Linux |
|-------|-------|---------|-------|
| ESP32 DevKit (CH340) | /dev/cu.usbserial-* | COM3-9 | /dev/ttyUSB0 |
| ESP32-C3 SuperMini | /dev/cu.usbmodem* | COM10+ | /dev/ttyACM0 |

---

Nếu gặp vấn đề, xem [README.md](README.md) hoặc hỏi trên forum ESP32!

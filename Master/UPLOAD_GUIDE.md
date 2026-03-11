# HƯỚNG DẪN UPLOAD CODE LÊN ESP32 - MASTER

Guide chi tiết để upload code Master lên ESP32 DevKit V1.

---

## Chuẩn Bị

### Phần Cứng
- ✅ ESP32 DevKit V1 đã đấu dây với LCD ST7789
- ✅ Cáp USB Micro-USB
- ✅ Máy tính (Windows/macOS/Linux)

### Phần Mềm

**Lựa chọn 1: VS Code + PlatformIO (Khuyến nghị)**
- Download VS Code: https://code.visualstudio.com
- Cài extension "PlatformIO IDE"

**Lựa chọn 2: PlatformIO CLI**
- Cài Python 3.x
- Run: `pip install platformio`

---

## Phương Pháp 1: VS Code + PlatformIO (Dễ nhất)

### Bước 1: Cài đặt

1. Mở **VS Code**
2. Vào **Extensions** (Ctrl+Shift+X)
3. Tìm **"PlatformIO IDE"**
4. Click **Install**
5. Khởi động lại VS Code

### Bước 2: Mở Project

1. File → Open Folder
2. Chọn thư mục `Master/`
3. Đợi PlatformIO load (thanh dưới xuất hiện)

### Bước 3: Kết Nối ESP32 & Cài Driver (Quan trọng cho Mac)

Mạch Master dùng chip nạp **CP2102**, do đó trên Windows có thể tự nhận, nhưng **trên Mac bắt buộc phải cài Driver thủ công**.

1. **Với người dùng macOS:**
   - Truy cập trang chủ Silicon Labs: [CP210x VCP Mac OSX Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   - Tải về gói cài đặt cho dòng Mac của bạn, mở file đính kèm `.pkg` và tiến hành cài đặt.
   - ⚠️ *Mẹo cài đặt MacOS:* Trong quá trình cài, Mac sẽ chặn phần mềm. Bạn phải mở `System Settings` -> `Privacy & Security` (Quyền riêng tư & Bảo mật) -> Kéo xuống dưới cùng và nhấn **Allow** (Cho phép) phần mềm từ "Silicon Laboratories Inc".
   - Sau khi cho phép, khởi động lại Mac.
2. Cắm cáp USB nối ESP32 Master với máy tính.
3. Kiểm tra Port nhận diện (bấm biểu tượng 🔌 Dấu phích cắm ở viền dưới VS Code):
   - **macOS:** Nếu thành công, bạn sẽ thấy cổng tên là `/dev/cu.usbserial-XXXX` (VD: `0001` hoặc một mã số).
   - **Windows:** Hiện `COM3`, `COM4`,...

### Bước 4: Upload

**Cách 1: Dùng nút (Khuyến nghị)**
1. Click nút **→** (Upload) ở thanh dưới
2. Đợi compile (~1-2 phút lần đầu)
3. Code tự động upload

**Cách 2: Dùng menu**
1. Click biểu tượng **PlatformIO** (con kiến)
2. PROJECT TASKS → esp32dev → General
3. Click **Upload**

### Bước 5: Kiểm Tra

1. Click nút **🔌** (Serial Monitor) ở thanh dưới
2. Chọn baudrate: **115200**
3. Xem output:
```
=== TALLY MASTER (ESP32 DevKit) ===
[Display] Initializing TFT LCD...
[Network] WiFi connected!
```

---

## Phương Pháp 2: PlatformIO CLI

### Bước 1: Cài đặt

```bash
# Kiểm tra Python
python3 --version

# Cài PlatformIO
pip3 install platformio

# hoặc trên macOS
brew install platformio
```

### Bước 2: Navigate đến thư mục

```bash
cd /Users/kimminhtien/Desktop/TEST/TALLY/Master
```

### Bước 3: Upload

```bash
# Build và upload
pio run -t upload

# Nếu có nhiều board, chỉ định port
pio run -t upload --upload-port /dev/cu.usbserial-*
```

### Bước 4: Monitor Serial

```bash
pio device monitor -b 115200
```

Thoát monitor: `Ctrl+C`

---

## Phương Pháp 3: Arduino IDE (Dành cho người quen dùng Arduino trên Windows)

> ⚠️ **Khuyến nghị dùng PlatformIO (VS Code)** để biên dịch vì cấu hình thư viện TFT_eSPI trong Arduino IDE làm thủ công khá rườm rà và dễ sai sót. Nếu bạn bắt buộc phải dùng Arduino IDE **trên Windows**, hãy chắc chắn bám sát hướng dẫn dưới đây.

### Bước 1: Cài đặt Driver và tìm cổng COM (Quan trọng trên Windows)

Khác với Mac, Windows gặp nhiều lỗi khi nhận mạch ESP32 nếu thiếu Driver.
1. Rút mạch ESP32. Nhấn nút Windows, gõ **Device Manager** để mở. 
2. Mở rộng mục **Ports (COM & LPT)**. Ghi nhớ các cổng COM hiện có.
3. Cắm mạch ESP32 qua cáp USB. Xem mục Ports tải lại, nếu có xuất hiện cổng báo **`Silicon Labs CP210x USB to UART Bridge (COM...)`** hoặc **`USB-SERIAL CH340 (COM...)`** thì máy đã nhận. (Ví dụ: `COM4`).
4. Nếu không xuất hiện gì, hoặc báo chấm than vàng, tức là bạn cần tải và cài Driver tuỳ theo chip nạp trên mạch:
   - Chip CH340 (Chữ nhật dài): Tải [Driver CH340](http://www.wch-ic.com/downloads/CH341SER_ZIP.html).
   - Chip CP2102 (Vuông bé tí): Tải [Driver CP210x Windows](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).
5. Phải đảm bảo thấy cổng `COMx` trong Device Manager mới được làm bước tiếp theo.

### Bước 2: Cài đặt Arduino IDE

1. Download: https://www.arduino.cc/en/software
2. Cài board hỗ trợ ESP32:
   - File → Preferences
   - Dán link vào Additional Board URLs: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager
   - Tìm "ESP32" → Install

### Bước 3: Cài Libraries

Tools → Manage Libraries, tìm và cài 3 thư viện sau (chọn chính xác phiên bản/tác giả):
- **TFT_eSPI** by Bodmer
- **ArduinoJson** by Benoit Blanchon
- **Ethernet** by arduino-libraries (bản 2.x)

### Bước 4: Cấu hình bắt buộc thư viện TFT_eSPI

Khác với PlatformIO tự nhận cấu hình, Arduino IDE cần bạn phải sửa file thư viện gốc bằng tay:
1. Mở thư mục chứa thư viện Arduino trên Windows của bạn: 
   *(`C:\Users\<Tên_Bạn>\Documents\Arduino\libraries\TFT_eSPI`)*
2. Mở file `User_Setup.h` bằng Notepad hoặc Text Editor.
3. Chỉnh sửa bỏ dấu comment (`//`) và gán các giá trị dưới đây, comment tuỳ chỉnh lại tất cả các thông số màn hình khác:

```cpp
#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// Khai báo chân SPI cho ESP32 Master
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  17
#define TFT_BL   25    // Lưu ý: Đèn nền là chân 25 (không dùng GPIO 4)
#define SPI_FREQUENCY  40000000
```
4. Lưu (`Ctrl+S`) và đóng file `User_Setup.h`.

### Bước 5: Upload Code (Cách khắc phục lỗi Connecting trên Windows)

1. Do Master được chia làm nhiều luồng `.h` và `.cpp`, hãy mở thư mục `Master/src` và kéo thả file `main.cpp` vào giao diện Arduino IDE chạy rỗng.
2. Lưu file này lại thành một Project Arduino có đuôi `.ino` (VD: `Master.ino`). Chép đè nội dung `main.cpp` cũ vào `Master.ino`, giữ nguyên 2 file `Display.h` và `Network.h` trong cùng thư mục gốc của nó.
3. Cắm mạch ESP32 **TRƠ TRỌI**, RÚT hết dây cắm sang màn hình/modul mạng.
4. Chọn đúng phần cứng: Tools → Board → Cột "ESP32 Arduino" → Chọn **"DOIT ESP32 DEVKIT V1"**.
5. Chọn cổng giao tiếp: Tools → Port → **(Chọn cổng COM4, COM5... đã xác định ở Bước 1)**.
6. Bấm icon Mũi tên (Upload) nằm ở trên cùng.
7. Khi mạch biên dịch xong hiện chữ **"Connecting........___"** ở cửa sổ console đen xì bên dưới: LẬP TỨC lấy tay **ĐÈ LÌ NÚT `BOOT`** trên thân ESP32 cho đến khi xuất hiện phần trăm Upload.
8. Upload xong, cắm lại các dây, Mở Tools → Serial Monitor → đổi thành **115200 baud** để xem kết quả.



---

## Xử Lý Lỗi

### Lỗi: "Port not found"

**macOS/Linux:**
```bash
# Kiểm tra port
ls /dev/cu.* # macOS
ls /dev/ttyUSB* # Linux

# Cấp quyền (Linux)
sudo chmod 666 /dev/ttyUSB0
sudo usermod -a -G dialout $USER
```

**Windows:**
- Cài CH340/CP2102 driver từ website nhà sản xuất
- Kiểm tra Device Manager

### Lỗi: "Connecting..."

ESP32 không vào download mode:

**Cách 1: Nút Boot (Khuyến nghị)**
1. Giữ nút **BOOT**
2. Nhấn nút **EN** (reset)
3. Thả nút **EN**
4. Đợi 1s, thả nút **BOOT**
5. Click Upload lại

**Cách 2: Unplug/Replug**
1. Rút USB
2. Giữ nút BOOT
3. Cắm lại USB
4. Thả BOOT sau 2s
5. Upload

### Lỗi: "Compilation failed"

```bash
# Xóa cache và rebuild
pio run -t clean
pio run
```

### Lỗi: "Library not found"

```bash
# Cài thủ công
pio lib install "bodmer/TFT_eSPI@^2.5.23"
pio lib install "bblanchon/ArduinoJson@^6.21.3"
```

### LCD không sáng sau upload

1. Kiểm tra backlight: GPIO 25
2. Test bằng code đơn giản:
```cpp
pinMode(25, OUTPUT);
digitalWrite(25, HIGH); // Bật backlight
```

### Serial Monitor không có output

1. Kiểm tra baudrate: **115200**
2. Thử reset ESP32 (nút EN)
3. macOS: Đổi từ /dev/cu.* sang /dev/tty.*

---

## Các Lệnh Hữu Ích

```bash
# Chỉ build (không upload)
pio run

# Upload qua OTA (sau khi đã config)
pio run -t upload --upload-port 192.168.1.100

# Xem thông tin board
pio device list

# Update PlatformIO
pio upgrade

# Xóa hết build files
pio run -t clean
rm -rf .pio/
```

---

## Tips

### Upload nhanh hơn

Edit `platformio.ini`:
```ini
upload_speed = 921600  # Tăng từ 460800
```

### Tự động mở Serial Monitor sau upload

```bash
pio run -t upload && pio device monitor
```

### Dùng nhiều ESP32

Tạo alias trong `platformio.ini`:
```ini
[env:master1]
; ...
upload_port = /dev/cu.usbserial-0001

[env:master2]
; ...
upload_port = /dev/cu.usbserial-0002
```

Upload cụ thể:
```bash
pio run -e master1 -t upload
```

---

## Workflow Upload Hoàn Chỉnh

### Lần Đầu (Full Setup)

```bash
# 1. Mở VS Code tại thư mục Master
code .

# 2. Cắm ESP32

# 3. Click Upload button (→)

# 4. Click Serial Monitor (🔌)
```

### Những Lần Sau (Quick)

```bash
cd Master/
pio run -t upload && pio device monitor
```

Hoặc chỉ cần:
- Click **→** trong VS Code
- Click **🔌** để xem output

---

## Checklist Trước Upload

- [ ] ESP32 đã đấu dây đúng với LCD
- [ ] USB cable kết nối tốt
- [ ] Driver đã cài (Windows)
- [ ] Đã config WiFi SSID/Password
- [ ] Đã có MAC address của Slave
- [ ] PlatformIO/Arduino IDE đã sẵn sàng

---

## Sau Khi Upload Thành Công

### Kiểm tra LCD

LCD sẽ hiển thị:
```
TALLY MASTER
────────────
ETH: OK (xanh)
IP: 192.168.1.100
UDP: OK
NOW: OK

PGM: 0
PVW: 0
```

### Kiểm tra Serial Monitor

```
=== TALLY MASTER (ESP32 DevKit + Ethernet) ===
[Display] Initializing TFT LCD...
[Network] Initializing Ethernet...
[Network] Ethernet connected!
[Network] IP: 192.168.1.100      ← Lưu IP này để gán trên Osee!
[Network] Gateway: 192.168.1.1
[Network] UDP listening on port 19018
[Network] Initializing ESP-NOW...
[Network] ESP-NOW initialized
[Network] Added slave 1: 0C:4E:A0:XX:XX:XX
[Main] Setup complete!
```

### Test Gửi Lệnh Ảo Qua UDP (Tuỳ chọn)

```bash
# Từ máy tính dùng chung mạng nội bộ
echo '{"id":"pgmTally","value":[1]}' | nc -u 192.168.1.100 19018
```

LCD sẽ cập nhật dòng **PGM: 1** màu đỏ. Đồng thời hộp Slave 1 lập tức chuyển sáng sang màu đỏ.

---

Nếu gặp vấn đề, xem [README.md](README.md) phần Troubleshooting!

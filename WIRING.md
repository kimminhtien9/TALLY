# WIRING GUIDE - Tally System

Complete wiring instructions for Master and Slave devices.

## Master: ESP32 DevKit V1 + ST7789 LCD

### Required Components
- ESP32 DevKit V1 (with U.FL antenna connector)
- ST7789 1.3" TFT LCD 240x240 (SPI)
- 2.4GHz WiFi antenna with IPEX/U.FL connector (optional)
- Breadboard and jumper wires
- USB cable for power/programming

### Pin Connections

| ST7789 Pin | ESP32 GPIO | Description |
|------------|-----------|-------------|
| VCC | 3.3V | Power (3.3V only, NOT 5V!) |
| GND | GND | Ground |
| SCL | GPIO 18 | SPI Clock |
| SDA | GPIO 23 | SPI Data (MOSI) |
| RES | GPIO 17 | Reset |
| DC | GPIO 16 | Data/Command |
| CS | GPIO 5 | Chip Select |
| BL | GPIO 4 | Backlight (PWM capable) |

### Wiring Diagram

```
ESP32 DevKit V1                    ST7789 LCD
                                  ┌──────────┐
3.3V ─────────────────────────────┤ VCC      │
GND ──────────────────────────────┤ GND      │
                                  │          │
GPIO 23 (MOSI) ───────────────────┤ SDA      │
GPIO 18 (SCK) ────────────────────┤ SCL      │
GPIO 5  ──────────────────────────┤ CS       │
GPIO 16 ──────────────────────────┤ DC       │
GPIO 17 ──────────────────────────┤ RES      │
GPIO 4  ──────────────────────────┤ BL       │
                                  └──────────┘
```

### Antenna Connection (Optional)
- Attach 2.4GHz antenna to U.FL connector on ESP32
- Improves WiFi range from ~10m to 50m+

---

## Slave: ESP32-C3 SuperMini + RGB LED

### Required Components
- ESP32-C3 SuperMini board
- RGB LED (Common Cathode type recommended)
- 3x 220Ω resistors (for LED current limiting)
- Breadboard and jumper wires
- USB-C cable for power/programming

### Pin Connections

| Component | ESP32-C3 GPIO | Notes |
|-----------|----------------|-------|
| LED Red Anode | GPIO 8 | Via 220Ω resistor |
| LED Green Anode | GPIO 9 | Via 220Ω resistor |
| LED Blue Anode | GPIO 10 | Via 220Ω resistor |
| LED Cathode | GND | Common cathode |

### Wiring Diagram

```
ESP32-C3 SuperMini           RGB LED (Common Cathode)
                            
GPIO 8  ────[220Ω]──────────  Red Anode
GPIO 9  ────[220Ω]──────────  Green Anode
GPIO 10 ────[220Ω]──────────  Blue Anode
GND  ────────────────────────  Cathode (-)
```

### LED Type Selection

**Recommended: Common Cathode**
- Code is configured for this type
- Cathode to GND, Anodes to GPIO via resistors
- HIGH = LED ON

**If using Common Anode:**
- Modify `setLED()` function in Slave code
- Anode to 3.3V, Cathodes to GPIO via resistors  
- LOW = LED ON (inverted logic)

---

## Complete System Setup

### Step 1: Assemble Hardware

1. **Master:**
   - Wire ST7789 LCD to ESP32 DevKit per diagram above
   - Attach external antenna (optional)
   - Connect via USB

2. **Slave(s):**
   - Connect RGB LED to ESP32-C3 per diagram above
   - Connect via USB-C

### Step 2: Configure Software

1. **Set CAM_ID on each Slave:**
   - Edit `Slave/src/main.cpp` line 7
   - Slave 1: `#define CAM_ID 1`
   - Slave 2: `#define CAM_ID 2`, etc.

2. **Upload Slave code:**
   ```bash
   cd Slave/
   pio run -t upload
   pio device monitor -b 115200
   ```

3. **Copy Slave MAC addresses from Serial:**
   ```
   MAC Address: 34:85:18:XX:XX:XX
   ```

4. **Update Master with Slave MACs:**
   - Edit `/src/main.cpp` lines 10-13
   - Paste MAC addresses

5. **Configure WiFi:**
   - Edit `/src/main.cpp` lines 6-7
   - Set your WiFi SSID and password

6. **Upload Master code:**
   ```bash
   cd /path/to/TALLY/
   pio run -t upload
   pio device monitor -b 115200
   ```

### Step 3: Test System

1. Power on all devices
2. Master LCD should show:
   - WiFi: OK (green)
   - UDP: OK (green)
   - NOW: OK (green)

3. Test UDP commands:
   ```bash
   # From computer on same network
   echo '{"id":"pgmTally","value":[1]}' | nc -u <MASTER_IP> 19018
   ```

4. Verify:
   - Master LCD shows "PGM: 1" in red
   - Slave 1 LED turns red
   - Other slaves remain off

---

## Power Considerations

### Master (ESP32 DevKit)
- **USB Power:** 5V via USB (easiest)
- **External:** 3.3V regulated or 5V to VIN pin
- **Current:** ~200-300mA with LCD

### Slave (ESP32-C3)
- **USB Power:** 5V via USB-C
- **External:** 3.3V regulated or 5V to 5V pin
- **Current:** ~100mA (minimal with just LED)

### Battery Operation (Future)
- Use 3.7V LiPo with protection circuit
- Add voltage regulator if needed (3.3V)
- Typical capacity: 1000-2000mAh for hours of runtime

---

## Troubleshooting Wiring

### Master LCD Issues

**Blank screen:**
- Verify 3.3V connection (NOT 5V!)
- Check all pin connections
- Try `tft.setRotation(0-3)` in Display.h

**Flickering:**
- Add decoupling capacitor (100nF) near VCC/GND on LCD
- Use shorter wires (< 15cm)

**Wrong colors:**
- Likely wrong driver - verify ST7789
- Check SPI pin definitions

### Slave LED Issues

**LED not lighting:**
- Test LED with multimeter or simple sketch
- Check resistor values (220Ω)
- Verify Common Cathode type

**Wrong colors:**
- Check wire connections to correct GPIO
- Verify LED pinout

**Dim LED:**
- Try lower resistor (150Ω) or remove if LED has built-in resistor
- Check power supply

---

## Safety Notes

⚠️ **Important:**
- ST7789 requires 3.3V power - **NOT 5V!**
- Use resistors with LEDs to prevent burnout
- Don't exceed GPIO current limits (40mA max per pin)
- USB cables provide power during programming - disconnect one to avoid conflicts

## Shopping List (Vietnam)

### Master
- ESP32 DevKit V1 (có U.FL): ~70-90k
- LCD ST7789 1.3" 240x240: ~40-50k
- Anten 2.4GHz IPEX: ~10-20k
- **Total: ~150k**

### Slave (mỗi cái)
- ESP32-C3 SuperMini: ~30-50k  
- RGB LED Common Cathode: ~2-5k
- Điện trở 220Ω x3: ~1k
- **Total per slave: ~35-60k**

### Complete System (1 Master + 4 Slaves)
- **Total: ~350-450k VND**

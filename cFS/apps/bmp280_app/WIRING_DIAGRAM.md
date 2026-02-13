# BMP280 Sensor Wiring Guide for STM32MP257F-EV1

## Hardware Overview

### STM32MP257F-EV1 Board
- **I2C Interfaces**: I2C1, I2C2, I2C3, I2C4, I2C5
- **Primary I2C**: I2C1 on connector CN4
- **Operating Voltage**: 3.3V logic level

### BMP280 Sensor Module
- **Interface**: I2C
- **Supply Voltage**: 1.71V to 3.6V (3.3V recommended)
- **I2C Addresses**: 0x76 (SDO to GND) or 0x77 (SDO to VCC)
- **Logic Level**: 3.3V compatible
- **Pull-up resistors**: Usually included on breakout boards

## Pin Connections

### Option 1: Using CN4 Connector (Recommended)

```
BMP280 Module          STM32MP257F-EV1 (CN4)
┌─────────────┐        ┌──────────────────────┐
│             │        │                      │
│  VCC (3.3V) │──────► │ Pin 1 or 17 (3.3V)  │
│             │        │                      │
│  GND        │──────► │ Pin 6, 9, 14, 20,   │
│             │        │ 25, 30, 34, 39 (GND) │
│             │        │                      │
│  SCL        │◄─────► │ Pin 5 (I2C1_SCL)    │
│             │        │                      │
│  SDA        │◄─────► │ Pin 3 (I2C1_SDA)    │
│             │        │                      │
│  (SDO)      │─┐      │                      │
│  (CSB)      │ │      │ (Optional - see     │
└─────────────┘ │      │  below)             │
                │      └──────────────────────┘
                │
                └─► Connect to GND or VCC for
                    address selection (see below)
```

### CN4 Pinout Reference

```
Pin 1  - 3.3V Power
Pin 2  - 5V Power (DO NOT USE for BMP280)
Pin 3  - I2C1_SDA (GPIO_PZ6)
Pin 4  - 5V Power (DO NOT USE for BMP280)
Pin 5  - I2C1_SCL (GPIO_PZ7)
Pin 6  - Ground
Pin 7  - GPIO
Pin 8  - UART TX
Pin 9  - Ground
Pin 10 - UART RX
...
```

### Minimal Connection (4 wires)

For most BMP280 breakout boards with built-in pull-ups:

```
BMP280 ──► STM32MP257F-EV1 CN4
────────────────────────────────
VCC    ──► Pin 1 (3.3V)
GND    ──► Pin 6 (GND)
SCL    ──► Pin 5 (I2C1_SCL)
SDA    ──► Pin 3 (I2C1_SDA)
```

## I2C Address Configuration

The BMP280 supports two I2C addresses:

### Address 0x76 (Default in our code)
```
BMP280 SDO pin ──► Connect to GND
```

### Address 0x77 (Alternative)
```
BMP280 SDO pin ──► Connect to VCC (3.3V)
```

**Note:** Most BMP280 breakout boards have SDO pulled down to GND internally, giving address 0x76. If your module uses 0x77, update the code:

```c
// In bmp280_app.h, change:
#define BMP280_I2C_ADDR  0x77
```

## Physical Wiring Examples

### Example 1: Standard BMP280 Breakout Board

Most common BMP280 modules have 6 pins:

```
┌─────────────────────┐
│   BMP280 Module     │
│                     │
│  VCC  GND  SCL  SDA │  ◄── Connect these 4
│                     │
│  SDO  CSB           │  ◄── Optional
└─────────────────────┘
```

**Connections:**
1. VCC → CN4 Pin 1 (3.3V)
2. GND → CN4 Pin 6 (Ground)
3. SCL → CN4 Pin 5 (I2C1_SCL)
4. SDA → CN4 Pin 3 (I2C1_SDA)
5. SDO → Leave floating or connect to GND (for 0x76)
6. CSB → Leave floating or connect to VCC (selects I2C mode)

### Example 2: Using Jumper Wires

**Color Convention (common):**
- Red wire: VCC (3.3V)
- Black wire: GND
- Yellow wire: SCL
- Blue/Green wire: SDA

```
Red    (VCC) ─── CN4 Pin 1
Black  (GND) ─── CN4 Pin 6
Yellow (SCL) ─── CN4 Pin 5
Blue   (SDA) ─── CN4 Pin 3
```

## Pull-up Resistors

I2C requires pull-up resistors on both SDA and SCL lines.

### Check if you need external pull-ups:

Most BMP280 breakout boards include 10kΩ pull-up resistors. To verify:

```bash
# Look at the module - check for resistors labeled "103" (10kΩ)
# between VCC and SCL/SDA pins
```

### If your module lacks pull-ups:

Add 4.7kΩ or 10kΩ resistors:

```
        3.3V
         │
         ├─ 10kΩ resistor ─ SCL ─┐
         │                        │
         └─ 10kΩ resistor ─ SDA ─┤
                                  │
                              (to board)
```

## Verifying Connections

### Step 1: Power Check

```bash
# Measure voltage between VCC and GND pins on BMP280
# Should be 3.3V ±0.3V
```

### Step 2: I2C Bus Scan

```bash
# Install i2c-tools if needed
apt install i2c-tools

# Scan I2C bus 1
i2cdetect -y 1
```

**Expected output with BMP280 at 0x76:**
```
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- 76 --                         
```

### Step 3: Read Chip ID

```bash
# Read the BMP280 chip ID register (should return 0x58)
i2cget -y 1 0x76 0xD0
```

**Expected output:**
```
0x58
```

## Troubleshooting Hardware Issues

### Problem: No device detected (all "--" in i2cdetect)

**Possible Causes & Solutions:**

1. **Loose connections**
   ```
   Solution: Check all wire connections, ensure they're firmly seated
   ```

2. **Wrong I2C bus**
   ```bash
   # Try other buses
   i2cdetect -y 0
   i2cdetect -y 2
   ```

3. **Module not powered**
   ```
   Solution: Verify 3.3V present on VCC pin with multimeter
   ```

4. **I2C not enabled**
   ```bash
   # Check if I2C device exists
   ls /dev/i2c*
   
   # If missing, enable in device tree (requires rebuild)
   ```

### Problem: Device detected but chip ID is wrong

**Possible Causes:**

1. **Wrong sensor module** (BMP180, BME280 instead of BMP280)
   ```
   BMP180: Chip ID = 0x55
   BMP280: Chip ID = 0x58
   BME280: Chip ID = 0x60
   ```

2. **Communication errors**
   ```bash
   # Try multiple reads
   i2cget -y 1 0x76 0xD0
   i2cget -y 1 0x76 0xD0
   i2cget -y 1 0x76 0xD0
   ```

### Problem: Device at wrong address

**If device appears at 0x77 instead of 0x76:**

Update the code:
```c
// Edit bmp280_app.h
#define BMP280_I2C_ADDR  0x77
```

Or reconnect SDO pin to GND to use 0x76.

### Problem: Intermittent readings

**Possible Causes:**

1. **Poor connections** → Reseat all wires
2. **Insufficient pull-ups** → Add 4.7kΩ pull-up resistors
3. **Electrical noise** → Use shorter wires, add 100nF capacitor between VCC and GND
4. **Power supply issues** → Ensure clean 3.3V power

### Problem: I2C bus errors in cFS

**If you see errors like "I2C bus read failed":**

1. Check wiring (most common)
2. Verify pull-up resistors present
3. Check for multiple devices on same bus (address conflict)
4. Reduce I2C speed (if using custom kernel/device tree)

## Best Practices

### ✅ DO:
- Use 3.3V power supply only
- Keep wires short (<20cm is best)
- Use quality jumper wires with good contact
- Verify connections with multimeter before powering on
- Check i2cdetect before running cFS application
- Use twisted pair or shielded cable for long runs

### ❌ DON'T:
- Use 5V power (will damage sensor)
- Hot-plug sensor while cFS is running
- Use very long wires (>1m) without proper termination
- Forget pull-up resistors (if not on module)
- Mix up SDA and SCL (though I2C usually survives this)

## Advanced: Using Different I2C Bus

To use I2C2, I2C3, etc.:

### Hardware:
1. Find the alternate I2C pins in STM32MP257 datasheet
2. Connect BMP280 to those pins

### Software:
```c
// Edit bmp280_app.h
#define BMP280_I2C_BUS  "/dev/i2c-2"  // For I2C2
```

Then rebuild the application.

## Connection Checklist

Before running the application:

- [ ] VCC connected to 3.3V (NOT 5V!)
- [ ] GND connected to ground
- [ ] SCL connected to I2C1_SCL (CN4 Pin 5)
- [ ] SDA connected to I2C1_SDA (CN4 Pin 3)
- [ ] Sensor appears in `i2cdetect -y 1` at 0x76 or 0x77
- [ ] Chip ID reads as 0x58: `i2cget -y 1 0x76 0xD0`
- [ ] No other devices conflicting on same I2C address
- [ ] All connections secure and not loose

## Safety Notes

⚠️ **IMPORTANT:**
- BMP280 maximum voltage: 3.6V
- Do NOT use 5V - it will damage the sensor!
- Always connect GND first, then VCC
- Always disconnect VCC before removing wires

## Testing the Hardware Connection

Quick hardware test before running cFS:

```bash
#!/bin/bash
# bmp280_test.sh - Quick hardware verification

echo "BMP280 Hardware Test"
echo "===================="

echo "Checking I2C bus..."
if [ ! -e /dev/i2c-1 ]; then
    echo "ERROR: /dev/i2c-1 not found!"
    exit 1
fi

echo "Scanning for devices..."
i2cdetect -y 1

echo ""
echo "Reading Chip ID..."
CHIP_ID=$(i2cget -y 1 0x76 0xD0 2>/dev/null || i2cget -y 1 0x77 0xD0 2>/dev/null)
if [ "$CHIP_ID" = "0x58" ]; then
    echo "SUCCESS: BMP280 found with correct Chip ID"
else
    echo "ERROR: Chip ID is $CHIP_ID (expected 0x58)"
    exit 1
fi

echo ""
echo "Reading temperature..."
i2cget -y 1 0x76 0xFA 2>/dev/null || i2cget -y 1 0x77 0xFA 2>/dev/null

echo ""
echo "Hardware test complete!"
```

Save this as `bmp280_test.sh`, make it executable (`chmod +x bmp280_test.sh`), and run it before starting cFS.

## Additional Resources

- STM32MP257 Datasheet: https://www.st.com/en/microcontrollers-microprocessors/stm32mp257.html
- BMP280 Datasheet: https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/
- I2C Protocol Specification: https://www.nxp.com/docs/en/user-guide/UM10204.pdf

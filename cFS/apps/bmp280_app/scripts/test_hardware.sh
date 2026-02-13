#!/bin/bash
#
# BMP280 Hardware Test Script
# Tests I2C communication with BMP280 sensor before running cFS
#

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "======================================"
echo "  BMP280 Hardware Test for STM32MP257"
echo "======================================"
echo ""

# Test 1: Check I2C device exists
echo -n "Test 1: Checking I2C bus... "
if [ -e /dev/i2c-1 ]; then
    echo -e "${GREEN}PASS${NC}"
else
    echo -e "${RED}FAIL${NC}"
    echo "ERROR: /dev/i2c-1 not found!"
    echo "Available I2C buses:"
    ls /dev/i2c* 2>/dev/null || echo "None found"
    exit 1
fi

# Test 2: Scan for devices
echo -n "Test 2: Scanning I2C bus... "
SCAN_OUTPUT=$(i2cdetect -y 1 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    echo "Devices found:"
    echo "$SCAN_OUTPUT" | grep -v "^     " | grep -v "^$"
else
    echo -e "${RED}FAIL${NC}"
    echo "ERROR: Cannot scan I2C bus"
    exit 1
fi

# Test 3: Check for BMP280 at 0x76
echo -n "Test 3: Looking for BMP280 at 0x76... "
if i2cdetect -y 1 | grep -q " 76 "; then
    echo -e "${GREEN}FOUND${NC}"
    BMP280_ADDR="0x76"
elif i2cdetect -y 1 | grep -q " 77 "; then
    echo -e "${YELLOW}FOUND at 0x77${NC}"
    BMP280_ADDR="0x77"
    echo "NOTE: Update BMP280_I2C_ADDR in bmp280_app.h to 0x77"
else
    echo -e "${RED}NOT FOUND${NC}"
    echo "ERROR: BMP280 not detected at 0x76 or 0x77"
    echo "Check wiring and power supply"
    exit 1
fi

# Test 4: Read Chip ID
echo -n "Test 4: Reading Chip ID... "
CHIP_ID=$(i2cget -y 1 $BMP280_ADDR 0xD0 2>/dev/null)
if [ "$CHIP_ID" = "0x58" ]; then
    echo -e "${GREEN}PASS${NC} (ID: $CHIP_ID)"
elif [ "$CHIP_ID" = "0x60" ]; then
    echo -e "${YELLOW}WARN${NC} (ID: $CHIP_ID - This is BME280, not BMP280)"
    echo "BME280 is compatible but has additional humidity sensor"
elif [ "$CHIP_ID" = "0x55" ]; then
    echo -e "${RED}FAIL${NC} (ID: $CHIP_ID - This is BMP180, not BMP280)"
    echo "BMP180 uses different protocol - not compatible"
    exit 1
else
    echo -e "${RED}FAIL${NC} (ID: $CHIP_ID - Unknown chip)"
    exit 1
fi

# Test 5: Read temperature data
echo -n "Test 5: Reading temperature registers... "
TEMP_MSB=$(i2cget -y 1 $BMP280_ADDR 0xFA 2>/dev/null)
TEMP_LSB=$(i2cget -y 1 $BMP280_ADDR 0xFB 2>/dev/null)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    echo "  Raw temp MSB: $TEMP_MSB"
    echo "  Raw temp LSB: $TEMP_LSB"
else
    echo -e "${RED}FAIL${NC}"
    echo "ERROR: Cannot read temperature registers"
    exit 1
fi

# Test 6: Read pressure data
echo -n "Test 6: Reading pressure registers... "
PRESS_MSB=$(i2cget -y 1 $BMP280_ADDR 0xF7 2>/dev/null)
PRESS_LSB=$(i2cget -y 1 $BMP280_ADDR 0xF8 2>/dev/null)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    echo "  Raw press MSB: $PRESS_MSB"
    echo "  Raw press LSB: $PRESS_LSB"
else
    echo -e "${RED}FAIL${NC}"
    echo "ERROR: Cannot read pressure registers"
    exit 1
fi

# Test 7: Read calibration data
echo -n "Test 7: Reading calibration data... "
DIG_T1_LSB=$(i2cget -y 1 $BMP280_ADDR 0x88 2>/dev/null)
DIG_T1_MSB=$(i2cget -y 1 $BMP280_ADDR 0x89 2>/dev/null)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    echo "  dig_T1 LSB: $DIG_T1_LSB"
    echo "  dig_T1 MSB: $DIG_T1_MSB"
else
    echo -e "${RED}FAIL${NC}"
    echo "ERROR: Cannot read calibration registers"
    exit 1
fi

echo ""
echo "======================================"
echo -e "${GREEN}ALL TESTS PASSED!${NC}"
echo "======================================"
echo ""
echo "BMP280 is ready for cFS application."
echo ""
echo "Configuration detected:"
echo "  I2C Bus:     /dev/i2c-1"
echo "  I2C Address: $BMP280_ADDR"
echo "  Chip ID:     $CHIP_ID"
echo ""

if [ "$BMP280_ADDR" = "0x77" ]; then
    echo -e "${YELLOW}ACTION REQUIRED:${NC}"
    echo "Update bmp280_app.h before building:"
    echo "  #define BMP280_I2C_ADDR  0x77"
    echo ""
fi

echo "Next steps:"
echo "  1. Copy bmp280_app to cFS/apps/"
echo "  2. Add to targets.cmake"
echo "  3. Build cFS"
echo "  4. Update startup script"
echo "  5. Run cFS"
echo ""

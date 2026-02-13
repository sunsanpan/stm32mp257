# BMP280 App Quick Start Guide

## 5-Minute Setup

### Prerequisites Check
```bash
# 1. Verify hardware
i2cdetect -y 1  # Should show device at 0x76

# 2. Check cFS exists
ls /home/root/fsw/cFS/

# 3. Verify compiler
which aarch64-ostl-linux-gcc
```

### Installation
```bash
# Copy app
cd /home/root/fsw/cFS/apps/
cp -r /path/to/bmp280_app ./

# Add to build (edit targets.cmake)
nano ../sample_defs/targets.cmake
# Add: bmp280_app to list

# Build
cd ..
export CC=aarch64-ostl-linux-gcc
export CXX=aarch64-ostl-linux-g++
rm -rf build
make prep && make && make install

# Update startup script
nano build/exe/cpu1/cf/cfe_es_startup.scr
# Add line:
# CFE_APP, bmp280_app, /cf/bmp280_app.so, BMP280_APP_Main, BMP280_APP, 50, 8192, 0x0, 0;
```

### Run
```bash
cd build/exe/cpu1/
./core-cpu1
```

Look for:
```
BMP280 APP Initialized. Version 1.0.0.0
BMP280: Sensor initialized successfully (Chip ID: 0x58)
```

## Common Issues

### "Failed to open I2C bus"
```bash
sudo chmod 666 /dev/i2c-1
```

### "Invalid chip ID"
```bash
# Check address
i2cdetect -y 1

# If at 0x77, edit bmp280_app.h:
# #define BMP280_I2C_ADDR 0x77
```

### App doesn't load
```bash
# Check shared library exists
ls -la build/exe/cpu1/cf/bmp280_app.so

# Verify in startup script
grep bmp280 build/exe/cpu1/cf/cfe_es_startup.scr
```

## Next Steps

1. Send read commands via ground system
2. View telemetry with TO_LAB
3. Integrate with SCH_LAB for periodic readings

See README.md for detailed usage instructions.

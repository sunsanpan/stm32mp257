# BMP280 App Installation Guide for STM32MP257F-EV1

## Quick Start

```bash
# 1. Copy app to cFS
cd /home/root/fsw/cFS/apps/
cp -r /path/to/bmp280_app ./

# 2. Add to targets.cmake
nano ../sample_defs/targets.cmake
# Add: bmp280_app to MISSION_GLOBAL_APPLIST

# 3. Build
cd ..
export CC=aarch64-ostl-linux-gcc
export CXX=aarch64-ostl-linux-g++
rm -rf build
make prep && make && make install

# 4. Update startup script
nano build/exe/cpu1/cf/cfe_es_startup.scr
# Add BMP280 app entry

# 5. Run
cd build/exe/cpu1/
./core-cpu1
```

## Detailed Installation Steps

### Step 1: Verify Prerequisites

Before installing the BMP280 app, ensure:

```bash
# Check cFS is installed
ls -la /home/root/fsw/cFS/

# Check compiler
which aarch64-ostl-linux-gcc
aarch64-ostl-linux-gcc --version

# Check I2C device
ls -la /dev/i2c*

# Install i2c-tools if not present
apt install i2c-tools

# Check for BMP280 sensor
i2cdetect -y 1
```

The `i2cdetect` command should show the sensor at address 0x76 or 0x77:
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

### Step 2: Copy Application Files

```bash
# Navigate to cFS apps directory
cd /home/root/fsw/cFS/apps/

# If transferring from another machine, use scp:
# scp -r bmp280_app/ root@<board-ip>:/home/root/fsw/cFS/apps/

# Or if using USB/SD card:
# cp -r /media/usb/bmp280_app ./

# Verify directory structure
ls -la bmp280_app/
ls -la bmp280_app/fsw/src/
```

You should see:
```
bmp280_app/
├── CMakeLists.txt
├── README.md
├── INSTALL.md
└── fsw/
    └── src/
        ├── bmp280_app.c
        ├── bmp280_app.h
        ├── bmp280_app_events.h
        ├── bmp280_app_msg.h
        ├── bmp280_app_msgids.h
        ├── bmp280_app_perfids.h
        └── bmp280_app_version.h
```

### Step 3: Configure cFS to Build the App

Edit the targets configuration file:
```bash
cd /home/root/fsw/cFS/
nano sample_defs/targets.cmake
```

Find the section with `list(APPEND MISSION_GLOBAL_APPLIST` and add `bmp280_app`:

**Before:**
```cmake
list(APPEND MISSION_GLOBAL_APPLIST
    sample_app
    ci_lab
    to_lab
    sch_lab
)
```

**After:**
```cmake
list(APPEND MISSION_GLOBAL_APPLIST
    sample_app
    ci_lab
    to_lab
    sch_lab
    bmp280_app
)
```

Save and exit (Ctrl+X, Y, Enter in nano).

### Step 4: Configure I2C Settings (Optional)

If your I2C bus is not `/dev/i2c-1` or your BMP280 address is not `0x76`:

```bash
nano apps/bmp280_app/fsw/src/bmp280_app.h
```

Modify these lines:
```c
#define BMP280_I2C_BUS          "/dev/i2c-1"  // Change bus number
#define BMP280_I2C_ADDR         0x76          // Change to 0x77 if needed
```

### Step 5: Build cFS with BMP280 App

```bash
cd /home/root/fsw/cFS/

# Set environment variables
export CC=aarch64-ostl-linux-gcc
export CXX=aarch64-ostl-linux-g++

# Clean any previous builds
rm -rf build

# Configure
make prep

# Compile
make

# Install
make install
```

**Build Success Indicators:**
```
...
[ XX%] Building C object apps/bmp280_app/CMakeFiles/bmp280_app.dir/fsw/src/bmp280_app.c.o
[ XX%] Linking C shared library ../../libbmp280_app.so
[ XX%] Built target bmp280_app
...
[100%] Built target mission-install
```

**Verify the shared library was created:**
```bash
ls -la build/exe/cpu1/cf/bmp280_app.so
```

### Step 6: Update the Startup Script

The startup script tells cFS which apps to load on startup.

```bash
cd /home/root/fsw/cFS/
nano build/exe/cpu1/cf/cfe_es_startup.scr
```

Add this line after the `CFE_LIB, core_api` line:
```
CFE_APP, bmp280_app,    /cf/bmp280_app.so,      BMP280_APP_Main,    BMP280_APP,     50,  8192, 0x0, 0;
```

**Complete example:**
```
CFE_LIB, core_api,      /cf/core_api.so,        API_Init,           CORE_API,        0,     0,    0x0, 0;
CFE_APP, bmp280_app,    /cf/bmp280_app.so,      BMP280_APP_Main,    BMP280_APP,     50,  8192, 0x0, 0;
CFE_APP, sample_app,    /cf/sample_app.so,      SAMPLE_APP_Main,    SAMPLE_APP,     50, 16384, 0x0, 0;
CFE_APP, ci_lab,        /cf/ci_lab.so,          CI_Lab_AppMain,     CI_LAB_APP,     60,  8192, 0x0, 0;
CFE_APP, to_lab,        /cf/to_lab.so,          TO_Lab_AppMain,     TO_LAB_APP,     70,  8192, 0x0, 0;
CFE_APP, sch_lab,       /cf/sch_lab.so,         SCH_Lab_AppMain,    SCH_LAB_APP,    80, 16384, 0x0, 0;
```

**Field Meanings:**
- Field 1: `CFE_APP` (this is an application)
- Field 2: `bmp280_app` (app name)
- Field 3: `/cf/bmp280_app.so` (shared library path)
- Field 4: `BMP280_APP_Main` (entry point function)
- Field 5: `BMP280_APP` (cFE name)
- Field 6: `50` (priority)
- Field 7: `8192` (stack size in bytes)
- Field 8: `0x0` (load address)
- Field 9: `0` (exception action: restart app)

### Step 7: Set I2C Permissions

Ensure the I2C device is accessible:
```bash
# Check current permissions
ls -la /dev/i2c-1

# If needed, set permissions
sudo chmod 666 /dev/i2c-1

# For permanent access, add to udev rules:
echo 'KERNEL=="i2c-[0-9]*", MODE="0666"' | sudo tee /etc/udev/rules.d/99-i2c.rules
sudo udevadm control --reload-rules
```

### Step 8: Run cFS

```bash
cd /home/root/fsw/cFS/build/exe/cpu1/
./core-cpu1
```

**Expected output:**
```
CFE_PSP: Starting Up
CFE_PSP: Target: ARM AARCH64 / OpenSTLinux
CFE_ES Startup: CFE_ES_Main in EARLY_INIT
...
CFE_ES Startup: Loading file /cf/bmp280_app.so
BMP280 APP Initialized. Version 1.0.0.0
BMP280: Sensor initialized successfully (Chip ID: 0x58)
...
CFE_ES Startup: CFE_ES_Main entering OPERATIONAL state
Application SAMPLE_APP started
Application CI_LAB started
Application TO_LAB started
Application SCH_LAB started
Application BMP280_APP started
```

### Step 9: Verify Installation

While cFS is running, check that the app is loaded:

1. **Check the event log** for BMP280 messages
2. **Look for successful initialization** message
3. **Send a test command** (if you have ground system tools)

## Troubleshooting Installation

### Problem: "Failed to open I2C bus"

**Cause:** Permissions or device doesn't exist

**Solution:**
```bash
# Check device exists
ls -la /dev/i2c*

# Fix permissions
sudo chmod 666 /dev/i2c-1

# Check kernel module loaded
lsmod | grep i2c
```

### Problem: "Build fails with compiler not found"

**Cause:** Environment variables not set

**Solution:**
```bash
export CC=aarch64-ostl-linux-gcc
export CXX=aarch64-ostl-linux-g++
which $CC  # Should show path
```

### Problem: "App not in startup output"

**Cause:** Not added to startup script or script has errors

**Solution:**
```bash
# Verify script syntax
cat build/exe/cpu1/cf/cfe_es_startup.scr

# Check for BMP280_APP line
grep BMP280 build/exe/cpu1/cf/cfe_es_startup.scr

# Verify shared library exists
ls -la build/exe/cpu1/cf/bmp280_app.so
```

### Problem: "Invalid chip ID error"

**Cause:** Wrong I2C address or sensor not connected

**Solution:**
```bash
# Scan for devices
i2cdetect -y 1

# Try reading chip ID manually
i2cget -y 1 0x76 0xD0  # Should return 0x58

# If sensor is at 0x77, update bmp280_app.h
nano apps/bmp280_app/fsw/src/bmp280_app.h
# Change: #define BMP280_I2C_ADDR 0x77
```

### Problem: "Shared library not created"

**Cause:** Build failed or CMakeLists.txt error

**Solution:**
```bash
# Clean and rebuild
rm -rf build
make prep 2>&1 | tee prep.log
make 2>&1 | tee build.log

# Check for errors
grep -i error build.log

# Verify CMakeLists.txt
cat apps/bmp280_app/CMakeLists.txt
```

## Post-Installation Testing

### Test 1: Verify Sensor Communication

```bash
# While cFS is NOT running, test sensor directly
i2cget -y 1 0x76 0xD0  # Read chip ID, should return 0x58
i2cget -y 1 0x76 0xF7  # Read pressure MSB
i2cget -y 1 0x76 0xFA  # Read temperature MSB
```

### Test 2: Monitor cFS Output

```bash
# In one terminal, run cFS
cd /home/root/fsw/cFS/build/exe/cpu1/
./core-cpu1 2>&1 | tee cfs_output.log

# In another terminal, watch for BMP280 messages
tail -f cfs_output.log | grep BMP280
```

### Test 3: Send Commands (Manual)

If you have command tools available:
```bash
# Send READ_SENSOR command
# This depends on your ground system setup
```

## Uninstallation

To remove the BMP280 app:

```bash
cd /home/root/fsw/cFS/

# Remove from targets.cmake
nano sample_defs/targets.cmake
# Delete bmp280_app line

# Remove from startup script
nano build/exe/cpu1/cf/cfe_es_startup.scr
# Delete BMP280_APP line

# Rebuild
rm -rf build
make prep && make && make install

# Or just delete the app directory
rm -rf apps/bmp280_app/
```

## Next Steps

After successful installation:

1. **Read the README.md** for usage instructions
2. **Check WIRING_DIAGRAM.md** for hardware details
3. **Integrate with SCH_LAB** for automatic periodic reads
4. **Set up ground system** to send commands and receive telemetry

## Additional Resources

- cFS Build Guide: `/home/root/fsw/cFS_STM32MP257_Build_Guide.md`
- cFS Documentation: https://github.com/nasa/cFS
- BMP280 Datasheet: https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/

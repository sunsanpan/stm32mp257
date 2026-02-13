#!/bin/bash
#
# Build script for BMP280 cFS application
# Usage: ./build_bmp280.sh [clean]
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

CFS_DIR="/home/root/fsw/cFS"

echo -e "${BLUE}======================================"
echo "  BMP280 App Build Script"
echo -e "======================================${NC}"
echo ""

# Check if we're in the right place
if [ ! -d "$CFS_DIR" ]; then
    echo -e "${RED}ERROR: cFS directory not found at $CFS_DIR${NC}"
    echo "Please update CFS_DIR in this script"
    exit 1
fi

# Check if app is copied
if [ ! -d "$CFS_DIR/apps/bmp280_app" ]; then
    echo -e "${RED}ERROR: bmp280_app not found in $CFS_DIR/apps/${NC}"
    echo "Please copy the app first:"
    echo "  cp -r bmp280_app $CFS_DIR/apps/"
    exit 1
fi

# Set compiler environment
export CC=aarch64-ostl-linux-gcc
export CXX=aarch64-ostl-linux-g++

echo "Compiler configuration:"
echo "  CC:  $CC"
echo "  CXX: $CXX"
echo ""

# Check compiler exists
if ! which $CC > /dev/null 2>&1; then
    echo -e "${RED}ERROR: Compiler $CC not found${NC}"
    exit 1
fi

cd $CFS_DIR

# Clean if requested
if [ "$1" = "clean" ]; then
    echo -e "${YELLOW}Cleaning previous build...${NC}"
    rm -rf build
    echo "Clean complete"
    echo ""
fi

# Prep
echo -e "${GREEN}Step 1/3: Preparing build configuration...${NC}"
make prep
if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: make prep failed${NC}"
    exit 1
fi
echo ""

# Compile
echo -e "${GREEN}Step 2/3: Compiling...${NC}"
make
if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: make failed${NC}"
    exit 1
fi
echo ""

# Install
echo -e "${GREEN}Step 3/3: Installing...${NC}"
make install
if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: make install failed${NC}"
    exit 1
fi
echo ""

# Verify
echo "Verifying build..."
if [ -f "$CFS_DIR/build/exe/cpu1/cf/bmp280_app.so" ]; then
    echo -e "${GREEN}✓ bmp280_app.so created successfully${NC}"
    ls -lh "$CFS_DIR/build/exe/cpu1/cf/bmp280_app.so"
else
    echo -e "${RED}ERROR: bmp280_app.so not found${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}======================================"
echo "  Build Complete!"
echo -e "======================================${NC}"
echo ""
echo "Next steps:"
echo "  1. Update startup script if not done:"
echo "     nano $CFS_DIR/build/exe/cpu1/cf/cfe_es_startup.scr"
echo ""
echo "  2. Add this line after CFE_LIB entry:"
echo "     CFE_APP, bmp280_app, /cf/bmp280_app.so, BMP280_APP_Main, BMP280_APP, 50, 8192, 0x0, 0;"
echo ""
echo "  3. Run cFS:"
echo "     cd $CFS_DIR/build/exe/cpu1/"
echo "     ./core-cpu1"
echo ""

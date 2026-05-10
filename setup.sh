#!/bin/bash
# setup.sh - Download pre-built dependencies from GitHub Release
# Chạy 1 lần sau khi clone. Sau đó chỉ cần `make`.

set -e

REPO="KhanhAI-VN/A35M"
TAG="deps-latest"
DEPS_DIR="deps"
ARCHIVE="/tmp/a35m-deps.tar.gz"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${GREEN}  A35M Dependency Setup${NC}"
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

# Check if deps already exist
if [ -d "$DEPS_DIR/toolchain/bin" ] && [ -f "$DEPS_DIR/lib/libmbedtls.a" ]; then
    echo -e "${YELLOW}⚠️  Dependencies already exist in ${DEPS_DIR}/${NC}"
    read -p "Re-download? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Skipped. Run 'make' to build."
        exit 0
    fi
fi

# Download
echo -e "\n${GREEN}⬇️  Downloading deps from GitHub Release...${NC}"
DOWNLOAD_URL="https://github.com/${REPO}/releases/download/${TAG}/deps.tar.gz"

if command -v wget &>/dev/null; then
    wget -q --show-progress -O "$ARCHIVE" "$DOWNLOAD_URL"
elif command -v curl &>/dev/null; then
    curl -L --progress-bar -o "$ARCHIVE" "$DOWNLOAD_URL"
else
    echo -e "${RED}Error: wget or curl required${NC}"
    exit 1
fi

# Extract
echo -e "${GREEN}📦  Extracting to ${DEPS_DIR}/...${NC}"
rm -rf "$DEPS_DIR"
mkdir -p "$DEPS_DIR"
tar -xzf "$ARCHIVE" -C "$DEPS_DIR"
rm -f "$ARCHIVE"

# Verify
echo -e "\n${GREEN}✅  Setup complete!${NC}"
echo ""
echo "Toolchain: $($DEPS_DIR/toolchain/bin/arm-linux-musleabihf-gcc --version 2>/dev/null | head -1 || echo 'N/A')"
echo "Libraries:"
ls -lh "$DEPS_DIR/lib/"*.a 2>/dev/null || echo "  (none found)"
echo ""
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "  Run ${YELLOW}make${NC} to build predict"
echo -e "  Run ${YELLOW}make run COIN=BTC${NC} to test"
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

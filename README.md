# A35M — ARM Inference Engine

Lightweight, statically-linked ARM inference engine for real-time cryptocurrency price forecasting.
Runs on **Luckyfox Pico** (ARMv7-A) with hand-optimized NEON assembly.

## Features

- 🧠 PatchLinear neural network with SE attention blocks
- ⚡ ARM NEON assembly-optimized layers (conv1d, gelu, sigmoid, etc.)
- 🔒 Static musl-libc binary — zero runtime dependencies
- 📡 TLS via mbedtls — fetches live data from Binance API
- 🤖 Model weights auto-downloaded from GitHub at runtime

## Quick Start

```bash
# 1. Clone
git clone https://github.com/KhanhAI-VN/A35M.git
cd A35M

# 2. Download pre-built dependencies (toolchain + mbedtls)
./setup.sh

# 3. Build
make

# 4. Run (with qemu for testing on x86)
make run COIN=BTC
```

## Project Structure

```
A35M/
├── .github/workflows/
│   └── build-deps.yml      # Auto-build deps every 2 days
├── asm/                     # ARM NEON Assembly kernels
│   ├── conv1d.S
│   ├── gelu.S
│   ├── sigmoid.S
│   └── ...
├── src/                     # C source code
│   ├── predict.c            # Main inference engine
│   ├── network.h             # SSL/TLS networking
│   └── models.h              # Model weight loader
├── deps/                    # ⬇️ Auto-downloaded (not in git)
│   ├── toolchain/           # arm-linux-musleabihf-gcc
│   ├── include/             # mbedtls headers
│   └── lib/                 # libmbedtls.a, libmbedx509.a, libmbedcrypto.a
├── Makefile
├── setup.sh                 # One-time dependency download
└── README.md
```

## How Dependencies Work

Dependencies (`toolchain` + `mbedtls`) are **NOT** stored in this repo.
Instead, a GitHub Actions workflow automatically:

1. Downloads the latest `arm-linux-musleabihf` cross-compiler from [musl.cc](https://musl.cc)
2. Clones and cross-compiles `mbedtls` (branch `mbedtls-2.28`)
3. Packages everything into `deps.tar.gz` and uploads to [GitHub Releases](https://github.com/KhanhAI-VN/A35M/releases/tag/deps-latest)

This runs every **2 days** automatically. The `setup.sh` script downloads the latest release.

## Build Requirements

- Linux x86_64 (for cross-compilation)
- `wget` or `curl` (for `setup.sh`)
- `qemu-arm` (optional, for testing on x86)

## License

MIT

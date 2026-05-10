# A35M — ARM Inference Engine
# Cross-compile with musl-libc (static linking)
# Dependencies are in deps/ (downloaded by setup.sh)

# ─── Paths ───────────────────────────────────────────
DEPS_DIR   = deps
TOOLCHAIN  = $(DEPS_DIR)/toolchain
CC         = $(TOOLCHAIN)/bin/arm-linux-musleabihf-gcc
AS         = $(TOOLCHAIN)/bin/arm-linux-musleabihf-as

# ─── Flags ───────────────────────────────────────────
CFLAGS     = -march=armv7-a -mfpu=vfpv4 -O2 -Isrc -I$(DEPS_DIR)/include
AFLAGS     = -march=armv7-a -mfpu=vfpv4

# ─── Output ──────────────────────────────────────────
TARGET     = predict
BIN_DIR    = bin

# ─── Sources ─────────────────────────────────────────
SRC        = src/predict.c
ASM_SRCS   = asm/parse_close.S asm/rotate_buffer.S asm/compute_log_returns.S \
             asm/revin_normalize.S asm/moving_average.S asm/decompose.S \
             asm/conv1d.S asm/sigmoid.S asm/relu.S asm/gelu.S \
             asm/flatten_patches.S asm/load_model_bin.S

OBJS       = $(BIN_DIR)/predict.o $(patsubst asm/%.S,$(BIN_DIR)/%.o,$(ASM_SRCS))

# ─── Libraries ───────────────────────────────────────
LIBS       = $(DEPS_DIR)/lib/libmbedtls.a \
             $(DEPS_DIR)/lib/libmbedx509.a \
             $(DEPS_DIR)/lib/libmbedcrypto.a

# ─── Rules ───────────────────────────────────────────
all: check-deps $(BIN_DIR) $(TARGET)

check-deps:
	@test -d "$(TOOLCHAIN)/bin" || { echo "❌ Missing deps. Run: ./setup.sh"; exit 1; }
	@test -f "$(DEPS_DIR)/lib/libmbedtls.a" || { echo "❌ Missing libs. Run: ./setup.sh"; exit 1; }

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile C
$(BIN_DIR)/predict.o: src/predict.c src/network.h src/models.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile ASM
$(BIN_DIR)/%.o: asm/%.S | $(BIN_DIR)
	$(AS) $(AFLAGS) $< -o $@

# Link
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -static -no-pie -lm $(LIBS)

clean:
	rm -rf $(TARGET) $(BIN_DIR)

run: $(TARGET)
	qemu-arm ./$(TARGET) $(COIN)

.PHONY: all clean run check-deps

# ==========================================
# Variables
# ==========================================
# Make inherits environment variables, so $(LOCALAPPDATA) will work on Windows.
ARDUINO_CLI ?= arduino-cli
ESPTOOL ?= $(LOCALAPPDATA)/Arduino15/packages/esp32/tools/esptool_py/5.3.1/esptool.exe
MKLITTLEFS ?= $(LOCALAPPDATA)/Arduino15/packages/esp32/tools/mklittlefs/4.0.2-db0513a/mklittlefs.exe
BUILD_OUTPUT_DIR ?= ./release-build
BASENAME ?= mss-xcade-firmware

# Specify your serial port for the flash target
PORT ?= COM6

# ==========================================
# Phony Targets (Commands that aren't files)
# ==========================================
.PHONY: all firmware fs merge flash clean

# Default target when you just run `make`
all: merge

# ==========================================
# Build Targets
# ==========================================

firmware:
	@echo ""
	@echo "***********************************"
	@echo "  Building firmware"
	@echo "***********************************"
	@echo ""
	"$(ARDUINO_CLI)" compile -v --profile release \
		--fqbn=esp32:esp32:esp32s2:CDCOnBoot=cdc,PSRAM=enabled,FlashSize=4M \
		--build-property "build.custom_partitions=./partitions.csv" \
		--output-dir "$(BUILD_OUTPUT_DIR)" \
		./$(BASENAME).ino

fs:
	@echo ""
	@echo "***********************************"
	@echo "  Building default filesystem      "
	@echo "***********************************"
	@echo ""
	"$(MKLITTLEFS)" -c ./data -p 256 -b 4096 -s 0xE0000 "$(BUILD_OUTPUT_DIR)/$(BASENAME)-littlefs.bin"

merge: firmware fs
	@echo ""
	@echo "***********************************"
	@echo "  Merging firmware"
	@echo "***********************************"
	@echo ""
	"$(ESPTOOL)" --chip esp32s2 merge-bin -o "$(BUILD_OUTPUT_DIR)/$(BASENAME)-full.bin" --flash-mode dio --flash-freq 80m --flash-size 4MB \
		0x1000 "$(BUILD_OUTPUT_DIR)/$(BASENAME).ino.bootloader.bin" \
		0x8000 "$(BUILD_OUTPUT_DIR)/$(BASENAME).ino.partitions.bin" \
		0x10000 "$(BUILD_OUTPUT_DIR)/$(BASENAME).ino.bin" \
		0x310000 "$(BUILD_OUTPUT_DIR)/$(BASENAME)-littlefs.bin"
	"$(ESPTOOL)" --chip esp32s2 merge-bin -o "$(BUILD_OUTPUT_DIR)/$(BASENAME)-upgrade.bin" --flash-mode dio --flash-freq 80m --flash-size 4MB \
		0x1000 "$(BUILD_OUTPUT_DIR)/$(BASENAME).ino.bootloader.bin" \
		0x8000 "$(BUILD_OUTPUT_DIR)/$(BASENAME).ino.partitions.bin" \
		0x10000 "$(BUILD_OUTPUT_DIR)/$(BASENAME).ino.bin"

flash:
	@echo ""
	@echo "***********************************"
	@echo "  Flashing Device"
	@echo "***********************************"
	@echo ""
	"$(ESPTOOL)" --chip esp32s2 --port $(PORT) write-flash 0x0 "$(BUILD_OUTPUT_DIR)/$(BASENAME)-upgrade.bin"


flash-full:
	@echo ""
	@echo "***********************************"
	@echo "  Flashing Device"
	@echo "***********************************"
	@echo ""
	"$(ESPTOOL)" --chip esp32s2 --port $(PORT) write-flash 0x0 "$(BUILD_OUTPUT_DIR)/$(BASENAME)-full.bin"

clean:
	@echo "Cleaning build directory and generated binaries..."
	rm -rf "$(BUILD_OUTPUT_DIR)"
	
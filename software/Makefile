# Makefile for Zynq CNN Accelerator Project
# Cross-compilation for Arm processor

# Toolchain configuration
CROSS_COMPILE ?= arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar

# Directories
SRC_DIR = .
BUILD_DIR = build
BIN_DIR = bin
COMMON_DIR = common
DRIVERS_DIR = drivers
CPU_BASELINE_DIR = cpu_baseline
HW_ACCEL_DIR = hw_accelerated
MODEL_DIR = ../models

# Compiler flags
CFLAGS = -Wall -O3 -march=armv7-a -mfpu=neon -mfloat-abi=hard
CXXFLAGS = $(CFLAGS) -std=c++11
INCLUDES = -I$(COMMON_DIR) -I$(DRIVERS_DIR) -I$(MODEL_DIR)/configs
LDFLAGS = -lpthread -lm

# OpenCV flags (if available on target)
OPENCV_CFLAGS = $(shell pkg-config --cflags opencv4 2>/dev/null || echo "")
OPENCV_LIBS = $(shell pkg-config --libs opencv4 2>/dev/null || echo "-lopencv_core -lopencv_imgproc -lopencv_videoio")

# Source files
COMMON_SRCS = $(wildcard $(COMMON_DIR)/*.cpp)
DRIVER_SRCS = $(wildcard $(DRIVERS_DIR)/*.cpp)
CPU_BASELINE_SRCS = $(wildcard $(CPU_BASELINE_DIR)/*.cpp)
HW_ACCEL_SRCS = $(wildcard $(HW_ACCEL_DIR)/*.cpp)

# Object files
COMMON_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(COMMON_SRCS))
DRIVER_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(DRIVER_SRCS))
CPU_BASELINE_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CPU_BASELINE_SRCS))
HW_ACCEL_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(HW_ACCEL_SRCS))

# Targets
CPU_BASELINE_BIN = $(BIN_DIR)/cnn_inference_cpu
HW_ACCEL_BIN = $(BIN_DIR)/cnn_inference_hw

.PHONY: all clean cpu_baseline hw_accelerated deploy

all: cpu_baseline hw_accelerated

# Create directories
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@
	mkdir -p $(BUILD_DIR)/$(COMMON_DIR)
	mkdir -p $(BUILD_DIR)/$(DRIVERS_DIR)
	mkdir -p $(BUILD_DIR)/$(CPU_BASELINE_DIR)
	mkdir -p $(BUILD_DIR)/$(HW_ACCEL_DIR)

# Common object files
$(BUILD_DIR)/$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPENCV_CFLAGS) -c $< -o $@

# Driver object files
$(BUILD_DIR)/$(DRIVERS_DIR)/%.o: $(DRIVERS_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# CPU baseline object files
$(BUILD_DIR)/$(CPU_BASELINE_DIR)/%.o: $(CPU_BASELINE_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPENCV_CFLAGS) -c $< -o $@

# HW accelerated object files
$(BUILD_DIR)/$(HW_ACCEL_DIR)/%.o: $(HW_ACCEL_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPENCV_CFLAGS) -c $< -o $@

# CPU baseline binary
cpu_baseline: $(CPU_BASELINE_BIN)

$(CPU_BASELINE_BIN): $(COMMON_OBJS) $(CPU_BASELINE_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(OPENCV_LIBS)
	@echo "Built CPU baseline: $@"

# HW accelerated binary
hw_accelerated: $(HW_ACCEL_BIN)

$(HW_ACCEL_BIN): $(COMMON_OBJS) $(DRIVER_OBJS) $(HW_ACCEL_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(OPENCV_LIBS)
	@echo "Built HW accelerated: $@"

# Deploy to target (via SCP or SD card)
deploy: all
	@echo "Deploying binaries to target..."
	@if [ -n "$(TARGET_IP)" ]; then \
		scp $(CPU_BASELINE_BIN) $(HW_ACCEL_BIN) root@$(TARGET_IP):/root/; \
		echo "Deployed to $(TARGET_IP)"; \
	else \
		echo "Set TARGET_IP environment variable for network deployment"; \
		echo "Or copy $(BIN_DIR)/* to SD card manually"; \
	fi

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build artifacts"

# Help
help:
	@echo "Zynq CNN Accelerator Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all            - Build both CPU baseline and HW accelerated versions"
	@echo "  cpu_baseline   - Build CPU-only implementation"
	@echo "  hw_accelerated - Build FPGA-accelerated implementation"
	@echo "  deploy         - Deploy binaries to target (set TARGET_IP)"
	@echo "  clean          - Remove build artifacts"
	@echo ""
	@echo "Example:"
	@echo "  make all"
	@echo "  make deploy TARGET_IP=192.168.1.100"

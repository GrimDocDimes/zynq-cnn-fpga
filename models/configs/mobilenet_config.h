#ifndef MOBILENET_CONFIG_H
#define MOBILENET_CONFIG_H

#include <stdint.h>

// Model Configuration for MobileNetV1 (Quantized INT8)
// Input: 224x224x3 RGB image
// Output: 1000 class probabilities

// ============================================================================
// Layer Dimensions
// ============================================================================

#define INPUT_HEIGHT 224
#define INPUT_WIDTH 224
#define INPUT_CHANNELS 3
#define NUM_CLASSES 1000

// First standard convolution layer
#define CONV1_FILTERS 32
#define CONV1_KERNEL_SIZE 3
#define CONV1_STRIDE 2
#define CONV1_PADDING 1

// Depthwise separable convolution blocks (13 blocks)
// Format: {input_channels, output_channels, stride}
const int DEPTHWISE_BLOCKS[13][3] = {
    {32, 64, 1},    // Block 1
    {64, 128, 2},   // Block 2
    {128, 128, 1},  // Block 3
    {128, 256, 2},  // Block 4
    {256, 256, 1},  // Block 5
    {256, 512, 2},  // Block 6
    {512, 512, 1},  // Block 7
    {512, 512, 1},  // Block 8
    {512, 512, 1},  // Block 9
    {512, 512, 1},  // Block 10
    {512, 512, 1},  // Block 11
    {512, 1024, 2}, // Block 12
    {1024, 1024, 1} // Block 13
};

// Global average pooling output
#define GAP_OUTPUT_SIZE 1024

// Fully connected layer
#define FC_INPUT_SIZE 1024
#define FC_OUTPUT_SIZE NUM_CLASSES

// ============================================================================
// Quantization Parameters (INT8)
// ============================================================================

// Fixed-point representation: Q7.8 (8-bit integer, 8-bit fractional)
typedef int8_t qint8_t;
typedef int16_t qint16_t;
typedef int32_t qint32_t;

// Quantization scale and zero-point for each layer
// These would be populated from the quantized model
struct QuantParams {
    float scale;
    int32_t zero_point;
};

// Input quantization
#define INPUT_SCALE 0.007843f  // 1/127.5 for [-1, 1] normalization
#define INPUT_ZERO_POINT 128

// Layer-specific quantization parameters (example values)
extern const QuantParams CONV1_QUANT;
extern const QuantParams DEPTHWISE_QUANT[13];
extern const QuantParams POINTWISE_QUANT[13];
extern const QuantParams FC_QUANT;

// ============================================================================
// Memory Layout Configuration
// ============================================================================

// Maximum feature map dimensions (for buffer allocation)
#define MAX_FEATURE_MAP_HEIGHT 112
#define MAX_FEATURE_MAP_WIDTH 112
#define MAX_FEATURE_MAP_CHANNELS 1024

// Buffer sizes for FPGA accelerator
#define CONV_INPUT_BUFFER_SIZE (MAX_FEATURE_MAP_HEIGHT * MAX_FEATURE_MAP_WIDTH * MAX_FEATURE_MAP_CHANNELS)
#define CONV_OUTPUT_BUFFER_SIZE (MAX_FEATURE_MAP_HEIGHT * MAX_FEATURE_MAP_WIDTH * MAX_FEATURE_MAP_CHANNELS)
#define WEIGHT_BUFFER_SIZE (3 * 3 * MAX_FEATURE_MAP_CHANNELS * MAX_FEATURE_MAP_CHANNELS)

// ============================================================================
// FPGA Hardware Configuration
// ============================================================================

// Parallel processing units
#define CONV_PE_NUM 16          // Number of parallel processing elements for convolution
#define CONV_SIMD_FACTOR 8      // SIMD parallelism within each PE

// Tiling parameters for large feature maps
#define TILE_HEIGHT 14
#define TILE_WIDTH 14
#define TILE_CHANNELS 32

// AXI interface configuration
#define AXI_DATA_WIDTH 64       // 64-bit AXI data bus
#define AXI_BURST_LEN 256       // Maximum burst length

// Memory-mapped register addresses (example offsets)
#define FPGA_BASE_ADDR 0x43C00000
#define CONV_CTRL_REG (FPGA_BASE_ADDR + 0x00)
#define CONV_STATUS_REG (FPGA_BASE_ADDR + 0x04)
#define CONV_INPUT_ADDR_REG (FPGA_BASE_ADDR + 0x08)
#define CONV_OUTPUT_ADDR_REG (FPGA_BASE_ADDR + 0x0C)
#define CONV_WEIGHT_ADDR_REG (FPGA_BASE_ADDR + 0x10)
#define CONV_CONFIG_REG (FPGA_BASE_ADDR + 0x14)

// Control register bits
#define CTRL_START_BIT (1 << 0)
#define CTRL_RESET_BIT (1 << 1)
#define STATUS_DONE_BIT (1 << 0)
#define STATUS_IDLE_BIT (1 << 1)

// ============================================================================
// Performance Configuration
// ============================================================================

// Target performance metrics
#define TARGET_FPS 15
#define TARGET_LATENCY_MS 66  // ~15 FPS

// Optimization flags
#define ENABLE_DOUBLE_BUFFERING 1
#define ENABLE_LAYER_FUSION 1
#define ENABLE_WEIGHT_CACHING 1

#endif // MOBILENET_CONFIG_H

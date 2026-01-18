#!/bin/bash
# Build all HLS accelerators for CNN inference

set -e

echo "=== Building CNN HLS Accelerators ==="
echo ""

# Check if vitis_hls is available
if ! command -v vitis_hls &> /dev/null; then
    echo "Error: vitis_hls not found in PATH"
    echo "Please source Vivado/Vitis settings:"
    echo "  source /tools/Xilinx/Vitis_HLS/2020.2/settings64.sh"
    exit 1
fi

echo "Found Vitis HLS: $(which vitis_hls)"
echo ""

# Build convolution accelerator
echo "=== Building Convolution Accelerator ==="
cd ../hardware/hls/conv_accelerator
vitis_hls -f run_hls.tcl
if [ $? -eq 0 ]; then
    echo "✓ Convolution accelerator built successfully"
else
    echo "✗ Convolution accelerator build failed"
    exit 1
fi
echo ""

# Build activation accelerator
echo "=== Building Activation Accelerator ==="
cd ../activation
vitis_hls -f run_hls.tcl
if [ $? -eq 0 ]; then
    echo "✓ Activation accelerator built successfully"
else
    echo "✗ Activation accelerator build failed"
    exit 1
fi
echo ""

# Build pooling accelerator
echo "=== Building Pooling Accelerator ==="
cd ../pooling
vitis_hls -f run_hls.tcl
if [ $? -eq 0 ]; then
    echo "✓ Pooling accelerator built successfully"
else
    echo "✗ Pooling accelerator build failed"
    exit 1
fi
echo ""

echo "=== All HLS Builds Complete ==="
echo ""
echo "IP cores exported to:"
echo "  - hardware/hls/conv_accelerator/conv_accelerator_proj/solution1/impl/ip/"
echo "  - hardware/hls/activation/activation_accelerator_proj/solution1/impl/ip/"
echo "  - hardware/hls/pooling/pooling_accelerator_proj/solution1/impl/ip/"
echo ""
echo "Next steps:"
echo "  1. Open Vivado"
echo "  2. Create new project for your Zynq board"
echo "  3. Add IP repository paths (Tools > Settings > IP > Repository)"
echo "  4. Create block design and add the IP cores"
echo "  5. Connect to Zynq PS via AXI"
echo "  6. Generate bitstream"

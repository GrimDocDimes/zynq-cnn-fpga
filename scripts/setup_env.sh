#!/bin/bash
# Quick setup script for development environment

set -e

echo "=== Zynq CNN Accelerator - Development Environment Setup ==="
echo ""

# Check for required tools
echo "Checking for required tools..."

# Check for ARM toolchain
if command -v arm-linux-gnueabihf-gcc &> /dev/null; then
    echo "✓ ARM cross-compiler found: $(arm-linux-gnueabihf-gcc --version | head -n1)"
else
    echo "✗ ARM cross-compiler not found"
    echo "  Install with: sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf"
fi

# Check for Python
if command -v python3 &> /dev/null; then
    echo "✓ Python3 found: $(python3 --version)"
else
    echo "✗ Python3 not found"
fi

# Check for Vivado/Vitis HLS
if command -v vivado &> /dev/null; then
    echo "✓ Vivado found: $(vivado -version | head -n1)"
else
    echo "⚠ Vivado not found in PATH"
    echo "  Make sure Xilinx tools are installed and sourced"
fi

if command -v vitis_hls &> /dev/null; then
    echo "✓ Vitis HLS found"
else
    echo "⚠ Vitis HLS not found in PATH"
fi

echo ""
echo "Setting up Python environment..."

# Check for Python packages
python3 -c "import tensorflow" 2>/dev/null && echo "✓ TensorFlow installed" || echo "⚠ TensorFlow not found (pip3 install tensorflow)"
python3 -c "import numpy" 2>/dev/null && echo "✓ NumPy installed" || echo "⚠ NumPy not found (pip3 install numpy)"
python3 -c "import cv2" 2>/dev/null && echo "✓ OpenCV installed" || echo "⚠ OpenCV not found (pip3 install opencv-python)"

echo ""
echo "Creating build directories..."
mkdir -p software/bin
mkdir -p software/build
mkdir -p models/quantized/weights
mkdir -p models/quantized/configs
mkdir -p datasets
echo "✓ Directories created"

echo ""
echo "=== Setup Complete ==="
echo ""
echo "Next steps:"
echo "1. Run model quantization: cd models && python3 quantize_model.py"
echo "2. Build HLS IP cores: cd scripts && vitis_hls -f build_hls.tcl"
echo "3. Create Vivado project and generate bitstream"
echo "4. Compile software: cd software && make all"
echo "5. Deploy to Zynq board"
echo ""

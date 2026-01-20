#!/bin/bash
# Script to build Vivado project and generate bitstream

set -e

echo "=== Building Vivado Project & Bitstream ==="

# Check for Vivado
if ! command -v vivado &> /dev/null; then
    echo "Error: vivado not found in PATH"
    echo "Please source Xilinx tools first:"
    echo "  source /tools/Xilinx/Vivado/2019.2/settings64.sh"
    exit 1
fi

# Run TCL script
echo "Running Vivado TCL script..."
vivado -mode batch -source scripts/build_vivado.tcl

echo ""
echo "=== Project Created ==="
echo "To generate the bitstream, open the project in Vivado GUI:"
echo "  vivado vivado_project/zynq_cnn_system.xpr"
echo ""
echo "Then click 'Generate Bitstream' in the Flow Navigator."
echo "Once done, export the hardware (File > Export > Export Hardware) to get the XSA file."

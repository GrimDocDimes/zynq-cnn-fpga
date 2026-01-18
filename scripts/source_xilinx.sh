#!/bin/bash
# Source Xilinx Vivado/Vitis HLS tools
# Modify the paths below to match your installation

# Common installation paths - update these to match your system
VIVADO_PATHS=(
    "/tools/Xilinx/Vivado/2023.2/settings64.sh"
    "/tools/Xilinx/Vivado/2022.2/settings64.sh"
    "/tools/Xilinx/Vivado/2021.2/settings64.sh"
    "/tools/Xilinx/Vivado/2020.2/settings64.sh"
    "/opt/Xilinx/Vivado/2023.2/settings64.sh"
    "/opt/Xilinx/Vivado/2022.2/settings64.sh"
    "$HOME/Xilinx/Vivado/2023.2/settings64.sh"
)

VITIS_HLS_PATHS=(
    "/tools/Xilinx/Vitis_HLS/2023.2/settings64.sh"
    "/tools/Xilinx/Vitis_HLS/2022.2/settings64.sh"
    "/tools/Xilinx/Vitis_HLS/2021.2/settings64.sh"
    "/tools/Xilinx/Vitis_HLS/2020.2/settings64.sh"
    "/opt/Xilinx/Vitis_HLS/2023.2/settings64.sh"
    "$HOME/Xilinx/Vitis_HLS/2023.2/settings64.sh"
)

echo "Searching for Xilinx tools..."

# Try to find and source Vivado
VIVADO_FOUND=0
for path in "${VIVADO_PATHS[@]}"; do
    if [ -f "$path" ]; then
        echo "Found Vivado: $path"
        source "$path"
        VIVADO_FOUND=1
        break
    fi
done

# Try to find and source Vitis HLS
VITIS_HLS_FOUND=0
for path in "${VITIS_HLS_PATHS[@]}"; do
    if [ -f "$path" ]; then
        echo "Found Vitis HLS: $path"
        source "$path"
        VITIS_HLS_FOUND=1
        break
    fi
done

# If not found, provide instructions
if [ $VIVADO_FOUND -eq 0 ] && [ $VITIS_HLS_FOUND -eq 0 ]; then
    echo ""
    echo "ERROR: Could not find Xilinx tools automatically."
    echo ""
    echo "Please manually source your Vivado/Vitis HLS installation:"
    echo "  source /path/to/Xilinx/Vivado/<version>/settings64.sh"
    echo "  source /path/to/Xilinx/Vitis_HLS/<version>/settings64.sh"
    echo ""
    echo "Then run this script again or run the HLS build directly:"
    echo "  ./scripts/build_all_hls.sh"
    exit 1
fi

# Verify tools are available
echo ""
echo "Verifying tools..."
if command -v vivado &> /dev/null; then
    echo "✓ Vivado: $(vivado -version | head -n1)"
fi

if command -v vitis_hls &> /dev/null; then
    echo "✓ Vitis HLS: $(vitis_hls -version | head -n1)"
elif command -v vivado_hls &> /dev/null; then
    echo "✓ Vivado HLS: $(vivado_hls -version | head -n1)"
fi

echo ""
echo "Tools sourced successfully!"
echo "You can now run: ./scripts/build_all_hls.sh"

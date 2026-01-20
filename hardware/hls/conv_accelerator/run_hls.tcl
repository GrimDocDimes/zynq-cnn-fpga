# Vitis HLS Script for Convolution Accelerator
# Run with: vitis_hls -f run_hls.tcl

# Project settings
set proj_name "conv_accelerator_proj"
set top_func "conv_accelerator"
set device "xc7z020clg400-1"
set period 10.0

# Create new project
open_project -reset ${proj_name}

# Add source files
add_files conv_accelerator.cpp

# Set top function
set_top ${top_func}

# Create solution
open_solution -reset "solution1"

# Set target device
set_part ${device}

# Set clock period (100 MHz = 10ns)
create_clock -period ${period} -name default

# Synthesis configuration
config_compile -name_max_length 80

# Run C synthesis
puts "Running C Synthesis..."
csynth_design

# Export RTL as IP
puts "Exporting IP..."
export_design -format ip_catalog \
    -description "CNN Convolution Accelerator" \
    -vendor "user" \
    -library "cnn" \
    -version "1.0" \
    -display_name "Conv2D Accelerator"

# Generate reports
puts "\n=== Synthesis Report ==="
puts "Check ${proj_name}/solution1/syn/report/conv_accelerator_csynth.rpt"

close_project

puts "\n=== Convolution Accelerator HLS Complete ==="
exit

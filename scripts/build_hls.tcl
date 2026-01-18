# Vivado HLS Build Script for CNN Accelerators
# Automates synthesis, C simulation, and IP export

# Set project parameters
set proj_name "cnn_accelerator"
set top_func "conv_accelerator"
set device "xc7z020clg484-1"
set period 10

# Create project
open_project -reset ${proj_name}

# Add source files
add_files ../hls/conv_accelerator/conv_accelerator.cpp
add_files ../hls/activation/activation.cpp
add_files ../hls/pooling/pooling.cpp

# Add testbench (if available)
# add_files -tb ../hls/testbench/conv_tb.cpp

# Set top function
set_top ${top_func}

# Create solution
open_solution -reset "solution1"

# Set device and clock
set_part ${device}
create_clock -period ${period} -name default

# Run C simulation
# csim_design

# Run synthesis
csynth_design

# Run C/RTL co-simulation
# cosim_design

# Export IP
export_design -format ip_catalog -description "CNN Convolution Accelerator" -vendor "user" -version "1.0"

# Close project
close_project

puts "HLS build complete!"

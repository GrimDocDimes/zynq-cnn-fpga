# Vitis HLS Script for Activation Accelerator
# Run with: vitis_hls -f run_hls.tcl

set proj_name "activation_accelerator_proj"
set top_func "activation_accelerator"
set device "xc7z020clg400-1"
set period 10.0

open_project -reset ${proj_name}
add_files activation.cpp
set_top ${top_func}

open_solution -reset "solution1"
set_part ${device}
create_clock -period ${period} -name default

config_compile -name_max_length 80

puts "Running C Synthesis..."
csynth_design

puts "Exporting IP..."
if { [catch {
    export_design -format ip_catalog \
        -description "CNN Activation Accelerator" \
        -vendor "user" \
        -library "cnn" \
        -version "1.0" \
        -display_name "Activation Accelerator"
} result] } {
    puts "Warning: IP export failed, but RTL was generated successfully"
    puts "You can manually package the IP from impl/ip/ directory"
}

puts "\n=== Activation Accelerator HLS Complete ==="
close_project
exit

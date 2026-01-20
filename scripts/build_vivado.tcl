# Vivado TCL Script to Build Zynq CNN System
# Run with: vivado -mode batch -source build_vivado.tcl

# 1. Project Settings
set proj_name "zynq_cnn_system"
set part_name "xc7z020clg400-1"
set bd_name "system"

# Create project
create_project -force ${proj_name} ./vivado_project -part ${part_name}

# 2. Add IP Repositories
set ip_repos [list \
    "../hardware/hls/conv_accelerator/conv_accelerator_proj/solution1/impl/ip" \
    "../hardware/hls/activation/activation_accelerator_proj/solution1/impl/ip" \
    "../hardware/hls/pooling/pooling_accelerator_proj/solution1/impl/ip" \
]
set_property  ip_repo_paths  ${ip_repos} [current_project]
update_ip_catalog

# 3. Create Block Design
create_bd_design ${bd_name}

# 4. Instantiate Zynq Processing System
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
endgroup

# Configure Zynq PS (Enable HP ports and Interrupts)
set_property -dict [list \
    CONFIG.PCW_USE_S_AXI_HP0 {1} \
    CONFIG.PCW_USE_S_AXI_HP1 {1} \
    CONFIG.PCW_USE_S_AXI_HP2 {1} \
    CONFIG.PCW_USE_S_AXI_HP3 {1} \
    CONFIG.PCW_USE_FABRIC_INTERRUPT {1} \
    CONFIG.PCW_IRQ_F2P_INTR {1} \
] [get_bd_cells processing_system7_0]

# Try to apply board preset if PYNQ-Z2 board files are present
# If not, the user might need to manually configure DDR/Clock settings later
if {[catch {apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]} err]} {
    puts "Warning: Could not apply board preset. You may need to manually configure DDR settings for your board."
    # Apply basic automation without board preset
    apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]
}

# 5. Instantiate HLS Accelerators
create_bd_cell -type ip -vlnv user:cnn:Conv2D_Accelerator:1.0 conv_accelerator_0
create_bd_cell -type ip -vlnv user:cnn:Activation_Accelerator:1.0 activation_accelerator_0
create_bd_cell -type ip -vlnv user:cnn:Pooling_Accelerator:1.0 pooling_accelerator_0

# 6. Interconnects
# SmartConnect for High Performance Data Transfer (HLS -> DDR)
create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect:1.0 axi_mem_interconnect

# Configure SmartConnect for 6 Slave interfaces (4 for Conv, 1 Act, 1 Pool)
set_property -dict [list CONFIG.NUM_SI {6}] [get_bd_cells axi_mem_interconnect]

# AXI Interconnect for Control (Zynq -> HLS)
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 ps7_0_axi_periph
set_property -dict [list CONFIG.NUM_MI {3}] [get_bd_cells ps7_0_axi_periph]

# 7. Clocks and Resets
create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_ps7_0_100M

# Connect FCLK_CLK0 (100MHz)
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/S_AXI_HP1_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/S_AXI_HP2_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/S_AXI_HP3_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins rst_ps7_0_100M/slowest_sync_clk]

# Connect Reset
connect_bd_net [get_bd_pins processing_system7_0/FCLK_RESET0_N] [get_bd_pins rst_ps7_0_100M/ext_reset_in]

# Connect Clocks to HLS IPs and Interconnects
set ip_list [list conv_accelerator_0 activation_accelerator_0 pooling_accelerator_0 axi_mem_interconnect ps7_0_axi_periph]
foreach ip $ip_list {
    connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins $ip/aclk]
}
# Also connect ARESETN
foreach ip $ip_list {
    connect_bd_net [get_bd_pins rst_ps7_0_100M/peripheral_aresetn] [get_bd_pins $ip/aresetn]
}
# Connect Interconnect specific clocks/resets
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins ps7_0_axi_periph/S00_ACLK]
connect_bd_net [get_bd_pins rst_ps7_0_100M/peripheral_aresetn] [get_bd_pins ps7_0_axi_periph/S00_ARESETN]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins ps7_0_axi_periph/M00_ACLK]
connect_bd_net [get_bd_pins rst_ps7_0_100M/peripheral_aresetn] [get_bd_pins ps7_0_axi_periph/M00_ARESETN]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins ps7_0_axi_periph/M01_ACLK]
connect_bd_net [get_bd_pins rst_ps7_0_100M/peripheral_aresetn] [get_bd_pins ps7_0_axi_periph/M01_ARESETN]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins ps7_0_axi_periph/M02_ACLK]
connect_bd_net [get_bd_pins rst_ps7_0_100M/peripheral_aresetn] [get_bd_pins ps7_0_axi_periph/M02_ARESETN]


# 8. Data Connections (HLS -> SmartConnect -> Zynq HP)
# Conv Accelerator (4 ports)
connect_bd_intf_net [get_bd_intf_pins conv_accelerator_0/m_axi_gmem0] [get_bd_intf_pins axi_mem_interconnect/S00_AXI]
connect_bd_intf_net [get_bd_intf_pins conv_accelerator_0/m_axi_gmem1] [get_bd_intf_pins axi_mem_interconnect/S01_AXI]
connect_bd_intf_net [get_bd_intf_pins conv_accelerator_0/m_axi_gmem2] [get_bd_intf_pins axi_mem_interconnect/S02_AXI]
connect_bd_intf_net [get_bd_intf_pins conv_accelerator_0/m_axi_gmem3] [get_bd_intf_pins axi_mem_interconnect/S03_AXI]
# Activation (3 ports - gmem0, gmem1, gmem2) - Wait, activation has 3 master ports? 
# Let's check activation.cpp: gmem0(input), gmem1(output), gmem2(params)
connect_bd_intf_net [get_bd_intf_pins activation_accelerator_0/m_axi_gmem0] [get_bd_intf_pins axi_mem_interconnect/S04_AXI]
# We need more ports on SmartConnect! 
# Conv: 4, Act: 3, Pool: 2 = 9 ports. 
# Let's increase SmartConnect ports to 9
set_property -dict [list CONFIG.NUM_SI {9}] [get_bd_cells axi_mem_interconnect]
connect_bd_intf_net [get_bd_intf_pins activation_accelerator_0/m_axi_gmem1] [get_bd_intf_pins axi_mem_interconnect/S05_AXI]
connect_bd_intf_net [get_bd_intf_pins activation_accelerator_0/m_axi_gmem2] [get_bd_intf_pins axi_mem_interconnect/S06_AXI]
# Pooling (2 ports)
connect_bd_intf_net [get_bd_intf_pins pooling_accelerator_0/m_axi_gmem0] [get_bd_intf_pins axi_mem_interconnect/S07_AXI]
connect_bd_intf_net [get_bd_intf_pins pooling_accelerator_0/m_axi_gmem1] [get_bd_intf_pins axi_mem_interconnect/S08_AXI]

# Connect SmartConnect Master to Zynq HP0 (We can use just one HP port for simplicity, or distribute them)
# For simplicity, we route all through HP0. SmartConnect handles arbitration.
connect_bd_intf_net [get_bd_intf_pins axi_mem_interconnect/M00_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]


# 9. Control Connections (Zynq GP -> Interconnect -> HLS Slave)
connect_bd_intf_net [get_bd_intf_pins processing_system7_0/M_AXI_GP0] [get_bd_intf_pins ps7_0_axi_periph/S00_AXI]
connect_bd_intf_net [get_bd_intf_pins ps7_0_axi_periph/M00_AXI] [get_bd_intf_pins conv_accelerator_0/s_axi_control]
connect_bd_intf_net [get_bd_intf_pins ps7_0_axi_periph/M01_AXI] [get_bd_intf_pins activation_accelerator_0/s_axi_control]
connect_bd_intf_net [get_bd_intf_pins ps7_0_axi_periph/M02_AXI] [get_bd_intf_pins pooling_accelerator_0/s_axi_control]


# 10. Interrupts
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0
set_property -dict [list CONFIG.NUM_PORTS {3}] [get_bd_cells xlconcat_0]
connect_bd_net [get_bd_pins conv_accelerator_0/interrupt] [get_bd_pins xlconcat_0/In0]
connect_bd_net [get_bd_pins activation_accelerator_0/interrupt] [get_bd_pins xlconcat_0/In1]
connect_bd_net [get_bd_pins pooling_accelerator_0/interrupt] [get_bd_pins xlconcat_0/In2]
connect_bd_net [get_bd_pins xlconcat_0/dout] [get_bd_pins processing_system7_0/IRQ_F2P]


# 11. Address Assignment
assign_bd_address


# 12. Validate and Save
validate_bd_design
save_bd_design

puts "Block Design Created Successfully!"
puts "You can now open the project in Vivado GUI to verify:"
puts "  vivado vivado_project/zynq_cnn_system.xpr"

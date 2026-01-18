# Vivado Block Design Integration Guide

Now that HLS IP cores are generated, follow these steps to create the Zynq system.

## 1. Create Vivado Project
1. Open Vivado 2019.2
2. Create New Project -> "zynq_cnn_system"
3. Select your board (e.g., ZedBoard, Zybo Z7-20) or Part `xc7z020-clg484-1`

## 2. Add IP Repositories
1. Go to **Settings** -> **IP** -> **Repository**
2. Add the following paths:
   - `prjnew/hardware/hls/conv_accelerator/conv_accelerator_proj/solution1/impl/ip`
   - `prjnew/hardware/hls/activation/activation_accelerator_proj/solution1/impl/ip`
   - `prjnew/hardware/hls/pooling/pooling_accelerator_proj/solution1/impl/ip`
3. Click **Apply** (You should see 3 IPs detected)

## 3. Create Block Design
1. **Create Block Design** -> Name: `system`
2. Add **ZYNQ7 Processing System**
   - Run Block Automation (Apply Board Preset)
   - Enable High Performance AXI Slave Ports:
     - Double click Zynq IP -> PS-PL Configuration -> HP Slave AXI Interface
     - Enable `S AXI HP0` through `S AXI HP3` (we need 4 ports for conv accelerator)
   - Enable Interrupts:
     - Interrupts -> Fabric Interrupts -> Enable `IRQ_F2P` (PL to PS)

3. Add HLS IPs:
   - **Conv2D Accelerator**
   - **Activation Accelerator**
   - **Pooling Accelerator**

4. Add **AXI Interconnects**:
   - Add `AXI Interconnect` (for Master interfaces)
   - Configure for 4 Slave interfaces (from Conv) and 1 Master (to Zynq HP0)
   - *Note: You might need multiple interconnects or SmartConnects for better performance*

5. Add **AXI SmartConnect** (Recommended for HP ports):
   - Connect `gmem` ports of HLS IPs to Zynq `S_AXI_HPx` ports
   - **Conv Accelerator**:
     - `gmem0` (Input) -> SmartConnect -> `S_AXI_HP0`
     - `gmem1` (Weights) -> SmartConnect -> `S_AXI_HP1`
     - `gmem2` (Bias) -> SmartConnect -> `S_AXI_HP2`
     - `gmem3` (Output) -> SmartConnect -> `S_AXI_HP3`
   - **Activation/Pooling**:
     - Can share HP ports via SmartConnect

6. Connect Control Interfaces (`s_axi_control`):
   - Run **Connection Automation**
   - This will connect `s_axi_control` ports to Zynq `M_AXI_GP0` via AXI Interconnect

7. Connect Interrupts:
   - Add `Concat` IP
   - Connect `interrupt` lines from all 3 accelerators to Concat
   - Connect Concat output to Zynq `IRQ_F2P`

8. Connect Clocks/Resets:
   - Run **Connection Automation** to connect `ap_clk` and `ap_rst_n`

## 4. Generate Bitstream
1. **Validate Design** (F6) -> Fix any errors
2. In Sources tab, right-click `system.bd` -> **Create HDL Wrapper** (Let Vivado manage)
3. **Generate Bitstream**
4. **Export Hardware** -> Include Bitstream (XSA file)

## 5. Software Setup
Once you have the XSA file, we will update the software driver addresses in `mobilenet_config.h`.

**Next Step:** Let me know when you have generated the bitstream!

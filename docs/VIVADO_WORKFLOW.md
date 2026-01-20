# Vivado Workflow Guide - Zynq CNN Accelerator

This guide provides step-by-step instructions for building the FPGA hardware design in Vivado.

## Prerequisites

Before starting Vivado work, ensure you have:

- **Completed model quantization** (`models/quantized/` exists)
- **Vivado installed** (2020.2 or later)
- **Vitis HLS installed** (for IP core generation)
- **Target board** identified (e.g., ZedBoard, Zybo Z7-20)

## Workflow Overview

```mermaid
graph LR
    A[HLS Synthesis] --> B[Create Vivado Project]
    B --> C[Add Zynq PS]
    C --> D[Add HLS IPs]
    D --> E[Create Connections]
    E --> F[Add Constraints]
    F --> G[Generate Bitstream]
    G --> H[Export Hardware]
```

---

## Step 1: HLS IP Core Generation

First, generate the hardware accelerator IP cores using Vitis HLS.

### 1.1 Source Vivado/Vitis Environment

```bash
# Source Xilinx tools (adjust path to your installation)
source /tools/Xilinx/Vivado/2020.2/settings64.sh
source /tools/Xilinx/Vitis_HLS/2020.2/settings64.sh
```

### 1.2 Run HLS Synthesis

```bash
cd /home/jhush/prjnew/scripts
vitis_hls -f build_hls.tcl
```

This will:
- Synthesize the convolution accelerator
- Synthesize activation functions
- Synthesize pooling operations
- Export IP cores to `hardware/hls/*/solution1/impl/ip/`

**Expected time:** 20-30 minutes per accelerator

### 1.3 Verify HLS Output

Check that IP cores were generated:

```bash
ls -la ../hardware/hls/conv_accelerator/cnn_accelerator/solution1/impl/ip/
```

You should see `.zip` files containing the IP cores.

---

## Step 2: Create Vivado Project

### 2.1 Launch Vivado

```bash
cd /home/jhush/prjnew/hardware/vivado
vivado &
```

### 2.2 Create New Project

1. Click **Create Project**
2. Project name: `zynq_cnn_accelerator`
3. Project location: `/home/jhush/prjnew/hardware/vivado`
4. Project type: **RTL Project**
5. Check **Do not specify sources at this time**

### 2.3 Select Target Device

Choose your board:

**For ZedBoard:**
- Part: `xc7z020clg484-1`

**For Zybo Z7-20:**
- Part: `xc7z020clg400-1`

**For Zybo Z7-10:**
- Part: `xc7z010clg400-1`

Or use the **Boards** tab and select your board directly.

---

## Step 3: Add HLS IP Repository

### 3.1 Add IP Repository

1. Go to **Settings** → **IP** → **Repository**
2. Click **+** to add repository
3. Navigate to: `/home/jhush/prjnew/hardware/hls`
4. Click **Select**
5. Vivado will scan and find your HLS IP cores
6. Click **OK**

You should see:
- `Conv_accelerator v1.0`
- `Activation v1.0`
- `Pooling v1.0`

---

## Step 4: Create Block Design

### 4.1 Create Block Design

1. Click **Create Block Design**
2. Design name: `cnn_system`
3. Click **OK**

### 4.2 Add Zynq Processing System

1. Click **+** in the diagram
2. Search for: `ZYNQ7 Processing System`
3. Double-click to add
4. Click **Run Block Automation** (green banner)
5. Accept defaults and click **OK**

### 4.3 Configure Zynq PS

Double-click the **ZYNQ7 Processing System** block:

#### Enable AXI HP Ports (High Performance)
- **PS-PL Configuration** → **HP Slave AXI Interface**
- Enable: `S AXI HP0` (for DMA transfers)
- Enable: `S AXI HP1` (optional, for additional bandwidth)

#### Enable AXI GP Port (General Purpose)
- **PS-PL Configuration** → **GP Master AXI Interface**
- Enable: `M AXI GP0` (for register access)

#### Configure Interrupts
- **Interrupts** → **Fabric Interrupts** → **PL-PS Interrupt Ports**
- Enable: `IRQ_F2P[15:0]` (at least 3 interrupts for our accelerators)

#### Clock Configuration
- **Clock Configuration** → **PL Fabric Clocks**
- Enable: `FCLK_CLK0` → Set to **100 MHz** (main clock)
- Enable: `FCLK_CLK1` → Set to **200 MHz** (optional, for high-speed operations)

Click **OK** to save.

---

## Step 5: Add HLS Accelerator IP Cores

### 5.1 Add Convolution Accelerator

1. Click **+** in the diagram
2. Search for: `Conv_accelerator`
3. Double-click to add

### 5.2 Add Activation Accelerator

1. Click **+**
2. Search for: `Activation`
3. Double-click to add

### 5.3 Add Pooling Accelerator

1. Click **+**
2. Search for: `Pooling`
3. Double-click to add

---

## Step 6: Add AXI Interconnect

We need interconnects to connect the PS to the accelerators.

### 6.1 Add AXI Interconnect for Control

1. Click **+**
2. Search for: `AXI Interconnect`
3. Add to design
4. Rename to: `axi_interconnect_ctrl`
5. Double-click and configure:
   - **Number of Master Interfaces:** `3` (one for each accelerator)
   - **Number of Slave Interfaces:** `1` (from PS GP0)

### 6.2 Add AXI Interconnect for Memory

1. Add another **AXI Interconnect**
2. Rename to: `axi_interconnect_mem`
3. Configure:
   - **Number of Master Interfaces:** `1` (to PS HP0)
   - **Number of Slave Interfaces:** `3` (from each accelerator)

---

## Step 7: Make Connections

### 7.1 Connect Clocks

Connect `FCLK_CLK0` from Zynq PS to:
- All accelerator `ap_clk` ports
- Both AXI Interconnect `ACLK` and `S_ACLK` ports
- Both AXI Interconnect `M_ACLK` ports

### 7.2 Connect Resets

Connect `FCLK_RESET0_N` from Zynq PS to:
- All accelerator `ap_rst_n` ports
- Both AXI Interconnect `ARESETN` ports

### 7.3 Connect Control Path (AXI-Lite)

Connect for register access:

```
ZYNQ PS M_AXI_GP0 → axi_interconnect_ctrl → S_AXI
axi_interconnect_ctrl M00_AXI → Conv_accelerator s_axi_control
axi_interconnect_ctrl M01_AXI → Activation s_axi_control
axi_interconnect_ctrl M02_AXI → Pooling s_axi_control
```

### 7.4 Connect Data Path (AXI4)

Connect for memory access:

```
Conv_accelerator m_axi_gmem → axi_interconnect_mem S00_AXI
Activation m_axi_gmem → axi_interconnect_mem S01_AXI
Pooling m_axi_gmem → axi_interconnect_mem S02_AXI
axi_interconnect_mem M00_AXI → ZYNQ PS S_AXI_HP0
```

### 7.5 Connect Interrupts

Connect interrupt signals:

```
Conv_accelerator interrupt → ZYNQ PS IRQ_F2P[0]
Activation interrupt → ZYNQ PS IRQ_F2P[1]
Pooling interrupt → ZYNQ PS IRQ_F2P[2]
```

### 7.6 Run Connection Automation

1. Click **Run Connection Automation** (if available)
2. Select all connections
3. Click **OK**

---

## Step 8: Validate and Assign Addresses

### 8.1 Validate Design

1. Click **Validate Design** (F6)
2. Fix any errors or warnings
3. You should see: "Validation successful"

### 8.2 Assign Addresses

1. Go to **Address Editor** tab
2. Click **Auto Assign Addresses**
3. Note the base addresses (you'll need these in software):

Example addresses:
```
Conv_accelerator: 0x43C00000
Activation:       0x43C10000
Pooling:          0x43C20000
```

**Important:** Save these addresses - you'll need them in your driver code!

---

## Step 9: Create HDL Wrapper

### 9.1 Generate HDL Wrapper

1. In **Sources** tab, right-click on `cnn_system`
2. Select **Create HDL Wrapper**
3. Choose **Let Vivado manage wrapper and auto-update**
4. Click **OK**

This creates `cnn_system_wrapper.v` as the top-level module.

---

## Step 10: Add Constraints

### 10.1 Create Constraints File

Create a file: `/home/jhush/prjnew/hardware/constraints/timing.xdc`

```tcl
# Clock constraints
create_clock -period 10.000 -name clk_fpga_0 [get_pins "cnn_system_i/processing_system7_0/inst/PS7_i/FCLKCLK[0]"]

# Input/Output delays (adjust based on your board)
set_input_delay -clock clk_fpga_0 2.0 [all_inputs]
set_output_delay -clock clk_fpga_0 2.0 [all_outputs]

# False paths for reset
set_false_path -from [get_pins -hier *rst_n*]
```

### 10.2 Add Constraints to Project

1. In Vivado, click **Add Sources** (Alt+A)
2. Select **Add or create constraints**
3. Click **Add Files**
4. Navigate to `/home/jhush/prjnew/hardware/constraints/timing.xdc`
5. Click **Finish**

---

## Step 11: Run Synthesis

### 11.1 Start Synthesis

1. Click **Run Synthesis** in Flow Navigator
2. Wait for completion (15-30 minutes)

### 11.2 Review Synthesis Report

Check:
- **Resource Utilization:** Should be within device limits
- **Timing:** No critical warnings

Expected utilization (Zynq-7020):
- LUTs: ~70%
- FFs: ~45%
- BRAM: ~70%
- DSP48: ~80%

---

## Step 12: Run Implementation

### 12.1 Start Implementation

1. Click **Run Implementation**
2. Wait for completion (30-60 minutes)

### 12.2 Review Timing

1. Open **Implemented Design**
2. Go to **Reports** → **Timing** → **Report Timing Summary**
3. Verify: **All constraints met** (WNS ≥ 0)

If timing fails:
- Reduce clock frequency in PS configuration
- Add pipeline stages in HLS code
- Use different placement strategies

---

## Step 13: Generate Bitstream

### 13.1 Start Bitstream Generation

1. Click **Generate Bitstream**
2. Wait for completion (15-30 minutes)

### 13.2 Locate Bitstream

The bitstream will be at:
```
/home/jhush/prjnew/hardware/vivado/zynq_cnn_accelerator.runs/impl_1/cnn_system_wrapper.bit
```

---

## Step 14: Export Hardware

### 14.1 Export Hardware Definition

1. Go to **File** → **Export** → **Export Hardware**
2. Check **Include bitstream**
3. Export to: `/home/jhush/prjnew/hardware/vivado/cnn_system_wrapper.xsa`
4. Click **OK**

This `.xsa` file contains:
- Hardware description
- Bitstream
- Address map
- PS configuration

---

## Step 15: Extract Files for Deployment

### 15.1 Copy Bitstream

```bash
cp /home/jhush/prjnew/hardware/vivado/zynq_cnn_accelerator.runs/impl_1/cnn_system_wrapper.bit \
   /home/jhush/prjnew/hardware/vivado/design.bit
```

### 15.2 Note Memory Addresses

Document the base addresses from Address Editor:

Create `/home/jhush/prjnew/software/drivers/fpga_addresses.h`:

```c
#ifndef FPGA_ADDRESSES_H
#define FPGA_ADDRESSES_H

// Base addresses from Vivado Address Editor
#define CONV_ACCEL_BASE_ADDR    0x43C00000
#define ACTIVATION_BASE_ADDR    0x43C10000
#define POOLING_BASE_ADDR       0x43C20000

// Interrupt IDs
#define CONV_ACCEL_IRQ          61  // IRQ_F2P[0] = 61
#define ACTIVATION_IRQ          62  // IRQ_F2P[1] = 62
#define POOLING_IRQ             63  // IRQ_F2P[2] = 63

#endif // FPGA_ADDRESSES_H
```

---

## Verification Checklist

Before proceeding to software:

- [ ] Bitstream generated successfully
- [ ] Timing constraints met (WNS ≥ 0)
- [ ] Resource utilization within limits
- [ ] All AXI connections validated
- [ ] Base addresses documented
- [ ] Interrupt mappings documented
- [ ] `.xsa` file exported
- [ ] Bitstream copied to deployment location

---

## Next Steps

After completing Vivado:

1. **Update driver code** with correct base addresses
2. **Compile software** using the cross-compiler
3. **Prepare SD card** with bitstream and binaries
4. **Deploy to Zynq board** and test

See [GETTING_STARTED.md](GETTING_STARTED.md) for deployment instructions.

---

## Troubleshooting

### Synthesis Fails - Resource Overflow

**Problem:** Not enough LUTs/DSPs/BRAM

**Solution:**
- Reduce parallelism in HLS code (decrease `PE_NUM`)
- Use smaller tile sizes
- Disable unused accelerators

### Timing Not Met

**Problem:** WNS < 0 (negative slack)

**Solution:**
- Reduce clock frequency (e.g., 100 MHz → 75 MHz)
- Add more pipeline stages in HLS (`#pragma HLS PIPELINE`)
- Use different implementation strategy

### AXI Connection Errors

**Problem:** AXI protocol violations

**Solution:**
- Verify data widths match (32-bit or 64-bit)
- Check clock domains are properly connected
- Ensure all resets are connected

### IP Core Not Found

**Problem:** HLS IP not visible in Vivado

**Solution:**
- Verify HLS synthesis completed successfully
- Check IP repository path is correct
- Refresh IP catalog (Tools → Refresh IP Catalog)

---

## Useful Vivado Commands (Tcl Console)

```tcl
# Get resource utilization
report_utilization -file utilization.txt

# Get timing summary
report_timing_summary -file timing.txt

# Get power analysis
report_power -file power.txt

# List all IP cores
get_ips

# Get address map
report_bd_address_map
```

---

## Additional Resources

- [Vivado Design Suite User Guide](https://www.xilinx.com/support/documentation/sw_manuals/xilinx2020_2/ug893-vivado-ip-subsystems.pdf)
- [Zynq-7000 SoC Technical Reference Manual](https://www.xilinx.com/support/documentation/user_guides/ug585-Zynq-7000-TRM.pdf)
- [AXI Reference Guide](https://www.xilinx.com/support/documentation/ip_documentation/axi_ref_guide/latest/ug1037-vivado-axi-reference-guide.pdf)

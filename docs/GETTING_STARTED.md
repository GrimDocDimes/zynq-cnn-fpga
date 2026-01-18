# Getting Started Guide

This guide will walk you through the complete workflow from setup to deployment.

## Prerequisites Checklist

Before starting, ensure you have:

- [ ] Xilinx Zynq development board (ZedBoard, Zybo Z7, etc.)
- [ ] Xilinx Vivado/Vitis HLS installed (2020.2 or later)
- [ ] ARM cross-compilation toolchain
- [ ] Python 3.7+ with TensorFlow
- [ ] SD card (≥4GB) for board boot
- [ ] USB cable for UART console
- [ ] Network connection or USB camera (optional)

## Step-by-Step Workflow

### Step 1: Environment Setup

Run the setup script to verify your environment:

```bash
cd /home/jhush/prjnew
make setup
```

This will check for required tools and create necessary directories.

### Step 2: Model Quantization

Quantize the MobileNetV1 model to INT8 format:

```bash
make quantize
```

This generates:
- `models/quantized/weights/*.bin` - Quantized weight files
- `models/quantized/configs/quant_params.h` - Quantization parameters
- `models/quantized/configs/load_weights.cpp` - Weight loading code

**Expected time:** 5-10 minutes

### Step 3: HLS Synthesis

Synthesize the hardware accelerators:

```bash
make hls
```

This runs Vitis HLS to:
- Synthesize convolution accelerator
- Synthesize activation accelerator
- Synthesize pooling accelerator
- Export IP cores for Vivado

**Expected time:** 20-30 minutes per accelerator

### Step 4: Vivado Block Design (Manual)

1. Open Vivado:
   ```bash
   vivado &
   ```

2. Create new project targeting your Zynq device

3. Create block design:
   - Add Zynq Processing System IP
   - Configure PS (DDR, peripherals, clocks)
   - Add HLS IP cores from `hardware/hls/*/solution1/impl/ip/`
   - Connect AXI interfaces via AXI Interconnect
   - Connect interrupts to PS

4. Create HDL wrapper and add constraints

5. Run synthesis, implementation, and generate bitstream

**Expected time:** 1-2 hours (including place & route)

### Step 5: Software Compilation

Cross-compile the embedded software:

```bash
make software
```

This builds:
- `software/bin/cnn_inference_cpu` - CPU baseline
- `software/bin/cnn_inference_hw` - FPGA accelerated

**Expected time:** 2-5 minutes

### Step 6: SD Card Preparation

Prepare the SD card with:

1. **Boot partition (FAT32):**
   - `BOOT.bin` - First stage bootloader
   - `image.ub` - Linux kernel and device tree
   - `design.bit` - FPGA bitstream

2. **Root filesystem partition (ext4):**
   - Copy binaries: `cnn_inference_cpu`, `cnn_inference_hw`
   - Copy model weights: `models/quantized/weights/*`

### Step 7: Board Boot and Deployment

1. Insert SD card into Zynq board
2. Connect UART console (115200 baud)
3. Power on the board
4. Login (typically root/root)

5. Load FPGA bitstream:
   ```bash
   cat /mnt/design.bit > /dev/xdevcfg
   ```

6. Verify FPGA loaded:
   ```bash
   dmesg | grep xdevcfg
   ```

### Step 8: Run Inference

Test CPU baseline:
```bash
./cnn_inference_cpu
```

Test FPGA accelerated:
```bash
./cnn_inference_hw
```

### Step 9: Performance Benchmarking

Run automated benchmarks:
```bash
python3 run_benchmarks.py --iterations 100
```

This will:
- Run 100 iterations of each implementation
- Measure latency and throughput
- Calculate speedup
- Generate `benchmark_results.json`

## Troubleshooting

### HLS Synthesis Fails

**Issue:** Resource estimation exceeds device capacity

**Solution:**
- Reduce `PE_NUM` in `conv_accelerator.cpp`
- Reduce `SIMD_FACTOR`
- Use smaller tile sizes

### Bitstream Won't Load

**Issue:** `/dev/xdevcfg` not found

**Solution:**
- Check kernel config has FPGA manager enabled
- Use alternative: `cat design.bit > /sys/class/fpga_manager/fpga0/firmware`

### Software Crashes on FPGA Access

**Issue:** Segmentation fault when accessing FPGA registers

**Solution:**
- Verify bitstream is loaded
- Check memory addresses match Vivado address editor
- Ensure `/dev/mem` access permissions

### Poor Performance

**Issue:** FPGA not achieving expected speedup

**Solution:**
- Profile to identify bottlenecks
- Check for memory bandwidth saturation
- Verify DMA burst transfers are working
- Increase clock frequency if timing allows

## Next Steps

Once you have the basic system working:

1. **Optimize Performance:**
   - Tune HLS pragmas
   - Increase parallelism
   - Implement layer fusion

2. **Add Features:**
   - Camera input support
   - Real-time video processing
   - Multiple model support

3. **Measure Power:**
   - Use INA219 power monitor
   - Calculate energy per inference
   - Optimize for power efficiency

4. **Documentation:**
   - Record demo video
   - Document performance results
   - Create final presentation

## Useful Commands

```bash
# Check FPGA status
cat /sys/class/fpga_manager/fpga0/state

# Monitor system resources
htop

# Check memory usage
free -h

# View kernel messages
dmesg | tail -20

# Network deployment
scp bin/* root@192.168.1.100:/root/

# Quick rebuild
make clean-all && make software
```

## Resources

- [Vivado Design Suite User Guide](https://www.xilinx.com/support/documentation/sw_manuals/xilinx2020_2/ug893-vivado-ip-subsystems.pdf)
- [Vitis HLS User Guide](https://www.xilinx.com/support/documentation/sw_manuals/xilinx2020_2/ug1399-vitis-hls.pdf)
- [Zynq-7000 TRM](https://www.xilinx.com/support/documentation/user_guides/ug585-Zynq-7000-TRM.pdf)

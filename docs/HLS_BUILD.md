# HLS Build Instructions

## Prerequisites

Before running HLS synthesis, you need to source the Xilinx tools.

### Option 1: Automatic (Recommended)

Run the auto-detection script:

```bash
source scripts/source_xilinx.sh
```

This will automatically search for and source Vivado/Vitis HLS installations.

### Option 2: Manual

If you know your installation path, source it directly:

```bash
# For Vitis HLS (2020.2 and later)
source /path/to/Xilinx/Vitis_HLS/2023.2/settings64.sh

# For Vivado (includes older vivado_hls)
source /path/to/Xilinx/Vivado/2023.2/settings64.sh
```

Common installation paths:
- `/tools/Xilinx/Vitis_HLS/<version>/settings64.sh`
- `/opt/Xilinx/Vitis_HLS/<version>/settings64.sh`
- `$HOME/Xilinx/Vitis_HLS/<version>/settings64.sh`

### Verify Tools

After sourcing, verify the tools are available:

```bash
which vitis_hls
vitis_hls -version
```

## Running HLS Synthesis

### Build All Accelerators

To build all three accelerators (convolution, activation, pooling):

```bash
./scripts/build_all_hls.sh
```

This will:
1. Synthesize convolution accelerator (~10-15 minutes)
2. Synthesize activation accelerator (~5 minutes)
3. Synthesize pooling accelerator (~5 minutes)
4. Export IP cores for Vivado

**Total time:** ~20-30 minutes

### Build Individual Accelerators

To build just one accelerator:

```bash
# Convolution accelerator
cd hardware/hls/conv_accelerator
vitis_hls -f run_hls.tcl

# Activation accelerator
cd hardware/hls/activation
vitis_hls -f run_hls.tcl

# Pooling accelerator
cd hardware/hls/pooling
vitis_hls -f run_hls.tcl
```

## Output Locations

After successful synthesis, IP cores will be located at:

```
hardware/hls/conv_accelerator/conv_accelerator_proj/solution1/impl/ip/
hardware/hls/activation/activation_accelerator_proj/solution1/impl/ip/
hardware/hls/pooling/pooling_accelerator_proj/solution1/impl/ip/
```

## Synthesis Reports

Check synthesis reports for resource utilization and timing:

```bash
# Convolution accelerator report
cat hardware/hls/conv_accelerator/conv_accelerator_proj/solution1/syn/report/conv_accelerator_csynth.rpt

# Activation accelerator report
cat hardware/hls/activation/activation_accelerator_proj/solution1/syn/report/activation_accelerator_csynth.rpt

# Pooling accelerator report
cat hardware/hls/pooling/pooling_accelerator_proj/solution1/syn/report/pooling_accelerator_csynth.rpt
```

Key metrics to check:
- **Latency:** Clock cycles required
- **Interval:** Throughput (II = Initiation Interval)
- **Resources:** LUT, FF, BRAM, DSP usage
- **Timing:** Estimated clock period

## Troubleshooting

### Error: "vitis_hls: command not found"

**Solution:** Source the Xilinx tools first (see above)

### Error: "ap_int.h: No such file or directory"

**Solution:** Ensure you're using Vitis HLS 2020.2 or later. For older versions, use `vivado_hls` instead.

### Resource Estimation Exceeds Device

**Solution:** Reduce parallelism in the HLS code:
- Edit `conv_accelerator.cpp`
- Reduce `PE_NUM` (line 13) from 16 to 8
- Reduce `SIMD_FACTOR` (line 14) from 8 to 4
- Re-run synthesis

### Synthesis Takes Too Long

**Solution:** 
- Close other applications to free memory
- HLS synthesis is CPU-intensive and can take 15-30 minutes
- You can monitor progress in the console output

## Next Steps

After HLS synthesis completes successfully:

1. **Open Vivado** to create the FPGA design
2. **Add IP repositories** (Tools → Settings → IP → Repository)
3. **Create block design** with Zynq PS and HLS IP cores
4. **Run implementation** and generate bitstream
5. **Export hardware** for software development

See `docs/GETTING_STARTED.md` for detailed Vivado workflow.

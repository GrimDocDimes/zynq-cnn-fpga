# NixOS Development Environment Setup

This project includes a `shell.nix` configuration for NixOS users that provides all required Python dependencies.

## Quick Start

Enter the Nix shell environment:

```bash
nix-shell
```

This will automatically set up:
- Python 3.12
- TensorFlow 2.19.0
- NumPy 2.2.5
- OpenCV 4.11.0
- Matplotlib
- Pillow

## Running the Model Quantization

Once inside the nix-shell:

```bash
cd models
python quantize_model.py
```

## Running Commands Inside Nix Shell (One-liner)

If you want to run a command without entering the shell interactively:

```bash
nix-shell --run "cd models && python quantize_model.py"
```

## What's Included

The `shell.nix` file provides:
- **Python 3.12** with all ML dependencies
- **TensorFlow** for model loading and quantization
- **NumPy** for numerical operations
- **OpenCV** for image processing
- **Matplotlib** for visualization
- **Pillow** for image handling

## Optional: ARM Cross-Compilation

If you need ARM cross-compilation tools for the Zynq board, uncomment the following lines in `shell.nix`:

```nix
# pkgsCross.armv7l-hf-multiplatform.buildPackages.gcc
# pkgsCross.armv7l-hf-multiplatform.buildPackages.gdb
```

## Troubleshooting

### TensorFlow Warnings
You may see warnings like:
```
AttributeError: 'MessageFactory' object has no attribute 'GetPrototype'
```

These are harmless warnings and don't affect functionality.

### Performance Optimizations
TensorFlow is optimized for your CPU. You may see messages about AVX2, AVX512F, etc. These are informational only.

## Next Steps

After setting up the environment:
1. ✅ Run model quantization: `cd models && python quantize_model.py`
2. Build HLS IP cores (requires Vivado/Vitis HLS)
3. Create Vivado project and generate bitstream
4. Compile software for Zynq board
5. Deploy to hardware

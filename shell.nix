{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # Python and essential tools
    python312
    python312Packages.pip
    python312Packages.virtualenv
    
    # Python ML/Data Science packages
    python312Packages.numpy
    python312Packages.tensorflow
    python312Packages.keras
    python312Packages.opencv4
    python312Packages.matplotlib
    python312Packages.pillow
    
    # Development tools
    gnumake
    
    # ARM cross-compilation toolchain (optional, uncomment if needed)
    # pkgsCross.armv7l-hf-multiplatform.buildPackages.gcc
    # pkgsCross.armv7l-hf-multiplatform.buildPackages.gdb
  ];

  shellHook = ''
    echo "=== Zynq CNN Accelerator Development Environment ==="
    echo ""
    echo "Python version: $(python --version)"
    echo "NumPy version: $(python -c 'import numpy; print(numpy.__version__)')"
    echo "TensorFlow version: $(python -c 'import tensorflow; print(tensorflow.__version__)')"
    echo "OpenCV version: $(python -c 'import cv2; print(cv2.__version__)')"
    echo ""
    echo "Environment ready! You can now run:"
    echo "  cd models && python quantize_model.py"
    echo ""
    
    # Set up any additional environment variables if needed
    export PYTHONPATH="${PYTHONPATH:+$PYTHONPATH:}$PWD"
  '';
}

#!/usr/bin/env python3
"""
Model Quantization Script for FPGA Deployment
Converts a pre-trained floating-point CNN model to INT8 quantized format
"""

import numpy as np
import tensorflow as tf
from tensorflow import keras
import argparse
import os
import struct

def quantize_mobilenet_v1(model_path, output_dir):
    """
    Quantize MobileNetV1 model to INT8
    
    Args:
        model_path: Path to pre-trained model (.h5 or SavedModel)
        output_dir: Directory to save quantized weights
    """
    print(f"Loading model from {model_path}...")
    
    # Load pre-trained model
    if model_path.endswith('.h5'):
        model = keras.models.load_model(model_path)
    else:
        model = keras.applications.MobileNet(weights='imagenet', 
                                             input_shape=(224, 224, 3))
    
    print("Model loaded successfully")
    print(f"Total layers: {len(model.layers)}")
    
    # Create output directory
    os.makedirs(output_dir, exist_ok=True)
    os.makedirs(os.path.join(output_dir, 'weights'), exist_ok=True)
    os.makedirs(os.path.join(output_dir, 'configs'), exist_ok=True)
    
    # Quantization parameters
    quant_params = {}
    
    # Process each layer
    layer_idx = 0
    for layer in model.layers:
        if isinstance(layer, (keras.layers.Conv2D, keras.layers.DepthwiseConv2D)):
            print(f"\nProcessing layer: {layer.name}")
            
            # Get weights and biases
            weights = layer.get_weights()
            
            if len(weights) == 0:
                continue
                
            kernel = weights[0]  # Convolution kernel
            bias = weights[1] if len(weights) > 1 else np.zeros(kernel.shape[-1])
            
            # Compute quantization parameters
            kernel_min, kernel_max = kernel.min(), kernel.max()
            kernel_scale = (kernel_max - kernel_min) / 255.0
            kernel_zero_point = -int(kernel_min / kernel_scale)
            
            # Quantize kernel
            kernel_quantized = np.round(kernel / kernel_scale + kernel_zero_point)
            kernel_quantized = np.clip(kernel_quantized, -128, 127).astype(np.int8)
            
            # Quantize bias (use higher precision)
            bias_scale = kernel_scale * 0.007843  # Input scale
            bias_quantized = np.round(bias / bias_scale).astype(np.int32)
            
            # Save quantized weights
            kernel_file = os.path.join(output_dir, 'weights', f'layer_{layer_idx}_kernel.bin')
            bias_file = os.path.join(output_dir, 'weights', f'layer_{layer_idx}_bias.bin')
            
            kernel_quantized.tofile(kernel_file)
            bias_quantized.tofile(bias_file)
            
            # Store quantization parameters
            quant_params[layer.name] = {
                'layer_idx': layer_idx,
                'kernel_scale': kernel_scale,
                'kernel_zero_point': kernel_zero_point,
                'bias_scale': bias_scale,
                'kernel_shape': kernel.shape,
                'bias_shape': bias.shape
            }
            
            print(f"  Kernel shape: {kernel.shape}")
            print(f"  Kernel scale: {kernel_scale:.6f}")
            print(f"  Kernel zero point: {kernel_zero_point}")
            print(f"  Saved to {kernel_file}")
            
            layer_idx += 1
    
    # Generate C header file with quantization parameters
    generate_c_header(quant_params, os.path.join(output_dir, 'configs', 'quant_params.h'))
    
    # Generate weight loading code
    generate_weight_loader(quant_params, os.path.join(output_dir, 'configs', 'load_weights.cpp'))
    
    print(f"\nQuantization complete!")
    print(f"Total quantized layers: {layer_idx}")
    print(f"Output directory: {output_dir}")

def generate_c_header(quant_params, output_file):
    """Generate C header file with quantization parameters"""
    
    with open(output_file, 'w') as f:
        f.write("#ifndef QUANT_PARAMS_H\n")
        f.write("#define QUANT_PARAMS_H\n\n")
        f.write("#include <stdint.h>\n\n")
        
        f.write("// Quantization parameters for each layer\n")
        f.write("struct LayerQuantParams {\n")
        f.write("    float kernel_scale;\n")
        f.write("    int32_t kernel_zero_point;\n")
        f.write("    float bias_scale;\n")
        f.write("};\n\n")
        
        f.write(f"#define NUM_QUANTIZED_LAYERS {len(quant_params)}\n\n")
        
        f.write("const LayerQuantParams LAYER_QUANT_PARAMS[] = {\n")
        for layer_name, params in quant_params.items():
            f.write(f"    // {layer_name}\n")
            f.write(f"    {{{params['kernel_scale']:.8f}f, ")
            f.write(f"{params['kernel_zero_point']}, ")
            f.write(f"{params['bias_scale']:.8f}f}},\n")
        f.write("};\n\n")
        
        f.write("#endif // QUANT_PARAMS_H\n")
    
    print(f"Generated C header: {output_file}")

def generate_weight_loader(quant_params, output_file):
    """Generate C++ code to load quantized weights"""
    
    with open(output_file, 'w') as f:
        f.write("#include <fstream>\n")
        f.write("#include <vector>\n")
        f.write("#include <string>\n")
        f.write("#include <stdint.h>\n\n")
        
        f.write("// Load quantized weights from binary files\n")
        f.write("bool load_quantized_weights(const std::string& weights_dir) {\n")
        
        for layer_name, params in quant_params.items():
            layer_idx = params['layer_idx']
            kernel_shape = params['kernel_shape']
            bias_shape = params['bias_shape']
            
            kernel_size = np.prod(kernel_shape)
            bias_size = np.prod(bias_shape)
            
            f.write(f"\n    // Load {layer_name}\n")
            f.write(f"    std::vector<int8_t> kernel_{layer_idx}({kernel_size});\n")
            f.write(f"    std::vector<int32_t> bias_{layer_idx}({bias_size});\n")
            f.write(f"    \n")
            f.write(f"    std::ifstream kernel_file(weights_dir + \"/layer_{layer_idx}_kernel.bin\", std::ios::binary);\n")
            f.write(f"    kernel_file.read((char*)kernel_{layer_idx}.data(), {kernel_size});\n")
            f.write(f"    kernel_file.close();\n")
            f.write(f"    \n")
            f.write(f"    std::ifstream bias_file(weights_dir + \"/layer_{layer_idx}_bias.bin\", std::ios::binary);\n")
            f.write(f"    bias_file.read((char*)bias_{layer_idx}.data(), {bias_size} * sizeof(int32_t));\n")
            f.write(f"    bias_file.close();\n")
        
        f.write("\n    return true;\n")
        f.write("}\n")
    
    print(f"Generated weight loader: {output_file}")

def main():
    parser = argparse.ArgumentParser(description='Quantize CNN model for FPGA deployment')
    parser.add_argument('--model', type=str, default='imagenet',
                        help='Model path or "imagenet" to download pre-trained')
    parser.add_argument('--output', type=str, default='../models/quantized',
                        help='Output directory for quantized weights')
    
    args = parser.parse_args()
    
    quantize_mobilenet_v1(args.model, args.output)

if __name__ == '__main__':
    main()

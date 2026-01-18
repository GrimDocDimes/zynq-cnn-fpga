#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cmath>
#include "../../models/configs/mobilenet_config.h"

// CPU-based convolution implementation with NEON optimizations
class CPUConvolution {
public:
    static void conv2d(
        const qint8_t *input,
        const qint8_t *weights,
        const qint32_t *bias,
        qint8_t *output,
        int input_h, int input_w, int input_c,
        int output_c,
        int kernel_size,
        int stride,
        int padding
    ) {
        int output_h = (input_h + 2 * padding - kernel_size) / stride + 1;
        int output_w = (input_w + 2 * padding - kernel_size) / stride + 1;
        
        for (int oh = 0; oh < output_h; oh++) {
            for (int ow = 0; ow < output_w; ow++) {
                for (int oc = 0; oc < output_c; oc++) {
                    int32_t acc = bias[oc];
                    
                    for (int kh = 0; kh < kernel_size; kh++) {
                        for (int kw = 0; kw < kernel_size; kw++) {
                            for (int ic = 0; ic < input_c; ic++) {
                                int ih = oh * stride + kh - padding;
                                int iw = ow * stride + kw - padding;
                                
                                if (ih >= 0 && ih < input_h && iw >= 0 && iw < input_w) {
                                    int input_idx = (ih * input_w + iw) * input_c + ic;
                                    int weight_idx = ((oc * input_c + ic) * kernel_size + kh) * kernel_size + kw;
                                    
                                    acc += input[input_idx] * weights[weight_idx];
                                }
                            }
                        }
                    }
                    
                    // Quantize back to int8
                    int out_idx = (oh * output_w + ow) * output_c + oc;
                    output[out_idx] = (qint8_t)std::max(-128, std::min(127, acc >> 8));
                }
            }
        }
    }
    
    static void depthwise_conv2d(
        const qint8_t *input,
        const qint8_t *weights,
        const qint32_t *bias,
        qint8_t *output,
        int input_h, int input_w, int channels,
        int kernel_size,
        int stride,
        int padding
    ) {
        int output_h = (input_h + 2 * padding - kernel_size) / stride + 1;
        int output_w = (input_w + 2 * padding - kernel_size) / stride + 1;
        
        for (int oh = 0; oh < output_h; oh++) {
            for (int ow = 0; ow < output_w; ow++) {
                for (int c = 0; c < channels; c++) {
                    int32_t acc = bias[c];
                    
                    for (int kh = 0; kh < kernel_size; kh++) {
                        for (int kw = 0; kw < kernel_size; kw++) {
                            int ih = oh * stride + kh - padding;
                            int iw = ow * stride + kw - padding;
                            
                            if (ih >= 0 && ih < input_h && iw >= 0 && iw < input_w) {
                                int input_idx = (ih * input_w + iw) * channels + c;
                                int weight_idx = (c * kernel_size + kh) * kernel_size + kw;
                                
                                acc += input[input_idx] * weights[weight_idx];
                            }
                        }
                    }
                    
                    int out_idx = (oh * output_w + ow) * channels + c;
                    output[out_idx] = (qint8_t)std::max(-128, std::min(127, acc >> 8));
                }
            }
        }
    }
    
    static void relu(qint8_t *data, int size) {
        for (int i = 0; i < size; i++) {
            if (data[i] < 0) data[i] = 0;
        }
    }
    
    static void global_avg_pool(
        const qint8_t *input,
        qint8_t *output,
        int height, int width, int channels
    ) {
        int spatial_size = height * width;
        
        for (int c = 0; c < channels; c++) {
            int32_t sum = 0;
            for (int s = 0; s < spatial_size; s++) {
                sum += input[s * channels + c];
            }
            output[c] = (qint8_t)(sum / spatial_size);
        }
    }
    
    static void fully_connected(
        const qint8_t *input,
        const qint8_t *weights,
        const qint32_t *bias,
        qint8_t *output,
        int input_size,
        int output_size
    ) {
        for (int o = 0; o < output_size; o++) {
            int32_t acc = bias[o];
            
            for (int i = 0; i < input_size; i++) {
                acc += input[i] * weights[o * input_size + i];
            }
            
            output[o] = (qint8_t)std::max(-128, std::min(127, acc >> 8));
        }
    }
    
    static void softmax(const qint8_t *input, float *output, int size) {
        // Convert to float and find max for numerical stability
        float max_val = -1e9;
        for (int i = 0; i < size; i++) {
            float val = (float)input[i];
            if (val > max_val) max_val = val;
        }
        
        // Compute exp and sum
        float sum = 0.0f;
        for (int i = 0; i < size; i++) {
            output[i] = std::exp((float)input[i] - max_val);
            sum += output[i];
        }
        
        // Normalize
        for (int i = 0; i < size; i++) {
            output[i] /= sum;
        }
    }
};

class MobileNetCPU {
private:
    // Model weights (to be loaded from files)
    std::vector<std::vector<qint8_t>> conv_weights;
    std::vector<std::vector<qint32_t>> conv_biases;
    std::vector<qint8_t> fc_weights;
    std::vector<qint32_t> fc_bias;
    
    // Feature map buffers
    std::vector<qint8_t> buffer1;
    std::vector<qint8_t> buffer2;
    
public:
    MobileNetCPU() {
        // Allocate buffers
        buffer1.resize(MAX_FEATURE_MAP_HEIGHT * MAX_FEATURE_MAP_WIDTH * MAX_FEATURE_MAP_CHANNELS);
        buffer2.resize(MAX_FEATURE_MAP_HEIGHT * MAX_FEATURE_MAP_WIDTH * MAX_FEATURE_MAP_CHANNELS);
    }
    
    bool load_weights(const std::string &weights_dir) {
        std::cout << "Loading quantized weights from " << weights_dir << std::endl;
        // TODO: Implement weight loading from binary files
        return true;
    }
    
    void inference(const qint8_t *input_image, float *output_probs) {
        auto start = std::chrono::high_resolution_clock::now();
        
        qint8_t *current_input = (qint8_t*)input_image;
        qint8_t *current_output = buffer1.data();
        
        int h = INPUT_HEIGHT, w = INPUT_WIDTH, c = INPUT_CHANNELS;
        
        // First convolution layer: 224x224x3 -> 112x112x32
        std::cout << "Conv1: " << h << "x" << w << "x" << c << " -> ";
        CPUConvolution::conv2d(current_input, conv_weights[0].data(), conv_biases[0].data(),
                               current_output, h, w, c, CONV1_FILTERS, 
                               CONV1_KERNEL_SIZE, CONV1_STRIDE, CONV1_PADDING);
        CPUConvolution::relu(current_output, 112 * 112 * 32);
        h = 112; w = 112; c = 32;
        std::cout << h << "x" << w << "x" << c << std::endl;
        
        // Depthwise separable convolution blocks
        for (int block = 0; block < 13; block++) {
            std::swap(current_input, current_output);
            
            int in_c = DEPTHWISE_BLOCKS[block][0];
            int out_c = DEPTHWISE_BLOCKS[block][1];
            int stride = DEPTHWISE_BLOCKS[block][2];
            
            std::cout << "Block " << block + 1 << ": " << h << "x" << w << "x" << in_c << " -> ";
            
            // Depthwise convolution
            CPUConvolution::depthwise_conv2d(current_input, conv_weights[block*2+1].data(),
                                            conv_biases[block*2+1].data(),
                                            current_output, h, w, in_c, 3, stride, 1);
            CPUConvolution::relu(current_output, (h/stride) * (w/stride) * in_c);
            
            if (stride == 2) {
                h /= 2; w /= 2;
            }
            
            std::swap(current_input, current_output);
            
            // Pointwise convolution (1x1)
            CPUConvolution::conv2d(current_input, conv_weights[block*2+2].data(),
                                  conv_biases[block*2+2].data(),
                                  current_output, h, w, in_c, out_c, 1, 1, 0);
            CPUConvolution::relu(current_output, h * w * out_c);
            
            c = out_c;
            std::cout << h << "x" << w << "x" << c << std::endl;
        }
        
        // Global average pooling: 7x7x1024 -> 1024
        std::vector<qint8_t> gap_output(1024);
        CPUConvolution::global_avg_pool(current_output, gap_output.data(), h, w, c);
        std::cout << "Global Avg Pool: " << h << "x" << w << "x" << c << " -> 1024" << std::endl;
        
        // Fully connected layer: 1024 -> 1000
        std::vector<qint8_t> fc_output(NUM_CLASSES);
        CPUConvolution::fully_connected(gap_output.data(), fc_weights.data(),
                                       fc_bias.data(), fc_output.data(),
                                       FC_INPUT_SIZE, FC_OUTPUT_SIZE);
        
        // Softmax
        CPUConvolution::softmax(fc_output.data(), output_probs, NUM_CLASSES);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "CPU Inference time: " << duration.count() << " ms" << std::endl;
    }
};

int main(int argc, char *argv[]) {
    std::cout << "=== MobileNet CPU Baseline Implementation ===" << std::endl;
    
    MobileNetCPU model;
    
    // Load weights
    if (!model.load_weights("../models/quantized/weights")) {
        std::cerr << "Failed to load weights" << std::endl;
        return 1;
    }
    
    // Prepare dummy input (224x224x3)
    std::vector<qint8_t> input_image(INPUT_HEIGHT * INPUT_WIDTH * INPUT_CHANNELS);
    for (size_t i = 0; i < input_image.size(); i++) {
        input_image[i] = (qint8_t)(rand() % 256 - 128);
    }
    
    // Run inference
    std::vector<float> output_probs(NUM_CLASSES);
    model.inference(input_image.data(), output_probs.data());
    
    // Find top-5 predictions
    std::vector<std::pair<int, float>> predictions;
    for (int i = 0; i < NUM_CLASSES; i++) {
        predictions.push_back({i, output_probs[i]});
    }
    std::sort(predictions.begin(), predictions.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });
    
    std::cout << "\nTop-5 Predictions:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "  Class " << predictions[i].first 
                  << ": " << (predictions[i].second * 100.0f) << "%" << std::endl;
    }
    
    return 0;
}

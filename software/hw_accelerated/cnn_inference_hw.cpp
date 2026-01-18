#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include "../drivers/cnn_fpga_driver.h"
#include "../../models/configs/mobilenet_config.h"

class MobileNetFPGA {
private:
    CNNFPGADriver fpga;
    
    // Model weights
    std::vector<std::vector<qint8_t>> conv_weights;
    std::vector<std::vector<qint32_t>> conv_biases;
    std::vector<qint8_t> fc_weights;
    std::vector<qint32_t> fc_bias;
    
    // Feature map buffers
    std::vector<qint8_t> buffer1;
    std::vector<qint8_t> buffer2;
    
public:
    MobileNetFPGA() {
        buffer1.resize(MAX_FEATURE_MAP_HEIGHT * MAX_FEATURE_MAP_WIDTH * MAX_FEATURE_MAP_CHANNELS);
        buffer2.resize(MAX_FEATURE_MAP_HEIGHT * MAX_FEATURE_MAP_WIDTH * MAX_FEATURE_MAP_CHANNELS);
    }
    
    bool init() {
        std::cout << "Initializing FPGA accelerator..." << std::endl;
        if (!fpga.init()) {
            std::cerr << "Failed to initialize FPGA driver" << std::endl;
            return false;
        }
        std::cout << "FPGA initialized successfully" << std::endl;
        return true;
    }
    
    bool load_weights(const std::string &weights_dir) {
        std::cout << "Loading quantized weights from " << weights_dir << std::endl;
        // TODO: Implement weight loading
        return true;
    }
    
    void inference(const qint8_t *input_image, float *output_probs) {
        auto start = std::chrono::high_resolution_clock::now();
        
        qint8_t *current_input = (qint8_t*)input_image;
        qint8_t *current_output = buffer1.data();
        
        int h = INPUT_HEIGHT, w = INPUT_WIDTH, c = INPUT_CHANNELS;
        
        // First convolution layer: 224x224x3 -> 112x112x32 (FPGA)
        std::cout << "Conv1 (FPGA): " << h << "x" << w << "x" << c << " -> ";
        fpga.conv2d(current_input, conv_weights[0].data(), conv_biases[0].data(),
                   current_output, h, w, c, CONV1_FILTERS,
                   CONV1_KERNEL_SIZE, CONV1_STRIDE, CONV1_PADDING, true);
        h = 112; w = 112; c = 32;
        std::cout << h << "x" << w << "x" << c << std::endl;
        
        // Depthwise separable convolution blocks (FPGA accelerated)
        for (int block = 0; block < 13; block++) {
            std::swap(current_input, current_output);
            
            int in_c = DEPTHWISE_BLOCKS[block][0];
            int out_c = DEPTHWISE_BLOCKS[block][1];
            int stride = DEPTHWISE_BLOCKS[block][2];
            
            std::cout << "Block " << block + 1 << " (FPGA): " << h << "x" << w << "x" << in_c << " -> ";
            
            // Depthwise convolution (FPGA)
            fpga.conv2d(current_input, conv_weights[block*2+1].data(),
                       conv_biases[block*2+1].data(),
                       current_output, h, w, in_c, in_c, 3, stride, 1, true);
            
            if (stride == 2) {
                h /= 2; w /= 2;
            }
            
            std::swap(current_input, current_output);
            
            // Pointwise convolution (FPGA)
            fpga.conv2d(current_input, conv_weights[block*2+2].data(),
                       conv_biases[block*2+2].data(),
                       current_output, h, w, in_c, out_c, 1, 1, 0, true);
            
            c = out_c;
            std::cout << h << "x" << w << "x" << c << std::endl;
        }
        
        // Global average pooling (FPGA)
        std::vector<qint8_t> gap_output(1024);
        fpga.global_avg_pooling(current_output, gap_output.data(), h, w, c);
        std::cout << "Global Avg Pool (FPGA): " << h << "x" << w << "x" << c << " -> 1024" << std::endl;
        
        // Fully connected layer (CPU - small overhead)
        std::vector<qint8_t> fc_output(NUM_CLASSES);
        for (int o = 0; o < NUM_CLASSES; o++) {
            int32_t acc = fc_bias[o];
            for (int i = 0; i < FC_INPUT_SIZE; i++) {
                acc += gap_output[i] * fc_weights[o * FC_INPUT_SIZE + i];
            }
            fc_output[o] = (qint8_t)std::max(-128, std::min(127, acc >> 8));
        }
        
        // Softmax (CPU)
        float max_val = -1e9;
        for (int i = 0; i < NUM_CLASSES; i++) {
            float val = (float)fc_output[i];
            if (val > max_val) max_val = val;
        }
        
        float sum = 0.0f;
        for (int i = 0; i < NUM_CLASSES; i++) {
            output_probs[i] = std::exp((float)fc_output[i] - max_val);
            sum += output_probs[i];
        }
        
        for (int i = 0; i < NUM_CLASSES; i++) {
            output_probs[i] /= sum;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "FPGA Inference time: " << duration.count() << " ms" << std::endl;
    }
    
    void cleanup() {
        fpga.cleanup();
    }
};

int main(int argc, char *argv[]) {
    std::cout << "=== MobileNet FPGA-Accelerated Implementation ===" << std::endl;
    
    MobileNetFPGA model;
    
    // Initialize FPGA
    if (!model.init()) {
        return 1;
    }
    
    // Load weights
    if (!model.load_weights("../models/quantized/weights")) {
        std::cerr << "Failed to load weights" << std::endl;
        return 1;
    }
    
    // Prepare input image
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
    
    model.cleanup();
    
    return 0;
}

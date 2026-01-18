#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_stream.h"

// Fixed-point data types for HLS
typedef ap_fixed<16, 8> data_t;      // 16-bit fixed-point: 8 integer bits, 8 fractional bits
typedef ap_fixed<16, 8> weight_t;    // Weight data type
typedef ap_fixed<32, 16> acc_t;      // Accumulator type (wider to prevent overflow)

// Configuration parameters
#define MAX_KERNEL_SIZE 3
#define MAX_CHANNELS 1024
#define MAX_HEIGHT 224
#define MAX_WIDTH 224
#define PE_NUM 16                    // Number of parallel processing elements
#define SIMD_FACTOR 8                // SIMD parallelism

// Convolution layer configuration structure
struct ConvConfig {
    int input_height;
    int input_width;
    int input_channels;
    int output_channels;
    int kernel_size;
    int stride;
    int padding;
    bool use_relu;
};

// Line buffer for sliding window
template<int CHANNELS, int WIDTH, int KERNEL_SIZE>
class LineBuffer {
private:
    data_t buffer[KERNEL_SIZE-1][WIDTH][CHANNELS];
    
public:
    void shift_up() {
        #pragma HLS INLINE
        for (int k = 0; k < KERNEL_SIZE-2; k++) {
            #pragma HLS UNROLL
            for (int w = 0; w < WIDTH; w++) {
                for (int c = 0; c < CHANNELS; c++) {
                    #pragma HLS PIPELINE
                    buffer[k][w][c] = buffer[k+1][w][c];
                }
            }
        }
    }
    
    void insert_line(data_t line[WIDTH][CHANNELS]) {
        #pragma HLS INLINE
        for (int w = 0; w < WIDTH; w++) {
            for (int c = 0; c < CHANNELS; c++) {
                #pragma HLS PIPELINE
                buffer[KERNEL_SIZE-2][w][c] = line[w][c];
            }
        }
    }
    
    data_t get(int row, int col, int ch) {
        #pragma HLS INLINE
        return buffer[row][col][ch];
    }
};

// Convolution computation kernel
void conv2d_compute(
    data_t input_window[MAX_KERNEL_SIZE][MAX_KERNEL_SIZE][MAX_CHANNELS],
    weight_t weights[MAX_CHANNELS][MAX_CHANNELS][MAX_KERNEL_SIZE][MAX_KERNEL_SIZE],
    data_t bias[MAX_CHANNELS],
    data_t output[MAX_CHANNELS],
    ConvConfig config
) {
    #pragma HLS INLINE off
    
    // Initialize accumulators
    acc_t acc[MAX_CHANNELS];
    #pragma HLS ARRAY_PARTITION variable=acc complete
    
    for (int oc = 0; oc < config.output_channels; oc++) {
        #pragma HLS PIPELINE
        acc[oc] = bias[oc];
    }
    
    // Perform convolution
    for (int oc = 0; oc < config.output_channels; oc++) {
        for (int ic = 0; ic < config.input_channels; ic++) {
            for (int kh = 0; kh < config.kernel_size; kh++) {
                for (int kw = 0; kw < config.kernel_size; kw++) {
                    #pragma HLS PIPELINE II=1
                    acc[oc] += input_window[kh][kw][ic] * weights[oc][ic][kh][kw];
                }
            }
        }
    }
    
    // Apply activation and write output
    for (int oc = 0; oc < config.output_channels; oc++) {
        #pragma HLS PIPELINE
        data_t result = (data_t)acc[oc];
        if (config.use_relu && result < 0) {
            result = 0;
        }
        output[oc] = result;
    }
}

// Main convolution accelerator function
void conv_accelerator(
    data_t *input,           // Input feature map (DDR)
    weight_t *weights,       // Convolution weights (DDR)
    data_t *bias,            // Bias values (DDR)
    data_t *output,          // Output feature map (DDR)
    ConvConfig config        // Layer configuration
) {
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0 depth=50176
    #pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem1 depth=589824
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem2 depth=1024
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem3 depth=50176
    #pragma HLS INTERFACE s_axilite port=config bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control
    
    // Local buffers for tiling
    data_t input_buffer[MAX_HEIGHT][MAX_WIDTH][MAX_CHANNELS];
    #pragma HLS ARRAY_PARTITION variable=input_buffer cyclic factor=16 dim=3
    
    weight_t weight_buffer[MAX_CHANNELS][MAX_CHANNELS][MAX_KERNEL_SIZE][MAX_KERNEL_SIZE];
    #pragma HLS ARRAY_PARTITION variable=weight_buffer cyclic factor=16 dim=1
    
    data_t bias_buffer[MAX_CHANNELS];
    #pragma HLS ARRAY_PARTITION variable=bias_buffer complete
    
    // Load weights and biases (reused across all output pixels)
    LOAD_WEIGHTS:
    for (int oc = 0; oc < config.output_channels; oc++) {
        for (int ic = 0; ic < config.input_channels; ic++) {
            for (int kh = 0; kh < config.kernel_size; kh++) {
                for (int kw = 0; kw < config.kernel_size; kw++) {
                    #pragma HLS PIPELINE II=1
                    int idx = ((oc * config.input_channels + ic) * config.kernel_size + kh) * config.kernel_size + kw;
                    weight_buffer[oc][ic][kh][kw] = weights[idx];
                }
            }
        }
    }
    
    LOAD_BIAS:
    for (int oc = 0; oc < config.output_channels; oc++) {
        #pragma HLS PIPELINE
        bias_buffer[oc] = bias[oc];
    }
    
    // Calculate output dimensions
    int output_height = (config.input_height + 2 * config.padding - config.kernel_size) / config.stride + 1;
    int output_width = (config.input_width + 2 * config.padding - config.kernel_size) / config.stride + 1;
    
    // Sliding window convolution with line buffers
    int output_idx = 0;
    
    OUTPUT_HEIGHT:
    for (int oh = 0; oh < output_height; oh++) {
        OUTPUT_WIDTH:
        for (int ow = 0; ow < output_width; ow++) {
            #pragma HLS PIPELINE off
            
            // Extract input window
            data_t input_window[MAX_KERNEL_SIZE][MAX_KERNEL_SIZE][MAX_CHANNELS];
            #pragma HLS ARRAY_PARTITION variable=input_window complete dim=1
            #pragma HLS ARRAY_PARTITION variable=input_window complete dim=2
            
            int ih_base = oh * config.stride - config.padding;
            int iw_base = ow * config.stride - config.padding;
            
            WINDOW_HEIGHT:
            for (int kh = 0; kh < config.kernel_size; kh++) {
                WINDOW_WIDTH:
                for (int kw = 0; kw < config.kernel_size; kw++) {
                    int ih = ih_base + kh;
                    int iw = iw_base + kw;
                    
                    WINDOW_CHANNELS:
                    for (int ic = 0; ic < config.input_channels; ic++) {
                        #pragma HLS PIPELINE II=1
                        
                        // Handle padding
                        if (ih >= 0 && ih < config.input_height && iw >= 0 && iw < config.input_width) {
                            int input_idx = (ih * config.input_width + iw) * config.input_channels + ic;
                            input_window[kh][kw][ic] = input[input_idx];
                        } else {
                            input_window[kh][kw][ic] = 0;
                        }
                    }
                }
            }
            
            // Compute convolution for this output pixel
            data_t output_pixel[MAX_CHANNELS];
            #pragma HLS ARRAY_PARTITION variable=output_pixel complete
            
            conv2d_compute(input_window, weight_buffer, bias_buffer, output_pixel, config);
            
            // Write output
            WRITE_OUTPUT:
            for (int oc = 0; oc < config.output_channels; oc++) {
                #pragma HLS PIPELINE
                output[output_idx * config.output_channels + oc] = output_pixel[oc];
            }
            output_idx++;
        }
    }
}

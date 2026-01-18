#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_stream.h"

// Data types
typedef ap_fixed<16, 8> data_t;

// Pooling types
enum PoolingType {
    MAX_POOL,
    AVG_POOL
};

// Configuration structure
struct PoolConfig {
    int input_height;
    int input_width;
    int channels;
    int pool_size;      // Pooling window size (e.g., 2 for 2x2)
    int stride;
    int padding;
};

// Max pooling function
data_t max_pool_window(data_t window[], int size) {
    #pragma HLS INLINE
    data_t max_val = window[0];
    for (int i = 1; i < size; i++) {
        #pragma HLS UNROLL
        if (window[i] > max_val) {
            max_val = window[i];
        }
    }
    return max_val;
}

// Average pooling function
data_t avg_pool_window(data_t window[], int size) {
    #pragma HLS INLINE
    acc_t sum = 0;
    for (int i = 0; i < size; i++) {
        #pragma HLS UNROLL
        sum += window[i];
    }
    return (data_t)(sum / size);
}

// Main pooling accelerator
void pooling_accelerator(
    data_t *input,              // Input feature map
    data_t *output,             // Output feature map
    PoolConfig config,          // Pooling configuration
    PoolingType pool_type       // MAX_POOL or AVG_POOL
) {
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0 depth=50176
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1 depth=50176
    #pragma HLS INTERFACE s_axilite port=config bundle=control
    #pragma HLS INTERFACE s_axilite port=pool_type bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control
    
    // Calculate output dimensions
    int output_height = (config.input_height + 2 * config.padding - config.pool_size) / config.stride + 1;
    int output_width = (config.input_width + 2 * config.padding - config.pool_size) / config.stride + 1;
    
    int output_idx = 0;
    
    // Process each output position
    OUTPUT_HEIGHT:
    for (int oh = 0; oh < output_height; oh++) {
        OUTPUT_WIDTH:
        for (int ow = 0; ow < output_width; ow++) {
            CHANNELS:
            for (int c = 0; c < config.channels; c++) {
                #pragma HLS PIPELINE II=1
                
                // Extract pooling window
                data_t window[9];  // Maximum 3x3 window
                #pragma HLS ARRAY_PARTITION variable=window complete
                
                int window_idx = 0;
                int ih_base = oh * config.stride - config.padding;
                int iw_base = ow * config.stride - config.padding;
                
                POOL_HEIGHT:
                for (int ph = 0; ph < config.pool_size; ph++) {
                    POOL_WIDTH:
                    for (int pw = 0; pw < config.pool_size; pw++) {
                        int ih = ih_base + ph;
                        int iw = iw_base + pw;
                        
                        // Handle padding
                        if (ih >= 0 && ih < config.input_height && 
                            iw >= 0 && iw < config.input_width) {
                            int input_idx = (ih * config.input_width + iw) * config.channels + c;
                            window[window_idx] = input[input_idx];
                        } else {
                            // For max pooling, use minimum value; for avg pooling, use 0
                            window[window_idx] = (pool_type == MAX_POOL) ? -128 : 0;
                        }
                        window_idx++;
                    }
                }
                
                // Compute pooling result
                data_t result;
                int window_size = config.pool_size * config.pool_size;
                
                if (pool_type == MAX_POOL) {
                    result = max_pool_window(window, window_size);
                } else {
                    result = avg_pool_window(window, window_size);
                }
                
                output[output_idx * config.channels + c] = result;
            }
            output_idx++;
        }
    }
}

// Global average pooling (commonly used in MobileNet)
void global_avg_pooling(
    data_t *input,              // Input feature map [H x W x C]
    data_t *output,             // Output vector [C]
    int height,
    int width,
    int channels
) {
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=height bundle=control
    #pragma HLS INTERFACE s_axilite port=width bundle=control
    #pragma HLS INTERFACE s_axilite port=channels bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control
    
    int spatial_size = height * width;
    
    CHANNELS:
    for (int c = 0; c < channels; c++) {
        #pragma HLS PIPELINE off
        
        acc_t sum = 0;
        
        SPATIAL:
        for (int s = 0; s < spatial_size; s++) {
            #pragma HLS PIPELINE II=1
            sum += input[s * channels + c];
        }
        
        output[c] = (data_t)(sum / spatial_size);
    }
}

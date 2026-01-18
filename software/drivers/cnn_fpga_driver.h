#ifndef CNN_FPGA_DRIVER_H
#define CNN_FPGA_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "../models/configs/mobilenet_config.h"

// FPGA driver class for hardware-accelerated CNN operations
class CNNFPGADriver {
private:
    int mem_fd;                     // File descriptor for /dev/mem
    void *fpga_base;                // Mapped FPGA register base address
    void *dma_base;                 // Mapped DMA controller base
    
    // Memory buffers (physically contiguous)
    void *input_buffer_phys;
    void *output_buffer_phys;
    void *weight_buffer_phys;
    
    void *input_buffer_virt;
    void *output_buffer_virt;
    void *weight_buffer_virt;
    
    size_t buffer_size;
    
    // Helper functions
    void* map_physical_memory(uint32_t addr, size_t size);
    void unmap_memory(void *addr, size_t size);
    void* allocate_contiguous_memory(size_t size, void **phys_addr);
    void free_contiguous_memory(void *virt_addr, size_t size);
    
    void write_reg(uint32_t offset, uint32_t value);
    uint32_t read_reg(uint32_t offset);
    
    void wait_for_completion();
    
public:
    CNNFPGADriver();
    ~CNNFPGADriver();
    
    // Initialization and cleanup
    bool init();
    void cleanup();
    
    // Layer execution functions
    bool conv2d(
        const qint8_t *input,
        const qint8_t *weights,
        const qint32_t *bias,
        qint8_t *output,
        int input_h, int input_w, int input_c,
        int output_c,
        int kernel_size,
        int stride,
        int padding,
        bool use_relu
    );
    
    bool depthwise_conv2d(
        const qint8_t *input,
        const qint8_t *weights,
        const qint32_t *bias,
        qint8_t *output,
        int input_h, int input_w, int channels,
        int kernel_size,
        int stride,
        int padding
    );
    
    bool pointwise_conv2d(
        const qint8_t *input,
        const qint8_t *weights,
        const qint32_t *bias,
        qint8_t *output,
        int input_h, int input_w,
        int input_c, int output_c
    );
    
    bool activation(
        const qint8_t *input,
        qint8_t *output,
        int size,
        int act_type  // 0=ReLU, 1=ReLU6
    );
    
    bool max_pooling(
        const qint8_t *input,
        qint8_t *output,
        int input_h, int input_w, int channels,
        int pool_size,
        int stride
    );
    
    bool global_avg_pooling(
        const qint8_t *input,
        qint8_t *output,
        int height, int width, int channels
    );
    
    // Performance monitoring
    uint64_t get_cycle_count();
    void reset_cycle_counter();
};

#endif // CNN_FPGA_DRIVER_H

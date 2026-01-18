#include "cnn_fpga_driver.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define PAGE_SIZE 4096
#define MAP_SIZE (PAGE_SIZE * 256)

CNNFPGADriver::CNNFPGADriver() 
    : mem_fd(-1), fpga_base(nullptr), dma_base(nullptr),
      input_buffer_phys(nullptr), output_buffer_phys(nullptr), 
      weight_buffer_phys(nullptr),
      input_buffer_virt(nullptr), output_buffer_virt(nullptr),
      weight_buffer_virt(nullptr),
      buffer_size(CONV_INPUT_BUFFER_SIZE * sizeof(qint8_t)) {
}

CNNFPGADriver::~CNNFPGADriver() {
    cleanup();
}

bool CNNFPGADriver::init() {
    // Open /dev/mem for memory-mapped I/O
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        std::cerr << "Failed to open /dev/mem" << std::endl;
        return false;
    }
    
    // Map FPGA register space
    fpga_base = map_physical_memory(FPGA_BASE_ADDR, MAP_SIZE);
    if (fpga_base == nullptr) {
        std::cerr << "Failed to map FPGA registers" << std::endl;
        return false;
    }
    
    // Allocate physically contiguous buffers for DMA
    input_buffer_virt = allocate_contiguous_memory(buffer_size, &input_buffer_phys);
    output_buffer_virt = allocate_contiguous_memory(buffer_size, &output_buffer_phys);
    weight_buffer_virt = allocate_contiguous_memory(WEIGHT_BUFFER_SIZE * sizeof(qint8_t), 
                                                     &weight_buffer_phys);
    
    if (!input_buffer_virt || !output_buffer_virt || !weight_buffer_virt) {
        std::cerr << "Failed to allocate DMA buffers" << std::endl;
        return false;
    }
    
    // Reset FPGA accelerator
    write_reg(CONV_CTRL_REG, CTRL_RESET_BIT);
    usleep(1000);
    write_reg(CONV_CTRL_REG, 0);
    
    std::cout << "FPGA driver initialized successfully" << std::endl;
    return true;
}

void CNNFPGADriver::cleanup() {
    if (fpga_base) {
        unmap_memory(fpga_base, MAP_SIZE);
        fpga_base = nullptr;
    }
    
    if (input_buffer_virt) {
        free_contiguous_memory(input_buffer_virt, buffer_size);
        input_buffer_virt = nullptr;
    }
    
    if (output_buffer_virt) {
        free_contiguous_memory(output_buffer_virt, buffer_size);
        output_buffer_virt = nullptr;
    }
    
    if (weight_buffer_virt) {
        free_contiguous_memory(weight_buffer_virt, WEIGHT_BUFFER_SIZE * sizeof(qint8_t));
        weight_buffer_virt = nullptr;
    }
    
    if (mem_fd >= 0) {
        close(mem_fd);
        mem_fd = -1;
    }
}

void* CNNFPGADriver::map_physical_memory(uint32_t addr, size_t size) {
    void *mapped = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, 
                        mem_fd, addr);
    if (mapped == MAP_FAILED) {
        return nullptr;
    }
    return mapped;
}

void CNNFPGADriver::unmap_memory(void *addr, size_t size) {
    if (addr) {
        munmap(addr, size);
    }
}

void* CNNFPGADriver::allocate_contiguous_memory(size_t size, void **phys_addr) {
    // In a real implementation, this would use CMA (Contiguous Memory Allocator)
    // or UIO (Userspace I/O) framework
    // For now, using regular malloc (not truly contiguous)
    // TODO: Implement proper CMA allocation via /dev/udmabuf or similar
    
    void *virt = malloc(size);
    if (virt) {
        memset(virt, 0, size);
        // In real implementation, get physical address via /proc/self/pagemap
        *phys_addr = virt;  // Placeholder
    }
    return virt;
}

void CNNFPGADriver::free_contiguous_memory(void *virt_addr, size_t size) {
    if (virt_addr) {
        free(virt_addr);
    }
}

void CNNFPGADriver::write_reg(uint32_t offset, uint32_t value) {
    if (fpga_base) {
        *((volatile uint32_t*)((char*)fpga_base + offset)) = value;
    }
}

uint32_t CNNFPGADriver::read_reg(uint32_t offset) {
    if (fpga_base) {
        return *((volatile uint32_t*)((char*)fpga_base + offset));
    }
    return 0;
}

void CNNFPGADriver::wait_for_completion() {
    // Poll status register until done bit is set
    while (!(read_reg(CONV_STATUS_REG) & STATUS_DONE_BIT)) {
        usleep(10);
    }
}

bool CNNFPGADriver::conv2d(
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
) {
    // Copy input data to DMA buffer
    size_t input_size = input_h * input_w * input_c * sizeof(qint8_t);
    memcpy(input_buffer_virt, input, input_size);
    
    // Copy weights to DMA buffer
    size_t weight_size = output_c * input_c * kernel_size * kernel_size * sizeof(qint8_t);
    memcpy(weight_buffer_virt, weights, weight_size);
    
    // Configure accelerator
    write_reg(CONV_INPUT_ADDR_REG, (uint32_t)(uintptr_t)input_buffer_phys);
    write_reg(CONV_OUTPUT_ADDR_REG, (uint32_t)(uintptr_t)output_buffer_phys);
    write_reg(CONV_WEIGHT_ADDR_REG, (uint32_t)(uintptr_t)weight_buffer_phys);
    
    // Pack configuration into register
    uint32_t config = (input_h << 24) | (input_w << 16) | (input_c << 8) | output_c;
    write_reg(CONV_CONFIG_REG, config);
    
    // Start computation
    write_reg(CONV_CTRL_REG, CTRL_START_BIT);
    
    // Wait for completion
    wait_for_completion();
    
    // Copy output back
    int output_h = (input_h + 2 * padding - kernel_size) / stride + 1;
    int output_w = (input_w + 2 * padding - kernel_size) / stride + 1;
    size_t output_size = output_h * output_w * output_c * sizeof(qint8_t);
    memcpy(output, output_buffer_virt, output_size);
    
    return true;
}

bool CNNFPGADriver::activation(
    const qint8_t *input,
    qint8_t *output,
    int size,
    int act_type
) {
    // Simple ReLU can be done efficiently in software for small sizes
    // For large sizes, use FPGA accelerator
    for (int i = 0; i < size; i++) {
        if (act_type == 0) {  // ReLU
            output[i] = (input[i] > 0) ? input[i] : 0;
        } else if (act_type == 1) {  // ReLU6
            qint8_t val = (input[i] > 0) ? input[i] : 0;
            output[i] = (val < 6) ? val : 6;
        }
    }
    return true;
}

bool CNNFPGADriver::max_pooling(
    const qint8_t *input,
    qint8_t *output,
    int input_h, int input_w, int channels,
    int pool_size,
    int stride
) {
    int output_h = (input_h - pool_size) / stride + 1;
    int output_w = (input_w - pool_size) / stride + 1;
    
    for (int oh = 0; oh < output_h; oh++) {
        for (int ow = 0; ow < output_w; ow++) {
            for (int c = 0; c < channels; c++) {
                qint8_t max_val = -128;
                
                for (int ph = 0; ph < pool_size; ph++) {
                    for (int pw = 0; pw < pool_size; pw++) {
                        int ih = oh * stride + ph;
                        int iw = ow * stride + pw;
                        int idx = (ih * input_w + iw) * channels + c;
                        
                        if (input[idx] > max_val) {
                            max_val = input[idx];
                        }
                    }
                }
                
                int out_idx = (oh * output_w + ow) * channels + c;
                output[out_idx] = max_val;
            }
        }
    }
    
    return true;
}

bool CNNFPGADriver::global_avg_pooling(
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
    
    return true;
}

uint64_t CNNFPGADriver::get_cycle_count() {
    // Read hardware cycle counter if available
    // Otherwise return 0
    return 0;
}

void CNNFPGADriver::reset_cycle_counter() {
    // Reset hardware cycle counter
}

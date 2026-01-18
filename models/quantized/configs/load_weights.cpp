#include <fstream>
#include <vector>
#include <string>
#include <stdint.h>

// Load quantized weights from binary files
bool load_quantized_weights(const std::string& weights_dir) {

    // Load conv1
    std::vector<int8_t> kernel_0(864);
    std::vector<int32_t> bias_0(32);
    
    std::ifstream kernel_file(weights_dir + "/layer_0_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_0.data(), 864);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_0_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_0.data(), 32 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_1
    std::vector<int8_t> kernel_1(288);
    std::vector<int32_t> bias_1(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_1_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_1.data(), 288);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_1_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_1.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_1
    std::vector<int8_t> kernel_2(2048);
    std::vector<int32_t> bias_2(64);
    
    std::ifstream kernel_file(weights_dir + "/layer_2_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_2.data(), 2048);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_2_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_2.data(), 64 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_2
    std::vector<int8_t> kernel_3(576);
    std::vector<int32_t> bias_3(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_3_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_3.data(), 576);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_3_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_3.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_2
    std::vector<int8_t> kernel_4(8192);
    std::vector<int32_t> bias_4(128);
    
    std::ifstream kernel_file(weights_dir + "/layer_4_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_4.data(), 8192);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_4_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_4.data(), 128 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_3
    std::vector<int8_t> kernel_5(1152);
    std::vector<int32_t> bias_5(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_5_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_5.data(), 1152);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_5_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_5.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_3
    std::vector<int8_t> kernel_6(16384);
    std::vector<int32_t> bias_6(128);
    
    std::ifstream kernel_file(weights_dir + "/layer_6_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_6.data(), 16384);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_6_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_6.data(), 128 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_4
    std::vector<int8_t> kernel_7(1152);
    std::vector<int32_t> bias_7(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_7_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_7.data(), 1152);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_7_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_7.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_4
    std::vector<int8_t> kernel_8(32768);
    std::vector<int32_t> bias_8(256);
    
    std::ifstream kernel_file(weights_dir + "/layer_8_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_8.data(), 32768);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_8_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_8.data(), 256 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_5
    std::vector<int8_t> kernel_9(2304);
    std::vector<int32_t> bias_9(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_9_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_9.data(), 2304);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_9_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_9.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_5
    std::vector<int8_t> kernel_10(65536);
    std::vector<int32_t> bias_10(256);
    
    std::ifstream kernel_file(weights_dir + "/layer_10_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_10.data(), 65536);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_10_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_10.data(), 256 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_6
    std::vector<int8_t> kernel_11(2304);
    std::vector<int32_t> bias_11(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_11_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_11.data(), 2304);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_11_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_11.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_6
    std::vector<int8_t> kernel_12(131072);
    std::vector<int32_t> bias_12(512);
    
    std::ifstream kernel_file(weights_dir + "/layer_12_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_12.data(), 131072);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_12_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_12.data(), 512 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_7
    std::vector<int8_t> kernel_13(4608);
    std::vector<int32_t> bias_13(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_13_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_13.data(), 4608);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_13_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_13.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_7
    std::vector<int8_t> kernel_14(262144);
    std::vector<int32_t> bias_14(512);
    
    std::ifstream kernel_file(weights_dir + "/layer_14_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_14.data(), 262144);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_14_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_14.data(), 512 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_8
    std::vector<int8_t> kernel_15(4608);
    std::vector<int32_t> bias_15(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_15_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_15.data(), 4608);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_15_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_15.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_8
    std::vector<int8_t> kernel_16(262144);
    std::vector<int32_t> bias_16(512);
    
    std::ifstream kernel_file(weights_dir + "/layer_16_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_16.data(), 262144);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_16_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_16.data(), 512 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_9
    std::vector<int8_t> kernel_17(4608);
    std::vector<int32_t> bias_17(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_17_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_17.data(), 4608);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_17_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_17.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_9
    std::vector<int8_t> kernel_18(262144);
    std::vector<int32_t> bias_18(512);
    
    std::ifstream kernel_file(weights_dir + "/layer_18_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_18.data(), 262144);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_18_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_18.data(), 512 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_10
    std::vector<int8_t> kernel_19(4608);
    std::vector<int32_t> bias_19(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_19_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_19.data(), 4608);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_19_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_19.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_10
    std::vector<int8_t> kernel_20(262144);
    std::vector<int32_t> bias_20(512);
    
    std::ifstream kernel_file(weights_dir + "/layer_20_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_20.data(), 262144);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_20_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_20.data(), 512 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_11
    std::vector<int8_t> kernel_21(4608);
    std::vector<int32_t> bias_21(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_21_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_21.data(), 4608);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_21_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_21.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_11
    std::vector<int8_t> kernel_22(262144);
    std::vector<int32_t> bias_22(512);
    
    std::ifstream kernel_file(weights_dir + "/layer_22_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_22.data(), 262144);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_22_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_22.data(), 512 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_12
    std::vector<int8_t> kernel_23(4608);
    std::vector<int32_t> bias_23(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_23_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_23.data(), 4608);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_23_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_23.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_12
    std::vector<int8_t> kernel_24(524288);
    std::vector<int32_t> bias_24(1024);
    
    std::ifstream kernel_file(weights_dir + "/layer_24_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_24.data(), 524288);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_24_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_24.data(), 1024 * sizeof(int32_t));
    bias_file.close();

    // Load conv_dw_13
    std::vector<int8_t> kernel_25(9216);
    std::vector<int32_t> bias_25(1);
    
    std::ifstream kernel_file(weights_dir + "/layer_25_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_25.data(), 9216);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_25_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_25.data(), 1 * sizeof(int32_t));
    bias_file.close();

    // Load conv_pw_13
    std::vector<int8_t> kernel_26(1048576);
    std::vector<int32_t> bias_26(1024);
    
    std::ifstream kernel_file(weights_dir + "/layer_26_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_26.data(), 1048576);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_26_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_26.data(), 1024 * sizeof(int32_t));
    bias_file.close();

    // Load conv_preds
    std::vector<int8_t> kernel_27(1024000);
    std::vector<int32_t> bias_27(1000);
    
    std::ifstream kernel_file(weights_dir + "/layer_27_kernel.bin", std::ios::binary);
    kernel_file.read((char*)kernel_27.data(), 1024000);
    kernel_file.close();
    
    std::ifstream bias_file(weights_dir + "/layer_27_bias.bin", std::ios::binary);
    bias_file.read((char*)bias_27.data(), 1000 * sizeof(int32_t));
    bias_file.close();

    return true;
}

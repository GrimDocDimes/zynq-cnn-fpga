#ifndef QUANT_PARAMS_H
#define QUANT_PARAMS_H

#include <stdint.h>

// Quantization parameters for each layer
struct LayerQuantParams {
    float kernel_scale;
    int32_t kernel_zero_point;
    float bias_scale;
};

#define NUM_QUANTIZED_LAYERS 28

const LayerQuantParams LAYER_QUANT_PARAMS[] = {
    // conv1
    {0.00619981f, 141, 0.00004863f},
    // conv_dw_1
    {0.21418893f, 142, 0.00167988f},
    // conv_pw_1
    {0.00902165f, 123, 0.00007076f},
    // conv_dw_2
    {0.04947438f, 127, 0.00038803f},
    // conv_pw_2
    {0.00735773f, 119, 0.00005771f},
    // conv_dw_3
    {0.09209553f, 116, 0.00072231f},
    // conv_pw_3
    {0.01057674f, 107, 0.00008295f},
    // conv_dw_4
    {0.02119185f, 119, 0.00016621f},
    // conv_pw_4
    {0.00803924f, 94, 0.00006305f},
    // conv_dw_5
    {0.05459432f, 113, 0.00042818f},
    // conv_pw_5
    {0.00605461f, 135, 0.00004749f},
    // conv_dw_6
    {0.01804627f, 128, 0.00014154f},
    // conv_pw_6
    {0.00445892f, 122, 0.00003497f},
    // conv_dw_7
    {0.03906782f, 116, 0.00030641f},
    // conv_pw_7
    {0.00446904f, 130, 0.00003505f},
    // conv_dw_8
    {0.03754882f, 92, 0.00029450f},
    // conv_pw_8
    {0.00506053f, 103, 0.00003969f},
    // conv_dw_9
    {0.03510193f, 113, 0.00027530f},
    // conv_pw_9
    {0.00566147f, 121, 0.00004440f},
    // conv_dw_10
    {0.04128246f, 124, 0.00032378f},
    // conv_pw_10
    {0.00555322f, 79, 0.00004355f},
    // conv_dw_11
    {0.03710376f, 111, 0.00029100f},
    // conv_pw_11
    {0.00645269f, 81, 0.00005061f},
    // conv_dw_12
    {0.01618805f, 139, 0.00012696f},
    // conv_pw_12
    {0.00590253f, 139, 0.00004629f},
    // conv_dw_13
    {0.02020570f, 121, 0.00015847f},
    // conv_pw_13
    {0.00385759f, 130, 0.00003026f},
    // conv_preds
    {0.00604454f, 65, 0.00004741f},
};

#endif // QUANT_PARAMS_H

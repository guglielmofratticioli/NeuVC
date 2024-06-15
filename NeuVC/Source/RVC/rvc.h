#pragma once


#include <torch/torch.h>
#include <torch/script.h>
#include <c10/util/Optional.h>
#include <c10/core/DeviceType.h>


class RVC
{
public:
    // costructors
    RVC();
    std::vector<float> voiceConversion(const std::vector<float>& buffer_audio);
    void setDevice(const std::string &device);
    void load_net_g(const std::string &net_g_path);
    void transpose_f0(const std::int16_t &f0upKey);

private:
    at::Device device; 
    std::int16_t window;
    std::int16_t sr;
    std::int16_t x_pad;
    std::int16_t sid;
    std::int16_t tg_sr;
    std::int16_t f0upKey;
    at::Tensor pitch;
    at::Tensor pitchf;
    torch::jit::script::Module hubert_model;
    torch::jit::script::Module rmvpe_model;
    torch::jit::script::Module net_g;
    std::vector<torch::Tensor> butter_coeff;
    std::vector<torch::Tensor> trainInputs;
    std::vector<torch::Tensor> trainOutputs;
    torch::Tensor voiceConversion(torch::Tensor& buffer_audio);
    torch::Tensor get_vc(torch::Tensor &audio);
    void set_f0(const torch::Tensor& audio_padded);

};


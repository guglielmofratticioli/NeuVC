#include <iostream>
#include <cstdlib>
#include <memory>
#include "rvc.h"

namespace F = torch::nn::functional;

RVC::RVC()
: window{160}, sr{16000}, x_pad{16000}, device{torch::kCPU}, sid{0}, f0upKey{0}, tg_sr{40000}
{
    
    try
    {
        //hubert_model = torch::jit::load("../assets/hubert_traced.pt");
        //hubert_model = torch::jit::load("../../assets/hubert_traced.pt");
        hubert_model = torch::jit::load("assets\\hubert_traced2.pt");
        //hubert_model.to(device);
        //rmvpe_model = torch::jit::load("..\\..\\assets\\rmvpe_traced.pt");
        rmvpe_model = torch::jit::load("assets\\rmvpe_traced.pt");
        //rmvpe_model = torch::jit::load("..\\assets\\rmvpe_traced.pt");
        rmvpe_model.to(device);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    load_net_g("assets\\net_g_traced.pt");

}

void RVC::load_net_g(const std::string &net_g_path)
{
    try
    {
        net_g = torch::jit::load(net_g_path);
        net_g.to(device);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
  
}

void RVC::transpose_f0(const std::int16_t &f0upKey)
{
    this->f0upKey = f0upKey;
}

std::vector<float> RVC::voiceConversion(const std::vector<float> &buffer_audio)
{
     // copy input data into a tensor 
    int size = buffer_audio.size();
    torch::Tensor in_vc = torch::empty({1, size});
    for (long i=0; i<buffer_audio.size(); ++i){
       in_vc[0][i] = buffer_audio[i];    
    }
    // pass through the network:
    torch::Tensor out_vc = voiceConversion(in_vc.to(device));
    // copy output back out to a vector 
    std::vector<float> outputs(out_vc.size(1));// initialise size to n_outputs
    for (long i=0; i<out_vc.size(1); ++i){
       outputs[i] = out_vc[0][i].item<float>();
    }
    return outputs;

}

torch::Tensor RVC::voiceConversion(torch::Tensor &buffer_audio)
{
    torch::Tensor audio_padded;
    try
    {
        std::vector<int64_t> pad = { 16000, 16000 };
        audio_padded = F::pad(buffer_audio, F::PadFuncOptions({ 16000, 16000 }).mode(torch::kReflect)).to(device);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    torch::Tensor audio_vc = get_vc(audio_padded);
    //torch::Tensor audio_vc = get_vc(buffer_audio);
    //set_f0(audio_padded);

    return buffer_audio;
}

torch::Tensor RVC::get_vc(torch::Tensor &audio)
{
    std::vector<torch::jit::IValue> in;
    torch::Tensor source;
    torch::Tensor mask2;
    torch::Tensor layer2;

    torch::jit::script::Module inputs;

    //inputs = torch::jit::load("..\\..\\hb_inputs.pt");
    inputs = torch::jit::load("..\\hb_inputs.pt");
    
    // source = inputs.attr("source").toTensor();
    // mask2 = inputs.attr("padding_mask").toTensor();
    // layer2 = inputs.attr("output_layer").toTensor();

    source = inputs.attr("source").toTensor().to(device);
    mask2 = inputs.attr("padding_mask").toTensor().to(device);
    layer2 = inputs.attr("output_layer").toTensor().to(device);

    // in.push_back(&source);
    // in.push_back(&mask2);
    // in.push_back(&layer2);

    in.push_back(source);
    in.push_back(mask2);
    in.push_back(layer2);   

    // Execute the model and turn its output into a tensor.
    torch::Tensor output = hubert_model.forward(in).toTensor();
    torch::Tensor _audio_vc;

    audio.to(torch::kFloat32);
    torch::Tensor mask = torch::full({1, audio.size(1)}, false).to(device);
    std::vector<torch::jit::IValue> in_vc;
    torch::Tensor layer = torch::tensor({9}, torch::kInt32);

    // if (torch::equal(layer2, layer))
    // {
    //     std::cout << "layer ok" << std::endl;
    // }

    // if (torch::equal(mask2, mask))
    // {
    //     std::cout << "mask ok" << std::endl;
    // }

    // audio = torch::zeros({1,33600}).to(torch::kFloat32);

    // if (torch::equal(audio, source))
    // {
    //     std::cout << "audio ok" << std::endl;
    // }

    in_vc.push_back(audio);
    in_vc.push_back(mask);
    in_vc.push_back(layer);

    try
    {
        torch::NoGradGuard no_grad;
        _audio_vc = hubert_model.forward(in_vc).toTensor();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return _audio_vc;
}

void RVC::set_f0(const torch::Tensor &audio_padded)
{
    torch::Tensor f0;
    rmvpe_model.to(device);
    float thred = 0.03;

    std::vector<torch::jit::IValue> in_rmvpe;
    in_rmvpe.push_back(audio_padded);
    in_rmvpe.push_back(thred);


    f0 = rmvpe_model.forward(in_rmvpe).toTensor();
    std::cout << f0 << std::endl;
}



void RVC::setDevice(const std::string &device)
{
    if (device == "cpu")
        this->device = c10::kCPU;
    else if (device == "cuda")
        this->device = c10::kCUDA;    
}



int main(int argc, char* argv[]){

    int n=750;
    std::vector<float> audio(n);

    //for hubert model: 'max_sample_size': 250000, 'min_sample_size' : 32000

    RVC rvc;

    for (int i = 0; i < n; i++)
    {
        audio[i] = float(rand() % 100) / 100;
        //std::cout << audio[i] << ' ';
    }
    std::cout << std::endl;

    rvc.voiceConversion(audio);

    return 0;
}

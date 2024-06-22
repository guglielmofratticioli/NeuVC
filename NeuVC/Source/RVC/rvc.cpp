#include <iostream>
#include <cstdlib>
#include <memory>
#include "rvc.h"
#include <filesystem>
#include <cmath>

namespace F = torch::nn::functional;
namespace I = torch::indexing;

RVC::RVC()
: window{160}, sr{16000}, t_pad{1}, t_query{6}, t_center{38}, t_max{41}, device{torch::kCPU},
sid{0}, f0upKey{0}, tg_sr{20000}
{
    try
    {        
        hubert_model = torch::jit::load("../../assets/hubert_traced.pt");
        hubert_model.to(device);
        rmvpe_model = torch::jit::load("../../assets/rmvpe_traced.pt");
        rmvpe_model.to(device);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    load_net_g("../../assets/rvc_trained/ariana_grande.pt");

}

void RVC::load_net_g(const std::string &net_g_path)
{
    try
    {
        // load synth model from 
        net_g = torch::jit::load(net_g_path);
        net_g.to(device);

        // load 
        auto get_synth_params = net_g.get_method("get_synth_params");
        std::vector<c10::IValue> empty;
        auto synth_params = get_synth_params(empty).toTuple();
        version = synth_params->elements()[0].toStringRef();
        if_f0 = (bool) synth_params->elements()[1].toInt();
        tg_sr = (std::int32_t) synth_params->elements()[2].toInt();
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

void RVC::setDevice(const std::string &device)
{
    if (device == "cpu")
        this->device = c10::kCPU;
    else if (device == "cuda")
        this->device = c10::kCUDA;    
}

torch::Tensor RVC::get_vc(torch::Tensor &audio, torch::Tensor &pitch, torch::Tensor &pitchf)
{
    std::vector<torch::jit::IValue> in_hubert;
    torch::Tensor mask;
    torch::Tensor layer;
    torch::Tensor feats;

    std::vector<torch::jit::IValue> in_synth;
    torch::Tensor phone;
    torch::Tensor phone_lengths;
    torch::Tensor nsff0;
    torch::Tensor out_audio;

    bool is_pitch_and_pitchf = (!torch::equal(pitch, torch::tensor(NULL))) && (!torch::equal(pitchf, torch::tensor(NULL)));

    mask = torch::empty(audio.sizes(), torch::kBool).to(device).fill_(false);
    if (version == "v1")
        layer = torch::tensor({9});
    else
        layer = torch::tensor({12});
    in_hubert.push_back(audio);
    in_hubert.push_back(mask);
    in_hubert.push_back(layer);

    {
        torch::NoGradGuard no_grad;
        feats = hubert_model.forward(in_hubert).toTensor();
    }


    std::vector<double> scale_factor = {2};
    feats = feats.permute({0, 2, 1});
    feats = F::interpolate(feats, F::InterpolateFuncOptions().scale_factor(scale_factor).mode(torch::kLinear).align_corners(false));
    feats = feats.permute({0, 2, 1});

    int p_len = audio.size(1) / window;
    if (feats.size(1) < p_len)
    {
        p_len = feats.size(1);
        if (is_pitch_and_pitchf)  
        {
            pitch = pitch.slice(1, NULL, p_len);
            pitchf = pitchf.slice(1, NULL, p_len);
        }

    }

    torch::Tensor p_len_tensor = torch::tensor({p_len});
    torch::Tensor sid_tensor = torch::tensor({this->sid});

    in_synth.push_back(feats.to(torch::kF32));
    in_synth.push_back(p_len_tensor.to(torch::kI64));
    if (is_pitch_and_pitchf)
    {
        in_synth.push_back(pitch.to(torch::kI64));
        in_synth.push_back(pitchf.to(torch::kF32));
    }
    in_synth.push_back(sid_tensor.to(torch::kI64));
    



    // hubert input test ------------------------------------------------------------------------
    // torch::jit::script::Module inputs;
    //inputs = torch::jit::load("..\\..\\hb_inputs.pt");
    
    // source = inputs.attr("source").toTensor();
    // mask = inputs.attr("padding_mask").toTensor();
    // layer = inputs.attr("output_layer").toTensor();
    //-------------------------------------------------------------------------------------------
    



    // synth input test -------------------------------------------------------------------------
    // torch::jit::script::Module inputs;
    // inputs = torch::jit::load("../../assets/inputs/synth_inputs.pt");

    // phone = inputs.attr("phone").toTensor().to(device);
    // phone_lengths = inputs.attr("phone_lengths").toTensor().to(device);
    // pitch = inputs.attr("pitch").toTensor().to(device);
    // nsff0 = inputs.attr("nsff0").toTensor().to(device);
    // sid = inputs.attr("sid").toTensor().to(device);

    // in.push_back(phone);
    // in.push_back(phone_lengths);
    // in.push_back(pitch);
    // in.push_back(nsff0);
    // in.push_back(sid);
    //--------------------------------------------------------------------------------------------
    // Execute the model and turn its output into a tensor.
    

    
    {
        torch::NoGradGuard no_grad;
        auto output = net_g.forward(in_synth).toTuple();
        out_audio = output->elements()[0].toTensor()[0,0];
    }

    return out_audio;
}

std::vector<torch::Tensor> RVC::get_f0(const torch::Tensor &audio_padded)
{
    std::vector<torch::Tensor> pitch;
    torch::Tensor f0;
    torch::Tensor f0_mel;
    std::int16_t f0_min = 50;
    std::int16_t f0_max = 1100;
    torch::Tensor f0_mel_min = 1127 * torch::log(1 + torch::tensor(f0_min) / 700.0);
    torch::Tensor f0_mel_max = 1127 * torch::log(1 + torch::tensor(f0_max) / 700.0);
    
    std::vector<torch::jit::IValue> in_rmvpe;
    in_rmvpe.push_back(audio_padded.squeeze());

    rmvpe_model.to(device);
    {
        torch::NoGradGuard no_grad;
        f0 = rmvpe_model.forward(in_rmvpe).toTensor();
    }
    f0 *= pow(2, (float) f0upKey / 12);

    f0_mel = 1127 * torch::log(1 + f0 / 700);
    torch::Tensor f0_mel_temp = (f0_mel.index({f0_mel > 0}) - f0_mel_min) * 254 / (f0_mel_max - f0_mel_min) + 1;
    f0_mel.index_put_({f0_mel > 0}, f0_mel_temp);
    f0_mel.index_put_({f0_mel <=1}, torch::tensor(1));
    f0_mel.index_put_({f0_mel > 255}, torch::tensor(255));
    f0_mel.round_().to(torch::kInt32);

    pitch.push_back(f0_mel);
    pitch.push_back(f0);

    return pitch;
}




torch::Tensor RVC::voiceConversion(torch::Tensor &buffer_audio)
{
    torch::Tensor audio_padded;
    std::vector<std::int32_t> opt_ts;
    std::vector<torch::Tensor> audio_opt;
    std::int32_t x_pad = sr * t_pad;
    std::int32_t x_query = sr * t_query;
    std::int32_t x_center = sr * t_center;
    std::int32_t x_max = sr * t_max; 
    std::int32_t x_pad_tgt = tg_sr * t_pad;
    std::int32_t s = 0;
    std::int32_t p_len;
    std::vector<torch::Tensor> pitch_pitchf;
    torch::Tensor pitch;
    torch::Tensor pitchf;
    torch::Tensor audio_vc;

    try
    {
        audio_padded = F::pad(buffer_audio, F::PadFuncOptions({window / 2, window / 2}).mode(torch::kReflect));
        if (audio_padded.size(1) > x_max)
        {
            torch::Tensor audio_sum = torch::zeros_like(buffer_audio);
            for (int i = 0; i < window; i++)
            {
                audio_sum += torch::abs(audio_padded.index({0, I::Slice(i, i- window)}));
            }
            for (int x = x_center; x < audio_padded.size(1); x+=x_center)
            {
                auto query = (audio_sum.slice(1, x - x_query, x + x_query) == audio_sum.slice(1, x - x_query, x + x_query).min());  
                int opt_t = x - x_query + torch::nonzero(query)[0][1].item<int>();
                opt_ts.push_back(opt_t);
            }
            audio_padded = F::pad(buffer_audio, F::PadFuncOptions({x_pad, x_pad}).mode(torch::kReflect));
            p_len = audio_padded.size(1) / window;
            if (if_f0)
            {
                pitch_pitchf = get_f0(audio_padded);
                pitch = pitch_pitchf[0].slice(0, NULL, p_len).unsqueeze(0);
                pitchf = pitch_pitchf[1].slice(0, NULL, p_len).unsqueeze(0);
            }

            for (int i = 0; i < opt_ts.size(); i++)
            {
                int x = (opt_ts[i] / window) * window;
                if (if_f0)
                {
                    torch::Tensor audio_sliced = audio_padded.slice(1, s, x + x_pad * 2 + window);
                    torch::Tensor pitch_sliced = pitch.slice(1, s / window, (x + x_pad * 2) / window);
                    torch::Tensor pitchf_sliced = pitchf.slice(1, s / window, (x + x_pad * 2) / window);
                    torch::Tensor out_vc = get_vc(audio_sliced, pitch_sliced, pitchf_sliced);
                    audio_opt.push_back(out_vc.slice(1, x_pad_tgt, -x_pad_tgt));
                }
                else
                {
                    torch::Tensor audio_sliced = audio_padded.slice(0, s, x + x_pad * 2 + window);
                    torch::Tensor out_vc = get_vc(audio_sliced, torch::tensor(NULL), torch::tensor(NULL));
                    audio_opt.push_back(out_vc.slice(1, x_pad_tgt, -x_pad_tgt));
                }
                s = x;
            }
            if (if_f0)
            {
                torch::Tensor pitch_sliced;
                torch::Tensor pitchf_sliced;
                torch::Tensor audio_sliced = audio_padded.slice(1, s, c10::nullopt);
                if (s > 0)
                    pitch_sliced = pitch.slice(1, s / window, c10::nullopt);
                else 
                    pitch_sliced = pitch;
                if (s > 0) 
                    pitchf_sliced = pitchf.slice(1, s / window, c10::nullopt);
                else
                    pitchf_sliced = pitchf;
                torch::Tensor out_vc = get_vc(audio_sliced, pitch_sliced, pitchf_sliced);
                audio_opt.push_back(out_vc.slice(1, x_pad_tgt, -x_pad_tgt));
            }
            else
            {
                torch::Tensor audio_sliced = audio_padded.slice(1, s, c10::nullopt);
                torch::Tensor out_vc = get_vc(audio_sliced, torch::tensor(NULL), torch::tensor(NULL));
                audio_opt.push_back(out_vc.slice(1, x_pad_tgt, -x_pad_tgt));
            }
            audio_vc = torch::cat(audio_opt, 1);
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return audio_vc;
}


std::vector<float> RVC::voiceConversion(const std::vector<float> &buffer_audio)
{
     // copy input data into a tensor 
    torch::Tensor in_vc = torch::tensor(buffer_audio).unsqueeze(0).to(device);
    // pass through the network:
    torch::Tensor out_vc = voiceConversion(in_vc.to(device)).squeeze();
    // copy output back out to a vector
    std::vector<float> outputs(out_vc.data_ptr<float>(), out_vc.data_ptr<float>() + out_vc.numel());

    return outputs;

}

int main(int argc, char* argv[]){

    // IMPORTANT ! -------------------------------------------------------------
    // for hubert model: 'max_sample_size': 250000, 'min_sample_size' : 32000
    // -------------------------------------------------------------------------
    
    RVC rvc;

    // // initilize rand input    
    // int n=56789;
    // std::vector<float> audio(n);

    // for (int i = 0; i < n; i++)
    // {
    //     audio[i] = float(rand() % 100) / 100;
    //     //std::cout << audio[i] << ' ';
    // }

    // load input from python brake point during inference
    try
    {
        torch::jit::script::Module inputContainer;
        inputContainer = torch::jit::load("../../assets/inputs/input_audio.pt");
        torch::jit::IValue test = inputContainer.attr("audio");
        std::vector<double> audio_double = inputContainer.attr("audio").toDoubleVector();
        std::vector<float> audio(audio_double.begin(), audio_double.end());
        rvc.voiceConversion(audio);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
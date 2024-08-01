import os
import sys
import numpy as np
import torch

now_dir = os.getcwd()
sys.path.append(now_dir)

from infer.lib.jit import to_jit_model, load_inputs

torch.manual_seed(114514)

class InputContainer(torch.nn.Module):
    def __init__(self, obj):
        super().__init__()
        for key, value in obj.items():
            setattr(self, key, value)


def trace_model(argv):
    assert isinstance(argv[1], str)
    _, net_g_traced = to_jit_model(argv[1], "synthesizer", "trace", "assets/input_synth.pth")
    name_model = str(argv[1]).split('\\')[-1].split('/')[-1].split('.')[-2]
    net_g_traced.save(f"traced/{name_model}.pt")
    print('Your model is been traced successfully')

if __name__ == "__main__":

    trace_model(sys.argv)

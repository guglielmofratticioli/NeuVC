import torch



def load_inputs(path, device, is_half=False):
    parm = torch.load(path, map_location=torch.device("cpu"))
    for key in parm.keys():
        parm[key] = parm[key].to(device)
        if is_half and parm[key].dtype == torch.float32:
            parm[key] = parm[key].half()
        elif not is_half and parm[key].dtype == torch.float16:
            parm[key] = parm[key].float()
    return parm


def to_jit_model(
    model_path,
    model_type: str,
    mode: str = "trace",
    inputs_path: str = None,
    device=torch.device("cpu"),
    is_half=False,
    inputs=None,
):
    model = None
    if model_type.lower() == "synthesizer":
        from .get_synthesizer import get_synthesizer

        model, _ = get_synthesizer(model_path, device)
    else:
        raise ValueError(f"No model type named {model_type}")
    model = model.eval()
    model = model.half() if is_half else model.float()
    if mode == "trace":
        assert inputs_path
        if not inputs:
            inputs = load_inputs(inputs_path, device, is_half)
        with torch.no_grad():
            model_jit = torch.jit.trace(model, example_kwarg_inputs=inputs, check_trace=False, strict=False) #, check_inputs=check_inputs)
    elif mode == "script":
        model_jit = torch.jit.script(model)
    model_jit.to(device)
    model_jit = model_jit.half() if is_half else model_jit.float()
    return (model, model_jit)


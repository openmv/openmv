#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (C) 2026 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Exports the ExecuTorch models used by the ml_torch_* unit tests, and prints the
# expected outputs (the quantized models' outputs on the host for the tests'
# deterministic inputs). Requires the Python packages in lib/torch/libtorch's
# requirements.txt, and ethos-u-vela and tosa-tools for the Ethos-U models.
#
# Usage: python3 tools/torch_test_models.py --out-dir lib/models

import argparse
import os

import torch
import torch.nn as nn
from executorch.exir import to_edge, to_edge_transform_and_lower
from torch.export import export
from torchao.quantization.pt2e.quantize_pt2e import convert_pt2e, prepare_pt2e

TOP_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


class Classifier(nn.Module):
    # Exercises the CMSIS-NN conv, depthwise conv, pooling, linear and softmax ops.
    def __init__(self, classes=10):
        super().__init__()
        self.features = nn.Sequential(
            nn.Conv2d(3, 16, 3, 2, 1), nn.ReLU(),
            nn.Conv2d(16, 16, 3, 1, 1, groups=16), nn.ReLU(),
            nn.Conv2d(16, 32, 1), nn.ReLU(),
            nn.MaxPool2d(2),
        )
        self.fc = nn.Linear(32, classes)

    def forward(self, x):
        x = self.features(x)
        x = x.mean(dim=(2, 3))
        return torch.softmax(self.fc(x), dim=-1)


class Detector(nn.Module):
    # FOMO/CenterNet-style head. Exercises the portable cat and upsample ops.
    def __init__(self, classes=2):
        super().__init__()

        def block(cin, cout, stride):
            return nn.Sequential(nn.Conv2d(cin, cout, 3, stride, 1), nn.ReLU6())

        self.s1 = block(3, 8, 2)
        self.s2 = block(8, 16, 2)
        self.s3 = block(16, 32, 2)
        self.up = nn.Upsample(scale_factor=2, mode="nearest")
        self.fuse = block(48, 16, 1)
        self.head = nn.Conv2d(16, classes, 1)

    def forward(self, x):
        f2 = self.s2(self.s1(x))
        x = torch.cat([self.up(self.s3(f2)), f2], dim=1)
        return torch.sigmoid(self.head(self.fuse(x)))


class Layout(nn.Module):
    # Exercises the portable data movement ops on channels-last tensors: split,
    # cat, slice, flip and permute, between CMSIS-NN convolutions.
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv2d(3, 8, 3, 1, 1)
        self.conv2 = nn.Conv2d(8, 4, 3, 1, 1)

    def forward(self, x):
        x = torch.relu(self.conv1(x))
        a, b = torch.split(x, [3, 5], dim=1)
        x = torch.cat([b, a], dim=1)
        x = x[:, :, 2:14, 1:15]
        x = torch.flip(x, dims=[3])
        x = self.conv2(x)
        return x.permute(0, 2, 3, 1)


class MultiIO(nn.Module):
    # Two inputs (an NCHW image and a vector) and three outputs (float scores, and
    # their argmax as int64 and int32). The Cortex-M explicit-layout lowering can't
    # take an NCHW input straight into a spatial op (convolution or pooling), so the
    # image is reduced with amax.
    def forward(self, x, y):
        x = x.amax(dim=(2, 3)) * 2.0 + y
        i = torch.argmax(x, dim=1)
        return x, i, i.to(torch.int32)


class NHWCInput(nn.Module):
    # Takes an NHWC input, which is what the OpenMV ML library provides for images,
    # and passes it to the model as NCHW. With the explicit-layout lowering, every
    # tensor keeps the default dim order and the conversion folds into the first
    # NHWC kernel.
    def __init__(self, model):
        super().__init__()
        self.model = model

    def forward(self, x):
        return self.model(x.permute(0, 3, 1, 2))


def make_model(cls):
    torch.manual_seed(0)
    model = cls().eval()
    # Spread the outputs so the tests can check a clear result.
    with torch.no_grad():
        last = getattr(model, "fc", None) or getattr(model, "head", None)
        if last is not None:
            last.weight.mul_(20.0)
    return model


def test_input(h, w, c=3):
    # Same values the tests build on the device, NHWC.
    values = [((i * 37) % 255) / 127.5 - 1.0 for i in range(h * w * c)]
    return torch.tensor(values, dtype=torch.float32).reshape(1, h, w, c)


def quantize(model, example, quantizer, calibration=None):
    captured = export(model, example, strict=True).module(check_guards=False)
    prepared = prepare_pt2e(captured, quantizer)
    if calibration is None:
        gen = torch.Generator().manual_seed(1)
        calibration = [tuple(torch.rand(e.shape, generator=gen) * 2 - 1 for e in example) for _ in range(16)]
        calibration.append(example)
    for x in calibration:
        prepared(*x)
    return convert_pt2e(prepared, fold_quantize=True)


def load_image(path, size):
    # RGB image resized to size x size, scaled to [0, 1], NHWC.
    from PIL import Image

    img = Image.open(path).convert("RGB").resize((size, size), Image.BILINEAR)
    return torch.tensor(list(img.get_flattened_data()), dtype=torch.float32).reshape(1, size, size, 3) / 255.0


class Yolo(nn.Module):
    # Ultralytics YOLOv8n, with the detection head producing TFLite-style
    # (1, 84, anchors) outputs, as expected by ml.postprocessing.ultralytics.
    def __init__(self):
        super().__init__()
        from ultralytics import YOLO

        self.model = YOLO("yolov8n.pt").model.eval().fuse()
        head = self.model.model[-1]
        head.export = True
        head.format = "tflite"

    def forward(self, x):
        y = self.model(x)
        return y[0] if isinstance(y, (list, tuple)) else y


def yolo_calibration(size):
    import ultralytics

    assets = os.path.join(os.path.dirname(ultralytics.__file__), "assets")
    data = os.path.join(TOP_DIR, "scripts", "unittest", "data")
    paths = [os.path.join(assets, "bus.jpg"), os.path.join(assets, "zidane.jpg")]
    paths += [os.path.join(data, f) for f in ("faces.bmp", "hand.bmp", "graffiti.bmp")]
    return [(load_image(p, size),) for p in paths]


def export_cortex_m(model, example, cpu, calibration=None):
    from executorch.backends.cortex_m.edge_compile_config import cortex_m_edge_compile_config
    from executorch.backends.cortex_m.passes.cortex_m_pass_manager import CortexMPassManager
    from executorch.backends.cortex_m.quantizer.quantizer import CortexMQuantizer
    from executorch.backends.cortex_m.target_config import CortexM, CortexMTargetConfig
    from executorch.backends.transforms.duplicate_dynamic_quant_chain import (
        DuplicateDynamicQuantChainPass,
    )

    converted = quantize(model, example, CortexMQuantizer(), calibration)
    DuplicateDynamicQuantChainPass()(converted)
    edge = to_edge(export(converted, example), compile_config=cortex_m_edge_compile_config())
    target = CortexMTargetConfig(cpu={"m7": CortexM.M7, "m55": CortexM.M55}[cpu])
    # The explicit-layout lowering keeps every tensor in the default dim order and
    # converts layouts explicitly around the NHWC kernels, so portable ops never see
    # channels-last tensors (see pytorch/executorch#19299).
    edge = edge.transform(CortexMPassManager(target_config=target, use_explicit_layout=True))
    return converted, edge.to_executorch()


def export_ethos_u(model, example, calibration=None):
    from executorch.backends.arm.ethosu import EthosUCompileSpec, EthosUPartitioner
    from executorch.backends.arm.quantizer import EthosUQuantizer, get_symmetric_quantization_config

    # Same NPU and memory configuration as the MPS3_AN547 board's VELA_ARGS.
    spec = EthosUCompileSpec(
        "ethos-u55-256",
        system_config="RTSS_HE_SRAM_Only",
        memory_mode="Shared_Sram",
        config_ini=os.path.join(TOP_DIR, "tools", "vela.ini"),
    )
    quantizer = EthosUQuantizer(spec)
    quantizer.set_global(get_symmetric_quantization_config())
    converted = quantize(model, example, quantizer, calibration)
    edge = to_edge_transform_and_lower(export(converted, example), partitioner=[EthosUPartitioner(spec)])
    return converted, edge.to_executorch()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", default=os.path.join(TOP_DIR, "lib", "models"))
    parser.add_argument("--only", nargs="*", help="Export only these models")
    args = parser.parse_args()

    exports = [
        ("torch_classifier_m7", Classifier, 32, lambda m, x: export_cortex_m(m, x, "m7")),
        ("torch_classifier_m55", Classifier, 32, lambda m, x: export_cortex_m(m, x, "m55")),
        ("torch_detector_m7", Detector, 32, lambda m, x: export_cortex_m(m, x, "m7")),
        ("torch_detector_m55", Detector, 32, lambda m, x: export_cortex_m(m, x, "m55")),
        ("torch_classifier_u55", Classifier, 32, export_ethos_u),
        ("torch_layout_m7", Layout, 16, lambda m, x: export_cortex_m(m, x, "m7")),
        ("torch_layout_m55", Layout, 16, lambda m, x: export_cortex_m(m, x, "m55")),
        ("torch_multi_io_m7", MultiIO, 8, lambda m, x: export_cortex_m(m, x, "m7")),
        ("torch_multi_io_m55", MultiIO, 8, lambda m, x: export_cortex_m(m, x, "m55")),
        ("torch_multi_io_u55", MultiIO, 8, export_ethos_u),
    ]

    # YOLOv8n, calibrated on real images. The test input is person.bmp.
    yolo_size = 192
    yolo_calib = yolo_calibration(yolo_size)
    yolo_input = load_image(os.path.join(TOP_DIR, "scripts", "unittest", "data", "person.bmp"), yolo_size)
    exports += [
        ("torch_yolov8n_m55", Yolo, yolo_size, lambda m, x: export_cortex_m(m, x, "m55", yolo_calib)),
        ("torch_yolov8n_u55", Yolo, yolo_size, lambda m, x: export_ethos_u(m, x, yolo_calib)),
    ]

    for name, cls, size, export_fn in exports:
        if args.only and name not in args.only:
            continue
        if cls is Yolo:
            model = NHWCInput(Yolo()).eval()
            example = (yolo_input,)
            # The detection head builds its anchors on the first call for an input
            # size, which can't be exported, so run it once first.
            model(*example)
        elif cls is MultiIO:
            # Takes its image input as NCHW.
            model = make_model(cls)
            example = (test_input(size, size).permute(0, 3, 1, 2).contiguous(),
                       torch.tensor([[0.25, -0.5, 0.75]]))
        else:
            model = NHWCInput(make_model(cls)).eval()
            example = (test_input(size, size),)
        converted, program = export_fn(model, example)
        path = os.path.join(args.out_dir, name + ".pte")
        with open(path, "wb") as f:
            f.write(program.buffer)

        ops = sorted({f"{op.name}.{op.overload}" for plan in program.executorch_program.execution_plan
                      for op in plan.operators})
        delegates = sorted({d.id for plan in program.executorch_program.execution_plan
                            for d in plan.delegates})
        out = converted(*example)
        plan = program.executorch_program.execution_plan[0]
        print(f"{name}: {len(program.buffer)} bytes")
        print(f"  ops: {', '.join(ops)}")
        print(f"  delegates: {', '.join(delegates) or 'none'}")
        if isinstance(out, (list, tuple)):
            for o in out:
                print(f"  expected: {o.dtype} {[round(v, 4) for v in o.flatten().tolist()]}")
            continue
        if list(plan.values[plan.outputs[0]].val.dim_order) == [0, 2, 3, 1]:
            # The device reports channels-last outputs in NHWC order.
            out = out.permute(0, 2, 3, 1)
        out = out.flatten()
        if cls is Yolo:
            # Highest person (class 0) score, rows 4.. are the class scores.
            print(f"  expected: person score={float(out.reshape(84, -1)[4].max()):.4f}")
        elif out.numel() <= 16:
            print(f"  expected: {[round(v, 4) for v in out.tolist()]}")
        else:
            print(f"  expected: argmax={int(out.argmax())} max={float(out.max()):.4f} "
                  f"sum={float(out.sum()):.4f} numel={out.numel()}")


if __name__ == "__main__":
    main()

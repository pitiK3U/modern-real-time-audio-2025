import numpy as np

sample_size = 128

def sine_wave():
    return np.sin(2 * np.pi * np.linspace(0, 1, sample_size, endpoint=False))

def triangle_wave():
    t = np.linspace(0, 1, sample_size, endpoint=False)
    return 2 * np.abs(2 * t - 1) - 1

def saw_wave():
    t = np.linspace(0, 1, sample_size, endpoint=False)
    return 2 * t - 1

def square_wave():
    t = np.linspace(0, 1, sample_size, endpoint=False)
    return np.where(np.sin(2 * np.pi * t) >= 0, 1.0, -1.0)

def flat_wave(value=0.0, sample_size = sample_size):
    return np.full(sample_size, value, dtype=np.float32)

def wave_min(wave_a, wave_b):
    return np.minimum(wave_a, wave_b)

def format_wave(wave):
    return "{" + ", ".join(f"{x:.6f}f" for x in wave) + "}"

def main():
    presets = {
        "A": [
            ("// Sine", sine_wave()),
            ("// Triangle", triangle_wave()),
            ("// Sawtooth", saw_wave()),
            ("// Square", square_wave())
        ],
        "B": [
            ("// Sine", sine_wave()),
            ("// Sine bottoms", wave_min(sine_wave(), flat_wave()))
        ]
    }

    print("#pragma once\n")
    print("#include <vector>\n")
    print("namespace DSP::WavetablePlugins {")

    for name, waves in presets.items():
        print(f"\n    static const std::vector<std::vector<float>> plugin{name} = {{")
        for comment, wave in waves:
            print(f"        {comment}")
            print(f"        {format_wave(wave)},")
        print("    };")

    print("}\n")

if __name__ == "__main__":
    main()

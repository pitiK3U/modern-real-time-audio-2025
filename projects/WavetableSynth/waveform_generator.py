import numpy as np

sample_size = 2048 # Keep it same as the constant in Wavetable.h

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

def ghostly():
    t = np.linspace(0, 1, sample_size, endpoint=False)
    return [
        ("// Whisper Sine", 0.2 * sine_wave()),
        ("// Fade-in Inverted Sine", -sine_wave() * t),
        ("// Curved Triangle (tanh)", np.tanh(triangle_wave() * 2.0)),
        ("// Sine Pulse-Top", np.clip(sine_wave() + square_wave() * 0.3, -1.0, 1.0))
    ]

def seafoam():
    t = np.linspace(0, 1, sample_size, endpoint=False)
    mod = 0.5 * (1 + np.sin(2 * np.pi * t * 4))
    return [
        ("// Soft Saw", saw_wave() * 0.6),
        ("// Phase-Twisted Sine", np.sin(2 * np.pi * (t ** 1.5))),
        ("// Square-Triangle", 0.6 * square_wave() + 0.4 * triangle_wave()),
        ("// AM Sine", sine_wave() * mod),
        ("// Tri-Pulse Morph", np.where(triangle_wave() > 0, 1.0, -1.0) * 0.7 + triangle_wave() * 0.3)
    ]

def glitchpop():
    t = np.linspace(0, 1, sample_size, endpoint=False)
    sine = sine_wave()
    saw = saw_wave()
    return [
        ("// Bit Sine", np.round(sine * 5) / 5),
        ("// Hard Saw", np.clip(saw * 2.5, -1, 1)),
        ("// Sine Pulses", np.where((t * 16) % 1 < 0.5, sine, 0)),
        ("// Spike Comb", np.where((t * 8) % 1 < 0.1, -1, 1)),
        ("// DC Step", np.where(t < 0.5, -0.7, 0.7)),
        ("// Folded Sine", np.abs(sine * 2 - 1) * 2 - 1)
    ]

def format_wave(wave):
    # Scale waves which are not in range -1 1
    max_abs = np.max(np.abs(wave))
    if max_abs > 1.0:
        wave = wave / max_abs
    return "{" + ", ".join(f"{x:.6f}f" for x in wave) + "}"

def main():
    presets = {
        "Plugin_A": [
            ("// Sine", sine_wave()),
            ("// Triangle", triangle_wave()),
            ("// Sawtooth", saw_wave()),
            ("// Square", square_wave())
        ],
        "Plugin_B": [
            ("// Sine", sine_wave()),
            ("// Sine bottoms", wave_min(sine_wave(), flat_wave()))
        ],
        "Ghostly": ghostly(),
        "Seafoam": seafoam(),
        "Glitch_Pop": glitchpop(),
    }

    print("#pragma once\n")
    print("#include <vector>\n#include <JuceHeader.h>\n")
    print("namespace DSP::WavetablePlugins {")

    # Write wave data
    for name, waves in presets.items():
        print(f"\n    static const std::vector<std::vector<float>> plugin{name} = {{")
        for comment, wave in waves:
            print(f"        {comment}")
            print(f"        {format_wave(wave)},")
        print("    };")

    # Write enum
    print("\n    enum class PresetID")
    print("    {")
    for i, name in enumerate(presets.keys()):
        comma = "," if i < len(presets) - 1 else ""
        print(f"        {name}{comma}")
    print("    };")

    # getPresetNames
    print("\n    inline const juce::StringArray& getPresetNames()")
    print("    {")
    print("        static const juce::StringArray names {")
    for name in presets.keys():
        printable = name.replace("Plugin", "Plugin ") if "Plugin" in name else name
        print(f"            \"{printable}\",")
    print("        };")
    print("        return names;")
    print("    }")

    # getPreset
    print("\n    inline const std::vector<std::vector<float>>& getPreset(PresetID id)")
    print("    {")
    print("        switch (id)")
    print("        {")
    for name in presets.keys():
        print(f"            case PresetID::{name}: return plugin{name};")
    print("        }")
    print("        return pluginPlugin_A; // default fallback")
    print("    }")

    # getPresetIndex
    print("\n    inline const unsigned int getPresetIndex(PresetID id) {")
    print("        switch (id)")
    print("        {")
    for i, name in enumerate(presets.keys()):
        print(f"            case PresetID::{name}: return {i};")
    print("        }")
    print("        return 0;")
    print("    }")

    # getPresetId
    print("\n    inline const PresetID getPresetId(unsigned int id) {")
    print("        switch (id)")
    print("        {")
    for i, name in enumerate(presets.keys()):
        print(f"            case {i}: return PresetID::{name};")
    print("        }")
    print("        return PresetID::Plugin_A;")
    print("    }")

    print("} // namespace DSP::WavetablePlugins")

if __name__ == "__main__":
    main()

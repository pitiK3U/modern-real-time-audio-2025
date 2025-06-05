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

def format_wave(wave):
    return "{" + ", ".join(f"{x:.6f}f" for x in wave) + "}"

def main():
    waves = [
        ("// Sine", sine_wave()),
        ("// Triangle", triangle_wave()),
        ("// Sawtooth", saw_wave()),
        ("// Square", square_wave())
    ]

    print("std::vector<std::vector<float>> wavetables = {")
    for comment, wave in waves:
        print(f"    {comment}")
        print(f"    {format_wave(wave)},")
    print("};")

if __name__ == "__main__":
    main()

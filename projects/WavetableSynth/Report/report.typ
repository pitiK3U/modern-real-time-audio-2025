#set page(numbering: "1")
#set heading(numbering: "1.")
#set math.equation(numbering: "(1)")

#show link: set text(fill: blue)
#show link: underline

#let separate-supplement-style(supp, num) = {
  text(supp, fill: blue)
  [ ]
  box(num, stroke: 1pt + green, outset: (bottom:1.5pt, x:.5pt, y:.5pt))
}

#show ref: it => {
  let (element, target, supplement: supp) = it.fields()
  link(target, it)
}

#let authors = ("", "")
#let title = "Wavetable synthetizer"
#let date = datetime(day: 6, month: 6, year: 2025)

#set text(lang: "en")

#set document(author: authors, title: title, date: date)

#let font-size = (
  script: 7pt,
  footnote: 8pt,
  small: 10pt,
  normal: 11pt,
  author: 12pt,
  title: 17pt,
)

#set page(
  header: align(
    right + horizon,
    date.display()
  ),
)

#align(center,{
    par(leading: 0.75em, text(size: font-size.title, strong( title)))
    v(0pt)
    text(size: font-size.author, authors.join(", "))
    v(font-size.small)
    v(1.5em)
})


#let abstract = [
  We created polyphony wavetable synthetizer that allows you to select knob and put a LFO and envelope modifier on it. We have also built a nice visualization of the actual wavetable, LFO and envelope. Furthermore, we included several wavetable presets and even loading from wav files. The wavetable oscillator also, other than the usual morphing between waveforms has a unison voices with detune.
]

#align(center)[
  #set par(justify: false)
  *Abstract* \
  #abstract
]
#let alert(body, fill: red) = {
  set text(white)
  set align(center)
  rect(
    fill: fill,
    inset: 8pt,
    radius: 4pt,
    [*Warning:\ #body*],
  )
}

= Introduction

The overview of the project is mixed explained in #ref(<background>), since we are already above 6 pages.

#alert[
Write an introductory section, giving an overview of the project and explaining the motivation behind it.

The report should be *a maximum of 6 pages long*.
]

#figure(image("img/wavetable_synth.png"), caption: [
  Our final result of the wavetable synthetizer.
  ]
)

== Motivation

In the early 2010s I was very impressed by the popular wavetable synthetizer plugin Massive from Native Instruments (@massive).
At that time it provided new functionality to grab and drop LFO or envelope onto any knob and select how it affects given knob. This meant that the user had even more freedom to create new sounds and had been given more power control over the sound. Which lead to creative sounds and was often used for the wobbly sounds in dubstep.

Since the audio plugins are often very costly, especially for a teenager, I always wonder how hard it would be to create plugin like that.

#figure(image("img/Massive.png"), caption: [
  Native Instruments Massive.
  Plugin that was the initial inspiration and motivation for this project. Especially the feature of putting LFO and envelope onto any knob.
  ]
) <massive>

= Background <background>

#alert[
Write here about the theoretical background of the work, the algorithms used.
]

Nearly all of the components are own made, with the exception of the filter, which is the one from lectures.

== Wavetable
As is common, wavetable synthetizers usually contains single period of the used waveforms. However, since I wanted to try loading from files and then replaying the song, it supports waveforms with "different period counts" (i.e., sample block from the file that fits into the waveform buffer).

The other significant wavetable feature "position" or "morph" between the individual waveform is done by using linear interpolation. We know about the pre-calculation of the morphing, but this wasn't our priority and lerping was sufficiently fast.

The wavetable volume controls just the volume of the waveform, while you output volume is volume after applying unison and filter.

The unison is done in stereo with minus detune on left channel and positive on right channel for each voice number.

The actual processing is sample-based, rather than block-based.

The wavetable contains visual representation of the actual waveforms in the wavetable based on selected preset or from selected file. The visualization highlights the currently selected waveform and the morphing between them. The visualization draws each waveform in different color to easier recognize each waveform.

== "Effects"
This is what we call the LFO and envelope, that can affect other knobs.

The inspiration for this arose from Massive (@massive) and the GUI then was inspired by Pigments (@pigments) to have select affected component and have a row of modifier effects.

#figure(image("img/Arturia-Pigments.png"), caption: [
  Arturia Pigments. Inspiration for the row of effects.
  ]
) <pigments>

You can press the button "select component" and then click (select) which knob you want to affect. Unfortunately not all knobs can be affected by this (For example envelope knobs don't have that and unison voice and detune). Selecting a component also loads it's current multipliers of the effects for the selected component.

== LFO
The GUI visualization of LFO is also inspired by the Pigments (@pigments). The LFO contains the usual frequency knob with a knob for offset, type selection and its visualization of the current setting.

== Envelope
We used the usual ADSR (attack, decay, sustain, release) envelope with its visualization. The envelope can be either edited with the knobs or using the visualization, which also allows to make the envelope non-linear.

= Implementation 

#alert[
Please describe your implementation. You can illustrate the implementation with pseudo-code and/or diagrams. The following example of pseudo-code illustrates the programming of a delay line.
]

== Parameter

I think it is fair to start talking with the `Parameter` class, as this is meant to be abstraction for the "parameters" or "knobs", which smoothes all the value settings and provides way to set effects for each parameter. The smoothing is done by the juce `SmoothedValue` class. The `originalValue` is the value set from the GUI and `smoothedValue` is the result smoothed value after the effects processing.

The effects are done the following way: each parameter stores it effects that affects the parameter together with its the effect id, multiplier, reference and lambda to calculate the parameter value from the effect (`EffectEvaluator`) to generalize for the calculation of the value.

```cpp
using EffectEvaluator = std::function<FloatType(FloatType previousValue, FloatType originalValue, FloatType dspMultiplier, DSP<FloatType>& dsp)>;
```

The above mentioned values are store in the following vector: 
```cpp
std::vector<std::tuple<juce::String, FloatType, std::reference_wrapper<DSP<FloatType>>, EffectEvaluator>> effects;
```
This also means that it is not store in the plugin preset. The main issue would probably be the reference to the dsp.

The value when need gets calculated by iterating the effects vector, applying the lambdas of each effect on the originalValue and getting the final result.

The `DSP` class is meant to be an abstraction for a dsp can that affect the parameters value and each dsp would move it's state independently on the parameter. It is a pure virtual function that is implemented by the effects -- LFO and envelope.

This technique results in a bit chaotic and complicated approach of setting the value in `AudioProcessor.cpp` especially the functions: `getParameterEffect()` -- to get the lambda for calculating value from the dsp, `updateParameterCoefficients()` -- assign current modifier values to the gui from the parameter and `applyParameterEffect()`-- to apply the effect on the correct parameter (to all voices, to lfo, ...). 

== Wavetable

The class is basically simple wrap-around, with some helper methods for loading from preset of file.

```cpp
static constexpr std::size_t SampleSize { 2048ul };
std::vector<std::array<float, SampleSize>> wavetables;
```

== Synth Voice

Files: `WavetableSynth{.h,.cpp}`.

The voice is processed sample by sample rather than per block basis.

The increment from the wavetable is $"noteFrequency" / "originalSampleFrequency" * "sampleSize" / "sampleRate"$, where $"sampleSize"$ is the size of the array that holds one waveform "period" and $"originalSampleFrequency"$ is a value, that enables playback of a longer sample (song) at default note (A4).

The unison voices are accomplished simply by holding phases and increments for the individual voices on each channel.
```cpp
static constexpr int Channels = 2;
std::array<std::vector<float>, Channels> unisonPhases;
std::array<std::vector<float>, Channels> unisonIncrements;
```

For these the most important functions are `updateUnisonIncrements()` and in `renderNextBlock()`. Each voice increment is $"number of voice" * 0.1 * "detune"$ with left channel being negative and right one being positive values.

The output value from unison is they divided be the count of voices to avoid very loud result on low detune. We also added random initial phase for the unison voices to avoid oscillation effect in the `startNote()`.

Afterwards, the filter for each channel is processed and combined together using the panning value.

== LFO
The LFO provides the usual types: sine, square, triangle and sawtooth wave. The LFO uses the aliased version and calculates the value on the go (no wavetable used for the lfo). The frequency of the LFO is a public `Parameter` to allow changing it using the effects with selected component.

== Envelope

In the current implementation only the "Envelope A" affects the volume of the synth voice, while the "Envelope B" is only used for the custom effects.

= GUI

== Envelope

The envelope component in our synthesizer provides real-time, interactive control of the ADSR (Attack, Decay, Sustain, Release) envelope.

=== Interactive Curve Rendering

The `ADSREnvelopeComponent` is a JUCE-based component that visualizes the ADSR curve using three quadratic Bézier segments. These represent transitions between the envelope phases. The curve is defined by four main points (start, peak, sustain, end) and three control points that determine curvature.

Users can manipulate the envelope either through rotary sliders or by dragging handles within the graphical view. Endpoint and control point positions are mapped between real-world values and pixel coordinates. Sliders are linked to the audio engine using `AudioProcessorValueTreeState::SliderAttachment`.

Dragging endpoints updates ADSR values while enforcing timing constraints. Off-curve control points alter Bézier curvature and are stored in dedicated curve sliders (e.g. `attackCurveXSlider`).

=== Envelope State Visualization

To enhance feedback, the envelope includes a playhead visualization showing the phase of each active voice in real-time. Colored markers and guide lines move across the curve based on current phase and timing.

This is updated at 60Hz using a timer, which queries per-voice state from a dedicated collector. The vertical position of each playhead is computed using Bézier interpolation via the `getYForX(x)` method.

=== State Collector

Each voice's envelope state is tracked using `EnvelopeStateCollector`, which stores atomic `EnvelopeInfo` structures. These hold the current state (e.g., ATTACK, DECAY) and elapsed time.

When rendering a voice, its state is updated:

```cpp
envelopeStateCollectorA->setEnvelopeState(voiceIndex, envGenA.getCurrentState(), envGenA.getCurrentStateTimer());
```

And queried for GUI updates:

```cpp
auto states = envelopeStateCollector->getEnvelopeStateSnapshot();
```

This architecture decouples DSP-thread state from the GUI, ensuring thread safety and consistent feedback.

== LFO History

The LFO history component provides a scrolling visualization of recent values generated by an LFO. This helps users observe the shape, frequency, and modulation behavior of LFO signals in real time.

=== Visualization Logic

The HistoryPlotComponent is a JUCE-based custom component that stores a fixed-size circular buffer of floating-point values in the range −1 to 1. Each new value pushed into the buffer is plotted as a point in a scrolling waveform, with the newest value appearing on the right and the oldest on the left. 

The component is redrawn every time a new value is added using the addValue(float) method. It calculates pixel positions by scaling normalized values to fit the vertical center of the plotting area and lays out the waveform using a juce::Path. The component supports dynamic changes to buffer size via setBufferSize().

=== Feeding Values from DSP

LFOs running on the audio thread push their current values into a lock-free buffer using ParameterHistoryBuffer::pushSample(). This structure ensures real-time safety by allowing the audio thread to write continuously without blocking.

On the GUI thread, a timer periodically calls getHistory() to retrieve all collected values and append them to the plot’s display buffer. This separation guarantees that rendering remains responsive while maintaining synchronization with audio data.

Example usage from the voice render loop:

```cpp
lfo1.advancePhase();
lfo1History.pushSample(lfo1.getValue());
```

== Wavetable Plot

The wavetable component provides a dynamic visualization of the currently loaded waveforms and morph position within the wavetable. It allows users to observe all stored waveforms and how the currently playing waveform is interpolated between them.

=== Rendering and Layout

The WavetablePlotComponent is a JUCE-based component that displays multiple waveforms (ranging from 1 to 10 depending on the selected preset) stacked in a 3D-like layered view using perspective and scaling. The plot includes each waveform drawn in a distinct color, and a morphing waveform rendered between the two closest waveforms based on the current wavetablePosition.

Waveforms are drawn as filled paths overlaid on one another, and their geometry is transformed to simulate depth using a combination of offset, scaling, and perspective projection. The BoxCorners helper struct is used to create a bounding box around the waveform stack and render its edges, enhancing the spatial perception.

The animated playhead within the morphing waveform adds visual movement to the plot, giving the component a sense of life. It does not represent the actual oscillator phase but serves as a dynamic visual element alongside the morphing position.

=== Morphing Logic

To render the morphing waveform, two adjacent waveforms are linearly interpolated sample-by-sample. The current morph index is calculated from the wavetablePosition in the range [0, 1]. Color interpolation is applied between the colors of the surrounding waveforms to visually indicate blend.

The morphing waveform is also drawn slightly thicker and with a dynamic trail to highlight playback position. This effect is updated in real time using a timer running at 60Hz.

=== Controls

Users can load a new waveform file using the file picker button or select from available wavetable presets through the integrated plugin selector.

Presets are stored as arrays of individual waveforms, each consisting of a fixed number of samples. When loading a preset using loadFromPreset(), each waveform is copied from predefined data stored in the WavetablePlugins namespace (Presets were generated by a python script).

To visualize the waveforms from the presets in the UI, a preview is extracted using the getRenderPreview() method, which downsamples each waveform to a maximum number of points and limits the number of rendered waveforms to ten. using the file picker button or select from available wavetable presets through the integrated plugin selector.

= Discussion

#alert[Reflect on the project as a whole. Reflect on what was difficult and what did you learn? In what direction could the work be taken in the future?]

As synthetizer are quite robust plugins there is a lot still that can be improved or added. Many wavetable synthetizers provide their own reverbs, more filter, but we could also add multiple oscillators instead of just one.

In case of the wavetable, it could be extended to precompute the lerps between waveform.

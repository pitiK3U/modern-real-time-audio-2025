#pragma once

#include "PluginProcessor.h"
#include "juce_core/juce_core.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "mrta_utils/Source/GUI/GenericParameterEditor.h"
#include "HistoryPlotComponent.h"
#include "ADSREnvelopeComponent.h"

class WavetableSynthAudioProcessorEditor
: public juce::AudioProcessorEditor
, private juce::Timer
, private juce::AudioProcessorValueTreeState::Listener
{
public:
    WavetableSynthAudioProcessorEditor(WavetableSynthAudioProcessor&);
    ~WavetableSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const MouseEvent& mouseEvent) override;

    static constexpr int NUM_SECTIONS { 5 };
    static constexpr int SECTION_WIDTH { 250 };
    static constexpr int SECTION_SPACER_WIDTH { 20 };
    static constexpr int LABEL_HEIGHT { 50 };
    static constexpr int MAX_PARAM_COUNT { 5 };
    static constexpr int PARAM_HEIGHT { 100 };
    static constexpr int HISTORY_PLOT_HEIGHT { 100 };
    static constexpr float REFRESH_RATE { 60.0f };

private:
    WavetableSynthAudioProcessor& audioProcessor;
    mrta::GenericParameterEditor oscParamEditor;
    mrta::GenericParameterEditor filterParamEditor;
    mrta::GenericParameterEditor lfo1ParamEditor;
    mrta::GenericParameterEditor lfo2ParamEditor;
    mrta::GenericParameterEditor effectParamEditor;
    GUI::HistoryPlotComponent lfo1HistoryPlot;
    GUI::HistoryPlotComponent lfo2HistoryPlot;
    GUI::ADSREnvelopeComponent adsrComponent;

    juce::AudioProcessorValueTreeState& vts;

    juce::Label oscLabel;
    juce::Label vcaEnvLabel;
    juce::Label filterLabel;
    juce::Label lfo1Label;
    juce::Label lfo2Label;

    juce::Label selectedComponentLabel;
    juce::TextButton selectButton;
    bool selectMode { false };

    void toggleSelectMode();

    void setupLabel(juce::Label& label);
    void timerCallback() override;

    // Listener callback
    void parameterChanged (const juce::String& paramID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableSynthAudioProcessorEditor)
};
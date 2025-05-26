#pragma once

#include "PluginProcessor.h"
#include "mrta_utils/Source/GUI/GenericParameterEditor.h"
#include "HistoryPlotComponent.h"

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

    static constexpr int NUM_SECTIONS { 7 };
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
    mrta::GenericParameterEditor vcaEnvParamEditor;
    mrta::GenericParameterEditor vcfEnvParamEditor;
    mrta::GenericParameterEditor lfoParamEditor;
    mrta::GenericParameterEditor filterParamEditor;
    mrta::GenericParameterEditor testLfoEditor;
    mrta::GenericParameterEditor lfo1ParamEditor;
    HistoryPlotComponent lfoHistoryPlot;

    juce::AudioProcessorValueTreeState& vts;

    juce::Label oscLabel;
    juce::Label vcaEnvLabel;
    juce::Label vcfEnvLabel;
    juce::Label lfoLabel;
    juce::Label filterLabel;
    juce::Label finalLfoLabel;
    juce::Label lfo1Label;

    void setupLabel(juce::Label& label);
    void timerCallback() override;

    // Listener callback
    void parameterChanged (const juce::String& paramID, float newValue) override
    {
        if (paramID == Param::ID::HistoryPlotBufferSize) {
            // This is still the audio thread!, so queue onto the message thread (otherwise it will crash):
            juce::MessageManager::callAsync ([this, newValue]()
            {
                lfoHistoryPlot.setBufferSize ((int) newValue);
            });
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableSynthAudioProcessorEditor)
};
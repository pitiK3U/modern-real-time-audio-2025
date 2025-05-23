#pragma once

#include "PluginProcessor.h"
#include "mrta_utils/Source/GUI/GenericParameterEditor.h"

class WavetableSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    WavetableSynthAudioProcessorEditor(WavetableSynthAudioProcessor&);
    ~WavetableSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int NUM_SECTIONS { 6 };
    static constexpr int SECTION_WIDTH { 250 };
    static constexpr int SECTION_SPACER_WIDTH { 20 };
    static constexpr int LABEL_HEIGHT { 50 };
    static constexpr int MAX_PARAM_COUNT { 5 };
    static constexpr int PARAM_HEIGHT { 100 };

private:
    WavetableSynthAudioProcessor& audioProcessor;
    mrta::GenericParameterEditor oscParamEditor;
    mrta::GenericParameterEditor vcaEnvParamEditor;
    mrta::GenericParameterEditor vcfEnvParamEditor;
    mrta::GenericParameterEditor lfoParamEditor;
    mrta::GenericParameterEditor filterParamEditor;
    mrta::GenericParameterEditor testLfoEditor;

    juce::Label oscLabel;
    juce::Label vcaEnvLabel;
    juce::Label vcfEnvLabel;
    juce::Label lfoLabel;
    juce::Label filterLabel;
    juce::Label finalLfoLabel;

    void setupLabel(juce::Label& label);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableSynthAudioProcessorEditor)
};
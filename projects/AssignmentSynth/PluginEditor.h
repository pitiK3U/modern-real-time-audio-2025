#pragma once

#include "PluginProcessor.h"
#include "mrta_utils/Source/GUI/GenericParameterEditor.h"

class AssignmentSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AssignmentSynthAudioProcessorEditor(AssignmentSynthAudioProcessor&);
    ~AssignmentSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int ColumWidth = 300;
    static constexpr int Columns = 3;
    static constexpr int ParamHeight = 80;
    static constexpr int MaxNumberOfParams = 5;

private:
    AssignmentSynthAudioProcessor& audioProcessor;
    mrta::GenericParameterEditor synthEditor;
    mrta::GenericParameterEditor filterEditor;
    mrta::GenericParameterEditor lfoEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AssignmentSynthAudioProcessorEditor)
};
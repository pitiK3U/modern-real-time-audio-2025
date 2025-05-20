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

private:
    AssignmentSynthAudioProcessor& audioProcessor;
    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AssignmentSynthAudioProcessorEditor)
};
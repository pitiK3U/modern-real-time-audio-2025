#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "mrta_utils/Source/GUI/GenericParameterEditor.h"

AssignmentSynthAudioProcessorEditor::AssignmentSynthAudioProcessorEditor(AssignmentSynthAudioProcessor& p) :
    juce::AudioProcessorEditor(p), audioProcessor(p),
    synthEditor(audioProcessor.getParameterManager(), ParamHeight,
     {Param::ID::WaveType, Param::ID::AttackTime, Param::ID::DecayTime, Param::ID::SustainLevel, Param::ID::ReleaseTime}),
    filterEditor(audioProcessor.getParameterManager(), ParamHeight,
        {Param::ID::BandType, Param::ID::BandFreq, Param::ID::BandReso, Param::ID::BandGain}),
    lfoEditor(audioProcessor.getParameterManager(), ParamHeight,
        {Param::ID::LfoEnable, Param::ID::LfoType, Param::ID::LfoFreq})
{
    
    addAndMakeVisible(synthEditor);
    addAndMakeVisible(filterEditor);
    addAndMakeVisible(lfoEditor);
    setSize(ColumWidth * Columns, MaxNumberOfParams * ParamHeight);
}


AssignmentSynthAudioProcessorEditor::~AssignmentSynthAudioProcessorEditor()
{
}

//==============================================================================
void AssignmentSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AssignmentSynthAudioProcessorEditor::resized()
{
    auto localBounds { getLocalBounds() };

    synthEditor.setBounds(localBounds.removeFromLeft(ColumWidth));
    filterEditor.setBounds(localBounds.removeFromLeft(ColumWidth));
    lfoEditor.setBounds(localBounds);
}
#include "PluginEditor.h"
#include "mrta_utils/Source/GUI/GenericParameterEditor.h"

AssignmentSynthAudioProcessorEditor::AssignmentSynthAudioProcessorEditor(AssignmentSynthAudioProcessor& p) :
    juce::AudioProcessorEditor(p), audioProcessor(p),
    paramEditor(audioProcessor.getParameterManager())
{
    int height = static_cast<int>(audioProcessor.getParameterManager().getParameters().size())
               * paramEditor.parameterWidgetHeight;
    setSize(300, height);
    addAndMakeVisible(paramEditor);
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
    paramEditor.setBounds(getLocalBounds());
}
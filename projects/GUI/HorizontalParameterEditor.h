#pragma once

#include <JuceHeader.h>
#include "mrta_utils/Source/Parameter/ParameterManager.h"
#include "mrta_utils/Source/Parameter/ParameterInfo.h"

namespace GUI
{

class HorizontalParameterEditor : public juce::Component
{
public:
    HorizontalParameterEditor(mrta::ParameterManager& parameterManager,
                           const juce::StringArray& parameterIDs = {});
    HorizontalParameterEditor() = delete;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::StringArray parameterIDs;
    std::vector<std::unique_ptr<juce::Component>> parameterComponents;
    std::vector<std::unique_ptr<juce::Label>> parameterLabels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HorizontalParameterEditor)
};

}

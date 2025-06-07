#include "HorizontalParameterEditor.h"

namespace GUI
{

HorizontalParameterEditor::HorizontalParameterEditor(mrta::ParameterManager& parameterManager, const juce::StringArray& pids) :
    parameterIDs { pids }
{
    juce::AudioProcessorValueTreeState& apvts { parameterManager.getAPVTS() };

    const std::vector<mrta::ParameterInfo>& originalParameters { parameterManager.getParameters() };
    std::vector<mrta::ParameterInfo> parameters;

    if (parameterIDs.size())
    {
        for (const mrta::ParameterInfo& pi : originalParameters)
            if (parameterIDs.contains(pi.ID))
                parameters.push_back(pi);
    }
    else
    {
        for (const mrta::ParameterInfo& pi : originalParameters)
            parameters.push_back(pi);
    }

    std::transform(parameters.begin(), parameters.end(), std::back_inserter(parameterComponents),
    [&apvts] (const mrta::ParameterInfo& p)
    {
        std::unique_ptr<juce::Component> ptr;
        switch (p.type)
        {
            case mrta::ParameterInfo::Float:
                ptr = std::make_unique<mrta::ParameterSlider>(p.ID, apvts, p.startAngle, p.endAngle, p.stopAtEnd);
                break;

            case mrta::ParameterInfo::Choice:
                ptr = std::make_unique<mrta::ParameterComboBox>(p.ID, apvts);
                break;

            case mrta::ParameterInfo::Bool:
                ptr = std::make_unique<mrta::ParameterButton>(p.ID, apvts);
                break;
        }
        return ptr;
    });

    std::transform(parameters.begin(), parameters.end(), std::back_inserter(parameterLabels),
    [&apvts] (const mrta::ParameterInfo& p)
    {
        std::unique_ptr<juce::Label> ptr { std::make_unique<juce::Label>() };
        ptr->setText(p.name, juce::dontSendNotification);
        ptr->setFont(juce::FontOptions(12.f));
        return ptr;
    });

    std::for_each(parameterComponents.begin(), parameterComponents.end(), [this] (const auto& pc) { addAndMakeVisible(pc.get()); });
    std::for_each(parameterLabels.begin(), parameterLabels.end(), [this] (const auto& pl) { addAndMakeVisible(pl.get()); });
}

void HorizontalParameterEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
void HorizontalParameterEditor::resized()
{
    static juce::Rectangle<float> labelProportion { 0.025f, 0.175f, 0.25f, 0.65f };
    static juce::Rectangle<float> widgetProportion { 0.3f, 0.175f, 0.675f, 0.65f };

    auto localBounds { getLocalBounds() };
    int numOfParams { static_cast<int>(parameterComponents.size()) };
    auto parameterWidth = localBounds.getWidth() / numOfParams;
    for (int i = 0; i < numOfParams; ++i)
    {
        auto paramBounds { localBounds.removeFromLeft( parameterWidth ) };
        auto labelBounds { paramBounds.getProportion(labelProportion) };
        auto widgetBounds { paramBounds.getProportion(widgetProportion) };
        parameterLabels[i]->setBounds(labelBounds);
        parameterComponents[i]->setBounds(widgetBounds);
    }
}

}

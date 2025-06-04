#include "PluginEditor.h"
#include "ADSREnvelopeComponent.h"
#include "PluginProcessor.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "mrta_utils/Source/GUI/ParameterComponents.h"
#include <optional>

WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor(WavetableSynthAudioProcessor& p) :
    juce::AudioProcessorEditor(p), audioProcessor(p),
    oscParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::WavetablePosition, Param::ID::UnisonVoices, Param::ID::UnisonDetune, Param::ID::WavetableVol, Param::ID::OutputVol }),
    filterParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::VCF_Cutoff, Param::ID::VCF_Reso, Param::ID::VCF_Type }),
    lfo1ParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::LFO1_Freq, Param::ID::LFO1_Type, Param::ID::LFO1_Offset }),
    lfo2ParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::LFO2_Freq, Param::ID::LFO2_Type, Param::ID::LFO2_Offset, Param::ID::HistoryPlotBufferSize }),
    effectParamEditor(p.getParamManager(), { Param::ID::LFO1_mult, Param::ID::LFO2_mult }),
    oscLabel("", "Wavetable"),
    vcaEnvLabel("", "Amplitude Envelope"),
    filterLabel("", "Filter"),
    lfo1Label("", "LFO 1"),
    lfo2Label("", "LFO 2"),
    selectedComponentLabel("Selected component label", ""),
    lfo1HistoryPlot(32768),
    lfo2HistoryPlot(32768),
    adsrComponent(
        p.getParamManager().getAPVTS(),
        p.getEnvelopeStateCollector(),
        Param::ID::EnvelopeAttackTime,
        Param::ID::EnvelopeDecayTime,
        Param::ID::EnvelopeSustain,
        Param::ID::EnvelopeReleaseTime,
        Param::ID::EnvelopeAttackCurveX,
        Param::ID::EnvelopeAttackCurveY,
        Param::ID::EnvelopeDecayCurveX,
        Param::ID::EnvelopeDecayCurveY,
        Param::ID::EnvelopeReleaseCurveX,
        Param::ID::EnvelopeReleaseCurveY,
        Param::Ranges::ADSRPlotWidth
    ),
    vts (p.getParamManager().getAPVTS()),
    selectButton("Select component")
    {
    addAndMakeVisible(oscParamEditor);
    addAndMakeVisible(filterParamEditor);
    addAndMakeVisible(lfo1ParamEditor);
    addAndMakeVisible(lfo2ParamEditor);
    addAndMakeVisible(effectParamEditor);

    addAndMakeVisible (lfo1HistoryPlot);
    addAndMakeVisible(lfo2HistoryPlot);
    addAndMakeVisible (adsrComponent);

    setupLabel(oscLabel);
    setupLabel(filterLabel);
    setupLabel(lfo1Label);
    setupLabel(lfo2Label);
    setupLabel(selectedComponentLabel);

    addAndMakeVisible(selectButton);
    selectButton.onClick = [this] {
        toggleSelectMode();
    };

    vts.addParameterListener (Param::ID::HistoryPlotBufferSize, this);

    startTimerHz ((int) REFRESH_RATE);

    setSize(NUM_SECTIONS * SECTION_WIDTH + (NUM_SECTIONS - 1) * SECTION_SPACER_WIDTH, LABEL_HEIGHT + PARAM_HEIGHT * (MAX_PARAM_COUNT + 1));
}

WavetableSynthAudioProcessorEditor::~WavetableSynthAudioProcessorEditor()
{
    vts.removeParameterListener (Param::ID::HistoryPlotBufferSize, this);
}

void WavetableSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    for (int i = 1; i < NUM_SECTIONS; ++i)
        g.fillRect(i * (SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) - 1, 0, 2, getHeight());
}

void WavetableSynthAudioProcessorEditor::resized()
{
    auto bounds { getLocalBounds() };

    {
        auto bottomBounds { bounds.removeFromBottom(PARAM_HEIGHT) };
        auto buttonBounds { bottomBounds.removeFromLeft(SECTION_WIDTH) };
        selectedComponentLabel.setBounds(buttonBounds.removeFromTop(PARAM_HEIGHT / 2));
        selectButton.setBounds( buttonBounds );
        effectParamEditor.setBounds( bottomBounds );
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        adsrComponent.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        oscLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        oscParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        filterLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        filterParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        lfo1Label.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        lfo1HistoryPlot.setBounds(secBounds.removeFromTop(HISTORY_PLOT_HEIGHT));
        lfo1ParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        lfo2Label.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        lfo2HistoryPlot.setBounds(secBounds.removeFromTop(HISTORY_PLOT_HEIGHT));
        lfo2ParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }
}

void WavetableSynthAudioProcessorEditor::timerCallback()
{
    
    audioProcessor.getParamManager().updateParameters();

    // 1) pull the most recent LFO block from your processor
    std::vector<float> block1;
    std::vector<float> block2;
    audioProcessor.getLastLfo1Values (block1);
    audioProcessor.getLastLfo2Values (block2);

    // 2) feed each sample into the history plot
    for (auto v : block1)
        lfo1HistoryPlot.addValue (v);
    for (auto v : block2)
        lfo2HistoryPlot.addValue (v);
}

void WavetableSynthAudioProcessorEditor::setupLabel(juce::Label& label)
{
    label.setFont(juce::FontOptions((25.f)));
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void WavetableSynthAudioProcessorEditor::parameterChanged (const juce::String& paramID, float newValue) {
    if (paramID == Param::ID::HistoryPlotBufferSize) {
        // This is still the audio thread!, so queue onto the message thread (otherwise it will crash):
        juce::MessageManager::callAsync ([this, newValue]()
        {
            lfo1HistoryPlot.setBufferSize ((int) newValue);
            lfo2HistoryPlot.setBufferSize ((int) newValue);
        });
    }
}

void WavetableSynthAudioProcessorEditor::mouseDown(const MouseEvent& mouseEvent)
{
    if (!selectMode)
        return;

    toggleSelectMode();

    auto * mouseSelectedComponent = getComponentAt(mouseEvent.getPosition());
    if (mouseSelectedComponent == nullptr) return;
    
    auto * slider = dynamic_cast<mrta::ParameterSlider *>(mouseSelectedComponent);
    if (slider == nullptr)
        return;

    selectedComponentLabel.setText(slider->parameterID, juce::dontSendNotification);

    audioProcessor.selectedParameter = slider->parameterID;
}

void WavetableSynthAudioProcessorEditor::toggleSelectMode()
{
    selectMode = !selectMode;

    if (selectMode) {
        setInterceptsMouseClicks(true, false);
        setMouseCursor(juce::MouseCursor::CrosshairCursor);
        for (auto child : getChildren()) {
            child->setInterceptsMouseClicks(false, false);
        }
    } else {
        setInterceptsMouseClicks(true, true);
        setMouseCursor(juce::MouseCursor::NormalCursor);
        for (auto child : getChildren()) {
            child->setInterceptsMouseClicks(true, true);
        }
    }

    repaint();
}
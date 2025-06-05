#include "PluginEditor.h"
#include "ADSREnvelopeComponent.h"
#include "PluginProcessor.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "mrta_utils/Source/GUI/ParameterComponents.h"
#include "WavetablePlugins.h"
#include <optional>

WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor(WavetableSynthAudioProcessor& p) :
    juce::AudioProcessorEditor(p), audioProcessor(p),
    oscParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::WavetablePosition, Param::ID::UnisonVoices, Param::ID::UnisonDetune, Param::ID::WavetableVol, Param::ID::OutputVol }),
    filterParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::VCF_Cutoff, Param::ID::VCF_Reso, Param::ID::VCF_Type }),
    lfo1ParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::LFO1_Freq, Param::ID::LFO1_Type, Param::ID::LFO1_Offset }),
    lfo2ParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::LFO2_Freq, Param::ID::LFO2_Type, Param::ID::LFO2_Offset, Param::ID::HistoryPlotBufferSize }),
    effectParamEditor(p.getParamManager(), { Param::ID::EnvelopeA_mult, Param::ID::EnvelopeB_mult, Param::ID::LFO1_mult, Param::ID::LFO2_mult }),
    oscLabel("", "Wavetable"),
    vcaEnvLabel("", "Amplitude Envelope"),
    filterLabel("", "Filter"),
    lfo1Label("", "LFO 1"),
    lfo2Label("", "LFO 2"),
    envelopeLabel("", "Envelope"),
    wavetableLabel("", "Wavetable"),
    selectedComponentLabel("Selected component label", ""),
    lfo1HistoryPlot(32768),
    lfo2HistoryPlot(32768),
    adsrComponentA(
        p.getParamManager().getAPVTS(),
        p.getEnvelopeStateCollector(0),
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
    adsrComponentB(
        p.getParamManager().getAPVTS(),
        p.getEnvelopeStateCollector(1),
        Param::ID::Envelope_B_AttackTime,
        Param::ID::Envelope_B_DecayTime,
        Param::ID::Envelope_B_Sustain,
        Param::ID::Envelope_B_ReleaseTime,
        Param::ID::Envelope_B_AttackCurveX,
        Param::ID::Envelope_B_AttackCurveY,
        Param::ID::Envelope_B_DecayCurveX,
        Param::ID::Envelope_B_DecayCurveY,
        Param::ID::Envelope_B_ReleaseCurveX,
        Param::ID::Envelope_B_ReleaseCurveY,
        Param::Ranges::ADSRPlotWidth
    ),
    vts (p.getParamManager().getAPVTS()),
    wavetablePlotComponent(
        DSP::WavetablePlugins::pluginPlugin_A,
        p.getParamManager(),
        Param::ID::SelectedWavetablePreset,
        [this]() { openFilePicker(); }
    ),
    selectButton("Select component")
    {
    addAndMakeVisible(oscParamEditor);
    addAndMakeVisible(filterParamEditor);
    addAndMakeVisible(lfo1ParamEditor);
    addAndMakeVisible(lfo2ParamEditor);
    addAndMakeVisible(effectParamEditor);

    addAndMakeVisible (lfo1HistoryPlot);
    addAndMakeVisible(lfo2HistoryPlot);
    addAndMakeVisible (adsrComponentA);
    addAndMakeVisible (adsrComponentB);
    addAndMakeVisible(wavetablePlotComponent);

    addAndMakeVisible(envelopeAButton);
    addAndMakeVisible(envelopeBButton);

    setupLabel(oscLabel);
    setupLabel(filterLabel);
    setupLabel(lfo1Label);
    setupLabel(lfo2Label);
    setupLabel(envelopeLabel);
    setupLabel(wavetableLabel);
    setupLabel(selectedComponentLabel);

    addAndMakeVisible(selectButton);
    selectButton.onClick = [this] {
        toggleSelectMode();
    };

    vts.addParameterListener (Param::ID::HistoryPlotBufferSize, this);

    initializeEnvelopeSwitcher();

    startTimerHz ((int) REFRESH_RATE);

    p.getParamManager().registerParameterCallback(Param::ID::SelectedWavetablePreset, [this, &p](float value, bool /*force*/) {
        juce::MessageManager::callAsync([this, value]() {
            const auto presetId = DSP::WavetablePlugins::getPresetId((unsigned int)value);
            wavetablePlotComponent.setWavetable(DSP::WavetablePlugins::getPreset(presetId));
        });

        auto wavetableId = DSP::WavetablePlugins::getPresetId((unsigned int) value);
        p.setWavetable(wavetableId);
    });

    setSize(NUM_SECTIONS * SECTION_WIDTH + (NUM_SECTIONS - 1) * SECTION_SPACER_WIDTH, LABEL_HEIGHT + PARAM_HEIGHT * (MAX_PARAM_COUNT + 1));
}

WavetableSynthAudioProcessorEditor::~WavetableSynthAudioProcessorEditor()
{
    vts.removeParameterListener (Param::ID::HistoryPlotBufferSize, this);
}

void WavetableSynthAudioProcessorEditor::openFilePicker()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
        juce::File {},
        "*.wav"); // [7]
        auto chooserFlags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;
        fileChooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc) // [8]
        {
            auto file = fc.getResult();
            if (file != juce::File {}) // [9]
            {
                audioProcessor.loadFile(file);
                std::vector<std::vector<float>> wavetablePreview = audioProcessor.getWavetablePreview();

                juce::MessageManager::callAsync([this, wavetablePreview]() {
                    wavetablePlotComponent.setWavetable(wavetablePreview);
                });
            }
        });
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
        envelopeLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        
        constexpr int buttonHeight = 24;
        constexpr int buttonPadding = 6;

        auto adsrBounds = secBounds.removeFromTop(secBounds.getHeight() - (buttonHeight + buttonPadding));
        adsrComponentA.setBounds(adsrBounds.withSizeKeepingCentre(SECTION_WIDTH, adsrBounds.getHeight()));
        adsrComponentB.setBounds(adsrBounds.withSizeKeepingCentre(SECTION_WIDTH, adsrBounds.getHeight()));

        auto buttonRow = secBounds.reduced(6, 0); // horizontal padding from sides
        auto buttonWidth = (buttonRow.getWidth() - 6) / 2; // spacing between buttons = 6
        envelopeAButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
        buttonRow.removeFromLeft(6); // spacer
        envelopeBButton.setBounds(buttonRow);
        
        envelopeAButton.setBounds(envelopeAButton.getBounds().withHeight(buttonHeight));
        envelopeBButton.setBounds(envelopeBButton.getBounds().withHeight(buttonHeight));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        wavetableLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        wavetablePlotComponent.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
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

    // 3) Get wavetable position
    float index = audioProcessor.lastWavetablePosition.load(std::memory_order_relaxed);
    float normalized = index / static_cast<float>(audioProcessor.getWavetableCount() - 1);

    wavetablePlotComponent.setWavetablePosition(normalized);
    wavetablePlotComponent.repaint();
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

void WavetableSynthAudioProcessorEditor::initializeEnvelopeSwitcher() {
    envelopeAButton.onClick = [this]
    {
        showingEnvelopeA = true;
        adsrComponentA.setVisible(true);
        adsrComponentB.setVisible(false);

        envelopeAButton.setToggleState(true, juce::dontSendNotification);
        envelopeBButton.setToggleState(false, juce::dontSendNotification);
    };

    envelopeBButton.onClick = [this]
    {
        showingEnvelopeA = false;
        adsrComponentA.setVisible(false);
        adsrComponentB.setVisible(true);

        envelopeAButton.setToggleState(false, juce::dontSendNotification);
        envelopeBButton.setToggleState(true, juce::dontSendNotification);
    };

    envelopeAButton.setClickingTogglesState(true);
    envelopeBButton.setClickingTogglesState(true);
    adsrComponentA.setVisible(true);
    adsrComponentB.setVisible(false);
    envelopeAButton.setToggleState(true, juce::dontSendNotification);
    envelopeBButton.setToggleState(false, juce::dontSendNotification);
    auto& lf = getLookAndFeel();
    envelopeAButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::lightblue.darker(0.3f));
    envelopeBButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::lightblue.darker(0.3f));
}

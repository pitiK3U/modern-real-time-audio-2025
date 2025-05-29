#include "PluginEditor.h"
#include "ADSREnvelopeComponent.h"
#include "PluginProcessor.h"

WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor(WavetableSynthAudioProcessor& p) :
    juce::AudioProcessorEditor(p), audioProcessor(p),
    oscParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::WavetablePosition, Param::ID::WavetableVol, Param::ID::OutputVol }),
    vcaEnvParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::VCA_AttTime, Param::ID::VCA_DecayTime, Param::ID::VCA_Sustain, Param::ID::VCA_RelTime }),
    vcfEnvParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::VCF_AttTime, Param::ID::VCF_DecayTime, Param::ID::VCF_Sustain, Param::ID::VCF_RelTime }),
    lfoParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::VCF_LFOFreq, Param::ID::VCF_LFOType }),
    filterParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::VCF_Cutoff, Param::ID::VCF_Reso, Param::ID::VCF_Type, Param::ID::VCF_EnvAmount, Param::ID::VCF_LFOAmount }),
    testLfoEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::FinalVol, Param::ID::LFO_freq, Param::ID::LFO_mult }),
    lfo1ParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::LFO1_Freq, Param::ID::LFO1_Type, Param::ID::LFO1_Offset, Param::ID::HistoryPlotBufferSize }),
    lfo2ParamEditor(p.getParamManager(), PARAM_HEIGHT, { Param::ID::LFO2_Freq, Param::ID::LFO2_Type, Param::ID::LFO2_Offset }),
    oscLabel("", "Wavetable"),
    vcaEnvLabel("", "Amplitude Envelope"),
    vcfEnvLabel("", "Filter Envelope"),
    lfoLabel("", "Filter LFO"),
    filterLabel("", "Filter"),
    finalLfoLabel("", "Volume LFO"),
    lfo1Label("", "LFO 1"),
    lfo2Label("", "LFO 2"),
    lfo1HistoryPlot(32768),
    lfo2HistoryPlot(32768),
    adsrComponent(
        p.getParamManager().getAPVTS(),
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
    vts (p.getParamManager().getAPVTS())
{
    addAndMakeVisible(oscParamEditor);
    addAndMakeVisible(vcaEnvParamEditor);
    addAndMakeVisible(vcfEnvParamEditor);
    addAndMakeVisible(lfoParamEditor);
    addAndMakeVisible(filterParamEditor);
    addAndMakeVisible(testLfoEditor);
    addAndMakeVisible(lfo1ParamEditor);
    addAndMakeVisible(lfo2ParamEditor);

    addAndMakeVisible (lfo1HistoryPlot);
    addAndMakeVisible(lfo2HistoryPlot);
    addAndMakeVisible (adsrComponent);

    setupLabel(oscLabel);
    setupLabel(vcaEnvLabel);
    setupLabel(vcfEnvLabel);
    setupLabel(lfoLabel);
    setupLabel(filterLabel);
    setupLabel(finalLfoLabel);
    setupLabel(lfo1Label);
    setupLabel(lfo2Label);

    vts.addParameterListener (Param::ID::HistoryPlotBufferSize, this);

    startTimerHz ((int) REFRESH_RATE);

    setSize(NUM_SECTIONS * SECTION_WIDTH + (NUM_SECTIONS - 1) * SECTION_SPACER_WIDTH, LABEL_HEIGHT + PARAM_HEIGHT * MAX_PARAM_COUNT);
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
        vcaEnvLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        vcaEnvParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        filterLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        filterParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        vcfEnvLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        vcfEnvParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        lfoLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        lfoParamEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
    }

    {
        auto secBounds { bounds.removeFromLeft(SECTION_WIDTH + SECTION_SPACER_WIDTH / 2) };
        finalLfoLabel.setBounds(secBounds.removeFromTop(LABEL_HEIGHT));
        testLfoEditor.setBounds(secBounds.withSizeKeepingCentre(SECTION_WIDTH, secBounds.getHeight()));
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
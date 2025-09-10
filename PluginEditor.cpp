#include "PluginEditor.h"

//==============================================================================
EnhancedLowpassAudioProcessorEditor::EnhancedLowpassAudioProcessorEditor(EnhancedLowpassAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Cutoff slider
    cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    cutoffSlider.setPopupDisplayEnabled(true, false, this);
    cutoffSlider.setSkewFactorFromMidPoint(1000.0);
    addAndMakeVisible(cutoffSlider);

    // Cutoff label
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    cutoffLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(cutoffLabel);

    // Resonance slider
    qSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    qSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(qSlider);

    // Resonance label
    qLabel.setText("Resonance", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centred);
    qLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(qLabel);

    // Gain slider
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(gainSlider);

    // Gain label
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(gainLabel);

    // Wet/dry slider
    wetDrySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    wetDrySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    wetDrySlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(wetDrySlider);

    // Wet/dry label
    wetDryLabel.setText("Dry/Wet", juce::dontSendNotification);
    wetDryLabel.setJustificationType(juce::Justification::centred);
    wetDryLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(wetDryLabel);

    // Attach parameters
    cutoffAttachment = std::make_unique<Attachment>(processorRef.apvts, "cutoff", cutoffSlider);
    qAttachment = std::make_unique<Attachment>(processorRef.apvts, "q", qSlider);
    gainAttachment = std::make_unique<Attachment>(processorRef.apvts, "gain", gainSlider);
    wetDryAttachment = std::make_unique<Attachment>(processorRef.apvts, "wetdry", wetDrySlider);

    setSize(500, 350); // Adjusted for 4 sliders with labels
}

EnhancedLowpassAudioProcessorEditor::~EnhancedLowpassAudioProcessorEditor() {}

//==============================================================================
void EnhancedLowpassAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);

    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Enhanced Lowpass", getLocalBounds().removeFromTop(30),
        juce::Justification::centred, 1);
}

void EnhancedLowpassAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto header = area.removeFromTop(30);
    auto knobArea = area.removeFromTop(150);

    auto leftHalf = knobArea.removeFromLeft(knobArea.getWidth() / 2);
    auto cutoffArea = leftHalf.removeFromLeft(leftHalf.getWidth() / 2).reduced(10);
    cutoffSlider.setBounds(cutoffArea.removeFromTop(100));
    cutoffLabel.setBounds(cutoffArea);

    auto qArea = leftHalf.reduced(10);
    qSlider.setBounds(qArea.removeFromTop(100));
    qLabel.setBounds(qArea);

    auto rightHalf = knobArea;
    auto gainArea = rightHalf.removeFromLeft(rightHalf.getWidth() / 2).reduced(10);
    gainSlider.setBounds(gainArea.removeFromTop(100));
    gainLabel.setBounds(gainArea);

    auto wetDryArea = rightHalf.reduced(10);
    wetDrySlider.setBounds(wetDryArea.removeFromTop(100));
    wetDryLabel.setBounds(wetDryArea);
}
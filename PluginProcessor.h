#pragma once

#include <JuceHeader.h>

class EnhancedLowpassAudioProcessor : public juce::AudioProcessor
{
public:
    EnhancedLowpassAudioProcessor();
    ~EnhancedLowpassAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter; // Stereo IIR filter
    juce::dsp::DryWetMixer<float> dryWetMixer; // Dry/wet mixing

    juce::SmoothedValue<float> cutoffSmoothed;
    juce::SmoothedValue<float> qSmoothed;
    juce::SmoothedValue<float> gainSmoothed;
    juce::SmoothedValue<float> wetDrySmoothed;

    double currentSampleRate = 44100.0;
    float smoothingSeconds = 0.05f;

    // Dithering state (inspired by Airwindows)
    uint32_t fpdL = 1;
    uint32_t fpdR = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnhancedLowpassAudioProcessor)
};
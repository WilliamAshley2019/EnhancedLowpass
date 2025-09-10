#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
EnhancedLowpassAudioProcessor::EnhancedLowpassAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout()),
    dryWetMixer(256)
{
    auto* cutoffParam = apvts.getRawParameterValue("cutoff");
    auto* qParam = apvts.getRawParameterValue("q");
    auto* gainParam = apvts.getRawParameterValue("gain");
    auto* wetDryParam = apvts.getRawParameterValue("wetdry");

    cutoffSmoothed.reset(currentSampleRate, smoothingSeconds);
    qSmoothed.reset(currentSampleRate, smoothingSeconds);
    gainSmoothed.reset(currentSampleRate, smoothingSeconds);
    wetDrySmoothed.reset(currentSampleRate, smoothingSeconds);

    cutoffSmoothed.setCurrentAndTargetValue(*cutoffParam);
    qSmoothed.setCurrentAndTargetValue(*qParam);
    gainSmoothed.setCurrentAndTargetValue(*gainParam);
    wetDrySmoothed.setCurrentAndTargetValue(*wetDryParam);

    // Initialize dithering (inspired by XLowpass)
    while (fpdL < 16386) fpdL = rand() * UINT32_MAX;
    while (fpdR < 16386) fpdR = rand() * UINT32_MAX;
}

EnhancedLowpassAudioProcessor::~EnhancedLowpassAudioProcessor() {}

AudioProcessorValueTreeState::ParameterLayout EnhancedLowpassAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterFloat>(
        "cutoff", "Cutoff",
        NormalisableRange<float>(20.0f, 20000.0f, 0.01f, 0.3f), 2000.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        "q", "Resonance",
        NormalisableRange<float>(0.1f, 10.0f, 0.01f), 0.707f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        "gain", "Gain",
        NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f)); // dB gain

    params.push_back(std::make_unique<AudioParameterFloat>(
        "wetdry", "Dry/Wet",
        NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f)); // Fully wet by default

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String EnhancedLowpassAudioProcessor::getName() const { return "EnhancedLowpass"; }
bool EnhancedLowpassAudioProcessor::acceptsMidi() const { return false; }
bool EnhancedLowpassAudioProcessor::producesMidi() const { return false; }
bool EnhancedLowpassAudioProcessor::isMidiEffect() const { return false; }
double EnhancedLowpassAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int EnhancedLowpassAudioProcessor::getNumPrograms() { return 1; }
int EnhancedLowpassAudioProcessor::getCurrentProgram() { return 0; }
void EnhancedLowpassAudioProcessor::setCurrentProgram(int) {}
const juce::String EnhancedLowpassAudioProcessor::getProgramName(int) { return {}; }
void EnhancedLowpassAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void EnhancedLowpassAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    dsp::ProcessSpec spec{ sampleRate, static_cast<uint32>(samplesPerBlock), 2 };
    filter.prepare(spec);
    filter.reset();

    dryWetMixer.prepare(spec);
    dryWetMixer.setWetMixProportion(apvts.getRawParameterValue("wetdry")->load());

    cutoffSmoothed.reset(currentSampleRate, smoothingSeconds);
    qSmoothed.reset(currentSampleRate, smoothingSeconds);
    gainSmoothed.reset(currentSampleRate, smoothingSeconds);
    wetDrySmoothed.reset(currentSampleRate, smoothingSeconds);

    auto* cutoffParam = apvts.getRawParameterValue("cutoff");
    auto* qParam = apvts.getRawParameterValue("q");
    auto* gainParam = apvts.getRawParameterValue("gain");
    auto* wetDryParam = apvts.getRawParameterValue("wetdry");

    cutoffSmoothed.setCurrentAndTargetValue(*cutoffParam);
    qSmoothed.setCurrentAndTargetValue(*qParam);
    gainSmoothed.setCurrentAndTargetValue(*gainParam);
    wetDrySmoothed.setCurrentAndTargetValue(*wetDryParam);

    // Set initial filter coefficients
    *filter.state = *dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, *cutoffParam, *qParam);
}

void EnhancedLowpassAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EnhancedLowpassAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}
#endif

void EnhancedLowpassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // Get target parameter values
    const float targetCutoff = apvts.getRawParameterValue("cutoff")->load();
    const float targetQ = apvts.getRawParameterValue("q")->load();
    const float targetGain = apvts.getRawParameterValue("gain")->load();
    const float targetWetDry = apvts.getRawParameterValue("wetdry")->load();

    // Update smoothed values
    cutoffSmoothed.setTargetValue(targetCutoff);
    qSmoothed.setTargetValue(targetQ);
    gainSmoothed.setTargetValue(targetGain);
    wetDrySmoothed.setTargetValue(targetWetDry);

    // Process block
    dsp::AudioBlock<float> block(buffer);
    dryWetMixer.pushDrySamples(block); // Store dry signal

    // Update filter coefficients
    *filter.state = *dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, cutoffSmoothed.getNextValue(), qSmoothed.getNextValue());
    filter.process(dsp::ProcessContextReplacing<float>(block));

    // Apply gain (converted from dB)
    const float gainLinear = juce::Decibels::decibelsToGain(gainSmoothed.getNextValue());
    block.multiplyBy(gainLinear);

    // Apply Airwindows-style dithering
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        uint32_t* fpd = (channel == 0) ? &fpdL : &fpdR;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Generate dither (inspired by Airwindows)
            *fpd ^= *fpd << 13;
            *fpd ^= *fpd >> 17;
            *fpd ^= *fpd << 5;
            float dither = (float)(*fpd - uint32_t(0x7fffffff)) * 5.5e-36f * pow(2, 23);
            data[sample] += dither;
        }
    }

    // Apply dry/wet mix
    dryWetMixer.setWetMixProportion(wetDrySmoothed.getCurrentValue());
    dryWetMixer.mixWetSamples(block);
}

//==============================================================================
bool EnhancedLowpassAudioProcessor::hasEditor() const { return true; }
AudioProcessorEditor* EnhancedLowpassAudioProcessor::createEditor() { return new EnhancedLowpassAudioProcessorEditor(*this); }

//==============================================================================
void EnhancedLowpassAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.state.createXml())
        copyXmlToBinary(*xml, destData);
}

void EnhancedLowpassAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.state = juce::ValueTree::fromXml(*xml);
}

// Factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnhancedLowpassAudioProcessor();
}
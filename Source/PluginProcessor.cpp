/*
  ==============================================================================

    PluginProcessor.cpp
    -------------------
    This file implements the logic declared in PluginProcessor.h.
    It contains the actual audio processing code.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Constructor
// We initialize the APVTS here with our parameter layout.
Vst_saturatorAudioProcessor::Vst_saturatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       // Initialize APVTS with specific parameters
       apvts(*this, nullptr, "Parameters", createParameterLayout()),
       // Initialize Oversampling with 2 channels, 4x factor, and high-quality filter
       oversampling(2, 2, juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR)
#endif
{
}

Vst_saturatorAudioProcessor::~Vst_saturatorAudioProcessor()
{
}

//==============================================================================
// Parameter Layout
// Defines what parameters exist in the plugin.
juce::AudioProcessorValueTreeState::ParameterLayout Vst_saturatorAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // A. Saturation Globale
    // Note: ID "drive" is kept for preset compatibility, name is changed to "Saturation"
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "drive",       // Parameter ID
        "Saturation",  // Parameter Name
        0.0f,          // Min Value
        24.0f,         // Max Value
        0.0f           // Default Value
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "shape", "Shape", 0.0f, 1.0f, 0.0f));

    // Waveshape selection
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "waveshape",   // Parameter ID
        "Waveshape",   // Parameter Name
        juce::StringArray {
            "Tube", "SoftClip", "HardClip", "Diode 1", "Diode 2",
            "Linear Fold", "Sin Fold", "Zero-Square", "Downsample",
            "Asym", "Rectify", "X-Shaper", "X-Shaper (Asym)",
            "Sine Shaper", "Stomp Box", "Tape Sat.", "Overdrive", "Soft Sat."
        },
        0              // Default: Tube
    ));

    // B. Bande LOW (graves)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "lowEnable", "Low Enable", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lowFreq", "Low Freq", juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.3f), 200.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lowWarmth", "Low Warmth", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lowLevel", "Low Level", -24.0f, 24.0f, 0.0f));

    // C. Bande HIGH (aigus)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "highEnable", "High Enable", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "highFreq", "High Freq", juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.3f), 5000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "highSoftness", "High Softness", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "highLevel", "High Level", -24.0f, 24.0f, 0.0f));

    // D. Gain & routing
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "inputGain", "Input Gain", -24.0f, 24.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix", 0.0f, 100.0f, 100.0f));
    // Note: ID "output" is kept for preset compatibility, name is changed to "Output Gain"
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "output",      // Parameter ID
        "Output Gain", // Parameter Name
        -24.0f,        // Min Value
        24.0f,         // Max Value
        0.0f           // Default Value
    ));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "prePost", "Pre/Post", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "limiter", "Limiter", true));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "bypass", "Bypass", false));

    return layout;
}

//==============================================================================
// 1. Preparation
const juce::String Vst_saturatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Vst_saturatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Vst_saturatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Vst_saturatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Vst_saturatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Vst_saturatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Vst_saturatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Vst_saturatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Vst_saturatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void Vst_saturatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// Initialize DSP here
void Vst_saturatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // 1. Prepare DSP Spec
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumOutputChannels() };
    lastSampleRate = sampleRate;

    // 2. Initialize and Reset Crossover Filters
    lp1.prepare(spec); hp1.prepare(spec);
    lp2.prepare(spec); hp2.prepare(spec);
    lp1.reset(); hp1.reset();
    lp2.reset(); hp2.reset();

    // 3. Initialize and Reset Limiter
    limiter.prepare(spec);
    limiter.reset();

    // 4. Resize internal buffers
    lowBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
    midBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
    highBuffer.setSize(spec.numChannels, spec.maximumBlockSize);

    // 6. Prepare Oversampling
    oversampling.initProcessing(spec.maximumBlockSize);

    // 7. Force filter coefficient update
    lastLowFreq = 0.0f;
    lastHighFreq = 0.0f;
}

void Vst_saturatorAudioProcessor::releaseResources()
{
    // When playback stops, you can free memory here.
}

bool Vst_saturatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This checks if the DAW is trying to load the plugin in Mono, Stereo, etc.
    // We only support Stereo-to-Stereo or Mono-to-Mono usually.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input must match output channel count
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

//==============================================================================
void Vst_saturatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 1. Get Parameter Values
    // Global
    bool bypass = *apvts.getRawParameterValue("bypass");
    if (bypass) return;

    float saturation = *apvts.getRawParameterValue("drive");
    float shape = *apvts.getRawParameterValue("shape");

    // Low Band
    bool lowEnable = *apvts.getRawParameterValue("lowEnable");
    float lowFreq = *apvts.getRawParameterValue("lowFreq");
    float lowWarmth = *apvts.getRawParameterValue("lowWarmth");
    float lowLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("lowLevel")->load());

    // High Band
    bool highEnable = *apvts.getRawParameterValue("highEnable");
    float highFreq = *apvts.getRawParameterValue("highFreq");
    float highSoftness = *apvts.getRawParameterValue("highSoftness");
    float highLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("highLevel")->load());

    // Gain & Routing
    float inputGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGain")->load());
    float mix = *apvts.getRawParameterValue("mix") / 100.0f;
    float outputGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("output")->load());
    bool prePost = *apvts.getRawParameterValue("prePost");
    bool limiterEnable = *apvts.getRawParameterValue("limiter");


    // 2. Gain Staging
    // Store a clean copy of the input signal for the Dry/Wet mix.
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Apply Input Gain
    buffer.applyGain(inputGain);


    // 3. Update Filter Coefficients (if needed)
    if (lowFreq != lastLowFreq || getSampleRate() != lastSampleRate)
    {
        lp1.setCutoffFrequency(lowFreq);
        hp1.setCutoffFrequency(lowFreq);
        lastLowFreq = lowFreq;
    }
    if (highFreq != lastHighFreq || getSampleRate() != lastSampleRate)
    {
        lp2.setCutoffFrequency(highFreq);
        hp2.setCutoffFrequency(highFreq);
        lastHighFreq = highFreq;
    }

    // Get waveshape selection
    int waveshapeIndex = static_cast<int>(*apvts.getRawParameterValue("waveshape"));

    // 4. Pre/Post Processing Logic
    auto processSaturation = [&](juce::AudioBuffer<float>& audio)
    {
        // This lambda encapsulates the saturation logic
        float drive = juce::Decibels::decibelsToGain(saturation);

        for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        {
            auto* channelData = audio.getWritePointer(channel);
            for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            {
                float x = channelData[sample] * drive;
                float output = x;

                // Apply selected waveshape
                switch (waveshapeIndex)
                {
                    case 0: // Tube
                    {
                        float soft = std::tanh(x * (1.0f - shape * 0.5f));
                        float hard = (x - x*x*x/3.0f);
                        output = soft * (1.0f - shape) + hard * shape;
                        break;
                    }
                    case 1: // SoftClip
                        output = std::tanh(x * (1.0f + shape * 2.0f));
                        break;
                    case 2: // HardClip
                        output = juce::jlimit(-1.0f, 1.0f, x * (1.0f + shape * 3.0f));
                        break;
                    case 3: // Diode 1
                        output = x > 0.0f ? std::tanh(x * (1.0f + shape)) : x * 0.5f;
                        break;
                    case 4: // Diode 2
                        output = x > 0.0f ? x * 0.7f : std::tanh(x * (1.0f + shape * 2.0f));
                        break;
                    case 5: // Linear Fold
                    {
                        float threshold = 1.0f - shape * 0.5f;
                        if (std::abs(x) > threshold)
                            output = threshold - (std::abs(x) - threshold);
                        else
                            output = x;
                        output = juce::jlimit(-1.0f, 1.0f, output);
                        break;
                    }
                    case 6: // Sin Fold
                        output = std::sin(x * juce::MathConstants<float>::pi * (1.0f + shape * 2.0f));
                        break;
                    case 7: // Zero-Square
                        output = x * x * (x > 0.0f ? 1.0f : -1.0f) * (1.0f + shape);
                        break;
                    case 8: // Downsample
                    {
                        int factor = static_cast<int>(1.0f + shape * 8.0f);
                        if (sample % factor == 0)
                            output = std::tanh(x);
                        else
                            output = channelData[sample - (sample % factor)];
                        break;
                    }
                    case 9: // Asym
                        output = x > 0.0f ? std::tanh(x * (1.0f + shape * 2.0f)) : x * 0.3f;
                        break;
                    case 10: // Rectify
                        output = std::abs(x) * (1.0f - shape * 0.5f);
                        break;
                    case 11: // X-Shaper
                        output = x * (1.0f + shape) / (1.0f + shape * std::abs(x));
                        break;
                    case 12: // X-Shaper (Asym)
                        output = x > 0.0f ? x * (1.0f + shape * 2.0f) / (1.0f + shape * std::abs(x)) : x * 0.5f;
                        break;
                    case 13: // Sine Shaper
                        output = std::sin(std::tanh(x) * juce::MathConstants<float>::pi * 0.5f * (1.0f + shape));
                        break;
                    case 14: // Stomp Box
                        output = std::atan(x * (1.0f + shape * 5.0f)) / juce::MathConstants<float>::pi;
                        break;
                    case 15: // Tape Sat.
                    {
                        float wet = std::tanh(x * 1.5f);
                        output = x * (1.0f - shape) + wet * shape;
                        break;
                    }
                    case 16: // Overdrive
                        output = (2.0f / juce::MathConstants<float>::pi) * std::atan(x * (1.0f + shape * 10.0f));
                        break;
                    case 17: // Soft Sat.
                        output = x / (1.0f + std::abs(x) * shape);
                        break;
                    default:
                        output = std::tanh(x);
                        break;
                }

                channelData[sample] = output;
            }
        }
    };

    auto processBands = [&](juce::AudioBuffer<float>& audio)
    {
        // --- Standard 3-Band Linkwitz-Riley Crossover ---

        // 1. Create clean copies of the input signal for each filter chain.
        lowBuffer.makeCopyOf(audio);
        midBuffer.makeCopyOf(audio);
        highBuffer.makeCopyOf(audio);

        // 2. Create the LOW band signal.
        juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
        lp1.process(juce::dsp::ProcessContextReplacing<float>(lowBlock)); // Low-pass at lowFreq

        // 3. Create the HIGH band signal.
        juce::dsp::AudioBlock<float> highBlock(highBuffer);
        hp2.process(juce::dsp::ProcessContextReplacing<float>(highBlock)); // High-pass at highFreq

        // 4. Create the MID band signal.
        juce::dsp::AudioBlock<float> midBlock(midBuffer);
        hp1.process(juce::dsp::ProcessContextReplacing<float>(midBlock)); // High-pass at lowFreq
        lp2.process(juce::dsp::ProcessContextReplacing<float>(midBlock)); // Low-pass at highFreq

        // --- Per-Band Processing (In-Place) ---
        // If a band is enabled, its corresponding buffer is processed directly.
        // If not, the buffer contains the original, unprocessed audio for that band.
        if (lowEnable)
        {
            for (int channel = 0; channel < lowBuffer.getNumChannels(); ++channel) {
                auto* data = lowBuffer.getWritePointer(channel);
                for (int i = 0; i < lowBuffer.getNumSamples(); ++i) {
                    float x = data[i];
                    data[i] = x + (x * std::abs(x)) * lowWarmth;
                }
            }
            lowBuffer.applyGain(lowLevel);
        }

        if (highEnable)
        {
            for (int channel = 0; channel < highBuffer.getNumChannels(); ++channel) {
                auto* data = highBuffer.getWritePointer(channel);
                for (int i = 0; i < highBuffer.getNumSamples(); ++i) {
                    float x = data[i];
                    data[i] = x - std::tanh(x * highSoftness);
                }
            }
            highBuffer.applyGain(highLevel);
        }

        // --- Recombine Bands (No Copies Needed) ---
        // All member buffers are now in their final state (either processed or unprocessed).
        // We can now safely sum them into the main output buffer.
        audio.clear();
        for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        {
            audio.addFrom(channel, 0, lowBuffer, channel, 0, audio.getNumSamples());
            audio.addFrom(channel, 0, midBuffer, channel, 0, audio.getNumSamples());
            audio.addFrom(channel, 0, highBuffer, channel, 0, audio.getNumSamples());
        }
    };

    if (prePost) // Post: EQ -> Saturation
    {
        // 1. Process bands first
        processBands(buffer);

        // 2. Then apply oversampled saturation
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::AudioBlock<float> oversampledBlock = oversampling.processSamplesUp(block);
        // Apply saturation directly to the oversampled block
        for (int channel = 0; channel < (int)oversampledBlock.getNumChannels(); ++channel)
        {
            auto* channelData = oversampledBlock.getChannelPointer(channel);
            for (size_t sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
            {
                float x = channelData[sample] * juce::Decibels::decibelsToGain(saturation);
                float soft = std::tanh(x * (1.0f - shape * 0.5f));
                float hard = (x - x*x*x/3.0f);
                channelData[sample] = soft * (1.0f - shape) + hard * shape;
            }
        }
        oversampling.processSamplesDown(block);
    }
    else // Pre: Saturation -> EQ
    {
        // 1. Apply oversampled saturation first
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::AudioBlock<float> oversampledBlock = oversampling.processSamplesUp(block);
        // Apply saturation directly to the oversampled block
        for (int channel = 0; channel < (int)oversampledBlock.getNumChannels(); ++channel)
        {
            auto* channelData = oversampledBlock.getChannelPointer(channel);
            for (size_t sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
            {
                float x = channelData[sample] * juce::Decibels::decibelsToGain(saturation);
                float soft = std::tanh(x * (1.0f - shape * 0.5f));
                float hard = (x - x*x*x/3.0f);
                channelData[sample] = soft * (1.0f - shape) + hard * shape;
            }
        }
        oversampling.processSamplesDown(block);

        // 2. Then process bands
        processBands(buffer);
    }

    // 5. Final Stage: Mix, Output Gain, Limiter
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* dryData = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Dry/Wet Mix
            float wetSignal = channelData[sample];
            float drySignal = dryData[sample];
            channelData[sample] = drySignal * (1.0f - mix) + wetSignal * mix;
        }
    }

    // Apply Output Gain before Limiter
    buffer.applyGain(outputGain);

    if (limiterEnable)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        limiter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }
}

//==============================================================================
// 3. Editor Creation
bool Vst_saturatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Vst_saturatorAudioProcessor::createEditor()
{
    return new Vst_saturatorAudioProcessorEditor (*this);
}

//==============================================================================
// 4. Persistence
void Vst_saturatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the APVTS state to XML, then to binary
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void Vst_saturatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Load the APVTS state from binary
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// Factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Vst_saturatorAudioProcessor();
}

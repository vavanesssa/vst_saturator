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
       apvts(*this, nullptr, "Parameters", createParameterLayout())
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

    // Phase 2: Adding Parameters
    // -------------------------------------------------------------------------
    // "Drive" Parameter
    // - ID: "drive" (used in code)
    // - Name: "Drive" (visible in DAW)
    // - Range: 0.0 to 24.0 dB
    // - Default: 0.0 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "drive",       // Parameter ID
        "Drive",       // Parameter Name
        0.0f,          // Min Value
        24.0f,         // Max Value
        0.0f           // Default Value
    ));

    // "Output" Parameter
    // - ID: "output"
    // - Name: "Output"
    // - Range: -24.0 to 24.0 dB
    // - Default: 0.0 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "output",      // Parameter ID
        "Output",      // Parameter Name
        -24.0f,        // Min Value
        24.0f,         // Max Value
        0.0f           // Default Value
    ));

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
    // This method is called before audio starts flowing.
    // It's the place to set up DSP objects (like filters, delay lines) that need
    // to know the Sample Rate or Block Size.

    // For simple gain/tanh, we don't strictly need initialization,
    // but it's good practice to reset state here.
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
// 2. Audio Processing
// This is the REAL-TIME audio thread.
// CRITICAL:
// - NO memory allocation (no `new`, `malloc`, `std::vector` resizing).
// - NO blocking operations (no `std::mutex`, file I/O, `printf`).
void Vst_saturatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data.
    // (e.g., if we have 2 inputs but 4 outputs, silence the last 2).
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Phase 2: Retrieve Parameter Values
    // We get the atomic float value from APVTS. This is thread-safe.
    // We convert dB to linear gain: 10^(dB/20)
    float drivedB = *apvts.getRawParameterValue("drive");
    float outputdB = *apvts.getRawParameterValue("output");

    float driveGain = juce::Decibels::decibelsToGain(drivedB);
    float outputGain = juce::Decibels::decibelsToGain(outputdB);

    // Loop through each channel (Left, Right)
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // Get a pointer to the start of the array of samples for this channel
        auto* channelData = buffer.getWritePointer (channel);
        auto numSamples = buffer.getNumSamples();

        // Loop through every sample in the buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float inputSignal = channelData[sample];

            // -----------------------------------------------------------------
            // Phase 3: Saturation Logic
            // -----------------------------------------------------------------

            // 1. Apply Input Drive
            float drivenSignal = inputSignal * driveGain;

            // 2. Apply Saturation (Phase 3)
            // tanh() creates a soft-clipping effect.
            // As drivenSignal gets larger, tanh approaches +/- 1.0.
            float processedSignal = std::tanh(drivenSignal);

            // 3. Apply Output Gain
            float outputSignal = processedSignal * outputGain;

            // 4. Write back to buffer
            channelData[sample] = outputSignal;
        }
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

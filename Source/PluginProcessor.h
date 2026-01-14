/*
  ==============================================================================

    PluginProcessor.h
    -----------------
    This file declares the "Audio Processor".

    Role:
    The Audio Processor is the "brain" of the plugin. It handles:
    1.  Audio Processing (DSP): Receiving audio buffers and modifying them.
    2.  State Management: Handling parameters (Drive, Output).
    3.  Persistence: Saving and loading presets.

    It DOES NOT handle the UI (that's the Editor's job).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class Vst_saturatorAudioProcessor : public juce::AudioProcessor {
public:
  //==============================================================================
  // Constructor & Destructor
  Vst_saturatorAudioProcessor();
  ~Vst_saturatorAudioProcessor() override;

  //==============================================================================
  // 1. Preparation
  // Called before playback starts. Use this to initialize DSP with the sample
  // rate.
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;

  // Called when playback stops. Clean up if necessary.
  void releaseResources() override;

  // Helper to check which buses (Input/Output) are active.
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  //==============================================================================
  // 2. Processing (The most important loop!)
  // This is called thousands of times per second with a buffer of audio.
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  // 3. Editor Creation
  // Creates the GUI window for this plugin.
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  // 4. Persistence (Saving/Loading)
  // Used to save the state (parameters) when the user saves a project in the
  // DAW.
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  //==============================================================================
  // 5. Plugin Metadata
  // Information displayed in the DAW.
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  // 6. Parameter Management (APVTS)
  // This object manages our parameters (Drive, Output) and handles
  // communication with the DAW (Automation) and the UI (Undo/Redo).
  juce::AudioProcessorValueTreeState apvts;

private:
  // Helper function to define the parameters layout
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  // --- DSP Member Variables ---

  // 3-Band Crossover using Linkwitz-Riley filters
  using Filter = juce::dsp::LinkwitzRileyFilter<float>;
  Filter lp1, hp1, lp2, hp2; // Four filters for a 3-band split

  // Audio buffers for split-band processing
  juce::AudioBuffer<float> lowBuffer, midBuffer, highBuffer;

  // Previous parameter values to avoid unnecessary updates in the audio thread
  float lastLowFreq = 0.0f;
  float lastHighFreq = 0.0f;
  double lastSampleRate = 0.0;

  // Soft Limiter
  juce::dsp::Limiter<float> limiter;

  // Oversampling for non-aliased saturation
  juce::dsp::Oversampling<float> oversampling;

  // Delta monitoring crossfade state (for anti-click transitions)
  float deltaSmoothed =
      0.0f; // Current smoothed delta state (0.0 = normal, 1.0 = delta mode)
  bool lastDeltaState = false; // Previous delta button state

  // Delta crossfade parameters (calculated in prepareToPlay)
  float deltaCrossfadeStep =
      0.0f; // Amount to change per sample (for ~10ms fade)

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Vst_saturatorAudioProcessor)
};

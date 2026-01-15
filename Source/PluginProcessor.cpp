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
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      // Initialize APVTS with specific parameters
      apvts(*this, nullptr, "Parameters", createParameterLayout()),
      // Initialize Oversampling with 2 channels, 4x factor, and high-quality
      // filter
      oversampling(2, 2,
                   juce::dsp::Oversampling<
                       float>::FilterType::filterHalfBandPolyphaseIIR)
#endif
{
}

Vst_saturatorAudioProcessor::~Vst_saturatorAudioProcessor() {}

// Returns a deterministic noise sample in [-1, 1].
// This is RT-safe and avoids using rand() in the audio thread.
float Vst_saturatorAudioProcessor::nextNoiseSample() {
  noiseSeed = 1664525u * noiseSeed + 1013904223u;
  const float normalized =
      static_cast<float>(noiseSeed & 0x00FFFFFFu) /
      static_cast<float>(0x00FFFFFFu);
  return normalized * 2.0f - 1.0f;
}

//==============================================================================
// Parameter Layout
// Defines what parameters exist in the plugin.
juce::AudioProcessorValueTreeState::ParameterLayout
Vst_saturatorAudioProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  // A. Saturation Globale
  // Note: ID "drive" is kept for preset compatibility, name is changed to
  // "Saturation"
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "drive",      // Parameter ID
      "Saturation", // Parameter Name
      0.0f,         // Min Value
      24.0f,        // Max Value
      0.0f          // Default Value
      ));
  layout.add(std::make_unique<juce::AudioParameterFloat>("shape", "Shape", 0.0f,
                                                         1.0f, 0.0f));

  // Waveshape selection - 58 algorithms total
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "waveshape", // Parameter ID
      "Waveshape", // Parameter Name
      juce::StringArray{
          // === CLASSIC (0-9) ===
          "Tube",        // 0
          "SoftClip",    // 1
          "HardClip",    // 2
          "Diode 1",     // 3
          "Diode 2",     // 4
          "Linear Fold", // 5
          "Sin Fold",    // 6
          "Zero-Square", // 7
          "Downsample",  // 8
          "Asym",        // 9
          // === SHAPERS (10-19) ===
          "Rectify",         // 10
          "X-Shaper",        // 11
          "X-Shaper (Asym)", // 12
          "Sine Shaper",     // 13
          "Stomp Box",       // 14
          "Tape Sat.",       // 15
          "Overdrive",       // 16
          "Soft Sat.",       // 17
          "Bit-Crush",       // 18
          "Glitch Fold",     // 19
          // === ANALOG (20-27) ===
          "Valve",       // 20
          "Fuzz Fac",    // 21
          "Cheby 3",     // 22
          "Cheby 5",     // 23
          "Log Sat",     // 24
          "Half Wave",   // 25
          "Cubic",       // 26
          "Octaver Sat", // 27
          // === NEW: TUBE TYPES (28-33) - Inspired by Decapitator ===
          "Triode",    // 28 - Classic 12AX7 warmth
          "Pentode",   // 29 - EL34 power tube push
          "Class A",   // 30 - Single-ended warmth
          "Class AB",  // 31 - Push-pull punch
          "Class B",   // 32 - Crossover distortion
          "Germanium", // 33 - Vintage transistor fuzz
          // === NEW: TAPE MODES (34-38) - Inspired by Saturn ===
          "Tape 15ips",    // 34 - Fast tape, bright
          "Tape 7.5ips",   // 35 - Slow tape, warm
          "Tape Cassette", // 36 - Lo-fi cassette
          "Tape 456",      // 37 - Ampex 456 style
          "Tape SM900",    // 38 - Modern tape emulation
          // === NEW: TRANSFORMER (39-42) ===
          "Transformer", // 39 - Iron saturation
          "Console",     // 40 - Neve-style console
          "API Style",   // 41 - API 2500 character
          "SSL Style",   // 42 - SSL G-Series
          // === NEW: TRANSISTOR (43-47) ===
          "Silicon",   // 43 - Modern transistor
          "FET Clean", // 44 - FET limiter style
          "FET Dirty", // 45 - FET pushed hard
          "OpAmp",     // 46 - IC distortion
          "CMOS",      // 47 - Digital/analog hybrid
          // === NEW: CREATIVE (48-52) - Inspired by Trash 2 ===
          "Scream",  // 48 - Aggressive screamer
          "Buzz",    // 49 - Buzzy distortion
          "Crackle", // 50 - Random crackle
          "Wrap",    // 51 - Wrap-around distortion
          "Density", // 52 - Thick density
          // === NEW: MATH/EXOTIC (53-57) ===
          "Cheby 7",     // 53 - 7th order Chebyshev
          "Hyperbolic",  // 54 - sinh based
          "Exponential", // 55 - exp based limiting
          "Parabolic",   // 56 - Parabolic curve
          "Wavelet"      // 57 - Wavelet-inspired
      },
      0 // Default: Tube
      ));

  // B. Bande LOW (graves)
  layout.add(std::make_unique<juce::AudioParameterBool>("lowEnable",
                                                        "Low Enable", false));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lowFreq", "Low Freq",
      juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.3f), 200.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lowWarmth", "Low Warmth", 0.0f, 1.0f, 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lowLevel", "Low Level", -24.0f, 24.0f, 0.0f));

  // C. Bande HIGH (aigus)
  layout.add(std::make_unique<juce::AudioParameterBool>("highEnable",
                                                        "High Enable", false));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "highFreq", "High Freq",
      juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.3f), 5000.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "highSoftness", "High Softness", 0.0f, 1.0f, 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "highLevel", "High Level", -24.0f, 24.0f, 0.0f));

  // D. Gain & routing
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "inputGain", "Input Gain", -24.0f, 24.0f, 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f,
                                                         100.0f, 100.0f));
  // Note: ID "output" is kept for preset compatibility, name is changed to
  // "Output Gain"
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "output",      // Parameter ID
      "Output Gain", // Parameter Name
      -24.0f,        // Min Value
      24.0f,         // Max Value
      0.0f           // Default Value
      ));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("prePost", "Pre/Post", false));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("limiter", "Limiter", true));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));

  // E. Delta Monitoring (listen to harmonics only)
  layout.add(std::make_unique<juce::AudioParameterBool>(
      "delta", "Delta", false)); // Delta mode toggle
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "deltaGain", "Delta Gain", -24.0f, 0.0f,
      -12.0f)); // Delta gain in dB (default: -12 dB for safety)

  return layout;
}

//==============================================================================
// 1. Preparation
const juce::String Vst_saturatorAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool Vst_saturatorAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool Vst_saturatorAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool Vst_saturatorAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double Vst_saturatorAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int Vst_saturatorAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int Vst_saturatorAudioProcessor::getCurrentProgram() { return 0; }

void Vst_saturatorAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String Vst_saturatorAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void Vst_saturatorAudioProcessor::changeProgramName(
    int index, const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
// Initialize DSP here
void Vst_saturatorAudioProcessor::prepareToPlay(double sampleRate,
                                                int samplesPerBlock) {
  // 1. Prepare DSP Spec
  juce::dsp::ProcessSpec spec{sampleRate, (juce::uint32)samplesPerBlock,
                              (juce::uint32)getTotalNumOutputChannels()};
  lastSampleRate = sampleRate;

  // 2. Initialize and Reset Crossover Filters
  lp1.prepare(spec);
  hp1.prepare(spec);
  lp2.prepare(spec);
  hp2.prepare(spec);
  lp1.reset();
  hp1.reset();
  lp2.reset();
  hp2.reset();

  // 3. Initialize and Reset Limiter
  limiter.prepare(spec);
  limiter.reset();

  // 4. Resize internal buffers
  lowBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
  midBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
  highBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
  dryBuffer.setSize(spec.numChannels, spec.maximumBlockSize);

  // 6. Prepare Oversampling
  oversampling.initProcessing(spec.maximumBlockSize);

  // 7. Force filter coefficient update
  lastLowFreq = 0.0f;
  lastHighFreq = 0.0f;

  // 8. Delta monitoring crossfade: ~10ms fade time for anti-click
  // Calculate step per sample: 1.0 / (fadeTimeSeconds * sampleRate)
  const float fadeTimeMs = 10.0f;
  if (sampleRate > 0.0) {
    deltaCrossfadeStep =
        1.0f / (fadeTimeMs * 0.001f * static_cast<float>(sampleRate));
  } else {
    deltaCrossfadeStep = 0.0f;
  }
}

void Vst_saturatorAudioProcessor::releaseResources() {
  // When playback stops, you can free memory here.
}

bool Vst_saturatorAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  // This checks if the DAW is trying to load the plugin in Mono, Stereo, etc.
  // We only support Stereo-to-Stereo or Mono-to-Mono usually.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // Input must match output channel count
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;

  return true;
}

//==============================================================================
void Vst_saturatorAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                               juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // 1. Get Parameter Values
  // Global
  bool bypass = *apvts.getRawParameterValue("bypass");
  if (bypass)
    return;

  float saturation = *apvts.getRawParameterValue("drive");
  float shape = *apvts.getRawParameterValue("shape");

  // Low Band
  bool lowEnable = *apvts.getRawParameterValue("lowEnable");
  float lowFreq = *apvts.getRawParameterValue("lowFreq");
  float lowWarmth = *apvts.getRawParameterValue("lowWarmth");
  float lowLevel = juce::Decibels::decibelsToGain(
      apvts.getRawParameterValue("lowLevel")->load());

  // High Band
  bool highEnable = *apvts.getRawParameterValue("highEnable");
  float highFreq = *apvts.getRawParameterValue("highFreq");
  float highSoftness = *apvts.getRawParameterValue("highSoftness");
  float highLevel = juce::Decibels::decibelsToGain(
      apvts.getRawParameterValue("highLevel")->load());

  // Gain & Routing
  float inputGain = juce::Decibels::decibelsToGain(
      apvts.getRawParameterValue("inputGain")->load());
  float mix = *apvts.getRawParameterValue("mix") / 100.0f;
  float outputGain = juce::Decibels::decibelsToGain(
      apvts.getRawParameterValue("output")->load());
  bool prePost = *apvts.getRawParameterValue("prePost");
  bool limiterEnable = *apvts.getRawParameterValue("limiter");

  // Delta Monitoring
  bool deltaEnabled = *apvts.getRawParameterValue("delta");
  float deltaGainDb = *apvts.getRawParameterValue("deltaGain");
  float deltaGain = juce::Decibels::decibelsToGain(deltaGainDb);

  // Update delta crossfade state (smooth transitions to avoid clicks)
  float targetDeltaSmoothed = deltaEnabled ? 1.0f : 0.0f;
  // We'll update deltaSmoothed per-sample in the final stage

  // 2. Gain Staging
  // Store a clean copy of the input signal for the Dry/Wet mix.
  jassert(dryBuffer.getNumChannels() >= totalNumOutputChannels);
  jassert(dryBuffer.getNumSamples() >= buffer.getNumSamples());
  if (dryBuffer.getNumChannels() < totalNumOutputChannels ||
      dryBuffer.getNumSamples() < buffer.getNumSamples()) {
    // Defensive resize in case the host exceeds the expected block size.
    dryBuffer.setSize(totalNumOutputChannels, buffer.getNumSamples(), false,
                      false, true);
  }
  for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
    dryBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
  }

  // Apply Input Gain
  buffer.applyGain(inputGain);

  // 3. Update Filter Coefficients (if needed)
  const double currentSampleRate = getSampleRate();
  if (lowFreq != lastLowFreq || currentSampleRate != lastSampleRate) {
    lp1.setCutoffFrequency(lowFreq);
    hp1.setCutoffFrequency(lowFreq);
    lastLowFreq = lowFreq;
  }
  if (highFreq != lastHighFreq || currentSampleRate != lastSampleRate) {
    lp2.setCutoffFrequency(highFreq);
    hp2.setCutoffFrequency(highFreq);
    lastHighFreq = highFreq;
  }
  lastSampleRate = currentSampleRate;

  // Get waveshape selection
  int waveshapeIndex =
      static_cast<int>(*apvts.getRawParameterValue("waveshape"));

  // 4. Pre/Post Processing Logic

  auto processBands = [&](juce::AudioBuffer<float> &audio) {
    // --- Standard 3-Band Linkwitz-Riley Crossover ---

    // 1. Create clean copies of the input signal for each filter chain.
    lowBuffer.makeCopyOf(audio);
    midBuffer.makeCopyOf(audio);
    highBuffer.makeCopyOf(audio);

    // 2. Create the LOW band signal.
    juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
    lp1.process(juce::dsp::ProcessContextReplacing<float>(
        lowBlock)); // Low-pass at lowFreq

    // 3. Create the HIGH band signal.
    juce::dsp::AudioBlock<float> highBlock(highBuffer);
    hp2.process(juce::dsp::ProcessContextReplacing<float>(
        highBlock)); // High-pass at highFreq

    // 4. Create the MID band signal.
    juce::dsp::AudioBlock<float> midBlock(midBuffer);
    hp1.process(juce::dsp::ProcessContextReplacing<float>(
        midBlock)); // High-pass at lowFreq
    lp2.process(juce::dsp::ProcessContextReplacing<float>(
        midBlock)); // Low-pass at highFreq

    // --- Per-Band Processing (In-Place) ---
    // If a band is enabled, its corresponding buffer is processed directly.
    // If not, the buffer contains the original, unprocessed audio for that
    // band.
    if (lowEnable) {
      for (int channel = 0; channel < lowBuffer.getNumChannels(); ++channel) {
        auto *data = lowBuffer.getWritePointer(channel);
        for (int i = 0; i < lowBuffer.getNumSamples(); ++i) {
          float x = data[i];
          data[i] = x + (x * std::abs(x)) * lowWarmth;
        }
      }
      lowBuffer.applyGain(lowLevel);
    }

    if (highEnable) {
      for (int channel = 0; channel < highBuffer.getNumChannels(); ++channel) {
        auto *data = highBuffer.getWritePointer(channel);
        for (int i = 0; i < highBuffer.getNumSamples(); ++i) {
          float x = data[i];
          data[i] = x - std::tanh(x * highSoftness);
        }
      }
      highBuffer.applyGain(highLevel);
    }

    // --- Recombine Bands (No Copies Needed) ---
    // All member buffers are now in their final state (either processed or
    // unprocessed). We can now safely sum them into the main output buffer.
    audio.clear();
    for (int channel = 0; channel < audio.getNumChannels(); ++channel) {
      audio.addFrom(channel, 0, lowBuffer, channel, 0, audio.getNumSamples());
      audio.addFrom(channel, 0, midBuffer, channel, 0, audio.getNumSamples());
      audio.addFrom(channel, 0, highBuffer, channel, 0, audio.getNumSamples());
    }
  };

  // Lambda to apply selected waveshape to a single sample
  auto applyWaveshape = [&](float x, int waveshape, float shapeParam) -> float {
    float output = x;
    switch (waveshape) {
    case 0: // Tube
    {
      float soft = std::tanh(x * (1.0f - shapeParam * 0.5f));
      float hard = (x - x * x * x / 3.0f);
      output = soft * (1.0f - shapeParam) + hard * shapeParam;
      break;
    }
    case 1: // SoftClip
      output = std::tanh(x * (1.0f + shapeParam * 2.0f));
      break;
    case 2: // HardClip
      output = juce::jlimit(-1.0f, 1.0f, x * (1.0f + shapeParam * 3.0f));
      break;
    case 3: // Diode 1
      output = x > 0.0f ? std::tanh(x * (1.0f + shapeParam)) : x * 0.5f;
      break;
    case 4: // Diode 2
      output = x > 0.0f ? x * 0.7f : std::tanh(x * (1.0f + shapeParam * 2.0f));
      break;
    case 5: // Linear Fold
    {
      float threshold = 1.0f - shapeParam * 0.5f;
      if (std::abs(x) > threshold)
        output = threshold - (std::abs(x) - threshold);
      else
        output = x;
      output = juce::jlimit(-1.0f, 1.0f, output);
      break;
    }
    case 6: // Sin Fold
      output = std::sin(x * juce::MathConstants<float>::pi *
                        (1.0f + shapeParam * 2.0f));
      break;
    case 7: // Zero-Square
      output = x * x * (x > 0.0f ? 1.0f : -1.0f) * (1.0f + shapeParam);
      break;
    case 8: // Downsample (simplified for oversampling context)
      output = std::tanh(x * (1.0f + shapeParam));
      break;
    case 9: // Asym
      output = x > 0.0f ? std::tanh(x * (1.0f + shapeParam * 2.0f)) : x * 0.3f;
      break;
    case 10: // Rectify
      output = std::abs(x) * (1.0f - shapeParam * 0.5f);
      break;
    case 11: // X-Shaper
      output = x * (1.0f + shapeParam) / (1.0f + shapeParam * std::abs(x));
      break;
    case 12: // X-Shaper (Asym)
      output = x > 0.0f ? x * (1.0f + shapeParam * 2.0f) /
                              (1.0f + shapeParam * std::abs(x))
                        : x * 0.5f;
      break;
    case 13: // Sine Shaper
      output = std::sin(std::tanh(x) * juce::MathConstants<float>::pi * 0.5f *
                        (1.0f + shapeParam));
      break;
    case 14: // Stomp Box
      output = std::atan(x * (1.0f + shapeParam * 5.0f)) /
               juce::MathConstants<float>::pi;
      break;
    case 15: // Tape Sat.
    {
      float wet = std::tanh(x * 1.5f);
      output = x * (1.0f - shapeParam) + wet * shapeParam;
      break;
    }
    case 16: // Overdrive
      output = (2.0f / juce::MathConstants<float>::pi) *
               std::atan(x * (1.0f + shapeParam * 10.0f));
      break;
    case 17: // Soft Sat.
      output = x / (1.0f + std::abs(x) * shapeParam);
      break;
    case 18: // Bit-Crush
    {
      float levels = 2.0f + (1.0f - shapeParam) * 30.0f;
      output = std::round(x * levels) / levels;
      break;
    }
    case 19: // Glitch Fold
      output = x * std::sin(x * shapeParam * juce::MathConstants<float>::pi);
      break;
    case 20: // Valve
    {
      float bias = 0.2f * shapeParam;
      float x_biased = x + bias;
      output = x_biased / (1.0f + std::abs(x_biased));
      break;
    }
    case 21: // Fuzz Fac
      output = (x > 0.0f ? 1.0f : -1.0f) *
               (1.0f - std::exp(-std::abs(x * (1.0f + shapeParam * 10.0f))));
      break;
    case 22: // Cheby 3
    {
      float x_limited = juce::jlimit(-1.0f, 1.0f, x);
      output = (4.0f * x_limited * x_limited * x_limited - 3.0f * x_limited) *
               (0.5f + shapeParam * 0.5f);
      break;
    }
    case 23: // Cheby 5
    {
      float x_limited = juce::jlimit(-1.0f, 1.0f, x);
      output = (16.0f * std::pow(x_limited, 5) -
                20.0f * std::pow(x_limited, 3) + 5.0f * x_limited) *
               (0.5f + shapeParam * 0.5f);
      break;
    }
    case 24: // Log Sat
      output = (x > 0.0f ? 1.0f : -1.0f) *
               std::log(1.0f + (10.0f + shapeParam * 50.0f) * std::abs(x)) /
               std::log(11.0f + shapeParam * 50.0f);
      break;
    case 25: // Half Wave
      output = x > 0.0f ? std::tanh(x * (1.0f + shapeParam)) : x;
      break;
    case 26: // Cubic
    {
      float x_scaled = x * (1.0f + shapeParam);
      output = x_scaled - (1.0f / 3.0f) * x_scaled * x_scaled * x_scaled;
      break;
    }
    case 27: // Octaver Sat
      output = (std::abs(x) * 2.0f - 1.0f) * (0.5f + shapeParam * 0.5f);
      break;

    // ========== NEW WAVESHAPES (28-57) ==========

    // === TUBE TYPES (28-33) - Inspired by Decapitator ===
    case 28: // Triode - Classic 12AX7 warmth
    {
      float mu = 100.0f; // Amplification factor
      float kp = 1.2f + shapeParam * 0.8f;
      float vg = x * (1.0f + shapeParam);
      output = (vg > 0.0f) ? std::tanh(vg / kp) * kp
                           : vg / (1.0f + std::abs(vg) * mu * 0.01f);
      break;
    }
    case 29: // Pentode - EL34 power tube push
    {
      float screen = 0.7f + shapeParam * 0.3f;
      float plate = x * (1.5f + shapeParam);
      output = std::tanh(plate * screen) +
               0.1f * std::sin(plate * 3.0f) * shapeParam;
      break;
    }
    case 30: // Class A - Single-ended warmth
    {
      float bias = 0.3f * shapeParam;
      float biased = x + bias;
      output = std::tanh(biased * (1.0f + shapeParam)) - bias * 0.5f;
      break;
    }
    case 31: // Class AB - Push-pull punch
    {
      float threshold = 0.3f - shapeParam * 0.2f;
      if (std::abs(x) < threshold) {
        output = x * (1.0f + shapeParam * 2.0f);
      } else {
        output = (x > 0.0f ? 1.0f : -1.0f) *
                 (threshold + std::tanh((std::abs(x) - threshold) *
                                        (2.0f + shapeParam * 3.0f)));
      }
      break;
    }
    case 32: // Class B - Crossover distortion
    {
      float deadzone = 0.05f + shapeParam * 0.1f;
      if (std::abs(x) < deadzone) {
        output = 0.0f;
      } else {
        output =
            (x > 0.0f ? 1.0f : -1.0f) *
            std::tanh((std::abs(x) - deadzone) * (1.0f + shapeParam * 3.0f));
      }
      break;
    }
    case 33: // Germanium - Vintage transistor fuzz
    {
      float temp = 0.8f + shapeParam * 0.4f; // Temperature coefficient
      float biased = x + 0.1f * shapeParam;
      output = (biased > 0.0f ? 1.0f : -1.0f) *
               (1.0f - std::exp(-std::abs(biased) * temp * 5.0f));
      output *= 0.9f + 0.1f * shapeParam;
      break;
    }

    // === TAPE MODES (34-38) - Inspired by Saturn ===
    case 34: // Tape 15ips - Fast tape, bright
    {
      float headroom = 1.2f - shapeParam * 0.3f;
      float compression = std::tanh(x / headroom) * headroom;
      output = compression + 0.05f * x * shapeParam; // Some HF retention
      break;
    }
    case 35: // Tape 7.5ips - Slow tape, warm
    {
      float satPoint = 0.6f + shapeParam * 0.3f;
      float warmth = x + 0.15f * x * x * (x > 0.0f ? 1.0f : -1.0f);
      output = std::tanh(warmth / satPoint) * satPoint;
      break;
    }
    case 36: // Tape Cassette - Lo-fi cassette
    {
      float hfLoss = 1.0f - shapeParam * 0.4f;
      float saturated = std::tanh(x * (1.0f + shapeParam * 2.0f));
      output = saturated * hfLoss + x * (1.0f - hfLoss) * 0.5f;
      break;
    }
    case 37: // Tape 456 - Ampex 456 style (famous for punchy low end)
    {
      float hysteresis = x + 0.2f * x * std::abs(x) * shapeParam;
      output = std::tanh(hysteresis * (1.0f + shapeParam * 0.5f));
      break;
    }
    case 38: // Tape SM900 - Modern tape emulation
    {
      float modern = std::tanh(x * 1.1f);
      float vintage = x / (1.0f + std::abs(x) * 0.5f);
      output = modern * (1.0f - shapeParam) + vintage * shapeParam;
      break;
    }

    // === TRANSFORMER (39-42) ===
    case 39: // Transformer - Iron saturation
    {
      float iron = x + 0.3f * std::sin(x * 2.0f) * shapeParam;
      output = std::tanh(iron * (1.0f + shapeParam));
      break;
    }
    case 40: // Console - Neve-style console
    {
      float harmonic2 = 0.1f * x * std::abs(x) * shapeParam;
      float harmonic3 = 0.05f * x * x * x * shapeParam;
      output = std::tanh(x + harmonic2 + harmonic3);
      break;
    }
    case 41: // API Style - API 2500 character (punchy)
    {
      float punch = x * (1.0f + shapeParam * 0.5f);
      float clipped = juce::jlimit(-1.0f, 1.0f, punch * 1.5f);
      output = punch * (1.0f - shapeParam * 0.5f) + clipped * shapeParam * 0.5f;
      output = std::tanh(output);
      break;
    }
    case 42: // SSL Style - SSL G-Series (clean but present)
    {
      float compressed = x / (1.0f + std::abs(x) * shapeParam * 0.5f);
      float harmonic = 0.05f * x * x * x * shapeParam;
      output = compressed + harmonic;
      break;
    }

    // === TRANSISTOR (43-47) ===
    case 43: // Silicon - Modern transistor
    {
      float gain = 1.0f + shapeParam * 3.0f;
      float clipPoint = 0.8f - shapeParam * 0.2f;
      output = juce::jlimit(-clipPoint, clipPoint, x * gain);
      output = std::tanh(output / clipPoint) * clipPoint;
      break;
    }
    case 44: // FET Clean - FET limiter style (1176 clean)
    {
      float ratio = 4.0f + shapeParam * 16.0f;
      float threshold = 0.5f;
      if (std::abs(x) > threshold) {
        float excess = std::abs(x) - threshold;
        output = (x > 0.0f ? 1.0f : -1.0f) * (threshold + excess / ratio);
      } else {
        output = x;
      }
      break;
    }
    case 45: // FET Dirty - FET pushed hard (1176 all-buttons)
    {
      float attack = x * (2.0f + shapeParam * 4.0f);
      output = std::tanh(attack) * (0.8f + shapeParam * 0.2f);
      output += 0.1f * std::sin(x * 5.0f) * shapeParam; // Harmonics
      break;
    }
    case 46: // OpAmp - IC distortion
    {
      float gain = 1.0f + shapeParam * 10.0f;
      float clipped = juce::jlimit(-1.0f, 1.0f, x * gain);
      output = clipped * (1.0f - shapeParam * 0.3f) +
               std::tanh(x * gain) * shapeParam * 0.3f;
      break;
    }
    case 47: // CMOS - Digital/analog hybrid
    {
      float digital = (x > 0.0f ? 1.0f : -1.0f) * std::pow(std::abs(x), 0.5f);
      float analog = std::tanh(x * (1.0f + shapeParam));
      output = digital * shapeParam + analog * (1.0f - shapeParam);
      break;
    }

    // === CREATIVE (48-52) - Inspired by Trash 2 ===
    case 48: // Scream - Aggressive screamer
    {
      float scream = x * (3.0f + shapeParam * 7.0f);
      output = std::tanh(scream) + 0.2f * std::sin(scream * 3.0f) * shapeParam;
      output = juce::jlimit(-1.0f, 1.0f, output);
      break;
    }
    case 49: // Buzz - Buzzy distortion
    {
      float buzz = x + 0.3f * std::sin(x * 10.0f * (1.0f + shapeParam * 5.0f));
      output = std::tanh(buzz * (1.0f + shapeParam));
      break;
    }
    case 50: // Crackle - Subtle noise/crackle character
    {
      float noise = nextNoiseSample() * 0.02f * shapeParam;
      output = std::tanh(x * (1.0f + shapeParam * 2.0f)) + noise * std::abs(x);
      break;
    }
    case 51: // Wrap - Wrap-around distortion
    {
      float wrapped = x * (1.0f + shapeParam * 3.0f);
      wrapped = std::fmod(wrapped + 3.0f, 2.0f) - 1.0f;
      output = wrapped;
      break;
    }
    case 52: // Density - Thick density
    {
      float density = std::tanh(x * 2.0f) + std::tanh(x * 0.5f);
      output = density * 0.5f * (1.0f + shapeParam * 0.5f);
      break;
    }

    // === MATH/EXOTIC (53-57) ===
    case 53: // Cheby 7 - 7th order Chebyshev
    {
      float xl = juce::jlimit(-1.0f, 1.0f, x);
      float x2 = xl * xl;
      float x3 = x2 * xl;
      float x5 = x3 * x2;
      float x7 = x5 * x2;
      output = (64.0f * x7 - 112.0f * x5 + 56.0f * x3 - 7.0f * xl) *
               (0.3f + shapeParam * 0.7f);
      break;
    }
    case 54: // Hyperbolic - sinh based
    {
      float scale = 0.5f + shapeParam * 1.5f;
      output = std::sinh(x * scale) / std::cosh(x * scale * 2.0f);
      break;
    }
    case 55: // Exponential - exp based limiting
    {
      float sign = x > 0.0f ? 1.0f : -1.0f;
      float absX = std::abs(x);
      float rate = 2.0f + shapeParam * 4.0f;
      output = sign * (1.0f - std::exp(-absX * rate));
      break;
    }
    case 56: // Parabolic - Parabolic curve
    {
      float scaled = x * (1.0f + shapeParam * 2.0f);
      if (std::abs(scaled) < 1.0f) {
        output = scaled - (scaled * scaled * scaled) / 3.0f;
      } else {
        output = (scaled > 0.0f ? 1.0f : -1.0f) * (2.0f / 3.0f);
      }
      break;
    }
    case 57: // Wavelet - Wavelet-inspired (Mexican hat)
    {
      float t = x * (2.0f + shapeParam * 4.0f);
      float t2 = t * t;
      output = (1.0f - t2) * std::exp(-t2 * 0.5f);
      output *= (1.0f + shapeParam);
      break;
    }

    default:
      output = std::tanh(x);
      break;
    }
    return output;
  };

  if (prePost) // Post: EQ -> Saturation
  {
    // 1. Process bands first
    processBands(buffer);

    // 2. Then apply oversampled saturation with selected waveshape
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock =
        oversampling.processSamplesUp(block);

    float drive = juce::Decibels::decibelsToGain(saturation);
    for (int channel = 0; channel < (int)oversampledBlock.getNumChannels();
         ++channel) {
      auto *channelData = oversampledBlock.getChannelPointer(channel);
      for (size_t sample = 0; sample < oversampledBlock.getNumSamples();
           ++sample) {
        float x = channelData[sample] * drive;
        channelData[sample] = applyWaveshape(x, waveshapeIndex, shape);
      }
    }
    oversampling.processSamplesDown(block);
  } else // Pre: Saturation -> EQ
  {
    // 1. Apply oversampled saturation first with selected waveshape
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock =
        oversampling.processSamplesUp(block);

    float drive = juce::Decibels::decibelsToGain(saturation);
    for (int channel = 0; channel < (int)oversampledBlock.getNumChannels();
         ++channel) {
      auto *channelData = oversampledBlock.getChannelPointer(channel);
      for (size_t sample = 0; sample < oversampledBlock.getNumSamples();
           ++sample) {
        float x = channelData[sample] * drive;
        channelData[sample] = applyWaveshape(x, waveshapeIndex, shape);
      }
    }
    oversampling.processSamplesDown(block);

    // 2. Then process bands
    processBands(buffer);
  }

  // 5. Final Stage: Delta Monitor / Mix, Output Gain, Limiter
  //
  // DELTA MODE: Output = safety((wet - dry) * deltaGain)
  // - Ignores Mix parameter (forces 100% wet for delta calculation)
  // - Applies delta gain for level control
  // - Uses tanh soft clipper for safety
  // - Crossfades smoothly to avoid clicks
  //
  // NORMAL MODE: Output = dry * (1 - mix) + wet * mix

  for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
    auto *channelData = buffer.getWritePointer(channel);
    auto *dryData = dryBuffer.getReadPointer(channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
      float wetSignal = channelData[sample];
      float drySignal = dryData[sample];

      // Update deltaSmoothed towards target (per-sample for smooth crossfade)
      if (deltaSmoothed < targetDeltaSmoothed) {
        deltaSmoothed =
            juce::jmin(deltaSmoothed + deltaCrossfadeStep, targetDeltaSmoothed);
      } else if (deltaSmoothed > targetDeltaSmoothed) {
        deltaSmoothed =
            juce::jmax(deltaSmoothed - deltaCrossfadeStep, targetDeltaSmoothed);
      }

      // Calculate normal mix output
      float normalOutput = drySignal * (1.0f - mix) + wetSignal * mix;

      // Calculate delta output: (100% wet - dry) * deltaGain
      // Note: For delta, we use the full wet signal minus dry (ignoring mix)
      float deltaRaw = (wetSignal - drySignal) * deltaGain;

      // Safety soft clipper for delta (tanh clips to -1..+1 range)
      float deltaOutput = std::tanh(deltaRaw);

      // Crossfade between normal and delta mode
      channelData[sample] =
          normalOutput * (1.0f - deltaSmoothed) + deltaOutput * deltaSmoothed;
    }
  }

  // Apply Output Gain before Limiter
  buffer.applyGain(outputGain);

  if (limiterEnable) {
    juce::dsp::AudioBlock<float> block(buffer);
    limiter.process(juce::dsp::ProcessContextReplacing<float>(block));
  }

  // === WRITE TO VISUALIZER BUFFER ===
  // Sum to mono and write to circular buffer
  // We only need ~512 samples for a frame. To avoid writing too fast (since
  // audio is 44.1k+) we can just write every Nth sample or just write
  // everything and let the GUI catch what it sees. Writing everything is
  // smoother for the waveform.
  const int numSamples = buffer.getNumSamples();
  const int numChannels = buffer.getNumChannels();

  if (numChannels > 0) {
    const float *readPtr = buffer.getReadPointer(0); // Left channel or mono

    for (int i = 0; i < numSamples; ++i) {
      int idx = visualizerWriteIndex.load(std::memory_order_relaxed);
      visualizerBuffer[static_cast<size_t>(idx)] = readPtr[i];
      visualizerWriteIndex.store((idx + 1) % visualizerBufferSize,
                                 std::memory_order_relaxed);
    }
  }

  // === ENVELOPE FOLLOWER UPDATE ===
  // Calculate max peak of the output block to drive UI
  float maxPeak = 0.0f;
  for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
    maxPeak = juce::jmax(
        maxPeak, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));
  }

  // Simple smoothing/decay could be done here, or just push peak to UI
  // Pushing current peak is fine for "Is Talking" logic
  // We use atomic store
  currentRMSLevel.store(maxPeak, std::memory_order_relaxed);
}

//==============================================================================
// 3. Editor Creation
bool Vst_saturatorAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *Vst_saturatorAudioProcessor::createEditor() {
  return new Vst_saturatorAudioProcessorEditor(*this);
}

//==============================================================================
// 4. Persistence
void Vst_saturatorAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  // Save the APVTS state to XML, then to binary
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  if (xml != nullptr)
    copyXmlToBinary(*xml, destData);
}

void Vst_saturatorAudioProcessor::setStateInformation(const void *data,
                                                      int sizeInBytes) {
  // Load the APVTS state from binary
  if (data == nullptr || sizeInBytes <= 0)
    return;
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState == nullptr)
    return;
  if (!xmlState->hasTagName(apvts.state.getType()))
    return;
  apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// Factory
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new Vst_saturatorAudioProcessor();
}

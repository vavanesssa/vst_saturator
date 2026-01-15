/*
  ==============================================================================

    PluginEditor.h
    --------------
    This file declares the "Audio Processor Editor" (the UI).

    Role:
    The Editor handles the visual interface:
    1.  Displaying controls (Sliders, Buttons).
    2.  Handling user input (Mouse clicks, drags).
    3.  Connecting visual controls to the Processor's parameters.

  ==============================================================================
*/

#pragma once

#include "CustomLookAndFeel.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>

//==============================================================================
class ScrollableTooltipWindow final : public juce::TooltipWindow {
public:
  ScrollableTooltipWindow(juce::Component *parent, int delayMs,
                          CustomLookAndFeel &laf)
      : juce::TooltipWindow(parent, delayMs), lookAndFeel(laf) {
    setLookAndFeel(&lookAndFeel);
    setInterceptsMouseClicks(true, true);
  }

  void mouseWheelMove(const juce::MouseEvent &,
                      const juce::MouseWheelDetails &details) override {
    if (lookAndFeel.scrollTooltip(details.deltaY))
      repaint();
  }

private:
  CustomLookAndFeel &lookAndFeel;
};

class Vst_saturatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer {
public:
  // Constructor: Takes a reference to the Processor so we can access
  // parameters.
  Vst_saturatorAudioProcessorEditor(Vst_saturatorAudioProcessor &);
  ~Vst_saturatorAudioProcessorEditor() override;

  //==============================================================================
  // Drawing
  // Called when the component needs to be repainted (e.g., background color).
  void paint(juce::Graphics &) override;

  // Layout
  // Called when the window is resized. Position your components here.
  void resized() override;

  // Timer callback for periodic repaints
  void timerCallback() override;

private:
  Vst_saturatorAudioProcessor &audioProcessor;

  // === GLOBAL SCALING CONSTANTS ===
  // Fixed design size - all UI elements use these coordinates
  static constexpr int DESIGN_WIDTH = 1300;
  static constexpr int DESIGN_HEIGHT = 850;

  // Scaling variables (calculated on resize)
  float scaleFactor = 1.0f;
  int offsetX = 0;
  int offsetY = 0;

  // A. Saturation Globale
  juce::Slider saturationSlider, shapeSlider;
  juce::ComboBox waveshapeCombo;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      saturationAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      shapeAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      waveshapeAttachment;

  // B. Bande LOW
  juce::ToggleButton lowEnableButton;
  juce::Slider lowFreqSlider, lowWarmthSlider, lowLevelSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      lowEnableAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      lowFreqAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      lowWarmthAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      lowLevelAttachment;

  // C. Bande HIGH
  juce::ToggleButton highEnableButton;
  juce::Slider highFreqSlider, highSoftnessSlider, highLevelSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      highEnableAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      highFreqAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      highSoftnessAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      highLevelAttachment;

  // D. Gain & Routing
  juce::Slider inputGainSlider, mixSlider, outputGainSlider;
  juce::ToggleButton prePostButton, limiterButton, bypassButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      inputGainAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      mixAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      outputGainAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      prePostAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      limiterAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      bypassAttachment;

  // E. Delta Monitoring
  juce::ToggleButton deltaButton;
  juce::Slider deltaGainSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      deltaAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      deltaGainAttachment;

  // F. Presets Menu with navigation
  juce::ComboBox presetsCombo;
  juce::TextButton presetLeftBtn{"<"};
  juce::TextButton presetRightBtn{">"};
  int currentPresetIndex = 0; // Track current preset for arrow navigation

  // G. Waveshape navigation (arrows for the existing waveshapeCombo)
  juce::TextButton waveLeftBtn{"<"};
  juce::TextButton waveRightBtn{">"};

  // Preset data structure
  struct PresetData {
    juce::String name;
    int waveshape;    // 1-18
    float drive;      // 0-24
    float shape;      // 0-1
    float inputGain;  // -24 to +24
    float mix;        // 0-100
    float outputGain; // -24 to +24
    bool lowEnable;
    float lowFreq;   // 20-500
    float lowWarmth; // 0-1
    float lowLevel;  // 0-12
    bool highEnable;
    float highFreq;     // 500-16000
    float highSoftness; // 0-1
    float highLevel;    // 0-12
    bool limiter;
    bool prePost;
  };

  std::vector<PresetData> presets;
  void initializePresets();
  void applyPreset(int presetIndex);
  void navigatePreset(int direction);    // -1 for prev, +1 for next
  void navigateWaveshape(int direction); // -1 for prev, +1 for next

  // Custom UI styling
  CustomLookAndFeel customLookAndFeel;

  // Steve image for left side display
  juce::Image steveImage;
  juce::Image steve2Image;

  // Helper to load images robustly
  juce::Image loadImage(const juce::String &imageName);

  // Build hash for display
  juce::String buildHash;

  // Helper to transform design coordinates to scaled window coordinates
  juce::Rectangle<int> scaleDesignBounds(int x, int y, int width,
                                         int height) const;

  // Waveform visualization
  juce::Path wavePath;
  juce::Path waveFillPath;
  std::vector<float> localWaveform;

  // Tooltip window
  ScrollableTooltipWindow tooltipWindow;

  // Signature Link (Hyperlink)
  juce::HyperlinkButton signatureLink;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      Vst_saturatorAudioProcessorEditor)
};

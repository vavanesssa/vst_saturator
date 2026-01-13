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

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

//==============================================================================
class Vst_saturatorAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    // Constructor: Takes a reference to the Processor so we can access parameters.
    Vst_saturatorAudioProcessorEditor (Vst_saturatorAudioProcessor&);
    ~Vst_saturatorAudioProcessorEditor() override;

    //==============================================================================
    // Drawing
    // Called when the component needs to be repainted (e.g., background color).
    void paint (juce::Graphics&) override;

    // Layout
    // Called when the window is resized. Position your components here.
    void resized() override;

    // Timer callback for periodic repaints
    void timerCallback() override;

private:
    Vst_saturatorAudioProcessor& audioProcessor;

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
    juce::Label saturationLabel, shapeLabel;
    juce::ComboBox waveshapeCombo;
    juce::Label waveshapeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shapeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveshapeAttachment;

    // B. Bande LOW
    juce::ToggleButton lowEnableButton;
    juce::Slider lowFreqSlider, lowWarmthSlider, lowLevelSlider;
    juce::Label lowFreqLabel, lowWarmthLabel, lowLevelLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lowEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowWarmthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowLevelAttachment;

    // C. Bande HIGH
    juce::ToggleButton highEnableButton;
    juce::Slider highFreqSlider, highSoftnessSlider, highLevelSlider;
    juce::Label highFreqLabel, highSoftnessLabel, highLevelLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> highEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highSoftnessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highLevelAttachment;

    // D. Gain & Routing
    juce::Slider inputGainSlider, mixSlider, outputGainSlider;
    juce::Label inputGainLabel, mixLabel, outputGainLabel;
    juce::ToggleButton prePostButton, limiterButton, bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> prePostAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Custom UI styling
    CustomLookAndFeel customLookAndFeel;

    // Steve image for left side display
    juce::Image steveImage;

    // Build hash for display
    juce::String buildHash;

    // Helper to transform design coordinates to scaled window coordinates
    juce::Rectangle<int> scaleDesignBounds(int x, int y, int width, int height) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Vst_saturatorAudioProcessorEditor)
};

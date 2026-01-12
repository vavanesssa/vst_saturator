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

//==============================================================================
class Vst_saturatorAudioProcessorEditor  : public juce::AudioProcessorEditor
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

private:
    // Reference to the AudioProcessor (logic)
    Vst_saturatorAudioProcessor& audioProcessor;

    // Phase 4: UI Components
    // -------------------------------------------------------------------------
    // Sliders
    juce::Slider driveSlider;
    juce::Slider outputSlider;

    // Labels (to show which slider is which)
    juce::Label driveLabel;
    juce::Label outputLabel;

    // Attachments
    // These objects link the Slider to the APVTS parameter.
    // They handle updating the slider when automation happens,
    // and updating the parameter when the slider moves.
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Vst_saturatorAudioProcessorEditor)
};

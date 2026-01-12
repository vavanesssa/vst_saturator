/*
  ==============================================================================

    CustomLookAndFeel.h
    -------------------
    Custom UI styling for sliders and knobs.

    This file defines a custom LookAndFeel that:
    - Uses a custom knob image for rotary sliders
    - Applies custom colors and styling

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    // Draw the rotary slider (knob) using a custom image
    void drawRotarySlider(juce::Graphics& g,
                         int x, int y, int width, int height,
                         float sliderPosProportional,
                         float rotaryStartAngle,
                         float rotaryEndAngle,
                         juce::Slider& slider) override;

private:
    juce::Image knobImage;

    // Cache for resized knob images to avoid regenerating them constantly
    std::map<int, juce::Image> resizedKnobCache;

    // Helper function to get or create a resized version of the knob
    juce::Image getResizedKnob(int targetSize);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};

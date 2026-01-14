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

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
  CustomLookAndFeel();
  ~CustomLookAndFeel() override = default;

  // Draw the rotary slider (knob) with minimal design
  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPosProportional, float rotaryStartAngle,
                        float rotaryEndAngle, juce::Slider &slider) override;

  // Custom Slider Layout to center the text box
  juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;

  // Custom Slider Text Box creation
  juce::Label* createSliderTextBox(juce::Slider& slider) override;

  // Custom hit test for larger hover area
  bool hitTestRotarySlider(juce::Slider &slider, int x, int y);

  // Draw toggle buttons with glowy violet style
  void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                        bool isMouseOverButton, bool isButtonDown) override;

  // Draw ComboBox with custom styling
  void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                    int buttonX, int buttonY, int buttonW, int buttonH,
                    juce::ComboBox &box) override;

  void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override;

  // Tooltips customization
  juce::Rectangle<int>
  getTooltipBounds(const juce::String &tipText, juce::Point<int> screenPos,
                   juce::Rectangle<int> parentArea) override;

  void drawTooltip(juce::Graphics &g, const juce::String &text, int width,
                   int height) override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};

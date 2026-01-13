/*
  ==============================================================================

    CustomLookAndFeel.cpp
    ---------------------
    Minimal clean knob design drawn entirely in code.

    Creates a modern, flat knob with:
    - Concentric circles for depth
    - Arc showing current value
    - Indicator dot at the top

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPosProportional,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider& slider)
{
    // Calculate the angle based on slider position
    float angle = rotaryStartAngle + (sliderPosProportional * (rotaryEndAngle - rotaryStartAngle));

    // Center and radius
    float centerX = (float)x + (float)width / 2.0f;
    float centerY = (float)y + (float)height / 2.0f;
    float radius = juce::jmin(width, height) / 2.0f - 2.0f;

    // === OUTER RING ===
    g.setColour(juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 1.0f)); // Dark orange-brown
    g.drawEllipse(centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    // === BACKGROUND ARC (full circle, faint) ===
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centerX, centerY, radius * 0.85f, radius * 0.85f,
                               0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour::fromFloatRGBA(0.85f, 0.75f, 0.55f, 0.5f)); // Light beige
    g.strokePath(backgroundArc, juce::PathStrokeType(3.5f));

    // === VALUE ARC (solid dark orange) ===
    juce::Path valueArc;
    valueArc.addCentredArc(centerX, centerY, radius * 0.85f, radius * 0.85f,
                          0.0f, rotaryStartAngle, angle, true);

    // Solid dark orange color (RGB 204, 102, 0)
    juce::Colour arcColour = juce::Colour::fromFloatRGBA(0.8f, 0.4f, 0.0f, 1.0f); // Dark orange
    g.setColour(arcColour);
    g.strokePath(valueArc, juce::PathStrokeType(4.5f));

    // === CENTER CIRCLE ===
    g.setColour(juce::Colour::fromFloatRGBA(0.93f, 0.90f, 0.82f, 1.0f)); // Beige background
    g.fillEllipse(centerX - radius * 0.5f, centerY - radius * 0.5f, radius, radius);

    // === INDICATOR DOT ===
    float dotDistance = radius * 0.65f;
    float dotX = centerX + dotDistance * std::cos(angle - juce::MathConstants<float>::pi / 2.0f);
    float dotY = centerY + dotDistance * std::sin(angle - juce::MathConstants<float>::pi / 2.0f);

    g.setColour(arcColour);
    g.fillEllipse(dotX - 4.0f, dotY - 4.0f, 8.0f, 8.0f);

    // === OUTER HIGHLIGHT (subtle) ===
    g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.95f, 0.85f, 0.4f));
    g.drawEllipse(centerX - radius * 0.98f, centerY - radius * 0.98f, radius * 1.96f, radius * 1.96f, 0.5f);

    // === VALUE TEXT IN CENTER ===
    // Get the value from the slider
    float sliderValue = slider.getValue();
    juce::String valueText = juce::String(sliderValue, 2); // 2 decimal places

    // Draw the text centered in the knob
    g.setColour(juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 1.0f)); // Dark orange-brown
    g.setFont(juce::Font(14.0f, juce::Font::bold));

    juce::Rectangle<float> textBox(centerX - radius * 0.4f, centerY - 8.0f, radius * 0.8f, 16.0f);
    g.drawText(valueText, textBox.toNearestInt(), juce::Justification::centred, false);
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g,
                                         juce::ToggleButton& button,
                                         bool isMouseOverButton,
                                         bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    float padding = 5.0f;
    auto bgBounds = bounds.reduced(padding);

    // Draw background and border based on toggle state
    if (button.getToggleState())
    {
        // Active state: orange/golden glow
        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.6f, 0.2f, 0.3f)); // Orange glow
        g.fillRoundedRectangle(bgBounds.expanded(2.0f), 6.0f);
        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.5f, 0.1f, 0.95f)); // Bright orange
        g.drawRoundedRectangle(bgBounds, 6.0f, 2.0f);
    }
    else
    {
        // Inactive state: brown outline
        g.setColour(juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 0.4f));
        g.drawRoundedRectangle(bgBounds, 6.0f, 1.5f);
    }

    // Draw text
    g.setColour(button.getToggleState()
        ? juce::Colour::fromFloatRGBA(1.0f, 0.5f, 0.1f, 1.0f) // Bright orange
        : juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 0.7f)); // Dark brown
    g.setFont(juce::Font(18.0f, juce::Font::bold));  // Increased from 16pt to 18pt
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, true);
}

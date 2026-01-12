/*
  ==============================================================================

    CustomLookAndFeel.cpp
    ---------------------
    Implementation of custom UI styling.

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Try to load the knob image from the plugin's Resources folder
    juce::File knobFile;

    // First try: Bundle Resources folder (macOS VST3)
    auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    knobFile = appDir.getChildFile("Contents/Resources/knob.png");

    // Fallback: Try Assets in current working directory
    if (!knobFile.existsAsFile())
    {
        knobFile = juce::File::getCurrentWorkingDirectory().getChildFile("Assets/knob.png");
    }

    if (knobFile.existsAsFile())
    {
        knobImage = juce::ImageFileFormat::loadFrom(knobFile);
    }
    else
    {
        // Fallback: Create a simple colored circle if image not found
        int size = 120;
        knobImage = juce::Image(juce::Image::RGB, size, size, true);
        juce::Graphics g(knobImage);

        // Draw a gradient circle as fallback
        g.setColour(juce::Colours::orange);
        g.fillEllipse(10, 10, size - 20, size - 20);

        g.setColour(juce::Colours::white);
        g.drawEllipse(10, 10, size - 20, size - 20, 2.0f);
    }
}

juce::Image CustomLookAndFeel::getResizedKnob(int targetSize)
{
    // Check if we already have this size cached
    if (resizedKnobCache.find(targetSize) != resizedKnobCache.end())
    {
        return resizedKnobCache[targetSize];
    }

    // If no image, return empty
    if (knobImage.isNull())
    {
        return juce::Image();
    }

    // Create a resized version of the knob
    juce::Image resized(juce::Image::ARGB, targetSize, targetSize, true);
    juce::Graphics g(resized);

    // Use HighQualityScalingMode for better downscaling
    g.drawImage(knobImage, 0, 0, targetSize, targetSize, 0, 0, knobImage.getWidth(), knobImage.getHeight(), false);

    // Cache it for future use
    resizedKnobCache[targetSize] = resized;

    return resized;
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

    // If we have a knob image, draw it rotated
    if (!knobImage.isNull())
    {
        // Use the smaller of width and height to create a square knob
        int knobSize = juce::jmin(width, height);

        // Get the resized knob (cached for performance)
        juce::Image resizedKnob = getResizedKnob(knobSize);

        if (!resizedKnob.isNull())
        {
            int centerX = x + width / 2;
            int centerY = y + height / 2;

            // Draw the knob image rotated
            juce::AffineTransform transform = juce::AffineTransform::translation((float)centerX, (float)centerY)
                                             .rotated(angle)
                                             .translated(-(float)(knobSize / 2), -(float)(knobSize / 2));

            g.drawImageTransformed(resizedKnob, transform);
        }
    }
    else
    {
        // Fallback: use default drawing
        LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
                                        sliderPosProportional,
                                        rotaryStartAngle, rotaryEndAngle, slider);
    }
}

/*
  ==============================================================================

    PluginEditor.cpp
    ----------------
    This file implements the UI logic declared in PluginEditor.h.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Vst_saturatorAudioProcessorEditor::Vst_saturatorAudioProcessorEditor (Vst_saturatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Phase 4: Setting up the UI
    // -------------------------------------------------------------------------

    // Load background image from plugin's Resources folder
    juce::File backgroundFile;

    // First try: Bundle Resources folder (macOS VST3)
    auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    backgroundFile = appDir.getChildFile("Contents/Resources/background.png");

    // Fallback: Try Assets in current working directory
    if (!backgroundFile.existsAsFile())
    {
        backgroundFile = juce::File::getCurrentWorkingDirectory().getChildFile("Assets/background.png");
    }

    if (backgroundFile.existsAsFile())
    {
        backgroundImage = juce::ImageFileFormat::loadFrom(backgroundFile);
    }

    // Apply custom LookAndFeel to sliders
    driveSlider.setLookAndFeel(&customLookAndFeel);
    outputSlider.setLookAndFeel(&customLookAndFeel);

    // 1. Configure Drive Slider
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(driveSlider);

    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    driveLabel.attachToComponent(&driveSlider, false);

    // 2. Configure Output Slider
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(outputSlider);

    outputLabel.setText("Output", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);
    outputLabel.attachToComponent(&outputSlider, false);

    // 3. Create Attachments
    // This links the UI sliders to the parameters ID defined in PluginProcessor.cpp
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "drive", driveSlider);

    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "output", outputSlider);

    // Set the initial size of the window (Width, Height)
    setSize (400, 300);
}

Vst_saturatorAudioProcessorEditor::~Vst_saturatorAudioProcessorEditor()
{
}

//==============================================================================
void Vst_saturatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Draw background image scaled to current size, or fallback to color
    if (!backgroundImage.isNull())
    {
        // Only rescale if the window size has changed
        if (lastScaledWidth != getWidth() || lastScaledHeight != getHeight())
        {
            // Create a scaled version of the background
            scaledBackgroundImage = juce::Image(juce::Image::RGB, getWidth(), getHeight(), true);
            juce::Graphics bg(scaledBackgroundImage);
            bg.drawImage(backgroundImage, 0, 0, getWidth(), getHeight(),
                        0, 0, backgroundImage.getWidth(), backgroundImage.getHeight(), false);

            lastScaledWidth = getWidth();
            lastScaledHeight = getHeight();
        }

        g.drawImageAt(scaledBackgroundImage, 0, 0);
    }
    else
    {
        // Fallback: Fill with a dark color
        g.fillAll (juce::Colour::fromFloatRGBA (0.1f, 0.1f, 0.12f, 1.0f)); // Dark grey/blue
    }

    // Draw some text
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("VST Saturator", getLocalBounds(), juce::Justification::centredTop, 1);

    // Draw an educational note
    g.setFont (12.0f);
    g.setColour (juce::Colours::grey);
    g.drawFittedText ("Educational Project - v1.0", getLocalBounds().removeFromBottom(20), juce::Justification::centred, 1);
}

void Vst_saturatorAudioProcessorEditor::resized()
{
    // This is where we define where components go.
    // The layout is responsive and adapts to window size.

    // Reset the cached scaled background so it gets redrawn
    lastScaledWidth = 0;
    lastScaledHeight = 0;

    // Calculate responsive slider dimensions based on window size
    int sliderWidth = getWidth() / 4;  // 25% of width
    int sliderHeight = getHeight() * 2 / 3;  // 67% of height

    // Ensure minimum sizes for usability
    sliderWidth = juce::jmax(80, sliderWidth);
    sliderHeight = juce::jmax(100, sliderHeight);

    // Calculate positions to center them
    int spacing = juce::jmax(20, getWidth() / 20);
    int totalWidth = (sliderWidth * 2) + spacing;
    int startX = (getWidth() - totalWidth) / 2;
    int startY = (getHeight() - sliderHeight) / 2;

    driveSlider.setBounds(startX, startY, sliderWidth, sliderHeight);
    outputSlider.setBounds(startX + sliderWidth + spacing, startY, sliderWidth, sliderHeight);

    // Trigger a repaint to redraw the background
    repaint();
}

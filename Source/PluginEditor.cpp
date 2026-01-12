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
    // Fill the background with a dark color
    g.fillAll (juce::Colour::fromFloatRGBA (0.1f, 0.1f, 0.12f, 1.0f)); // Dark grey/blue

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
    // We use a "FlexBox" or simple bounds logic.

    auto area = getLocalBounds();
    auto topArea = area.removeFromTop(50); // Title area
    auto bottomArea = area.removeFromBottom(30); // Footer area

    // We have two sliders, let's put them side by side
    int sliderWidth = 120;
    int sliderHeight = 150;

    // Calculate positions to center them
    int spacing = 20;
    int totalWidth = (sliderWidth * 2) + spacing;
    int startX = (getWidth() - totalWidth) / 2;
    int startY = (getHeight() - sliderHeight) / 2 + 10; // Offset slightly down

    driveSlider.setBounds(startX, startY, sliderWidth, sliderHeight);
    outputSlider.setBounds(startX + sliderWidth + spacing, startY, sliderWidth, sliderHeight);
}

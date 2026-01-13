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
    // Load build hash from version.txt
    juce::File versionFile;
    auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    versionFile = appDir.getChildFile("Contents/Resources/version.txt");

    if (!versionFile.existsAsFile())
    {
        versionFile = juce::File::getCurrentWorkingDirectory().getChildFile("Assets/version.txt");
    }

    if (versionFile.existsAsFile())
    {
        buildHash = versionFile.loadFileAsString().upToFirstOccurrenceOf("\n", false, false).trim();
    }
    else
    {
        buildHash = "DEV";
    }

    // Load steve image from build/Assets directory
    juce::File steveFile;

    // Try relative path from executable using ../
    auto executablePath = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    steveFile = executablePath.getChildFile("../../../../../Assets/steve.png");

    // If that didn't work, try going to parent and then to Assets
    if (!steveFile.existsAsFile())
    {
        steveFile = executablePath;
        for (int i = 0; i < 7 && steveFile.exists(); ++i)
            steveFile = steveFile.getParentDirectory();
        steveFile = steveFile.getChildFile("Assets/steve.png");
    }

    // Try Resources folder (for bundled VST3 plugin)
    if (!steveFile.existsAsFile())
    {
        steveFile = executablePath.getParentDirectory()
                                   .getParentDirectory()
                                   .getChildFile("Resources/steve.png");
    }

    // Fallback: try absolute path to source Assets
    if (!steveFile.existsAsFile())
    {
        steveFile = juce::File("/Users/vava/Documents/GitHub/vst_saturator/Assets/steve.png");
    }

    if (steveFile.existsAsFile())
    {
        steveImage = juce::ImageFileFormat::loadFrom(steveFile);
    }

    // Helper lambda for configuring sliders
    auto configureSlider = [&](juce::Slider& slider, const juce::String& paramID)
    {
        slider.setLookAndFeel(&customLookAndFeel);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); // Hide text box
        addAndMakeVisible(slider);
    };

    // Helper lambda for attachments
    auto attachSlider = [&](auto& attachment, const juce::String& paramID, juce::Slider& slider)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    auto attachButton = [&](auto& attachment, const juce::String& paramID, juce::ToggleButton& button)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, paramID, button);
    };

    auto configureEnableButton = [&](juce::ToggleButton& button, const juce::String& text)
    {
        button.setButtonText(text);
        button.setLookAndFeel(&customLookAndFeel);
        addAndMakeVisible(button);
    };

    auto configureLabel = [&](juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour::fromFloatRGBA(0.5f, 0.3f, 0.1f, 1.0f)); // Dark brown
        label.setFont(juce::Font(20.0f, juce::Font::bold));  // Increased from 16pt to 20pt
        addAndMakeVisible(label);
    };

    // A. Saturation Globale
    configureSlider(saturationSlider, "drive");
    configureLabel(saturationLabel, "Saturation");
    saturationLabel.attachToComponent(&saturationSlider, false);

    configureSlider(shapeSlider, "shape");
    configureLabel(shapeLabel, "Shape");
    shapeLabel.attachToComponent(&shapeSlider, false);

    attachSlider(saturationAttachment, "drive", saturationSlider);
    attachSlider(shapeAttachment, "shape", shapeSlider);

    // B. Bande LOW
    configureEnableButton(lowEnableButton, "LOW");
    attachButton(lowEnableAttachment, "lowEnable", lowEnableButton);

    configureSlider(lowFreqSlider, "lowFreq");
    configureLabel(lowFreqLabel, "Low Freq");
    lowFreqLabel.attachToComponent(&lowFreqSlider, false);

    configureSlider(lowWarmthSlider, "lowWarmth");
    configureLabel(lowWarmthLabel, "Low Warmth");
    lowWarmthLabel.attachToComponent(&lowWarmthSlider, false);

    configureSlider(lowLevelSlider, "lowLevel");
    configureLabel(lowLevelLabel, "Low Level");
    lowLevelLabel.attachToComponent(&lowLevelSlider, false);

    attachSlider(lowFreqAttachment, "lowFreq", lowFreqSlider);
    attachSlider(lowWarmthAttachment, "lowWarmth", lowWarmthSlider);
    attachSlider(lowLevelAttachment, "lowLevel", lowLevelSlider);

    // C. Bande HIGH
    configureEnableButton(highEnableButton, "HIGH");
    attachButton(highEnableAttachment, "highEnable", highEnableButton);

    configureSlider(highFreqSlider, "highFreq");
    configureLabel(highFreqLabel, "High Freq");
    highFreqLabel.attachToComponent(&highFreqSlider, false);

    configureSlider(highSoftnessSlider, "highSoftness");
    configureLabel(highSoftnessLabel, "High Softness");
    highSoftnessLabel.attachToComponent(&highSoftnessSlider, false);

    configureSlider(highLevelSlider, "highLevel");
    configureLabel(highLevelLabel, "High Level");
    highLevelLabel.attachToComponent(&highLevelSlider, false);

    attachSlider(highFreqAttachment, "highFreq", highFreqSlider);
    attachSlider(highSoftnessAttachment, "highSoftness", highSoftnessSlider);
    attachSlider(highLevelAttachment, "highLevel", highLevelSlider);

    // D. Gain & Routing
    configureSlider(inputGainSlider, "inputGain");
    configureLabel(inputGainLabel, "Input");
    inputGainLabel.attachToComponent(&inputGainSlider, false);

    configureSlider(mixSlider, "mix");
    configureLabel(mixLabel, "Mix");
    mixLabel.attachToComponent(&mixSlider, false);

    configureSlider(outputGainSlider, "output");
    configureLabel(outputGainLabel, "Output");
    outputGainLabel.attachToComponent(&outputGainSlider, false);

    attachSlider(inputGainAttachment, "inputGain", inputGainSlider);
    attachSlider(mixAttachment, "mix", mixSlider);
    attachSlider(outputGainAttachment, "output", outputGainSlider);

    // Footer buttons with custom styling
    prePostButton.setButtonText("Pre/Post");
    prePostButton.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(prePostButton);
    attachButton(prePostAttachment, "prePost", prePostButton);

    limiterButton.setButtonText("Limiter");
    limiterButton.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(limiterButton);
    attachButton(limiterAttachment, "limiter", limiterButton);

    bypassButton.setButtonText("Bypass");
    bypassButton.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(bypassButton);
    attachButton(bypassAttachment, "bypass", bypassButton);

    // Set initial size to design size
    // Enable resizing with constraints (min 650x425 = 50% of design, max 2600x1700 = 200% of design)
    setSize(DESIGN_WIDTH, DESIGN_HEIGHT);
    setResizable(true, true);
    setResizeLimits(DESIGN_WIDTH / 2, DESIGN_HEIGHT / 2, DESIGN_WIDTH * 2, DESIGN_HEIGHT * 2);

    // Lock aspect ratio to maintain proper proportions
    getConstrainer()->setFixedAspectRatio(static_cast<double>(DESIGN_WIDTH) / static_cast<double>(DESIGN_HEIGHT));

    // Start a timer to update value labels
    startTimer(100);
}

Vst_saturatorAudioProcessorEditor::~Vst_saturatorAudioProcessorEditor()
{
}

//==============================================================================
void Vst_saturatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Warm beige background matching steve.png
    g.fillAll(juce::Colour::fromFloatRGBA(0.93f, 0.90f, 0.82f, 1.0f)); // Beige

    // Draw steve image on the left side if loaded - full height with proper aspect ratio
    if (!steveImage.isNull())
    {
        juce::Rectangle<int> imageBounds(20, 20, 440, getHeight() - 40);
        g.drawImageWithin(steveImage, imageBounds.getX(), imageBounds.getY(), imageBounds.getWidth(), imageBounds.getHeight(), juce::RectanglePlacement::centred, false);
    }

    // Build info in bottom right corner with dark text
    g.setColour(juce::Colour::fromFloatRGBA(0.5f, 0.3f, 0.1f, 0.95f)); // Dark brown - more opaque
    g.setFont(juce::Font(18.0f, juce::Font::bold)); // Larger, bold font
    juce::String versionText = "by NeiXXa / Version : " + buildHash;
    juce::Rectangle<int> bounds = getLocalBounds();
    juce::Rectangle<int> versionBox(bounds.getRight() - 350, bounds.getBottom() - 32, 340, 26);
    g.drawText(versionText, versionBox, juce::Justification::bottomRight, true);
}

void Vst_saturatorAudioProcessorEditor::resized()
{
    // FIXED PIXEL LAYOUT - All positions and sizes are absolute
    // Window is 1300x850 and does not resize
    // ULTRATHINK SPACING - Generous padding between all elements

    // === LAYOUT CONSTANTS (ALL IN FIXED PIXELS) ===
    const int imageX = 20;
    const int imageY = 20;
    const int imageWidth = 440;
    const int imageHeight = 810;  // 850 - 20 - 20

    const int contentStartX = imageX + imageWidth + 30;  // 490
    const int contentStartY = 100;  // Header space
    const int contentWidth = 1300 - contentStartX - 30;  // ~780px
    const int contentHeight = 850 - contentStartY - 110;  // ~640px

    // 4 columns with GENEROUS spacing
    const int columnCount = 4;
    const int columnSpacing = 50;  // INCREASED from 30 to 50
    const int columnWidth = (contentWidth - (columnCount - 1) * columnSpacing) / columnCount;  // ~156px

    // Fixed knob dimensions - LARGER
    const int knobWidth = columnWidth - 10;  // Account for margin
    const int knobHeight = 140;  // INCREASED from 120
    const int verticalSpacing = 45;  // ULTRATHINK vertical spacing between knobs
    const int buttonHeight = 55;  // Button height (LOW/HIGH)
    const int footerButtonWidth = 180;  // Slightly larger
    const int footerButtonHeight = 65;

    // === COLUMN POSITIONS (X coordinates) ===
    const int col1X = contentStartX;                          // GainIn
    const int col2X = col1X + columnWidth + columnSpacing;    // Low
    const int col3X = col2X + columnWidth + columnSpacing;    // High
    const int col4X = col3X + columnWidth + columnSpacing;    // Master

    // === ROW POSITIONS (Y coordinates) - GENEROUS VERTICAL SPACING ===
    const int row1Y = contentStartY + 20;
    const int row2Y = row1Y + knobHeight + verticalSpacing;
    const int row3Y = row2Y + knobHeight + verticalSpacing;
    const int row4Y = row3Y + knobHeight + verticalSpacing;

    // === FOOTER ===
    const int footerY = 850 - 100;

    // COLUMN 1: GAIN IN (Input slider only - centered vertically)
    inputGainSlider.setBounds(col1X + 5, row2Y, knobWidth, knobHeight);

    // COLUMN 2: LOW BAND
    lowEnableButton.setBounds(col2X + (columnWidth - 110) / 2, row1Y - 30, 110, buttonHeight);
    lowFreqSlider.setBounds(col2X + 5, row1Y, knobWidth, knobHeight);
    lowWarmthSlider.setBounds(col2X + 5, row2Y, knobWidth, knobHeight);
    lowLevelSlider.setBounds(col2X + 5, row3Y, knobWidth, knobHeight);

    // COLUMN 3: HIGH BAND
    highEnableButton.setBounds(col3X + (columnWidth - 110) / 2, row1Y - 30, 110, buttonHeight);
    highFreqSlider.setBounds(col3X + 5, row1Y, knobWidth, knobHeight);
    highSoftnessSlider.setBounds(col3X + 5, row2Y, knobWidth, knobHeight);
    highLevelSlider.setBounds(col3X + 5, row3Y, knobWidth, knobHeight);

    // COLUMN 4: MASTER (4 knobs)
    saturationSlider.setBounds(col4X + 5, row1Y, knobWidth, knobHeight);
    shapeSlider.setBounds(col4X + 5, row2Y, knobWidth, knobHeight);
    mixSlider.setBounds(col4X + 5, row3Y, knobWidth, knobHeight);
    outputGainSlider.setBounds(col4X + 5, row4Y, knobWidth, knobHeight);

    // === FOOTER BUTTONS (centered) ===
    const int footerContentWidth = footerButtonWidth * 3 + columnSpacing;
    const int footerStartX = (1300 - footerContentWidth) / 2;

    bypassButton.setBounds(footerStartX, footerY, footerButtonWidth, footerButtonHeight);
    limiterButton.setBounds(footerStartX + footerButtonWidth + columnSpacing / 2, footerY, footerButtonWidth, footerButtonHeight);
    prePostButton.setBounds(footerStartX + (footerButtonWidth + columnSpacing / 2) * 2, footerY, footerButtonWidth, footerButtonHeight);

    repaint();
}

void Vst_saturatorAudioProcessorEditor::timerCallback()
{
    // Timer is used to trigger repaints to show updated knob values
    repaint();
}

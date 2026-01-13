/*
  ==============================================================================

    PluginEditor.cpp
    ----------------
    This file implements the UI logic declared in PluginEditor.h.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
Vst_saturatorAudioProcessorEditor::Vst_saturatorAudioProcessorEditor(
    Vst_saturatorAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // Load build hash from version.txt
  juce::File versionFile;
  auto appDir =
      juce::File::getSpecialLocation(juce::File::currentApplicationFile);
  versionFile = appDir.getChildFile("Contents/Resources/version.txt");

  if (!versionFile.existsAsFile()) {
    versionFile = juce::File::getCurrentWorkingDirectory().getChildFile(
        "Assets/version.txt");
  }

  if (versionFile.existsAsFile()) {
    buildHash = versionFile.loadFileAsString()
                    .upToFirstOccurrenceOf("\n", false, false)
                    .trim();
  } else {
    buildHash = "DEV";
  }

  // DEBUG: Write to a log file to see what paths are being checked
  juce::File steveFile; // Declaration added
  juce::File logFile("/Users/vava/Desktop/steverator_debug.txt");
  logFile.replaceWithText("Debugging Image Loading...\n");
  logFile.appendText("App File: " + appDir.getFullPathName() + "\n");

  // Path 1: Inside App Bundle (Standard macOS)
  // If appDir is the .app bundle:
  juce::File path1 = appDir.getChildFile("Contents/Resources/steve.png");
  logFile.appendText("Path 1 (AppBundle): " + path1.getFullPathName() +
                     " - Exists: " + (path1.existsAsFile() ? "YES" : "NO") +
                     "\n");

  // Path 2: Relative to Executable (Binary inside MacOS)
  // If appDir is .../Contents/MacOS/Steverator
  juce::File path2 =
      appDir.getParentDirectory().getParentDirectory().getChildFile(
          "Resources/steve.png");
  logFile.appendText("Path 2 (BinaryRel): " + path2.getFullPathName() +
                     " - Exists: " + (path2.existsAsFile() ? "YES" : "NO") +
                     "\n");

  // Path 3: Dev path fallback
  juce::File path3 =
      juce::File("/Users/vava/Documents/GitHub/vst_saturator/Assets/steve.png");
  logFile.appendText("Path 3 (Hardcoded): " + path3.getFullPathName() +
                     " - Exists: " + (path3.existsAsFile() ? "YES" : "NO") +
                     "\n");

  // Logic implementation
  if (path1.existsAsFile())
    steveFile = path1;
  else if (path2.existsAsFile())
    steveFile = path2;
  else if (path3.existsAsFile())
    steveFile = path3;

  if (steveFile.existsAsFile()) {
    steveImage = juce::ImageFileFormat::loadFrom(steveFile);
    logFile.appendText("Selected Image: " + steveFile.getFullPathName() + "\n");
    logFile.appendText(
        "Image Valid: " +
        juce::String(steveImage.isNull() ? "NO (Null Image)" : "YES") + "\n");
    logFile.appendText(
        "Image Dimensions: " + juce::String(steveImage.getWidth()) + "x" +
        juce::String(steveImage.getHeight()) + "\n");
  } else {
    logFile.appendText("❌ FAILED TO FIND ANY IMAGE\n");
  }

  // Helper lambda for configuring sliders
  auto configureSlider = [&](juce::Slider &slider,
                             const juce::String &paramID) {
    slider.setLookAndFeel(&customLookAndFeel);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0,
                           0); // Hide text box
    addAndMakeVisible(slider);
  };

  // Helper lambda for attachments
  auto attachSlider = [&](auto &attachment, const juce::String &paramID,
                          juce::Slider &slider) {
    attachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, paramID, slider);
  };

  auto attachButton = [&](auto &attachment, const juce::String &paramID,
                          juce::ToggleButton &button) {
    attachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            audioProcessor.apvts, paramID, button);
  };

  auto configureEnableButton = [&](juce::ToggleButton &button,
                                   const juce::String &text) {
    button.setButtonText(text);
    button.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(button);
  };

  auto configureLabel = [&](juce::Label &label, const juce::String &text) {
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(
        juce::Label::textColourId,
        juce::Colour::fromFloatRGBA(0.5f, 0.3f, 0.1f, 1.0f)); // Dark brown
    label.setFont(
        juce::Font(20.0f, juce::Font::bold)); // Increased from 16pt to 20pt
    addAndMakeVisible(label);
  };

  // A. Saturation Globale
  configureSlider(saturationSlider, "drive");
  configureLabel(saturationLabel, "Saturation");
  saturationLabel.attachToComponent(&saturationSlider, false);

  configureSlider(shapeSlider, "shape");
  configureLabel(shapeLabel, "Shape");
  shapeLabel.attachToComponent(&shapeSlider, false);

  // Waveshape ComboBox
  waveshapeCombo.addItemList(
      {"Tube", "SoftClip", "HardClip", "Diode 1", "Diode 2", "Linear Fold",
       "Sin Fold", "Zero-Square", "Downsample", "Asym", "Rectify", "X-Shaper",
       "X-Shaper (Asym)", "Sine Shaper", "Stomp Box", "Tape Sat.", "Overdrive",
       "Soft Sat."},
      1);
  waveshapeCombo.setLookAndFeel(&customLookAndFeel);
  addAndMakeVisible(waveshapeCombo);
  configureLabel(waveshapeLabel, "WAVE");
  waveshapeLabel.attachToComponent(&waveshapeCombo, false);

  attachSlider(saturationAttachment, "drive", saturationSlider);
  attachSlider(shapeAttachment, "shape", shapeSlider);
  waveshapeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          audioProcessor.apvts, "waveshape", waveshapeCombo);

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
  // Enable resizing with constraints (min 650x425 = 50% of design, max
  // 2600x1700 = 200% of design)
  setSize(DESIGN_WIDTH, DESIGN_HEIGHT);
  setResizable(true, true);
  setResizeLimits(DESIGN_WIDTH / 2, DESIGN_HEIGHT / 2, DESIGN_WIDTH * 2,
                  DESIGN_HEIGHT * 2);

  // Lock aspect ratio to maintain proper proportions
  getConstrainer()->setFixedAspectRatio(static_cast<double>(DESIGN_WIDTH) /
                                        static_cast<double>(DESIGN_HEIGHT));

  // Start a timer to update value labels
  startTimer(100);
}

Vst_saturatorAudioProcessorEditor::~Vst_saturatorAudioProcessorEditor() {}

//==============================================================================
juce::Rectangle<int>
Vst_saturatorAudioProcessorEditor::scaleDesignBounds(int x, int y, int width,
                                                     int height) const {
  // Transform design coordinates to scaled window coordinates
  int scaledX = static_cast<int>(x * scaleFactor) + offsetX;
  int scaledY = static_cast<int>(y * scaleFactor) + offsetY;
  int scaledWidth = static_cast<int>(width * scaleFactor);
  int scaledHeight = static_cast<int>(height * scaleFactor);

  return juce::Rectangle<int>(scaledX, scaledY, scaledWidth, scaledHeight);
}

//==============================================================================
void Vst_saturatorAudioProcessorEditor::paint(juce::Graphics &g) {
  // Fill entire window with background (including letterbox areas)
  g.fillAll(juce::Colour::fromFloatRGBA(0.15f, 0.15f, 0.15f,
                                        1.0f)); // Dark grey for letterbox

  // Apply global transform for scaled content area
  juce::Graphics::ScopedSaveState saveState(g);
  g.addTransform(juce::AffineTransform::scale(scaleFactor)
                     .translated(static_cast<float>(offsetX),
                                 static_cast<float>(offsetY)));

  // Warm beige background in design space (1300x850)
  g.fillRect(0, 0, DESIGN_WIDTH, DESIGN_HEIGHT);
  g.setColour(juce::Colour::fromFloatRGBA(0.93f, 0.90f, 0.82f, 1.0f)); // Beige
  g.fillRect(0, 0, DESIGN_WIDTH, DESIGN_HEIGHT);

  // Draw steve image on the left side if loaded - full height with proper
  // aspect ratio
  if (!steveImage.isNull()) {
    juce::Rectangle<int> imageBounds(20, 20, 440, DESIGN_HEIGHT - 40);
    g.drawImageWithin(steveImage, imageBounds.getX(), imageBounds.getY(),
                      imageBounds.getWidth(), imageBounds.getHeight(),
                      juce::RectanglePlacement::centred, false);
  }

  // Build info in bottom right corner with dark text (in design space)
  g.setColour(juce::Colour::fromFloatRGBA(0.5f, 0.3f, 0.1f,
                                          0.95f)); // Dark brown - more opaque
  g.setFont(juce::Font(18.0f, juce::Font::bold));  // Larger, bold font
  juce::String versionText = "by NeiXXa / Version : " + buildHash;
  juce::Rectangle<int> versionBox(DESIGN_WIDTH - 350, DESIGN_HEIGHT - 32, 340,
                                  26);
  g.drawText(versionText, versionBox, juce::Justification::bottomRight, true);
}

void Vst_saturatorAudioProcessorEditor::resized() {
  // === GLOBAL SCALING CALCULATION ===
  // Calculate uniform scale factor to fit design size into window
  // while maintaining aspect ratio (letterboxing if needed)

  int windowWidth = getWidth();
  int windowHeight = getHeight();

  float scaleX =
      static_cast<float>(windowWidth) / static_cast<float>(DESIGN_WIDTH);
  float scaleY =
      static_cast<float>(windowHeight) / static_cast<float>(DESIGN_HEIGHT);

  // Use minimum scale to ensure entire UI fits (uniform scaling)
  scaleFactor = juce::jmin(scaleX, scaleY);

  // Clamp scale factor to reasonable bounds (50% to 200%)
  scaleFactor = juce::jlimit(0.5f, 2.0f, scaleFactor);

  // Calculate offsets to center the scaled UI
  int scaledWidth = static_cast<int>(DESIGN_WIDTH * scaleFactor);
  int scaledHeight = static_cast<int>(DESIGN_HEIGHT * scaleFactor);
  offsetX = (windowWidth - scaledWidth) / 2;
  offsetY = (windowHeight - scaledHeight) / 2;

  // === FIXED PIXEL LAYOUT IN DESIGN SPACE ===
  // All positions and sizes use design coordinates (1300x850)
  // Global scaling is applied via transform in paint()

  // === LAYOUT CONSTANTS (ALL IN DESIGN SPACE PIXELS) ===
  const int imageX = 20;
  const int imageY = 20;
  const int imageWidth = 440;
  const int imageHeight = 810; // 850 - 20 - 20

  const int contentStartX = imageX + imageWidth + 30;  // 490
  const int contentStartY = 100;                       // Header space
  const int contentWidth = 1300 - contentStartX - 30;  // ~780px
  const int contentHeight = 850 - contentStartY - 110; // ~640px

  // 4 columns with GENEROUS spacing
  const int columnCount = 4;
  const int columnSpacing = 50; // INCREASED from 30 to 50
  const int columnWidth = (contentWidth - (columnCount - 1) * columnSpacing) /
                          columnCount; // ~156px

  // Fixed knob dimensions - LARGER
  const int knobWidth = columnWidth - 10; // Account for margin
  const int knobHeight = 140;             // INCREASED from 120
  const int verticalSpacing = 45; // ULTRATHINK vertical spacing between knobs
  const int buttonHeight = 55;    // Button height (LOW/HIGH)
  const int footerButtonWidth = 180; // Slightly larger
  const int footerButtonHeight = 65;

  // === COLUMN POSITIONS (X coordinates) ===
  const int col1X = contentStartX;                       // GainIn
  const int col2X = col1X + columnWidth + columnSpacing; // Low
  const int col3X = col2X + columnWidth + columnSpacing; // High
  const int col4X = col3X + columnWidth + columnSpacing; // Master

  // === ROW POSITIONS (Y coordinates) - GENEROUS VERTICAL SPACING ===
  const int buttonRowY = contentStartY; // Row for LOW/HIGH buttons
  const int row1Y = contentStartY + 80; // More space after buttons
  const int row2Y = row1Y + knobHeight + verticalSpacing;
  const int row3Y = row2Y + knobHeight + verticalSpacing;
  const int row4Y = row3Y + knobHeight + verticalSpacing;

  // === FOOTER ===
  const int footerY = 850 - 100;

  // === APPLY SCALED BOUNDS TO ALL COMPONENTS ===
  // All coordinates are in design space (1300x850), transformed to window space

  // COLUMN 1: INPUT + OUTPUT
  inputGainSlider.setBounds(
      scaleDesignBounds(col1X + 5, row2Y, knobWidth, knobHeight));
  outputGainSlider.setBounds(
      scaleDesignBounds(col1X + 5, row3Y, knobWidth, knobHeight));

  // COLUMN 2: LOW BAND
  lowEnableButton.setBounds(scaleDesignBounds(col2X + (columnWidth - 110) / 2,
                                              buttonRowY, 110, buttonHeight));
  lowFreqSlider.setBounds(
      scaleDesignBounds(col2X + 5, row1Y, knobWidth, knobHeight));
  lowWarmthSlider.setBounds(
      scaleDesignBounds(col2X + 5, row2Y, knobWidth, knobHeight));
  lowLevelSlider.setBounds(
      scaleDesignBounds(col2X + 5, row3Y, knobWidth, knobHeight));

  // COLUMN 3: HIGH BAND
  highEnableButton.setBounds(scaleDesignBounds(col3X + (columnWidth - 110) / 2,
                                               buttonRowY, 110, buttonHeight));
  highFreqSlider.setBounds(
      scaleDesignBounds(col3X + 5, row1Y, knobWidth, knobHeight));
  highSoftnessSlider.setBounds(
      scaleDesignBounds(col3X + 5, row2Y, knobWidth, knobHeight));
  highLevelSlider.setBounds(
      scaleDesignBounds(col3X + 5, row3Y, knobWidth, knobHeight));

  // COLUMN 4: MASTER (Waveshape selector + 3 knobs - Output moved to column 1)
  waveshapeCombo.setBounds(
      scaleDesignBounds(col4X, buttonRowY + 10, columnWidth, 40));
  saturationSlider.setBounds(
      scaleDesignBounds(col4X + 5, row1Y, knobWidth, knobHeight));
  shapeSlider.setBounds(
      scaleDesignBounds(col4X + 5, row2Y, knobWidth, knobHeight));
  mixSlider.setBounds(
      scaleDesignBounds(col4X + 5, row3Y, knobWidth, knobHeight));

  // === FOOTER BUTTONS (centered) ===
  const int footerContentWidth = footerButtonWidth * 3 + columnSpacing;
  const int footerStartX = (DESIGN_WIDTH - footerContentWidth) / 2;

  bypassButton.setBounds(scaleDesignBounds(
      footerStartX, footerY, footerButtonWidth, footerButtonHeight));
  limiterButton.setBounds(
      scaleDesignBounds(footerStartX + footerButtonWidth + columnSpacing / 2,
                        footerY, footerButtonWidth, footerButtonHeight));
  prePostButton.setBounds(scaleDesignBounds(
      footerStartX + (footerButtonWidth + columnSpacing / 2) * 2, footerY,
      footerButtonWidth, footerButtonHeight));

  repaint();
}

void Vst_saturatorAudioProcessorEditor::timerCallback() {
  // Timer is used to trigger repaints to show updated knob values
  repaint();
}

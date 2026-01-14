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
    : AudioProcessorEditor(&p), audioProcessor(p), tooltipWindow(this, 1500) {
  // Ensure the tooltip window uses our custom LookAndFeel for drawing
  tooltipWindow.setLookAndFeel(&customLookAndFeel);

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

  // Load Steve Images
  steveImage = loadImage("steve.png");
  steve2Image = loadImage("steve2.png");

  if (!steveImage.isNull()) {
    // Log success if needed, or just relax.
    // Already logging in helper if we kept it, but let's keep it simple.
  }

  // Helper lambda for configuring sliders
  auto configureSlider = [&](juce::Slider &slider, const juce::String &paramID,
                             const juce::String &tooltip) {
    slider.setLookAndFeel(&customLookAndFeel);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    // Enable text box for double-click editing (custom layout places it in
    // center)
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    slider.setTooltip(tooltip);
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
                                   const juce::String &text,
                                   const juce::String &tooltip) {
    button.setButtonText(text);
    button.setLookAndFeel(&customLookAndFeel);
    button.setTooltip(tooltip);
    addAndMakeVisible(button);
  };

  // A. Saturation Globale
  // A. Saturation Globale
  configureSlider(
      saturationSlider, "drive",
      juce::CharPointer_UTF8("Contrôle la quantité de saturation globale"));

  configureSlider(shapeSlider, "shape",
                  juce::CharPointer_UTF8(
                      "Modifie la couleur et l'agressivité de la distorsion"));

  // Waveshape ComboBox
  waveshapeCombo.addItemList(
      {"Tube", "SoftClip", "HardClip", "Diode 1", "Diode 2", "Linear Fold",
       "Sin Fold", "Zero-Square", "Downsample", "Asym", "Rectify", "X-Shaper",
       "X-Shaper (Asym)", "Sine Shaper", "Stomp Box", "Tape Sat.", "Overdrive",
       "Soft Sat."},
      1);
  waveshapeCombo.setLookAndFeel(&customLookAndFeel);
  waveshapeCombo.setTooltip(
      juce::CharPointer_UTF8("Choisissez le type d'algorithme de saturation"));
  addAndMakeVisible(waveshapeCombo);

  attachSlider(saturationAttachment, "drive", saturationSlider);
  attachSlider(shapeAttachment, "shape", shapeSlider);
  waveshapeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          audioProcessor.apvts, "waveshape", waveshapeCombo);

  // B. Bande LOW
  configureEnableButton(
      lowEnableButton, "LOW",
      juce::CharPointer_UTF8(
          "Active/Désactive le traitement de la bande basse"));
  attachButton(lowEnableAttachment, "lowEnable", lowEnableButton);

  configureSlider(lowFreqSlider, "lowFreq",
                  juce::CharPointer_UTF8(
                      "Fréquence de coupure de la bande basse (Crossover)"));

  configureSlider(
      lowWarmthSlider, "lowWarmth",
      juce::CharPointer_UTF8(
          "Ajoute de la chaleur et du corps aux basses fréquences"));

  configureSlider(lowLevelSlider, "lowLevel",
                  juce::CharPointer_UTF8("Volume de sortie de la bande basse"));

  attachSlider(lowFreqAttachment, "lowFreq", lowFreqSlider);
  attachSlider(lowWarmthAttachment, "lowWarmth", lowWarmthSlider);
  attachSlider(lowLevelAttachment, "lowLevel", lowLevelSlider);

  // C. Bande HIGH
  configureEnableButton(
      highEnableButton, "HIGH",
      juce::CharPointer_UTF8(
          "Active/Désactive le traitement de la bande haute"));
  attachButton(highEnableAttachment, "highEnable", highEnableButton);

  configureSlider(highFreqSlider, "highFreq",
                  juce::CharPointer_UTF8(
                      "Fréquence de coupure de la bande haute (Crossover)"));

  configureSlider(
      highSoftnessSlider, "highSoftness",
      juce::CharPointer_UTF8("Adoucit les hautes fréquences (effet ''Tape'')"));

  configureSlider(highLevelSlider, "highLevel",
                  juce::CharPointer_UTF8("Volume de sortie de la bande haute"));

  attachSlider(highFreqAttachment, "highFreq", highFreqSlider);
  attachSlider(highSoftnessAttachment, "highSoftness", highSoftnessSlider);
  attachSlider(highLevelAttachment, "highLevel", highLevelSlider);

  // D. Gain & Routing
  configureSlider(inputGainSlider, "inputGain",
                  juce::CharPointer_UTF8("Gain d'entrée avant le traitement"));

  configureSlider(mixSlider, "mix",
                  juce::CharPointer_UTF8("Mélange Dry/Wet du signal global"));

  configureSlider(outputGainSlider, "output",
                  juce::CharPointer_UTF8("Gain de sortie final"));

  attachSlider(inputGainAttachment, "inputGain", inputGainSlider);
  attachSlider(mixAttachment, "mix", mixSlider);
  attachSlider(outputGainAttachment, "output", outputGainSlider);

  // Footer buttons with custom styling
  prePostButton.setButtonText("Pre/Post");
  prePostButton.setLookAndFeel(&customLookAndFeel);
  prePostButton.setTooltip(juce::CharPointer_UTF8(
      "Place le gain d'entrée avant ou après la saturation"));
  addAndMakeVisible(prePostButton);
  attachButton(prePostAttachment, "prePost", prePostButton);

  limiterButton.setButtonText("Limiter");
  limiterButton.setLookAndFeel(&customLookAndFeel);
  limiterButton.setTooltip(
      juce::CharPointer_UTF8("Active le limiteur de sécurité en sortie"));
  addAndMakeVisible(limiterButton);
  attachButton(limiterAttachment, "limiter", limiterButton);

  bypassButton.setButtonText("Bypass");
  bypassButton.setLookAndFeel(&customLookAndFeel);
  bypassButton.setTooltip(
      juce::CharPointer_UTF8("Désactive tout le traitement"));
  addAndMakeVisible(bypassButton);
  attachButton(bypassAttachment, "bypass", bypassButton);

  // E. Delta Monitoring
  deltaButton.setButtonText("DELTA");
  deltaButton.setLookAndFeel(&customLookAndFeel);
  deltaButton.setTooltip(
      juce::CharPointer_UTF8("Mode Delta: écouter uniquement les harmoniques "
                             "ajoutées par la saturation (wet - dry)"));
  addAndMakeVisible(deltaButton);
  attachButton(deltaAttachment, "delta", deltaButton);

  configureSlider(
      deltaGainSlider, "deltaGain",
      juce::CharPointer_UTF8(
          "Gain du signal Delta (réduction de niveau pour la sécurité audio)"));

  attachSlider(deltaGainAttachment, "deltaGain", deltaGainSlider);

  // F. Presets Menu (Top bar with navigation arrows)
  initializePresets(); // Load all 30 presets

  // Populate presets combo
  for (int i = 0; i < static_cast<int>(presets.size()); ++i) {
    presetsCombo.addItem(presets[static_cast<size_t>(i)].name, i + 1);
  }
  presetsCombo.setSelectedId(
      1, juce::dontSendNotification); // Start with first preset
  currentPresetIndex = 0;
  presetsCombo.setLookAndFeel(&customLookAndFeel);
  presetsCombo.setTooltip(juce::CharPointer_UTF8(
      "Sélectionnez un preset pour charger des réglages prédéfinis"));
  presetsCombo.onChange = [this]() {
    int selectedId = presetsCombo.getSelectedId();
    if (selectedId > 0) {
      currentPresetIndex = selectedId - 1;
      applyPreset(currentPresetIndex);
    }
  };
  addAndMakeVisible(presetsCombo);

  // Presets Label

  // Preset navigation buttons (arrows)
  auto configureNavButton = [&](juce::TextButton &btn,
                                const juce::String &tooltip) {
    btn.setLookAndFeel(&customLookAndFeel);
    btn.setTooltip(tooltip);
    btn.setColour(
        juce::TextButton::buttonColourId,
        juce::Colour::fromFloatRGBA(1.0f, 0.5f, 0.1f, 1.0f)); // Orange
    btn.setColour(
        juce::TextButton::textColourOffId,
        juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 1.0f)); // White text
    addAndMakeVisible(btn);
  };

  configureNavButton(presetLeftBtn, juce::CharPointer_UTF8("Preset précédent"));
  configureNavButton(presetRightBtn, juce::CharPointer_UTF8("Preset suivant"));
  presetLeftBtn.onClick = [this]() { navigatePreset(-1); };
  presetRightBtn.onClick = [this]() { navigatePreset(1); };

  // G. Waveshape navigation buttons
  configureNavButton(waveLeftBtn,
                     juce::CharPointer_UTF8("Waveform précédente"));
  configureNavButton(waveRightBtn, juce::CharPointer_UTF8("Waveform suivante"));
  waveLeftBtn.onClick = [this]() { navigateWaveshape(-1); };
  waveRightBtn.onClick = [this]() { navigateWaveshape(1); };

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

  // Start a timer to update value labels and visualization
  // 60 FPS ~ 16ms
  startTimer(16);

  // Configure Signature Link
  signatureLink.setButtonText("by NeiXXa / Version : " + buildHash);
  signatureLink.setURL(juce::URL("https://soundcloud.com/neixxatek"));
  signatureLink.setColour(juce::HyperlinkButton::textColourId,
                          juce::Colour::fromFloatRGBA(0.5f, 0.3f, 0.1f, 1.0f));
  signatureLink.setFont(
      customLookAndFeel.getCustomFont(24.0f, juce::Font::bold), false,
      juce::Justification::bottomRight);
  addAndMakeVisible(signatureLink);
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

  // === BACKGROUND WAVEFORM VISUALIZATION ===
  // Draw subtle waveform behind everything (but on top of beige)
  // Low contrast orange/brown
  // Draw filled waveform zone (light orange)
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.6f, 0.1f, 0.15f));
  g.fillPath(waveFillPath);

  // Optional: Brighter top edge line
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.8f, 0.3f, 0.5f));
  g.strokePath(wavePath,
               juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                    juce::PathStrokeType::rounded));

  // Optional: Fill below the curve for a more solid look
  // g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.6f, 0.1f, 0.05f));
  // juce::Path fillPath = wavePath;
  // fillPath.lineTo(DESIGN_WIDTH, DESIGN_HEIGHT / 2); // Close properly based
  // on logic but straight stroke is cleaner for "Saturn" style usually.

  // Draw steve image on the left side - full height with proper aspect ratio
  // Check if audio is playing (Threshold ~ -50dB)
  float currentLevel =
      audioProcessor.currentRMSLevel.load(std::memory_order_relaxed);
  bool isTalking = currentLevel > 0.005f;

  // === Draw Steve Image ===
  juce::Image *imgToDraw = &steveImage;

  // If talking and we have the second image, swap!
  if (isTalking && !steve2Image.isNull()) {
    imgToDraw = &steve2Image;
  }

  if (imgToDraw != nullptr && !imgToDraw->isNull()) {
    juce::Rectangle<int> imageBounds(20, 20, 440, DESIGN_HEIGHT - 40);
    // Draw at 100% opacity
    g.setOpacity(1.0f);
    g.drawImageWithin(*imgToDraw, imageBounds.getX(), imageBounds.getY(),
                      imageBounds.getWidth(), imageBounds.getHeight(),
                      juce::RectanglePlacement::centred, false);
  }

  // === SECOND WAVEFORM VISUALIZER (over Steve image, more opaque) ===
  // Draw a second instance of the waveform over the Steve image area
  // with higher opacity for better visibility
  // Draw filled waveform zone on Steve (more opaque)
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.6f, 0.1f, 0.35f));
  g.fillPath(waveFillPath);

  // Brighter top edge on Steve
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.8f, 0.3f, 0.7f));
  g.strokePath(wavePath,
               juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                    juce::PathStrokeType::rounded));

  // === DRAW ALL LABELS WITH CUSTOM FONT (like min/max values) ===
  // This works because we use g.setFont() directly in paint()
  // IMPORTANT: Don't use juce::Font::bold or it will replace the custom
  // typeface!
  g.setColour(
      juce::Colour::fromFloatRGBA(0.5f, 0.3f, 0.1f, 1.0f)); // Dark brown
  g.setFont(customLookAndFeel.getCustomFont(22.0f)); // NO STYLE - just size!

  // Same calculations as resized()
  const int imageX = 20;
  const int imageWidth = 440;
  const int contentStartX = imageX + imageWidth + 30; // 490
  const int contentStartY = 100;
  const int contentWidth = 1300 - contentStartX - 30;
  const int columnCount = 4;
  const int columnSpacing = 50;
  const int columnWidth =
      (contentWidth - (columnCount - 1) * columnSpacing) / columnCount;
  const int knobHeight = 140;
  const int verticalSpacing = 45;
  const int buttonHeight = 55;

  const int col1X = contentStartX;
  const int col2X = col1X + columnWidth + columnSpacing;
  const int col3X = col2X + columnWidth + columnSpacing;
  const int col4X = col3X + columnWidth + columnSpacing;

  const int buttonRowY = contentStartY;
  const int row1Y = contentStartY + 80;
  const int row2Y = row1Y + knobHeight + verticalSpacing;
  const int row3Y = row2Y + knobHeight + verticalSpacing;

  const int labelHeight = 28;
  const int labelMarginAbove = 5;
  const int topBarY = 45;
  const int navBtnWidth = 35;
  const int navSpacing = 5;
  const int comboWidth = 180;
  const int presetStartX = 480;
  const int waveStartX = 900;
  const int knobWidth = columnWidth - 10;

  const int footerY = 850 - 100;
  const int footerButtonWidth = 180;
  const int deltaKnobWidth = 110;

  // Top bar labels
  g.drawText("PRESETS", presetStartX, topBarY - labelHeight - 5,
             navBtnWidth * 2 + comboWidth + navSpacing * 2, labelHeight,
             juce::Justification::centred, true);
  g.drawText("WAVE", waveStartX + navBtnWidth + navSpacing,
             topBarY - labelHeight - 5, comboWidth, labelHeight,
             juce::Justification::centred, true);

  // Column 1: Input + Output
  g.drawText("Input", col1X + 5, row2Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);
  g.drawText("Output", col1X + 5, row3Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);

  // Column 2: LOW band
  g.drawText("Low Freq", col2X + 5, row1Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);
  g.drawText("Low Warmth", col2X + 5, row2Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);
  g.drawText("Low Level", col2X + 5, row3Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);

  // Column 3: HIGH band
  g.drawText("High Freq", col3X + 5, row1Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);
  g.drawText("High Softness", col3X + 5, row2Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);
  g.drawText("High Level", col3X + 5, row3Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);

  // Column 4: MASTER
  g.drawText("Saturation", col4X + 5, row1Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);
  g.drawText("Shape", col4X + 5, row2Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);
  g.drawText("Mix", col4X + 5, row3Y - labelHeight - labelMarginAbove,
             knobWidth, labelHeight, juce::Justification::centred, true);

  // Delta Gain label
  const int footerTotalWidth =
      footerButtonWidth * 4 + columnSpacing * 1.5 + deltaKnobWidth;
  const int footerStartX = (DESIGN_WIDTH - footerTotalWidth) / 2;
  g.drawText("Delta Gain",
             footerStartX + (footerButtonWidth + columnSpacing / 3) * 4 + 10,
             footerY - 10 - labelHeight - labelMarginAbove, deltaKnobWidth,
             labelHeight, juce::Justification::centred, true);

  // Build info is now a HyperlinkButton (signatureLink)
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
  // All positions and sizes use design coordinates (1300x850),
  // Global scaling is applied via transform in paint()

  const int imageX = 20;
  const int imageWidth = 440;

  const int contentStartX = imageX + imageWidth + 30; // 490
  const int contentStartY = 100;                      // Header space
  const int contentWidth = 1300 - contentStartX - 30; // ~780px

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

  // === FOOTER ===
  const int footerY = 850 - 100;

  // === APPLY SCALED BOUNDS TO ALL COMPONENTS ===
  // All coordinates are in design space (1300x850), transformed to window space

  // === TOP BAR: Presets (left) and Waveshape (right) ===
  const int topBarY = 45; // Moved down from 15 to 45 to avoid being cut off
  const int navBtnWidth = 35;
  const int navBtnHeight = 35;
  const int comboWidth = 180;
  const int comboHeight = 35;
  const int navSpacing = 5;

  // PRESETS section (left side of top bar, after Steve image area)
  const int presetStartX = 480;
  presetLeftBtn.setBounds(
      scaleDesignBounds(presetStartX, topBarY, navBtnWidth, navBtnHeight));
  presetsCombo.setBounds(
      scaleDesignBounds(presetStartX + navBtnWidth + navSpacing, topBarY,
                        comboWidth, comboHeight));
  presetRightBtn.setBounds(scaleDesignBounds(
      presetStartX + navBtnWidth + navSpacing + comboWidth + navSpacing,
      topBarY, navBtnWidth, navBtnHeight));

  // WAVESHAPE section (right side of top bar)
  const int waveStartX = 900;
  waveLeftBtn.setBounds(
      scaleDesignBounds(waveStartX, topBarY, navBtnWidth, navBtnHeight));
  waveshapeCombo.setBounds(scaleDesignBounds(
      waveStartX + navBtnWidth + navSpacing, topBarY, comboWidth, comboHeight));
  waveRightBtn.setBounds(scaleDesignBounds(
      waveStartX + navBtnWidth + navSpacing + comboWidth + navSpacing, topBarY,
      navBtnWidth, navBtnHeight));

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

  // COLUMN 4: MASTER (3 knobs - Waveshape moved to top bar)
  saturationSlider.setBounds(
      scaleDesignBounds(col4X + 5, row1Y, knobWidth, knobHeight));
  shapeSlider.setBounds(
      scaleDesignBounds(col4X + 5, row2Y, knobWidth, knobHeight));
  mixSlider.setBounds(
      scaleDesignBounds(col4X + 5, row3Y, knobWidth, knobHeight));

  // === FOOTER BUTTONS (centered with delta) ===
  // Layout: [Bypass] [Limiter] [Pre/Post] [DELTA] [Δ Gain knob]
  const int deltaKnobWidth = 110; // Slightly larger for "Delta Gain" text
  const int deltaKnobHeight = 80;
  const int footerTotalWidth =
      footerButtonWidth * 4 + columnSpacing * 1.5 + deltaKnobWidth;
  const int footerStartX = (DESIGN_WIDTH - footerTotalWidth) / 2;

  bypassButton.setBounds(scaleDesignBounds(
      footerStartX, footerY, footerButtonWidth, footerButtonHeight));
  limiterButton.setBounds(
      scaleDesignBounds(footerStartX + footerButtonWidth + columnSpacing / 3,
                        footerY, footerButtonWidth, footerButtonHeight));
  prePostButton.setBounds(scaleDesignBounds(
      footerStartX + (footerButtonWidth + columnSpacing / 3) * 2, footerY,
      footerButtonWidth, footerButtonHeight));
  deltaButton.setBounds(scaleDesignBounds(
      footerStartX + (footerButtonWidth + columnSpacing / 3) * 3, footerY,
      footerButtonWidth, footerButtonHeight));

  deltaGainSlider.setBounds(scaleDesignBounds(
      footerStartX + (footerButtonWidth + columnSpacing / 3) * 4 + 10,
      footerY - 10, // Slightly higher to account for label
      deltaKnobWidth, deltaKnobHeight));

  // Footer signature link
  signatureLink.setBounds(
      scaleDesignBounds(DESIGN_WIDTH - 350, DESIGN_HEIGHT - 32, 340, 26));

  repaint();
}

void Vst_saturatorAudioProcessorEditor::timerCallback() {
  // 1. Read Visualization Data
  // We recreate the path every frame.

  // Copy data from processor's circular buffer
  // We just take a snapshot of the buffer as is.
  // Ideally we would trigger/sync, but for background decoration, scrolling is
  // fine.

  // Resize local buffer if needed
  if (localWaveform.size() != Vst_saturatorAudioProcessor::visualizerBufferSize)
    localWaveform.resize(Vst_saturatorAudioProcessor::visualizerBufferSize);

  // Read blindly from the buffer (atomic read of index isn't strictly necessary
  // for a blurry background view, but let's try to get a contiguous block logic
  // if we want) The simplest way for a circular buffer visualizer is just to
  // map the whole buffer to the screen width. It will "scroll" if the write
  // pointer moves, which is what we want for an oscilloscope feel (or trigger).
  // WITHOUT trigger, it will just be a mess usually.
  // Simple "Trigger" logic: Find a zero crossing in the first 1/4 of buffer?
  // Actually, for "Saturn style" background, a slow rolling wave is pretty.
  // Let's just map the circular buffer starting from the current write index
  // (oldest data).

  int writeIndex =
      audioProcessor.visualizerWriteIndex.load(std::memory_order_relaxed);

  // Unroll the circular buffer into our local linear buffer
  for (int i = 0; i < Vst_saturatorAudioProcessor::visualizerBufferSize; ++i) {
    int index =
        (writeIndex + i) % Vst_saturatorAudioProcessor::visualizerBufferSize;
    localWaveform[(size_t)i] = audioProcessor.visualizerBuffer[(size_t)index];
  }

  // 2. Build Paths
  wavePath.clear();
  waveFillPath.clear();

  // Map 512 samples to DESIGN_WIDTH (1300px)
  // Center Y is DESIGN_HEIGHT / 2 (~425)
  // Height amplitude: +/- 150px

  float startX = 0.0f;
  float endX = (float)DESIGN_WIDTH;
  float centerY = (float)DESIGN_HEIGHT * 0.5f;
  float amplitudeScale = 220.0f; // Slightly more dramatic

  // We skip some samples to smoothen or just draw all points
  int numSamples = (int)localWaveform.size();
  float xStep = (endX - startX) / (float)(numSamples - 1);

  // Start top line path
  float startY = centerY - localWaveform[0] * amplitudeScale;
  wavePath.startNewSubPath(startX, startY);
  waveFillPath.startNewSubPath(startX, startY);

  for (int i = 1; i < numSamples; ++i) {
    float val = localWaveform[(size_t)i];
    float x = startX + (float)i * xStep;
    float y = centerY - val * amplitudeScale;
    wavePath.lineTo(x, y);
    waveFillPath.lineTo(x, y);
  }

  // Create the "zone" by closing the fill path until the bottom of the VST
  // top-right -> bottom-right -> bottom-left -> top-left
  waveFillPath.lineTo(endX, (float)DESIGN_HEIGHT);
  waveFillPath.lineTo(startX, (float)DESIGN_HEIGHT);
  waveFillPath.closeSubPath();

  // Timer is used to trigger repaints
  repaint();
}

//==============================================================================
// PRESETS SYSTEM
//==============================================================================

void Vst_saturatorAudioProcessorEditor::initializePresets() {
  // Clear any existing presets
  presets.clear();

  // ============ CLASSICS (1-6) ============
  presets.push_back({"Warm Tape", 16, 6.0f, 0.4f, 0.0f, 75.0f, 0.0f, true,
                     120.0f, 0.5f, 2.0f, true, 4000.0f, 0.6f, 1.0f, true,
                     false});
  presets.push_back({"Tube Glow", 1, 8.0f, 0.3f, 0.0f, 80.0f, -1.0f, true,
                     100.0f, 0.6f, 3.0f, true, 6000.0f, 0.4f, 2.0f, true,
                     false});
  presets.push_back({"Soft Clip", 2, 5.0f, 0.5f, 0.0f, 70.0f, 0.0f, false,
                     80.0f, 0.3f, 0.0f, false, 3000.0f, 0.5f, 0.0f, false,
                     false});
  presets.push_back({"Vintage Console", 1, 4.0f, 0.2f, 2.0f, 60.0f, -2.0f, true,
                     150.0f, 0.4f, 1.5f, true, 8000.0f, 0.7f, 1.0f, true,
                     true});
  presets.push_back({"Analog Warmth", 18, 3.0f, 0.3f, 0.0f, 50.0f, 0.0f, true,
                     100.0f, 0.5f, 2.0f, true, 5000.0f, 0.5f, 1.5f, false,
                     false});
  presets.push_back({"Classic Overdrive", 17, 10.0f, 0.4f, 0.0f, 85.0f, -2.0f,
                     true, 80.0f, 0.3f, 1.0f, true, 4500.0f, 0.3f, 1.5f, true,
                     false});

  // ============ MUSIC STYLES (7-12) ============
  presets.push_back({"Hip-Hop Low End", 1, 7.0f, 0.2f, 3.0f, 65.0f, 0.0f, true,
                     200.0f, 0.8f, 4.0f, false, 2000.0f, 0.5f, 0.0f, true,
                     false});
  presets.push_back({"EDM Punch", 3, 12.0f, 0.6f, 2.0f, 90.0f, -3.0f, true,
                     100.0f, 0.4f, 3.0f, true, 6000.0f, 0.2f, 2.5f, true,
                     true});
  presets.push_back({"Rock Crunch", 17, 14.0f, 0.5f, 0.0f, 100.0f, -4.0f, true,
                     150.0f, 0.3f, 2.0f, true, 5000.0f, 0.3f, 2.0f, true,
                     false});
  presets.push_back({"Jazz Warmth", 1, 3.0f, 0.2f, 0.0f, 40.0f, 0.0f, true,
                     80.0f, 0.6f, 1.5f, true, 7000.0f, 0.8f, 0.5f, false,
                     false});
  presets.push_back({"Lo-Fi Beats", 9, 8.0f, 0.7f, -2.0f, 70.0f, 0.0f, true,
                     300.0f, 0.6f, 2.0f, true, 3000.0f, 0.9f, -1.0f, false,
                     true});
  presets.push_back({"Metal Aggression", 3, 20.0f, 0.8f, 4.0f, 100.0f, -6.0f,
                     true, 120.0f, 0.2f, 3.0f, true, 4000.0f, 0.1f, 3.5f, true,
                     true});

  // ============ INSTRUMENTS (13-20) ============
  presets.push_back({"Bass Growl", 4, 9.0f, 0.4f, 2.0f, 80.0f, -1.0f, true,
                     250.0f, 0.7f, 4.0f, false, 1500.0f, 0.6f, 0.0f, true,
                     false});
  presets.push_back({"Vocal Warmth", 18, 4.0f, 0.3f, 0.0f, 45.0f, 1.0f, false,
                     100.0f, 0.4f, 0.0f, true, 8000.0f, 0.7f, 1.0f, false,
                     true});
  presets.push_back({"Drums Punch", 2, 8.0f, 0.5f, 3.0f, 75.0f, -2.0f, true,
                     80.0f, 0.3f, 2.5f, true, 6000.0f, 0.4f, 2.0f, true, true});
  presets.push_back({"Guitar Amp", 15, 11.0f, 0.6f, 0.0f, 90.0f, -3.0f, true,
                     100.0f, 0.4f, 1.5f, true, 5000.0f, 0.3f, 2.5f, true,
                     false});
  presets.push_back({"Synth Edge", 12, 7.0f, 0.7f, 1.0f, 70.0f, 0.0f, true,
                     60.0f, 0.2f, 1.0f, true, 7000.0f, 0.3f, 3.0f, false,
                     true});
  presets.push_back({"Piano Glue", 1, 2.5f, 0.2f, 0.0f, 35.0f, 0.0f, true,
                     100.0f, 0.4f, 1.0f, true, 6000.0f, 0.6f, 0.5f, false,
                     false});
  presets.push_back({"Strings Silk", 16, 3.0f, 0.3f, 0.0f, 40.0f, 0.5f, false,
                     150.0f, 0.5f, 0.0f, true, 8000.0f, 0.8f, 1.0f, false,
                     false});
  presets.push_back({"Horns Presence", 1, 5.0f, 0.4f, 1.0f, 55.0f, 0.0f, true,
                     200.0f, 0.3f, 1.5f, true, 5000.0f, 0.4f, 2.0f, false,
                     true});

  // ============ CREATIVE / FX (21-26) ============
  presets.push_back({"Bitcrushed", 9, 15.0f, 0.9f, 0.0f, 80.0f, -4.0f, false,
                     100.0f, 0.5f, 0.0f, false, 4000.0f, 0.5f, 0.0f, true,
                     true});
  presets.push_back({"Fuzz Box", 4, 18.0f, 0.7f, 3.0f, 95.0f, -5.0f, true,
                     80.0f, 0.5f, 2.0f, true, 3500.0f, 0.2f, 2.5f, true,
                     false});
  presets.push_back({"Wave Folder", 6, 10.0f, 0.6f, 0.0f, 85.0f, -3.0f, true,
                     100.0f, 0.3f, 1.0f, true, 5000.0f, 0.4f, 1.5f, true,
                     true});
  presets.push_back({"Sin Fold", 7, 12.0f, 0.8f, 0.0f, 75.0f, -4.0f, true,
                     120.0f, 0.4f, 1.5f, true, 6000.0f, 0.3f, 2.0f, true,
                     false});
  presets.push_back({"Rectifier", 11, 8.0f, 0.5f, 0.0f, 70.0f, -2.0f, true,
                     100.0f, 0.4f, 2.0f, true, 4500.0f, 0.5f, 1.5f, true,
                     true});
  presets.push_back({"Extreme Destroy", 3, 24.0f, 1.0f, 6.0f, 100.0f, -8.0f,
                     true, 50.0f, 0.2f, 4.0f, true, 3000.0f, 0.1f, 4.0f, true,
                     true});

  // ============ MASTERING / SUBTLE (27-30) ============
  presets.push_back({"Master Glue", 18, 2.0f, 0.15f, 0.0f, 25.0f, 0.0f, true,
                     80.0f, 0.4f, 0.5f, true, 10000.0f, 0.7f, 0.5f, true,
                     false});
  presets.push_back({"Parallel Crush", 2, 12.0f, 0.5f, 0.0f, 30.0f, 0.0f, true,
                     100.0f, 0.5f, 2.0f, true, 5000.0f, 0.4f, 1.5f, true,
                     true});
  presets.push_back({"Subtle Harmonics", 1, 1.5f, 0.1f, 0.0f, 20.0f, 0.5f, true,
                     100.0f, 0.3f, 0.5f, true, 8000.0f, 0.6f, 0.5f, false,
                     false});
  presets.push_back({"Bus Warmth", 16, 4.0f, 0.25f, 0.0f, 40.0f, -0.5f, true,
                     120.0f, 0.5f, 1.5f, true, 7000.0f, 0.6f, 1.0f, true,
                     false});
}

void Vst_saturatorAudioProcessorEditor::applyPreset(int presetIndex) {
  if (presetIndex < 0 || presetIndex >= static_cast<int>(presets.size()))
    return;

  const auto &p = presets[static_cast<size_t>(presetIndex)];

  // Apply all parameter values using the APVTS
  auto &apvts = audioProcessor.apvts;

  // Waveshape (ComboBox - 1-indexed)
  if (auto *param = apvts.getParameter("waveshape"))
    param->setValueNotifyingHost(
        param->convertTo0to1(static_cast<float>(p.waveshape)));

  // Global controls
  if (auto *param = apvts.getParameter("drive"))
    param->setValueNotifyingHost(param->convertTo0to1(p.drive));
  if (auto *param = apvts.getParameter("shape"))
    param->setValueNotifyingHost(param->convertTo0to1(p.shape));
  if (auto *param = apvts.getParameter("inputGain"))
    param->setValueNotifyingHost(param->convertTo0to1(p.inputGain));
  if (auto *param = apvts.getParameter("mix"))
    param->setValueNotifyingHost(param->convertTo0to1(p.mix));
  if (auto *param = apvts.getParameter("output"))
    param->setValueNotifyingHost(param->convertTo0to1(p.outputGain));

  // Low band
  if (auto *param = apvts.getParameter("lowEnable"))
    param->setValueNotifyingHost(p.lowEnable ? 1.0f : 0.0f);
  if (auto *param = apvts.getParameter("lowFreq"))
    param->setValueNotifyingHost(param->convertTo0to1(p.lowFreq));
  if (auto *param = apvts.getParameter("lowWarmth"))
    param->setValueNotifyingHost(param->convertTo0to1(p.lowWarmth));
  if (auto *param = apvts.getParameter("lowLevel"))
    param->setValueNotifyingHost(param->convertTo0to1(p.lowLevel));

  // High band
  if (auto *param = apvts.getParameter("highEnable"))
    param->setValueNotifyingHost(p.highEnable ? 1.0f : 0.0f);
  if (auto *param = apvts.getParameter("highFreq"))
    param->setValueNotifyingHost(param->convertTo0to1(p.highFreq));
  if (auto *param = apvts.getParameter("highSoftness"))
    param->setValueNotifyingHost(param->convertTo0to1(p.highSoftness));
  if (auto *param = apvts.getParameter("highLevel"))
    param->setValueNotifyingHost(param->convertTo0to1(p.highLevel));

  // Routing
  if (auto *param = apvts.getParameter("limiter"))
    param->setValueNotifyingHost(p.limiter ? 1.0f : 0.0f);
  if (auto *param = apvts.getParameter("prePost"))
    param->setValueNotifyingHost(p.prePost ? 1.0f : 0.0f);
}

void Vst_saturatorAudioProcessorEditor::navigatePreset(int direction) {
  int numPresets = static_cast<int>(presets.size());
  if (numPresets == 0)
    return;

  // Calculate new index with wrap-around
  currentPresetIndex += direction;
  if (currentPresetIndex < 0)
    currentPresetIndex = numPresets - 1;
  else if (currentPresetIndex >= numPresets)
    currentPresetIndex = 0;

  // Update combo and apply preset
  presetsCombo.setSelectedId(currentPresetIndex + 1,
                             juce::dontSendNotification);
  applyPreset(currentPresetIndex);
}

void Vst_saturatorAudioProcessorEditor::navigateWaveshape(int direction) {
  int currentWave = waveshapeCombo.getSelectedItemIndex();
  int numWaves = waveshapeCombo.getNumItems();
  if (numWaves == 0)
    return;

  // Calculate new index with wrap-around
  currentWave += direction;
  if (currentWave < 0)
    currentWave = numWaves - 1;
  else if (currentWave >= numWaves)
    currentWave = 0;

  // Update combo (this triggers onChange which updates the parameter)
  waveshapeCombo.setSelectedItemIndex(currentWave, juce::sendNotification);
}

//==============================================================================
juce::Image
Vst_saturatorAudioProcessorEditor::loadImage(const juce::String &imageName) {
  juce::File imageFile;
  auto appDir =
      juce::File::getSpecialLocation(juce::File::currentApplicationFile);

  // Path 1: Inside App Bundle (Standard macOS)
  juce::File path1 = appDir.getChildFile("Contents/Resources/" + imageName);

  // Path 2: Relative to Executable (Binary inside MacOS)
  juce::File path2 =
      appDir.getParentDirectory().getParentDirectory().getChildFile(
          "Resources/" + imageName);

  // Path 3: Dev path fallback
  juce::File path3 = juce::File(
      "/Users/vava/Documents/GitHub/vst_saturator/Assets/" + imageName);

  if (path1.existsAsFile())
    imageFile = path1;
  else if (path2.existsAsFile())
    imageFile = path2;
  else if (path3.existsAsFile())
    imageFile = path3;

  if (imageFile.existsAsFile())
    return juce::ImageFileFormat::loadFrom(imageFile);

  return juce::Image(); // Null image
}

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
#include "BinaryData.h"

CustomLookAndFeel::CustomLookAndFeel() {
  // Force load the custom font immediately
  DBG("CustomLookAndFeel constructor - loading font...");
  ensureFontLoaded();
}

void CustomLookAndFeel::ensureImageLoaded() {
  if (indicatorImage.isNull()) {
    juce::File imageFile;
    auto appDir =
        juce::File::getSpecialLocation(juce::File::currentApplicationFile);

    // Try multiple paths similar to PluginEditor logic
    juce::File path1 = appDir.getChildFile("Contents/Resources/indicator.png");
    juce::File path2 =
        appDir.getParentDirectory().getParentDirectory().getChildFile(
            "Resources/indicator.png");
    juce::File path3 = juce::File(
        "/Users/vava/Documents/GitHub/vst_saturator/Assets/indicator.png");

    if (path1.existsAsFile())
      imageFile = path1;
    else if (path2.existsAsFile())
      imageFile = path2;
    else if (path3.existsAsFile())
      imageFile = path3;

    if (imageFile.existsAsFile())
      indicatorImage = juce::ImageFileFormat::loadFrom(imageFile);
  }
}

void CustomLookAndFeel::ensureFontLoaded() {
  if (customTypeface == nullptr) {
    // Direct access to BinaryData is most reliable
    customTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::NanumPenScriptRegular_ttf,
        (size_t)BinaryData::NanumPenScriptRegular_ttfSize);

    if (customTypeface != nullptr) {
      return;
    }

    // Fallback: Try to load from filesystem if BinaryData fails (unlikely)
    juce::File fontFile =
        juce::File::getSpecialLocation(juce::File::currentApplicationFile)
            .getChildFile("Contents/Resources/NanumPenScript-Regular.ttf");

    if (!fontFile.existsAsFile()) {
      fontFile = juce::File("/Users/vava/Documents/GitHub/vst_saturator/"
                            "Assets/NanumPenScript-Regular.ttf");
    }

    if (fontFile.existsAsFile()) {
      juce::MemoryBlock mb;
      fontFile.loadFileAsData(mb);
      customTypeface =
          juce::Typeface::createSystemTypefaceFor(mb.getData(), mb.getSize());
    }
  }
}

juce::Font CustomLookAndFeel::getCustomFont(float height, int style) {
  ensureFontLoaded();
  if (customTypeface != nullptr) {
    juce::Font font(customTypeface);
    font = font.withHeight(height);
    // Note: Nanum Pen Script doesn't have a bold version.
    // If we force bold, JUCE might switch to a default font or synthesize it
    // poorly. We'll ignore the style and always use the custom typeface
    // properly.
    return font;
  }
  return juce::Font(height, style);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y,
                                         int width, int height,
                                         float sliderPosProportional,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider &slider) {
  // Check for hover and click states
  bool isHovered = slider.isMouseOverOrDragging();
  bool isPressed = slider.isMouseButtonDown();

  // Center and radius (Fixed size, no zoom)
  float centerX = (float)x + (float)width / 2.0f;
  float centerY = (float)y + (float)height / 2.0f;
  float baseRadius = juce::jmin(width, height) / 2.0f - 2.0f;
  float radius = baseRadius; // No zoom factor

  // Design Parameters
  const float trackWidth = 8.0f;
  const float mainRadius = radius * 0.9f;
  const float indicatorRadius = mainRadius - trackWidth * 1.5f;

  // Current Angle
  float angle = rotaryStartAngle +
                sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

  // === 1. DROP SHADOW (Soft depth) ===
  juce::Path knobBackground;
  knobBackground.addEllipse(centerX - mainRadius, centerY - mainRadius,
                            mainRadius * 2.0f, mainRadius * 2.0f);

  juce::DropShadow shadow(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 0.15f),
                          6, juce::Point<int>(0, 3));
  shadow.drawForPath(g, knobBackground);

  // === 2. KNOB BODY (Glassy/Modern Gradient) ===
  // Hover effect: Warm soft orange glow
  // Logic: Pressed -> Darker | Hover -> Lighter/Warmer | Default -> Standard
  juce::Colour startCol;
  juce::Colour endCol;

  if (isPressed) {
    // Much stronger visual feedback for pressed state
    // Darker, reddish brown for pressed state to be very obvious
    startCol = juce::Colour::fromFloatRGBA(0.85f, 0.75f, 0.65f, 1.0f);
    endCol = juce::Colour::fromFloatRGBA(0.80f, 0.65f, 0.55f, 1.0f);
  } else if (isHovered) {
    startCol = juce::Colour::fromFloatRGBA(1.0f, 0.96f, 0.92f, 1.0f); // Lighter
    endCol = juce::Colour::fromFloatRGBA(1.0f, 0.90f, 0.80f, 1.0f); // Peachtree
  } else {
    startCol =
        juce::Colour::fromFloatRGBA(0.98f, 0.96f, 0.92f, 1.0f); // Standard
    endCol = juce::Colour::fromFloatRGBA(0.92f, 0.88f, 0.82f, 1.0f);
  }

  juce::ColourGradient knobGradient(startCol, centerX, centerY - mainRadius,
                                    endCol, centerX, centerY + mainRadius,
                                    false);

  g.setGradientFill(knobGradient);
  g.fillEllipse(centerX - mainRadius, centerY - mainRadius, mainRadius * 2.0f,
                mainRadius * 2.0f);

  // Inner subtle highlight edge
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 0.6f));
  g.drawEllipse(centerX - mainRadius + 1.0f, centerY - mainRadius + 1.0f,
                (mainRadius - 1.0f) * 2.0f, (mainRadius - 1.0f) * 2.0f, 1.0f);

  // === 3. TRACK BACKGROUND (Subtle Groove) ===
  juce::Path trackPath;
  trackPath.addCentredArc(centerX, centerY, mainRadius - trackWidth * 0.5f,
                          mainRadius - trackWidth * 0.5f, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);

  g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f,
                                          0.05f)); // Very faint indent
  g.strokePath(trackPath,
               juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved,
                                    juce::PathStrokeType::rounded));

  // === 4. ACTIVE VALUE ARC (Orange Gradient) ===
  juce::Path valuePath;
  valuePath.addCentredArc(centerX, centerY, mainRadius - trackWidth * 0.5f,
                          mainRadius - trackWidth * 0.5f, 0.0f,
                          rotaryStartAngle, angle, true);

  // Dynamic gradient (Orange to Golden)
  juce::ColourGradient arcGradient(
      juce::Colour::fromFloatRGBA(1.0f, 0.6f, 0.1f, 1.0f), centerX - mainRadius,
      centerY + mainRadius,
      juce::Colour::fromFloatRGBA(1.0f, 0.45f, 0.0f, 1.0f),
      centerX + mainRadius, centerY - mainRadius, false);

  g.setGradientFill(arcGradient);
  g.strokePath(valuePath,
               juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved,
                                    juce::PathStrokeType::rounded));

  // === 5. INDICATOR DOT (Image) ===
  ensureImageLoaded(); // Lazy load if needed

  float dotX =
      centerX + (mainRadius - trackWidth * 0.5f) *
                    std::cos(angle - juce::MathConstants<float>::pi / 2.0f);
  float dotY =
      centerY + (mainRadius - trackWidth * 0.5f) *
                    std::sin(angle - juce::MathConstants<float>::pi / 2.0f);

  if (!indicatorImage.isNull()) {
    // Draw image ROTATED to match the knob angle
    // 1. Save state
    juce::Graphics::ScopedSaveState saveState(g);

    // 2. Translate to the dot position (center of the image)
    g.addTransform(juce::AffineTransform::translation(dotX, dotY));

    // 3. Rotate by the current angle
    // We explicitly lock the rotation to the knob's angle so the fish "swims"
    // around the circle
    g.addTransform(juce::AffineTransform::rotation(angle));

    // 4. Draw image centered at (0,0) in this new coordinate system
    float imgSize = 25.0f;
    // Note: We might need to adjust rotation offset if the source image isn't
    // facing "up". Assuming source is upright at 12 o'clock.
    g.drawImage(indicatorImage, -imgSize / 2, -imgSize / 2, imgSize, imgSize, 0,
                0, indicatorImage.getWidth(), indicatorImage.getHeight());
  } else {
    // Fallback to Code Drawing if image missing
    float dotSize = trackWidth * 1.8f;

    // Dot Shadow
    g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 0.2f));
    g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f + 1.0f, dotSize,
                  dotSize);

    // Dot Fill (Gold/Orange)
    juce::ColourGradient dotGradient(
        juce::Colour::fromFloatRGBA(1.0f, 0.8f, 0.2f, 1.0f),
        dotX - dotSize * 0.5f, dotY - dotSize * 0.5f,
        juce::Colour::fromFloatRGBA(0.9f, 0.5f, 0.0f, 1.0f),
        dotX + dotSize * 0.5f, dotY + dotSize * 0.5f, false);
    g.setGradientFill(dotGradient);
    g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f, dotSize,
                  dotSize);
  }

  // === 6. MIN / MAX TEXT (Small, light orange) ===
  g.setColour(juce::Colour::fromFloatRGBA(0.8f, 0.5f, 0.2f, 0.7f));
  g.setFont(getCustomFont(14.0f)); // Slightly larger for pen script

  juce::String minText =
      juce::String(slider.getMinimum(), 0); // No decimals usually
  juce::String maxText = juce::String(slider.getMaxValue(), 0);

  // If values are small float (like 0.0 to 1.0), show decimals
  if (slider.getMaximum() <= 10.0f) {
    minText = juce::String(slider.getMinimum(), 1);
    maxText = juce::String(slider.getMaximum(), 1);
  } else {
    minText = juce::String(slider.getMinimum(), 0);
    maxText = juce::String(slider.getMaximum(), 0);
  }

  // Position: Below the center value
  // We estimate the center value area is roughly 20px high in the middle.
  // So we put this at y + 15
  int labelY = centerY + 12;

  // Draw Min (Leftish) and Max (Rightish) or just single range string?
  // "on montre la valeur min et max" -> implies showing the range limits.
  // Design choice: Show "0" on left and "10" on right?
  // Or just min/max centered below? "0 - 10" ?
  // The request says "en minuscule sous le chiffre...".
  // Let's draw them at the ends of the arc openings or just below center?
  // User image shows simply the center value.
  // "sous le chiffre" -> Below the number.
  // Let's draw "Min  Max" spaced out, or just valid range.
  // Let's try: "0.00 ... 1.00" spaced slightly.

  // Actually, standard is to put min at start angle and max at end angle?
  // "sous le chiffre" suggests centrally aligned below the value.

  juce::String rangeText = minText + " / " + maxText;
  g.drawText(rangeText, centerX - 40, labelY, 80, 15,
             juce::Justification::centred, false);
}

// === CUSTOM LAYOUT (Centers the TextBox) ===
juce::Slider::SliderLayout
CustomLookAndFeel::getSliderLayout(juce::Slider &slider) {
  juce::Slider::SliderLayout layout;
  int size = juce::jmin(slider.getWidth(), slider.getHeight());

  // We want the text box to be in the center, large enough for the number.
  // 60x20 is usually enough for values.
  layout.textBoxBounds = juce::Rectangle<int>(0, 0, 70, 20)
                             .withCentre(slider.getLocalBounds().getCentre());
  layout.sliderBounds = slider.getLocalBounds();
  return layout;
}

// Helper class for hover and click effects on labels
class HoverLabel : public juce::Label {
public:
  HoverLabel() : juce::Label() {}

  void mouseEnter(const juce::MouseEvent &e) override {
    juce::Label::mouseEnter(e);
    updateColour(true, false);
  }

  void mouseExit(const juce::MouseEvent &e) override {
    juce::Label::mouseExit(e);
    updateColour(false, false);
  }

  void mouseDown(const juce::MouseEvent &e) override {
    juce::Label::mouseDown(e);
    updateColour(true, true);
  }

  void mouseUp(const juce::MouseEvent &e) override {
    juce::Label::mouseUp(e);
    updateColour(isMouseOver(), false);
  }

private:
  void updateColour(bool isHovered, bool isPressed) {
    if (isPressed) {
      // Stronger visual feedback: Darker pink/rose for click
      setColour(juce::Label::backgroundColourId,
                juce::Colour::fromFloatRGBA(0.9f, 0.50f, 0.60f, 0.8f));
    } else if (isHovered) {
      // Light pink background (Rose Clair)
      setColour(juce::Label::backgroundColourId,
                juce::Colour::fromFloatRGBA(1.0f, 0.75f, 0.85f, 0.5f));
    } else {
      setColour(juce::Label::backgroundColourId,
                juce::Colours::transparentBlack);
    }
  }
};

// === CUSTOM TEXT BOX (Transparent, formatted) ===
juce::Label *CustomLookAndFeel::createSliderTextBox(juce::Slider &slider) {
  auto *l = new HoverLabel();

  // Transparent background
  l->setColour(juce::Label::backgroundColourId,
               juce::Colours::transparentBlack);
  l->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
  l->setColour(juce::Label::outlineWhenEditingColourId,
               juce::Colours::transparentBlack);

  // Text Color (Dark Orange/Brown like before)
  l->setColour(juce::Label::textColourId,
               juce::Colour::fromFloatRGBA(0.5f, 0.25f, 0.05f, 1.0f));
  l->setColour(juce::Label::textWhenEditingColourId,
               juce::Colour::fromFloatRGBA(0.2f, 0.1f, 0.0f, 1.0f));

  // Font
  l->setFont(getCustomFont(22.0f, juce::Font::bold)); // Bold Pen Script
  l->setJustificationType(juce::Justification::centred);

  // Crucial: Edit on DOUBLE CLICK
  // This makes the label require a double click to enter edit mode.
  // Single click won't trigger edit (so dragging works better).
  l->setEditable(false, true, false);

  return l;
}

bool CustomLookAndFeel::hitTestRotarySlider(juce::Slider &slider, int x,
                                            int y) {
  // Always return true if within bounds (rectangular check by caller usually,
  // but specific to rotary) By returning true here whenever called, we ensure
  // the entire area assigned to the slider is hit-testable, not just the
  // circle.
  return true;
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics &g,
                                         juce::ToggleButton &button,
                                         bool isMouseOverButton,
                                         bool isButtonDown) {
  auto bounds = button.getLocalBounds().toFloat();
  float padding = 5.0f;
  auto bgBounds = bounds.reduced(padding);

  // Draw background and border based on toggle state
  if (button.getToggleState()) {
    // Active state: orange/golden glow
    g.setColour(
        juce::Colour::fromFloatRGBA(1.0f, 0.6f, 0.2f, 0.3f)); // Orange glow
    g.fillRoundedRectangle(bgBounds.expanded(2.0f), 6.0f);
    g.setColour(
        juce::Colour::fromFloatRGBA(1.0f, 0.5f, 0.1f, 0.95f)); // Bright orange
    g.drawRoundedRectangle(bgBounds, 6.0f, 2.0f);
  } else {
    // Inactive state: brown outline
    g.setColour(juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 0.4f));
    g.drawRoundedRectangle(bgBounds, 6.0f, 1.5f);
  }

  // Draw text
  g.setColour(
      button.getToggleState()
          ? juce::Colour::fromFloatRGBA(1.0f, 0.5f, 0.1f, 1.0f) // Bright orange
          : juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 0.7f)); // Dark brown
  g.setFont(getCustomFont(24.0f, juce::Font::bold)); // Larger for pen script
  g.drawText(button.getButtonText(), bounds, juce::Justification::centred,
             true);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics &g, int width, int height,
                                     bool isButtonDown, int buttonX,
                                     int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox &box) {
  auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width),
                                       static_cast<float>(height));

  // Background - warm beige
  g.setColour(juce::Colour::fromFloatRGBA(0.98f, 0.95f, 0.90f, 1.0f));
  g.fillRoundedRectangle(bounds, 6.0f);

  // Border - dark orange-brown
  g.setColour(juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 0.8f));
  g.drawRoundedRectangle(bounds, 6.0f, 2.0f);

  // Arrow button area
  auto arrowZone = juce::Rectangle<float>(
      static_cast<float>(buttonX), static_cast<float>(buttonY),
      static_cast<float>(buttonW), static_cast<float>(buttonH));

  // Draw arrow (down triangle)
  juce::Path arrow;
  float arrowSize = 8.0f;
  float centerX = arrowZone.getCentreX();
  float centerY = arrowZone.getCentreY();

  arrow.addTriangle(centerX - arrowSize * 0.5f, centerY - arrowSize * 0.3f,
                    centerX + arrowSize * 0.5f, centerY - arrowSize * 0.3f,
                    centerX, centerY + arrowSize * 0.3f);

  g.setColour(juce::Colour::fromFloatRGBA(0.6f, 0.35f, 0.1f, 0.9f));
  g.fillPath(arrow);
}

void CustomLookAndFeel::positionComboBoxText(juce::ComboBox &box,
                                             juce::Label &label) {
  label.setBounds(8, 1, box.getWidth() - 30, box.getHeight() - 2);
  label.setFont(getCustomFont(20.0f, juce::Font::bold)); // Combo text
  label.setColour(juce::Label::textColourId,
                  juce::Colour::fromFloatRGBA(0.5f, 0.3f, 0.1f, 1.0f));
}

juce::Rectangle<int>
CustomLookAndFeel::getTooltipBounds(const juce::String &tipText,
                                    juce::Point<int> screenPos,
                                    juce::Rectangle<int> parentArea) {
  const int padding = 6;
  juce::TextLayout layout;
  juce::AttributedString str(tipText);
  str.setFont(getCustomFont(18.0f)); // Use NanumPenScript font
  layout.createLayout(str, 300.0f);

  int w = (int)layout.getWidth() + padding * 2;
  int h = (int)layout.getHeight() + padding * 2;

  return juce::Rectangle<int>(screenPos.x - w / 2, screenPos.y + 16, w, h)
      .constrainedWithin(parentArea);
}

void CustomLookAndFeel::drawTooltip(juce::Graphics &g, const juce::String &text,
                                    int width, int height) {
  juce::Rectangle<int> bounds(width, height);

  // Background: Joli orange (Warm orange)
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.65f, 0.3f, 0.95f));
  g.fillRoundedRectangle(bounds.toFloat(), 5.0f);

  // Border: Slightly darker orange
  g.setColour(juce::Colour::fromFloatRGBA(0.9f, 0.5f, 0.1f, 1.0f));
  g.drawRoundedRectangle(bounds.toFloat(), 5.0f, 1.0f);

  // Text: Dark brown/black for contrast
  g.setColour(juce::Colour::fromFloatRGBA(0.2f, 0.1f, 0.0f, 1.0f));
  g.setFont(getCustomFont(18.0f)); // Tooltip text
  g.drawText(text, bounds.reduced(3), juce::Justification::centred, true);
}

// === CUSTOM TEXT BUTTON (Transparent background, Orange Bold Text) ===
void CustomLookAndFeel::drawButtonBackground(
    juce::Graphics &g, juce::Button &button,
    const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) {
  // No background, no border, purely transparent
}

void CustomLookAndFeel::drawButtonText(juce::Graphics &g,
                                       juce::TextButton &button,
                                       bool shouldDrawButtonAsHighlighted,
                                       bool shouldDrawButtonAsDown) {
  g.setFont(
      getCustomFont(30.0f, juce::Font::bold)); // Big bold arrows for Nanum

  // Orange color depending on state
  // Normal: Orange
  // Hover: Lighter Orange
  // Down: Darker Orange
  juce::Colour textCol;
  if (shouldDrawButtonAsDown)
    textCol = juce::Colour::fromFloatRGBA(0.8f, 0.4f, 0.0f, 1.0f); // Darker
  else if (shouldDrawButtonAsHighlighted)
    textCol = juce::Colour::fromFloatRGBA(1.0f, 0.7f, 0.2f, 1.0f); // Lighter
  else
    textCol =
        juce::Colour::fromFloatRGBA(1.0f, 0.55f, 0.1f, 1.0f); // Standard Orange

  g.setColour(textCol);

  // Draw text centered
  g.drawText(button.getButtonText(), button.getLocalBounds(),
             juce::Justification::centred, false);
}

// === OVERRIDE DEFAULT FONTS ===
juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox &) {
  return getCustomFont(20.0f, juce::Font::bold);
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label &label) {
  // Return custom font for all labels
  // Try to keep the size if already set, otherwise default to 20pt
  float currentHeight = label.getFont().getHeight();
  if (currentHeight < 12.0f)
    currentHeight = 20.0f; // Ensure minimum readable size
  return getCustomFont(currentHeight, juce::Font::bold);
}

juce::Font CustomLookAndFeel::getPopupMenuFont() {
  return getCustomFont(18.0f, juce::Font::plain);
}

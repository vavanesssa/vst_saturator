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

CustomLookAndFeel::CustomLookAndFeel() {}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y,
                                         int width, int height,
                                         float sliderPosProportional,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider &slider) {
  // Check for hover and click states
  bool isHovered = slider.isMouseOverOrDragging();
  bool isPressed = slider.isMouseButtonDown();

  // Basic dimensions
  float radius = juce::jmin(width, height) / 2.0f;
  float centerX = x + width * 0.5f;
  float centerY = y + height * 0.5f;

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
  juce::ColourGradient knobGradient(
      juce::Colour::fromFloatRGBA(0.98f, 0.96f, 0.92f, 1.0f), centerX,
      centerY - mainRadius,
      juce::Colour::fromFloatRGBA(0.92f, 0.88f, 0.82f, 1.0f), centerX,
      centerY + mainRadius, false);

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

  // === 5. INDICATOR DOT (3D Effect) ===
  float dotX =
      centerX + (mainRadius - trackWidth * 0.5f) *
                    std::cos(angle - juce::MathConstants<float>::pi / 2.0f);
  float dotY =
      centerY + (mainRadius - trackWidth * 0.5f) *
                    std::sin(angle - juce::MathConstants<float>::pi / 2.0f);
  float dotSize = trackWidth * 1.8f; // Slightly larger than track

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
  g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f, dotSize, dotSize);

  // === 6. MIN / MAX TEXT (Small, light orange) ===
  g.setColour(juce::Colour::fromFloatRGBA(0.8f, 0.5f, 0.2f, 0.7f));
  g.setFont(juce::Font(10.0f)); // Minuscule

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

// === CUSTOM TEXT BOX (Transparent, formatted) ===
juce::Label *CustomLookAndFeel::createSliderTextBox(juce::Slider &slider) {
  auto *l = new juce::Label();

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
  l->setFont(juce::Font(16.0f, juce::Font::bold));
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
  g.setFont(juce::Font(18.0f, juce::Font::bold)); // Increased from 16pt to 18pt
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
  label.setFont(juce::Font(14.0f, juce::Font::bold));
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
  str.setFont(juce::Font(13.0f));
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
  g.setFont(juce::Font(13.0f)); // Petit texte
  g.drawText(text, bounds.reduced(3), juce::Justification::centred, true);
}

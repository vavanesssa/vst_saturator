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

  // Ensure resources (image + font) are loaded
  void ensureImageLoaded();
  void ensureFontLoaded();

  // Helper to get consistent font
  juce::Font getCustomFont(float height, int style = juce::Font::plain);

  // Draw the rotary slider (knob) with minimal design
  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPosProportional, float rotaryStartAngle,
                        float rotaryEndAngle, juce::Slider &slider) override;

  // Custom Slider Layout to center the text box
  juce::Slider::SliderLayout getSliderLayout(juce::Slider &slider) override;

  // Custom Slider Text Box creation
  juce::Label *createSliderTextBox(juce::Slider &slider) override;

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

  juce::Font getTooltipFont() const;
  bool scrollTooltip(float deltaY);

  // Custom styling for PopupMenus (Lists)
  void drawPopupMenuBackground(juce::Graphics &g, int width,
                               int height) override;
  void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                         bool isSeparator, bool isActive, bool isHighlighted,
                         bool isChecked, bool hasSubMenu,
                         const juce::String &text,
                         const juce::String &shortcutKeyText,
                         const juce::Drawable *icon,
                         const juce::Colour *textColourToUse) override;

  // Section headers styling (orange separators for categories)
  void drawPopupMenuSectionHeader(juce::Graphics &g,
                                  const juce::Rectangle<int> &area,
                                  const juce::String &sectionName) override;

  // Item height control for scrollable menus
  void getIdealPopupMenuItemSize(const juce::String &text, bool isSeparator,
                                 int standardMenuItemHeight, int &idealWidth,
                                 int &idealHeight) override;

  // Menu flags and combo popup options
  int getMenuWindowFlags() override;
  juce::PopupMenu::Options
  getOptionsForComboBoxPopupMenu(juce::ComboBox &box,
                                 juce::Label &label) override;

  // Custom styling for TextButtons (Nav arrows)
  void drawButtonBackground(juce::Graphics &g, juce::Button &button,
                            const juce::Colour &backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;

  void drawButtonText(juce::Graphics &g, juce::TextButton &button,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

  // Override default fonts
  juce::Font getComboBoxFont(juce::ComboBox &) override;
  juce::Font getLabelFont(juce::Label &) override;
  juce::Font getPopupMenuFont() override;

private:
  static constexpr int tooltipMaxWidth = 300;
  static constexpr int tooltipMaxHeight = 500;
  static constexpr int tooltipPadding = 8;
  juce::String lastTooltipText;
  int lastTooltipContentHeight = 0;
  int lastTooltipVisibleHeight = 0;
  float tooltipScrollOffset = 0.0f;

private:
  juce::Image indicatorImage;
  juce::Typeface::Ptr customTypeface;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};

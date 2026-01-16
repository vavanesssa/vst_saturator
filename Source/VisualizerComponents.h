#pragma once

#include "VisualizerAnalysis.h"
#include <JuceHeader.h>
#include <array>
#include <deque>

enum class VisualizerMode { Waveform, Bars, Line, Heat, Harmonics };

struct VisualizerPanelState {
  VisualizerMode mode = VisualizerMode::Waveform;
  bool showPre = true;
  bool peakHold = false;
  int smoothingIndex = 2;
};

class VisualizerPanelComponent final : public juce::Component {
public:
  using ExpandCallback = std::function<void(int)>;

  VisualizerPanelComponent(int panelIndex, const juce::String &panelTitle,
                           juce::Colour panelTint);

  void setPanelState(const VisualizerPanelState &newState);
  VisualizerPanelState getPanelState() const;
  void setFrameData(const VisualizerFrameData &newFrame);
  void setExpanded(bool shouldExpand);
  void setExpandCallback(ExpandCallback callback);
  void setModeAvailability(const std::array<bool, 5> &availability);
  void setToggleVisibility(bool showPreToggle, bool showHoldToggle,
                           bool showSmoothingToggle);

  void paint(juce::Graphics &g) override;
  void resized() override;

private:
  void configureHeader();
  void updateExpandButton();
  void updateModeAvailability();
  void drawWaveform(juce::Graphics &g, juce::Rectangle<float> area,
                    const std::vector<float> &wave, juce::Colour colour) const;
  void drawSpectrumBars(juce::Graphics &g, juce::Rectangle<float> area,
                        const std::vector<float> &spectrum,
                        juce::Colour colour) const;
  void drawSpectrumLine(juce::Graphics &g, juce::Rectangle<float> area,
                        const std::vector<float> &spectrum,
                        juce::Colour colour) const;
  void drawHeatmap(juce::Graphics &g, juce::Rectangle<float> area);
  void drawCrestMeter(juce::Graphics &g, juce::Rectangle<float> area);
  void drawHarmonicBalance(juce::Graphics &g, juce::Rectangle<float> area);

  int panelIndex = 0;
  juce::String title;
  juce::Colour tint;
  juce::ComboBox modeSelector;
  juce::ToggleButton prePostToggle;
  juce::ToggleButton peakHoldToggle;
  juce::ComboBox smoothingSelector;
  juce::TextButton expandButton;
  VisualizerPanelState state;
  VisualizerFrameData frame;
  ExpandCallback expandCallback;
  bool isExpanded = false;
  std::array<bool, 5> modeAvailability{{true, true, true, true, true}};
  std::deque<std::vector<float>> heatHistory;
  int maxHeatHistory = 24;
};

class VisualizerGridComponent final : public juce::Component {
public:
  explicit VisualizerGridComponent(
      std::array<VisualizerPanelComponent *, 5> panelComponents);

  void setExpandedPanelIndex(int index);
  void resized() override;

private:
  std::array<VisualizerPanelComponent *, 5> panels;
  int expandedPanelIndex = -1;
};

class VisualizerTabComponent final : public juce::Component,
                                     private juce::Timer {
public:
  VisualizerTabComponent(AnalyzerTap &tapToUse, juce::ValueTree stateRoot);
  ~VisualizerTabComponent() override;

  void setActive(bool shouldBeActive);
  double getLastFrameTimeMs() const;
  double getRefreshIntervalMs() const;
  bool isActiveNow() const;
  void resized() override;
  void paint(juce::Graphics &g) override;

private:
  void timerCallback() override;
  void updateFrameRate(double frameTimeMs);
  void setExpandedPanel(int index);
  void restorePanelState();
  void storePanelState();
  VisualizerPanelComponent &panelForIndex(int index);
  const VisualizerPanelComponent &panelForIndex(int index) const;
  void configurePanelModes();

  AnalyzerTap &tap;
  VisualizerAnalysisEngine analysis;
  VisualizerFrameData frame;
  VisualizerPanelComponent deltaPanel;
  VisualizerPanelComponent shaperPanel;
  VisualizerPanelComponent dynamicsPanel;
  VisualizerPanelComponent balancePanel;
  VisualizerPanelComponent utilityPanel;
  std::array<VisualizerPanelComponent *, 5> panels;
  VisualizerGridComponent grid;
  int expandedPanelIndex = -1;
  bool isActive = false;
  juce::ValueTree stateTree;
  double lastFrameTimeMs = 0.0;
  double fpsTimerMs = 16.0;
  int stableHighFpsFrames = 0;
};

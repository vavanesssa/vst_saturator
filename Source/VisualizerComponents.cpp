#include "VisualizerComponents.h"
#include <cmath>

namespace {
constexpr int headerHeight = 34;
constexpr int headerPadding = 8;
constexpr int headerGap = 6;

juce::String modeToLabel(VisualizerMode mode) {
  switch (mode) {
  case VisualizerMode::Waveform:
    return "Waveform";
  case VisualizerMode::Bars:
    return "Bars";
  case VisualizerMode::Line:
    return "Line";
  case VisualizerMode::Heat:
    return "Heat";
  case VisualizerMode::Harmonics:
    return "Harmonics";
  }
  return "Waveform";
}

VisualizerMode labelToMode(const juce::String &label) {
  if (label == "Bars")
    return VisualizerMode::Bars;
  if (label == "Line")
    return VisualizerMode::Line;
  if (label == "Heat")
    return VisualizerMode::Heat;
  if (label == "Harmonics")
    return VisualizerMode::Harmonics;
  return VisualizerMode::Waveform;
}
} // namespace

VisualizerPanelComponent::VisualizerPanelComponent(
    int index, const juce::String &panelTitle, juce::Colour panelTint)
    : panelIndex(index), title(panelTitle), tint(panelTint), expandButton("⤢") {
  configureHeader();
}

void VisualizerPanelComponent::configureHeader() {
  modeSelector.addItem("Waveform", 1);
  modeSelector.addItem("Bars", 2);
  modeSelector.addItem("Line", 3);
  modeSelector.addItem("Heat", 4);
  modeSelector.addItem("Harmonics", 5);
  modeSelector.onChange = [this]() {
    state.mode = labelToMode(modeSelector.getText());
    repaint();
  };
  addAndMakeVisible(modeSelector);

  prePostToggle.setButtonText("Pre");
  prePostToggle.onClick = [this]() {
    state.showPre = prePostToggle.getToggleState();
    repaint();
  };
  addAndMakeVisible(prePostToggle);

  peakHoldToggle.setButtonText("Hold");
  peakHoldToggle.onClick = [this]() {
    state.peakHold = peakHoldToggle.getToggleState();
    repaint();
  };
  addAndMakeVisible(peakHoldToggle);

  smoothingSelector.addItem("Low", 1);
  smoothingSelector.addItem("Med", 2);
  smoothingSelector.addItem("High", 3);
  smoothingSelector.onChange = [this]() {
    state.smoothingIndex = smoothingSelector.getSelectedId();
    repaint();
  };
  addAndMakeVisible(smoothingSelector);

  expandButton.onClick = [this]() {
    if (expandCallback)
      expandCallback(panelIndex);
  };
  addAndMakeVisible(expandButton);

  modeSelector.setSelectedId(1, juce::dontSendNotification);
  smoothingSelector.setSelectedId(2, juce::dontSendNotification);
}

void VisualizerPanelComponent::setPanelState(
    const VisualizerPanelState &newState) {
  state = newState;
  modeSelector.setText(modeToLabel(state.mode), juce::dontSendNotification);
  prePostToggle.setToggleState(state.showPre, juce::dontSendNotification);
  peakHoldToggle.setToggleState(state.peakHold, juce::dontSendNotification);
  smoothingSelector.setSelectedId(state.smoothingIndex,
                                  juce::dontSendNotification);
}

VisualizerPanelState VisualizerPanelComponent::getPanelState() const {
  return state;
}

void VisualizerPanelComponent::setFrameData(
    const VisualizerFrameData &newFrame) {
  frame = newFrame;

  if (state.mode == VisualizerMode::Heat && frame.hasData) {
    const auto &spectrumSource =
        (panelIndex == 4) ? frame.postSpectrum : frame.deltaSpectrum;
    heatHistory.push_front(spectrumSource);
    if (static_cast<int>(heatHistory.size()) > maxHeatHistory)
      heatHistory.pop_back();
  }
}

void VisualizerPanelComponent::setExpanded(bool shouldExpand) {
  isExpanded = shouldExpand;
  updateExpandButton();
}

void VisualizerPanelComponent::setExpandCallback(ExpandCallback callback) {
  expandCallback = std::move(callback);
}

void VisualizerPanelComponent::setModeAvailability(
    const std::array<bool, 5> &availability) {
  modeAvailability = availability;
  updateModeAvailability();
}

void VisualizerPanelComponent::setToggleVisibility(bool showPreToggle,
                                                   bool showHoldToggle,
                                                   bool showSmoothingToggle) {
  prePostToggle.setVisible(showPreToggle);
  peakHoldToggle.setVisible(showHoldToggle);
  smoothingSelector.setVisible(showSmoothingToggle);
}

void VisualizerPanelComponent::updateExpandButton() {
  expandButton.setButtonText(isExpanded ? "⤡" : "⤢");
}

void VisualizerPanelComponent::updateModeAvailability() {
  for (int i = 0; i < 5; ++i) {
    modeSelector.setItemEnabled(i + 1,
                                modeAvailability[static_cast<size_t>(i)]);
  }

  if (!modeAvailability[static_cast<size_t>(state.mode)]) {
    for (int i = 0; i < 5; ++i) {
      if (modeAvailability[static_cast<size_t>(i)]) {
        state.mode = static_cast<VisualizerMode>(i);
        modeSelector.setText(modeToLabel(state.mode),
                             juce::dontSendNotification);
        break;
      }
    }
  }
}

void VisualizerPanelComponent::paint(juce::Graphics &g) {
  auto bounds = getLocalBounds().toFloat();
  g.setColour(tint);
  g.fillRoundedRectangle(bounds, 12.0f);

  g.setColour(tint.darker(0.08f));
  g.fillRoundedRectangle(bounds.removeFromTop(headerHeight), 12.0f);

  g.setColour(juce::Colour::fromFloatRGBA(0.35f, 0.2f, 0.1f, 0.8f));
  g.setFont(juce::Font(14.0f, juce::Font::bold));
  g.drawText(title, 12, 6, getWidth() - 24, headerHeight - 8,
             juce::Justification::centredLeft, true);

  auto contentArea = getLocalBounds().toFloat();
  contentArea.removeFromTop(headerHeight + headerGap);
  contentArea.reduce(10.0f, 10.0f);

  if (!frame.hasData) {
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.setFont(juce::Font(12.0f));
    g.drawText("Awaiting audio...", contentArea.toNearestInt(),
               juce::Justification::centred, true);
    return;
  }

  if (panelIndex == 1) {
    juce::Path transferCurve;
    auto curveArea = contentArea.reduced(10.0f);
    const float drive = 1.2f + static_cast<float>(state.smoothingIndex) * 0.4f;
    transferCurve.startNewSubPath(curveArea.getX(), curveArea.getBottom());
    for (int i = 0; i <= 64; ++i) {
      float norm = static_cast<float>(i) / 64.0f;
      float x = juce::jmap(norm, -1.0f, 1.0f);
      float y = std::tanh(x * drive);
      float drawX =
          juce::jmap(x, -1.0f, 1.0f, curveArea.getX(), curveArea.getRight());
      float drawY =
          juce::jmap(y, -1.0f, 1.0f, curveArea.getBottom(), curveArea.getY());
      transferCurve.lineTo(drawX, drawY);
    }
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.strokePath(transferCurve, juce::PathStrokeType(2.0f));

    if (state.mode == VisualizerMode::Waveform) {
      drawWaveform(g, contentArea, frame.postWaveform,
                   juce::Colours::white.withAlpha(0.7f));
      if (state.showPre) {
        drawWaveform(g, contentArea, frame.preWaveform,
                     juce::Colours::orange.withAlpha(0.5f));
      }
    } else if (state.mode == VisualizerMode::Line) {
      drawSpectrumLine(g, contentArea, frame.postSpectrum,
                       juce::Colours::white.withAlpha(0.8f));
    }
  } else if (panelIndex == 2) {
    drawWaveform(g, contentArea, frame.postWaveform,
                 juce::Colours::white.withAlpha(0.7f));
    drawCrestMeter(g, contentArea);
  } else if (panelIndex == 3) {
    if (state.mode == VisualizerMode::Bars) {
      drawSpectrumBars(g, contentArea, frame.postSpectrum,
                       juce::Colours::lightgreen.withAlpha(0.8f));
    } else if (state.mode == VisualizerMode::Line) {
      drawSpectrumLine(g, contentArea, frame.postSpectrum,
                       juce::Colours::white.withAlpha(0.8f));
    } else if (state.mode == VisualizerMode::Harmonics) {
      drawSpectrumBars(g, contentArea, frame.postSpectrum,
                       juce::Colours::gold.withAlpha(0.9f));
    }
    drawHarmonicBalance(g, contentArea);
  } else if (panelIndex == 4) {
    if (state.mode == VisualizerMode::Waveform) {
      drawWaveform(g, contentArea, frame.postWaveform,
                   juce::Colours::white.withAlpha(0.7f));
      if (state.showPre) {
        drawWaveform(g, contentArea, frame.preWaveform,
                     juce::Colours::orange.withAlpha(0.5f));
      }
    } else if (state.mode == VisualizerMode::Bars) {
      drawSpectrumBars(g, contentArea, frame.postSpectrum,
                       juce::Colours::lightblue.withAlpha(0.8f));
    } else if (state.mode == VisualizerMode::Line) {
      drawSpectrumLine(g, contentArea, frame.postSpectrum,
                       juce::Colours::white.withAlpha(0.8f));
    } else if (state.mode == VisualizerMode::Heat) {
      drawHeatmap(g, contentArea);
    }
  } else {
    switch (state.mode) {
    case VisualizerMode::Waveform:
      drawWaveform(g, contentArea, frame.deltaWaveform,
                   juce::Colours::orange.withAlpha(0.7f));
      if (state.showPre) {
        drawWaveform(g, contentArea, frame.preWaveform,
                     juce::Colours::white.withAlpha(0.4f));
      }
      break;
    case VisualizerMode::Bars:
      drawSpectrumBars(g, contentArea, frame.deltaSpectrum,
                       juce::Colours::orange.withAlpha(0.8f));
      break;
    case VisualizerMode::Line:
      drawSpectrumLine(g, contentArea, frame.deltaSpectrum,
                       juce::Colours::white.withAlpha(0.9f));
      break;
    case VisualizerMode::Heat:
      drawHeatmap(g, contentArea);
      break;
    case VisualizerMode::Harmonics:
      drawSpectrumBars(g, contentArea, frame.deltaSpectrum,
                       juce::Colours::gold.withAlpha(0.9f));
      break;
    }
  }
}

void VisualizerPanelComponent::resized() {
  auto header = getLocalBounds().removeFromTop(headerHeight);
  auto rightArea = header.removeFromRight(80);
  expandButton.setBounds(rightArea.removeFromRight(32).reduced(2));

  auto leftArea = header.removeFromLeft(header.getWidth() - 80);
  modeSelector.setBounds(leftArea.removeFromLeft(90).reduced(2));
  smoothingSelector.setBounds(leftArea.removeFromRight(70).reduced(2));
  prePostToggle.setBounds(leftArea.removeFromLeft(45).reduced(2));
  peakHoldToggle.setBounds(leftArea.removeFromLeft(48).reduced(2));
}

void VisualizerPanelComponent::drawWaveform(juce::Graphics &g,
                                            juce::Rectangle<float> area,
                                            const std::vector<float> &wave,
                                            juce::Colour colour) const {
  if (wave.empty())
    return;
  if (wave.size() < 2)
    return;

  juce::Path path;
  const float midY = area.getCentreY();
  const float xStep = area.getWidth() / static_cast<float>(wave.size() - 1);

  path.startNewSubPath(area.getX(),
                       midY - wave.front() * area.getHeight() * 0.4f);
  for (size_t i = 1; i < wave.size(); ++i) {
    float x = area.getX() + static_cast<float>(i) * xStep;
    float y = midY - wave[i] * area.getHeight() * 0.4f;
    path.lineTo(x, y);
  }

  g.setColour(colour);
  g.strokePath(path, juce::PathStrokeType(1.5f));
}

void VisualizerPanelComponent::drawSpectrumBars(
    juce::Graphics &g, juce::Rectangle<float> area,
    const std::vector<float> &spectrum, juce::Colour colour) const {
  if (spectrum.empty())
    return;
  if (spectrum.size() < 2)
    return;

  const int barCount = static_cast<int>(spectrum.size());
  const float barWidth = area.getWidth() / static_cast<float>(barCount);

  g.setColour(colour);
  for (int i = 0; i < barCount; ++i) {
    float magnitude =
        juce::jlimit(0.0f, 1.0f, spectrum[static_cast<size_t>(i)]);
    float barHeight = magnitude * area.getHeight();
    g.fillRect(area.getX() + i * barWidth, area.getBottom() - barHeight,
               barWidth * 0.8f, barHeight);
  }
}

void VisualizerPanelComponent::drawSpectrumLine(
    juce::Graphics &g, juce::Rectangle<float> area,
    const std::vector<float> &spectrum, juce::Colour colour) const {
  if (spectrum.empty())
    return;

  juce::Path path;
  const float xStep = area.getWidth() / static_cast<float>(spectrum.size() - 1);
  path.startNewSubPath(area.getX(),
                       area.getBottom() - spectrum.front() * area.getHeight());
  for (size_t i = 1; i < spectrum.size(); ++i) {
    float x = area.getX() + static_cast<float>(i) * xStep;
    float y = area.getBottom() - spectrum[i] * area.getHeight();
    path.lineTo(x, y);
  }

  g.setColour(colour);
  g.strokePath(path, juce::PathStrokeType(1.5f));
}

void VisualizerPanelComponent::drawHeatmap(juce::Graphics &g,
                                           juce::Rectangle<float> area) {
  if (heatHistory.empty())
    return;

  const float rowHeight =
      area.getHeight() / static_cast<float>(heatHistory.size());
  for (size_t row = 0; row < heatHistory.size(); ++row) {
    const auto &spectrum = heatHistory[row];
    const float y = area.getY() + static_cast<float>(row) * rowHeight;
    const float binWidth =
        area.getWidth() / static_cast<float>(spectrum.size());

    for (size_t i = 0; i < spectrum.size(); ++i) {
      float intensity = juce::jlimit(0.0f, 1.0f, spectrum[i]);
      g.setColour(juce::Colour::fromHSV(0.1f + intensity * 0.1f, 0.8f,
                                        intensity, 0.6f));
      g.fillRect(area.getX() + static_cast<float>(i) * binWidth, y, binWidth,
                 rowHeight);
    }
  }
}

void VisualizerPanelComponent::drawCrestMeter(juce::Graphics &g,
                                              juce::Rectangle<float> area) {
  auto meter = area.removeFromBottom(40.0f).reduced(4.0f);
  g.setColour(juce::Colours::white.withAlpha(0.2f));
  g.fillRoundedRectangle(meter, 6.0f);

  float crestNorm = juce::jlimit(0.0f, 1.0f, frame.crestChange / 4.0f + 0.5f);
  auto fill = meter.withWidth(meter.getWidth() * crestNorm);
  g.setColour(juce::Colours::orange.withAlpha(0.8f));
  g.fillRoundedRectangle(fill, 6.0f);

  g.setColour(juce::Colours::white.withAlpha(0.8f));
  g.setFont(juce::Font(11.0f, juce::Font::bold));
  g.drawText("Crest Δ " + juce::String(frame.crestChange, 2),
             meter.toNearestInt(), juce::Justification::centred, true);
}

void VisualizerPanelComponent::drawHarmonicBalance(
    juce::Graphics &g, juce::Rectangle<float> area) {
  auto meter = area.removeFromBottom(40.0f).reduced(4.0f);
  g.setColour(juce::Colours::white.withAlpha(0.2f));
  g.fillRoundedRectangle(meter, 6.0f);

  auto lowFill = meter.withWidth(meter.getWidth() * frame.lowHighBalance);
  g.setColour(juce::Colours::lightgreen.withAlpha(0.8f));
  g.fillRoundedRectangle(lowFill, 6.0f);
  g.setColour(juce::Colours::lightblue.withAlpha(0.6f));
  g.fillRoundedRectangle(meter.withTrimmedLeft(lowFill.getWidth())
                             .withWidth(meter.getWidth() - lowFill.getWidth()),
                         6.0f);

  g.setColour(juce::Colours::white.withAlpha(0.8f));
  g.setFont(juce::Font(11.0f, juce::Font::bold));
  g.drawText("Low/High " + juce::String(frame.lowHighBalance * 100.0f, 0) + "%",
             meter.toNearestInt(), juce::Justification::centred, true);
}

VisualizerGridComponent::VisualizerGridComponent(
    std::array<VisualizerPanelComponent *, 5> panelComponents)
    : panels(panelComponents) {
  for (auto *panel : panels) {
    if (panel != nullptr)
      addAndMakeVisible(panel);
  }
}

void VisualizerGridComponent::setExpandedPanelIndex(int index) {
  expandedPanelIndex = index;
  resized();
}

void VisualizerGridComponent::resized() {
  auto bounds = getLocalBounds(); // No padding - full area
  const int spacing = 4;          // Minimal spacing between panels

  if (expandedPanelIndex >= 0 && expandedPanelIndex < 5) {
    for (int i = 0; i < 5; ++i) {
      auto *panel = panels[static_cast<size_t>(i)];
      if (panel != nullptr)
        panel->setVisible(i == expandedPanelIndex);
    }

    panels[static_cast<size_t>(expandedPanelIndex)]->setBounds(bounds);
    return;
  }

  for (auto *panel : panels) {
    if (panel != nullptr)
      panel->setVisible(true);
  }

  const int minPanelWidth = 180;
  const int width = bounds.getWidth();
  bool singleRow = width >= (minPanelWidth * 5 + spacing * 4);

  if (singleRow) {
    const int panelWidth = (width - spacing * 4) / 5;
    auto row = bounds.removeFromTop(bounds.getHeight());
    for (int i = 0; i < 5; ++i) {
      auto panelBounds = row.removeFromLeft(panelWidth);
      if (i < 4)
        row.removeFromLeft(spacing);
      panels[static_cast<size_t>(i)]->setBounds(panelBounds);
    }
    return;
  }

  const int rowHeight = (bounds.getHeight() - spacing) / 2;
  auto row1 = bounds.removeFromTop(rowHeight);
  auto row2 = bounds;

  const int row1Width = (row1.getWidth() - spacing * 2) / 3;
  for (int i = 0; i < 3; ++i) {
    auto panelBounds = row1.removeFromLeft(row1Width);
    if (i < 2)
      row1.removeFromLeft(spacing);
    panels[static_cast<size_t>(i)]->setBounds(panelBounds);
  }

  const int row2Width = (row2.getWidth() - spacing) / 2;
  for (int i = 0; i < 2; ++i) {
    auto panelBounds = row2.removeFromLeft(row2Width);
    if (i < 1)
      row2.removeFromLeft(spacing);
    panels[static_cast<size_t>(i + 3)]->setBounds(panelBounds);
  }
}

VisualizerTabComponent::VisualizerTabComponent(AnalyzerTap &tapToUse,
                                               juce::ValueTree stateRoot)
    : tap(tapToUse), analysis(tapToUse),
      deltaPanel(0, "Delta / Harmonics",
                 juce::Colour::fromFloatRGBA(0.93f, 0.90f, 0.82f, 1.0f)),
      shaperPanel(1, "Wave Shaper",
                  juce::Colour::fromFloatRGBA(0.97f, 0.84f, 0.72f, 1.0f)),
      dynamicsPanel(2, "Dynamics / Crest",
                    juce::Colour::fromFloatRGBA(0.86f, 0.84f, 0.92f, 1.0f)),
      balancePanel(3, "Harmonic Balance",
                   juce::Colour::fromFloatRGBA(0.85f, 0.94f, 0.88f, 1.0f)),
      utilityPanel(4, "Utility Scope",
                   juce::Colour::fromFloatRGBA(0.82f, 0.88f, 0.95f, 1.0f)),
      panels({&deltaPanel, &shaperPanel, &dynamicsPanel, &balancePanel,
              &utilityPanel}),
      grid({&deltaPanel, &shaperPanel, &dynamicsPanel, &balancePanel,
            &utilityPanel}),
      stateTree(stateRoot) {
  addAndMakeVisible(grid);

  configurePanelModes();

  for (auto *panel : panels) {
    panel->setExpandCallback([this](int index) { setExpandedPanel(index); });
  }

  restorePanelState();
}

VisualizerTabComponent::~VisualizerTabComponent() { setActive(false); }

void VisualizerTabComponent::setActive(bool shouldBeActive) {
  if (isActive == shouldBeActive)
    return;

  isActive = shouldBeActive;
  tap.setEnabled(isActive);

  if (isActive)
    startTimer(static_cast<int>(fpsTimerMs));
  else
    stopTimer();
}

double VisualizerTabComponent::getLastFrameTimeMs() const {
  return lastFrameTimeMs;
}

double VisualizerTabComponent::getRefreshIntervalMs() const {
  return fpsTimerMs;
}

bool VisualizerTabComponent::isActiveNow() const { return isActive; }

void VisualizerTabComponent::resized() { grid.setBounds(getLocalBounds()); }

void VisualizerTabComponent::paint(juce::Graphics &g) {
  g.setColour(juce::Colours::transparentBlack);
}

void VisualizerTabComponent::timerCallback() {
  const double startTime = juce::Time::getMillisecondCounterHiRes();
  analysis.updateFrame(frame);

  for (auto *panel : panels) {
    panel->setFrameData(frame);
    panel->repaint();
  }

  const double endTime = juce::Time::getMillisecondCounterHiRes();
  updateFrameRate(endTime - startTime);
}

void VisualizerTabComponent::updateFrameRate(double frameTimeMs) {
  lastFrameTimeMs = frameTimeMs;

  if (frameTimeMs > 20.0) {
    fpsTimerMs = 33.0;
    stableHighFpsFrames = 0;
    startTimer(static_cast<int>(fpsTimerMs));
    return;
  }

  if (frameTimeMs < 10.0 && fpsTimerMs == 16.0) {
    stableHighFpsFrames++;
    if (stableHighFpsFrames > 120) {
      fpsTimerMs = 8.0;
      startTimer(static_cast<int>(fpsTimerMs));
    }
  } else if (frameTimeMs < 14.0 && fpsTimerMs == 33.0) {
    fpsTimerMs = 16.0;
    startTimer(static_cast<int>(fpsTimerMs));
  }
}

void VisualizerTabComponent::setExpandedPanel(int index) {
  if (expandedPanelIndex == index) {
    expandedPanelIndex = -1;
  } else {
    expandedPanelIndex = index;
  }

  grid.setExpandedPanelIndex(expandedPanelIndex);
  for (int i = 0; i < 5; ++i) {
    panelForIndex(i).setExpanded(i == expandedPanelIndex);
  }

  storePanelState();
}

void VisualizerTabComponent::restorePanelState() {
  auto visualizerState =
      stateTree.getOrCreateChildWithName("visualizers", nullptr);

  expandedPanelIndex = visualizerState.getProperty("expandedPanel", -1);

  for (int i = 0; i < 5; ++i) {
    auto panelStateTree = visualizerState.getOrCreateChildWithName(
        "panel" + juce::String(i), nullptr);
    VisualizerPanelState panelState;
    panelState.mode = static_cast<VisualizerMode>(
        static_cast<int>(panelStateTree.getProperty("mode", 0)));
    panelState.showPre = panelStateTree.getProperty("showPre", true);
    panelState.peakHold = panelStateTree.getProperty("peakHold", false);
    panelState.smoothingIndex =
        static_cast<int>(panelStateTree.getProperty("smoothing", 1));
    panelForIndex(i).setPanelState(panelState);
    panelForIndex(i).setExpanded(i == expandedPanelIndex);
  }

  grid.setExpandedPanelIndex(expandedPanelIndex);
}

void VisualizerTabComponent::storePanelState() {
  auto visualizerState =
      stateTree.getOrCreateChildWithName("visualizers", nullptr);
  visualizerState.setProperty("expandedPanel", expandedPanelIndex, nullptr);

  for (int i = 0; i < 5; ++i) {
    auto panelStateTree = visualizerState.getOrCreateChildWithName(
        "panel" + juce::String(i), nullptr);
    auto state = panelForIndex(i).getPanelState();
    panelStateTree.setProperty("mode", static_cast<int>(state.mode), nullptr);
    panelStateTree.setProperty("showPre", state.showPre, nullptr);
    panelStateTree.setProperty("peakHold", state.peakHold, nullptr);
    panelStateTree.setProperty("smoothing", state.smoothingIndex, nullptr);
  }
}

VisualizerPanelComponent &VisualizerTabComponent::panelForIndex(int index) {
  return *panels[static_cast<size_t>(index)];
}

const VisualizerPanelComponent &
VisualizerTabComponent::panelForIndex(int index) const {
  return *panels[static_cast<size_t>(index)];
}

void VisualizerTabComponent::configurePanelModes() {
  deltaPanel.setModeAvailability({true, true, true, true, true});
  shaperPanel.setModeAvailability({true, false, true, false, false});
  dynamicsPanel.setModeAvailability({true, false, false, false, false});
  balancePanel.setModeAvailability({false, true, true, false, true});
  utilityPanel.setModeAvailability({true, true, true, true, false});

  deltaPanel.setToggleVisibility(true, true, true);
  shaperPanel.setToggleVisibility(true, false, true);
  dynamicsPanel.setToggleVisibility(false, true, false);
  balancePanel.setToggleVisibility(false, false, false);
  utilityPanel.setToggleVisibility(true, false, true);
}

# 🎛️ PRD — steverator "Le Poisson Steve"

(VST Saturator, Living Document)

---

## 🎯 Project Vision & Intention

The `steverator` project is a VST3 audio plugin built with JUCE, demonstrating audio processing, UI design, and plugin architecture.
Themed around "Le Poisson Steve" (Steve the Fish), this plugin showcases:
*   How a VST functions within a DAW (e.g., Ableton).
*   How sound is processed in real-time with multi-band processing.
*   How the UI is connected to the audio engine via JUCE's parameter system.
*   How to structure an audio project cleanly.
*   How to iterate and refine without breaking existing functionality.

The project remains readable and modifiable at every stage.

---

## 🧠 Development Philosophy

This project is conceived as a long-term project, not a sprint.

**Key Principles:**
*   Code must be understandable before being performant.
*   Every file must explain its role.
*   Every important function must be commented.
*   Key variables must be easily tweakable.
*   The project must encourage experimentation (UI, parameters, DSP).

**This PRD is a living document:**
👉 It must be updated as implementation progresses, including:
*   What has been done.
*   What remains to be done.
*   Technical decisions made.
*   Problems encountered and their solutions.

---

## 🤖 Technical Guide & Architecture (AI Context)
*This section is designed to quickly orient AI on the project's structure.*

### 🏗 Global Architecture (Model-View-Controller)
The project uses **JUCE** and strictly separates audio processing from the graphical interface:

*   **MODEL (State)**: `AudioProcessorValueTreeState (apvts)`
    *   Located in `PluginProcessor`.
    *   Single source of truth for all parameters (Drive, Mix, Freqs...).
    *   Handles preset saving/loading (`getStateInformation`).

*   **CONTROLLER (DSP & Logic)**: `PluginProcessor`
    *   **File**: `Source/PluginProcessor.cpp`
    *   **Core**: `processBlock()` is the real-time audio loop. **Critical:** No memory allocation, no locks here.
    *   **Init**: `createParameterLayout()` defines all parameters at startup.

*   **VIEW (Interface)**: `PluginEditor`
    *   **File**: `Source/PluginEditor.cpp`
    *   **Role**: Receives a reference to `PluginProcessor` to link Sliders to Parameters via `Attachments`.
    *   **Rendering**: Uses `CustomLookAndFeel` for vector drawing (no PNGs for knobs).

### 📍 Critical Entry Points

| Action | Target File | Function / Zone |
| :--- | :--- | :--- |
| **Add a Parameter** | `PluginProcessor.cpp` | `createParameterLayout()` (ID & Range definition) |
| **Modify Sound** | `PluginProcessor.cpp` | `processBlock()` (DSP Algorithm) |
| **Modify UI Layout** | `PluginEditor.cpp` | `resized()` (FlexBox Positioning) |
| **Style Knobs** | `CustomLookAndFeel.cpp` | `drawRotarySlider()` |
| **Build & Test** | Root | `./build_and_deploy.sh` (Master Script) |

### ⚠️ Golden Rules
1.  **Real-Time**: In `processBlock`, **FORBIDDEN** to use `new`, `malloc`, `std::vector::push_back`, or blocking functions.
2.  **State Management**: Never store UI state separate from APVTS if it affects the sound.
3.  **Assets**: Images (e.g., mascot) are in `Assets/` and loaded in `PluginEditor`.

---

## 📦 Functional Description

`steverator` is a 3-band audio saturation effect with multi-frequency processing.

### Main Function

Apply soft saturation to the incoming audio signal with independent control of low, mid, and high frequencies to add warmth, density, and controlled harmonic distortion.

### Complete Parameter List (Current)

**Global Controls:**
*   **Waveshape**: Saturation algorithm selector (18 different waveshaping curves)
    - Tube, SoftClip, HardClip, Diode 1, Diode 2, Linear Fold, Sin Fold, Zero-Square
    - Downsample, Asym, Rectify, X-Shaper, X-Shaper (Asym), Sine Shaper, Stomp Box
    - Tape Sat., Overdrive, Soft Sat.
*   **Drive**: Pre-saturation gain (0 - 24 dB, controls overall saturation intensity)
*   **Shape**: Saturation curve modulation (0 - 1, modulates each waveshape differently)
*   **Input Gain**: Pre-processing gain (-24 to +24 dB)
*   **Mix**: Wet/dry blend (0 - 100%)
*   **Output Gain**: Post-processing gain (-24 to +24 dB)

**LOW Band (100 Hz - 1 kHz):**
*   **LOW Enable**: Toggle low-frequency band processing
*   **Low Freq**: Crossover frequency for low band (20 - 500 Hz)
*   **Low Warmth**: Saturation intensity for lows (0 - 1)
*   **Low Level**: Output level for low band (0 - 12 dB)

**HIGH Band (1 kHz - 20 kHz):**
*   **HIGH Enable**: Toggle high-frequency band processing
*   **High Freq**: Crossover frequency for high band (500 Hz - 16 kHz)
*   **High Softness**: Saturation softness for highs (0 - 1)
*   **High Level**: Output level for high band (0 - 12 dB)

**Routing & Effects:**
*   **Pre/Post**: Route saturation before/after EQ bands
*   **Limiter**: Soft limiter on output
*   **Bypass**: Full effect bypass

All parameters are:
*   Automatable in the DAW.
*   Visible and editable via the UI.
*   Clearly documented in the code.
*   Connected via JUCE's AudioProcessorValueTreeState (APVTS).

---

## 🧱 Target Tech Stack

*   **Language**: C++
*   **Framework**: JUCE
*   **Plugin Format**: VST3
*   **Platforms**:
    *   macOS Apple Silicon + Intel (Universal Binary)
    *   Windows x64

---

## 💻 Development Environment

The development is targeted for macOS (M3 Pro) compatibility, but the code is cross-platform.

The project structure must allow for:
*   Windows builds.
*   Future AU extension (optional).
*   DSP evolutions without a complete rewrite.

---

## 🎨 UI Design — "Le Poisson Steve"

The interface features a warm, inviting design inspired by the "Steve the Fish" theme:

**Current Design (January 2026):**
*   **Background**: Warm beige (RGB 0.93, 0.90, 0.82) reminiscent of vintage amplifier aesthetics
*   **Theme Image**: "Steve the Fish" mascot displayed on the left side (220px wide)
*   **Title**: "LE POISSON STEVE" in bold brown-orange (RGB 0.6, 0.35, 0.1)
*   **Knob Design**: All-procedural knobs drawn in C++ with:
    - Concentric circle design with golden-to-orange gradient arcs
    - Center value display (numeric, bold, dark brown text)
    - Smooth rotation indicator dot
    - No image dependencies
*   **Buttons**: Toggle buttons for LOW/HIGH with visual state feedback:
    - Active state: Bright orange glow with border
    - Inactive state: Subtle brown outline
    - Large, readable text (16pt bold)
*   **Color Palette**: Warm earth tones (beige, golden, orange, brown) for consistent aesthetic
*   **Typography**: Large, readable text throughout (14pt-32pt depending on element)
*   **Spacing**: Generous spacing (30px margins, 20px+ between elements) to avoid crowding

**Key UI Features:**
*   **Window Size**: 1100x700 pixels for comfortable viewing and interaction
*   **Left Panel**: Steve image + input gain control
*   **Center-Left**: LOW band controls (enable button, 3 parameters)
*   **Center-Right**: HIGH band controls (enable button, 3 parameters)
*   **Right Panel**: Master controls (drive, shape, mix, output)
*   **Footer**: Bypass, Limiter, Pre/Post buttons + version hash display
*   **Responsive Layout**: FlexBox-based layout that scales with window size

**Design Philosophy:**
- All UI elements are drawn procedurally (no PNG assets for UI elements)
- Colors are defined in code for easy tweaking
- Values are displayed directly on knobs for clarity
- Buttons show clear active/inactive states with color changes

---

## 🔊 DSP — Fundamental Rules

Audio processing must strictly respect real-time rules:
*   **No memory allocation** in the audio callback.
*   **No logging** in the audio thread.
*   **No mutex / locks**.
*   Processing must be clear, readable, and commented.

The saturation used initially is intentionally simple (e.g., `tanh`) to focus on understanding rather than sophistication.

---

## 🚀 Roadmap & Future Features

### 📚 User Documentation System
*   [ ] Implement a **User Manual Modal** accessible via a "?" button in the top-left corner.
*   [ ] Content: Explanation of all parameters (Drive, Waveshapes, Bands).
*   [ ] **Bilingual Support**: Toggle between **English** and **French**.
*   [ ] Implementation:
    *   New `DocumentationComponent` overlay.
    *   JSON or string-based localization system.

---
## 📁 File Architecture & Key Functions

```plaintext
steverator/
├── CMakeLists.txt              # 🛠 Build configuration (JUCE VST3 settings)
├── build_and_deploy.sh         # 🚀 MASTER SCRIPT: Builds, signs, deploys, and launches standalone
├── Source/
│   ├── PluginProcessor.h       # 📋 Header: Processor declaration
│   ├── PluginProcessor.cpp     # 🧠 BRAIN (Controller & DSP):
│   │   ├── createParameterLayout() # 🎚 Defines all 13 parameters (IDs, Ranges, Names)
│   │   ├── prepareToPlay()         # ⚙️ Init: Sample rate, buffer size setup
│   │   ├── processBlock()          # ⚡️ DSP LOOP: Real-time audio processing (CRITICAL: No allocs!)
│   │   │   └── [Saturator Logic]   #     -> 3-Band Split -> Saturation -> Mix -> Limiter
│   │   └── getStateInformation()   # 💾 Persistence: Saves/Loads parameters for presets
│   │
│   ├── PluginEditor.h          # 📋 Header: Editor declaration
│   ├── PluginEditor.cpp        # 🎨 FACE (View & UI):
│   │   ├── PluginEditor()          # 🏗 Constructor: Loads assets, inits attachments
│   │   ├── paint()                 # 🖌 Background drawing (Steve image, Titles)
│   │   └── resized()               # 📐 LAYOUT: FlexBox positioning (Responsive calculations)
│   │
│   ├── CustomLookAndFeel.h     # 📋 Header: Custom styling
│   └── CustomLookAndFeel.cpp   # 💅 STYLE (vector graphics):
│       ├── drawRotarySlider()      # 🎛 Procedural Knob: Golden/Orange gradients, value text
│       └── drawToggleButton()      # 🔘 Custom Buttons: Active/Inactive states with glow
│
└── Assets/
    └── steve.png               # 🐟 Mascot: "Steve the Fish" image (loaded in Editor)
```




## 📝 Code Documentation Standards

All code is documented with:
*   Comprehensive comments explaining the "why" not just the "what"
*   Clear function signatures with parameter documentation
*   Inline comments for non-obvious logic
*   Real-time safety annotations for audio callbacks

Each source file includes:
*   A header comment block explaining its role
*   High-level descriptions of major functions
*   References to related files for context

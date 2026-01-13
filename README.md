# 🎛️ PRD — steverator "Le Poisson Steve"

(Educational VST Saturator, Living Document)

---

## 🎯 Project Vision & Intention

The `steverator` project is an educational VST3 audio plugin built with JUCE, demonstrating audio processing, UI design, and plugin architecture.
Themed around "Le Poisson Steve" (Steve the Fish), this plugin showcases:
*   How a VST functions within a DAW (e.g., Ableton).
*   How sound is processed in real-time with multi-band processing.
*   How the UI is connected to the audio engine via JUCE's parameter system.
*   How to structure an audio project cleanly and educationally.
*   How to iterate and refine without breaking existing functionality.

The project remains readable, modifiable, and didactic at every stage.

---

## 🧠 Development Philosophy

This project is conceived as a long-term learning project, not a sprint.

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

## 📦 Functional Description

`steverator` is a 3-band audio saturation effect with multi-frequency processing.

### Main Function

Apply soft saturation to the incoming audio signal with independent control of low, mid, and high frequencies to add warmth, density, and controlled harmonic distortion.

### Complete Parameter List (Current)

**Global Controls:**
*   **Drive**: Pre-saturation gain (0 - 12 dB, controls overall saturation intensity)
*   **Shape**: Saturation curve shape (0 - 1, controls softness vs aggression)
*   **Input Gain**: Pre-processing gain (0 - 12 dB)
*   **Mix**: Wet/dry blend (0 - 100%)
*   **Output Gain**: Post-processing gain (0 - 12 dB)

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

## 🧪 Explicit Educational Objectives

By the end of v1, the project should allow you to understand:
*   How a DAW calls an audio plugin.
*   How audio buffers are processed.
*   How a UI parameter impacts the DSP.
*   How to structure a plugin properly.
*   How to test and debug a VST.

---

## 🗺️ Implementation Status

### ✅ Completed Phases

### Phase 0 — Preparation 🧱
*Goal: Environment ready and project compilable*
*   [x] Install necessary tools (CMake, C++ Compiler).
*   [x] Create JUCE project structure.
*   [x] Configure VST3 format.
*   [x] Compile an empty plugin.

### Phase 1 — Audio Pass-through 🔊
*Goal: Verify sound passes through the plugin without modification*
*   [x] Implement `processBlock`.
*   [x] Ensure sound is unchanged.
*   [x] Load plugin in DAW.
*   [x] Document audio flow.

### Phase 2 — Core Parameters 🎚️
*Goal: Control sound via parameters*
*   [x] Create global parameters (Drive, Shape, Input, Output, Mix).
*   [x] Implement AudioProcessorValueTreeState (APVTS).
*   [x] Link parameters to audio engine.
*   [x] Verify automation in DAW.
*   [x] Document parameter system.

### Phase 3 — Multi-Band Saturation 🔥
*Goal: Add 3-band saturation with independent control*
*   [x] Implement Linkwitz-Riley crossover filters (2/2/2 topology).
*   [x] Create LOW band parameters (freq, warmth, level, enable).
*   [x] Create HIGH band parameters (freq, softness, level, enable).
*   [x] Implement saturation function with tanh.
*   [x] Handle gain staging and mixing.
*   [x] Test at different frequencies and levels.
*   [x] Document DSP architecture.
*   [x] Implement Pre/Post routing toggle.
*   [x] Add soft limiter on output.

### Phase 4 — Advanced UI 🎨
*Goal: Functional, visually appealing interface with "Steve the Fish" theme*
*   [x] Create rotary sliders (knobs).
*   [x] Implement all 13 parameter controls.
*   [x] Link UI ↔ Parameters via APVTS attachments.
*   [x] Design and implement custom knob drawing (no image dependencies).
*   [x] Design and implement toggle buttons with visual state feedback.
*   [x] Integrate "Steve the Fish" mascot image on left side.
*   [x] Implement warm beige color scheme matching theme.
*   [x] Add value display directly on knobs.
*   [x] Optimize spacing and layout for readability.
*   [x] Implement FlexBox responsive layout.
*   [x] Add version hash display in bottom corner.

### Phase 5 — Polish & Documentation 🧪
*Goal: Stable, documented, and maintainable codebase*
*   [x] Test at different sample rates (44.1k, 48k, 96k).
*   [x] Test at different buffer sizes.
*   [x] Fix compilation warnings and errors.
*   [x] Code organization and cleanup.
*   [x] Update README with current implementation.
*   [x] Update AGENTS.md with developer guidelines.
*   [x] Document all parameters and controls.
*   [x] Implement automated build script with version hashing.

---

## 🔧 Development Workflow

### Quick Build & Deploy

For development, use the automated build script:

```bash
cd /Users/vava/Documents/GitHub/steverator
./build_and_deploy.sh
```

This script:
- Generates a **unique 5-character build hash** (e.g., `09FA1`)
- Compiles the plugin (Release mode) with the hash embedded
- Deploys to `/Library/Audio/Plug-Ins/VST3/steverator.vst3`
- Copies all theme assets (`Assets/steve.png`)
- Signs the plugin
- Clears all Ableton caches
- Launches the standalone version for quick testing

**Build Hash (Important for Testing):**

After running the script, you'll see:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✨ BUILD COMPLETE!

🔑 BUILD HASH: 09FA1
📅 TIMESTAMP: 2026-01-13 22:08:18

You should see '09FA1' prominently at the BOTTOM
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**The build hash appears in the bottom-right corner of the plugin UI.** This ensures you're testing the fresh version, not a cached one.

**After running the script:**
1. For **Standalone testing**: The script automatically launches the app
2. For **Ableton testing**:
   - Restart Ableton completely (Cmd+Q)
   - Reopen Ableton
   - Rescan plugins in Preferences → File/Folder → Rescan
   - Search for "steverator" in the plugin browser
   - Open the plugin and check the build hash in the bottom-right corner
   - It should match the hash shown in the terminal output

If you see an old hash or "DEV", your cache wasn't cleared properly. Run the script again.

### Theme Assets

Theme assets are stored in `/Assets/`:
- **steve.png** - "Steve the Fish" mascot image (displayed on left side of UI)

Assets are:
- Loaded at editor startup from `Assets/` or bundled resources
- Scaled to fit the left panel (220px wide)
- Cached for performance

To update the theme image:
1. Replace `Assets/steve.png` with new image
2. Run `./build_and_deploy.sh`
3. Restart Ableton and rescan plugins

---

## 📁 Source Code Files

### Core Audio Processing
- **`Source/PluginProcessor.h`** - Audio plugin interface and parameter declarations
- **`Source/PluginProcessor.cpp`** - Real-time audio DSP implementation
  - `createParameterLayout()` - Defines all 13 parameters
  - `processBlock()` - Main audio processing loop with multi-band saturation
  - 3-band Linkwitz-Riley crossover filter implementation
  - Soft tanh saturation with gain staging

### User Interface
- **`Source/PluginEditor.h`** - UI component declarations and layout
- **`Source/PluginEditor.cpp`** - UI rendering and event handling
  - Window layout and spacing (1100x700)
  - Parameter control positioning via FlexBox
  - "Steve the Fish" image loading and display
  - Build hash display in bottom-right corner
- **`Source/CustomLookAndFeel.h`** - Custom UI styling interface
- **`Source/CustomLookAndFeel.cpp`** - Procedural drawing for knobs and buttons
  - `drawRotarySlider()` - Golden/orange gradient knobs with value display
  - `drawToggleButton()` - LOW/HIGH buttons with active/inactive states

### Build Configuration
- **`CMakeLists.txt`** - JUCE VST3 build configuration
- **`build_and_deploy.sh`** - Automated build, sign, and deployment script

### Assets
- **`Assets/steve.png`** - "Steve the Fish" mascot (220px wide display)
- **`Assets/version.txt`** - Auto-generated build hash and timestamp

## 📝 Code Documentation Standards

All code is documented with:
*   Educational comments explaining the "why" not just the "what"
*   Clear function signatures with parameter documentation
*   Inline comments for non-obvious logic
*   Real-time safety annotations for audio callbacks

Each source file includes:
*   A header comment block explaining its role
*   High-level descriptions of major functions
*   References to related files for context

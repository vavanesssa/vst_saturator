# 🎛️ PRD — vst_saturator

(Educational VST Saturator, Living Document)

---

## 🎯 Project Vision & Intention

The `vst_saturator` project aims to create an extremely simple audio saturation plugin, primarily designed for learning VST development.
This plugin is not intended, in its first version, to be "perfect", optimized, or marketable. It serves as a controlled playground to understand:
*   How a VST functions within a DAW (e.g., Ableton).
*   How sound is processed in real-time.
*   How the UI is connected to the audio engine.
*   How to structure an audio project cleanly.
*   How to iterate without breaking existing functionality.

The project must remain readable, modifiable, and didactic at every stage.

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

`vst_saturator` is an audio effect (not an instrument).

### Main Function

Apply soft saturation to the incoming audio signal to add warmth, density, and light harmonic distortion.

### Initial Parameters (v1)

*   **Drive**: Controls the gain level applied before saturation.
*   **Output**: Controls the output level after saturation.

These parameters must be:
*   Automatable in the DAW.
*   Visible and editable via the UI.
*   Clearly documented in the code.

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

## 🗂️ Project Structure (Intentional)

The project structure must be simple, logical, and commented.

Each folder and file must:
*   Have a clear role.
*   Contain a header comment explaining its responsibility.
*   Avoid unexplained "magical values".

Important constants (parameter ranges, UI colors, DSP behaviors) must be centralized and easy to modify.

---

## 🎛️ UI — Intention & Creative Freedom

The graphical interface is intentionally:
*   Simple in structure.
*   Free in style.

The goal is for you to be able to:
*   Change colors.
*   Modify sizes.
*   Experiment with layouts.
*   Test different looks (clean, dirty, cyberpunk, etc.).

👉 The UI code must be written to be explorable, not opaque.

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

## 🗺️ Detailed Action Plan

### Phase 0 — Preparation 🧱
*Goal: Environment ready and project compilable*
*   [x] Install necessary tools (CMake, C++ Compiler).
*   [x] Create JUCE project structure.
*   [x] Configure VST3 format.
*   [x] Compile an empty plugin.

### Phase 1 — Audio Pass-through 🔊
*Goal: Verify sound passes through the plugin without modification*
*   [ ] Implement `processBlock`.
*   [ ] Ensure sound is unchanged.
*   [ ] Load plugin in DAW.
*   [ ] Document audio flow.

### Phase 2 — Parameters 🎚️
*Goal: Control sound via parameters*
*   [ ] Create Drive and Output parameters.
*   [ ] Link parameters to audio engine.
*   [ ] Verify automation.
*   [ ] Comment parameter system.

### Phase 3 — Saturation 🔥
*Goal: Add audible and controllable saturation*
*   [ ] Implement saturation function.
*   [ ] Handle gain staging.
*   [ ] Test at different levels.
*   [ ] Document DSP.

### Phase 4 — Minimal UI 🎨
*Goal: Functional and modifiable interface*
*   [ ] Create sliders.
*   [ ] Link UI ↔ Parameters.
*   [ ] Structure UI code.
*   [ ] Comment visual choices.

### Phase 5 — Stabilization 🧪
*Goal: Stable and understandable plugin*
*   [ ] Test at different sample rates.
*   [ ] Buffer size tests.
*   [ ] Code cleanup.
*   [ ] Full documentation update.

---

## 📝 Documentation & Comments (Mandatory)

All code must be:
*   Commented in an educational manner.
*   Readable without prior knowledge.
*   Structured for future review.

Each file must explain:
*   What it does.
*   Why it exists.
*   How it interacts with others.

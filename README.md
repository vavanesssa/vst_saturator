# 🎛️ Steverator — "Le Saturateur à Poisson"

![C++](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=c%2B%2B)
![JUCE](https://img.shields.io/badge/JUCE-Framework-8da033?style=for-the-badge&logo=juce)
![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Windows-lightgrey?style=for-the-badge&logo=apple)
![VST3](https://img.shields.io/badge/Format-VST3-orange?style=for-the-badge)
![Steve](https://img.shields.io/badge/Mascot-Steve_The_Fish-blueviolet?style=for-the-badge)

[![Download Artifacts](https://img.shields.io/badge/📥_Download-Latest_Build_(Artifacts)-success?style=for-the-badge&logo=github)](../../actions)
[![Releases](https://img.shields.io/badge/🚀_Download-Stable_Release-blue?style=for-the-badge&logo=github)](../../releases)

Bienvenue sur la documentation officielle du **Steverator**, un plugin de saturation audio unique en son genre, accompagné de Steve le poisson.

---

# 🇫🇷 MANUEL UTILISATEUR

### Introduction
Le Steverator est un processeur de saturation multi-bandes conçu pour apporter chaleur, caractère et grain à vos pistes audio. Que ce soit pour réchauffer une basse, donner du mordant à une voix ou salir une batterie, Steve est là pour vous aider.

### Interface & Réglages

L'interface est divisée en plusieurs sections intuitives :

#### 1. Entrée & Steve (Gauche)
*   **Input (Entrée)** : Contrôle le niveau du signal entrant dans le plugin (-24dB à +24dB). Utilisez-le pour attaquer les étages de saturation plus ou moins fort.
*   **Steve** : Notre mascotte réagit à la musique ! Il vous accompagne visuellement pendant le traitement. 
*   **Presets** : Une sélection de préréglages pour démarrer rapidement.

#### 2. Bandes de Fréquences (Centre)
Le Steverator divise le son en trois parties (Basses, Médiums, Hautes) pour un traitement précis :

*   **Low Freq & High Freq** : Ces knobs définissent les points de coupure (crossover).
    *   Tout ce qui est en dessous de *Low Freq* est traité par la bande **BASSE**.
    *   Tout ce qui est au-dessus de *High Freq* est traité par la bande **HAUTE**.
    *   Entre les deux se trouve la bande **MÉDIUM**.

*   **Low Warmth (Chaleur des Basses)** : Ajoute une saturation spécifique et ronde aux basses fréquences.
*   **High Softness (Douceur des Aigus)** : Une saturation plus douce pour éviter l'agressivité dans les aigus.
*   **Level (Low/High)** : Ajustez le volume de sortie de chaque bande pour rééquilibrer le spectre.

#### 3. Saturation Globale (Droite)
Le cœur du traitement :

*   **Wave (Waveshape)** : Choisissez le "type" de distorsion parmi une vaste collection (Tube, Tape, Diode, Fold, etc.). Chaque forme d'onde a sa propre couleur.
*   **Saturation** : Le gros bouton ! Il contrôle l'intensité générale de l'effet (Drive).
*   **Shape** : Modifie la texture de la distorsion. Tournez-le pour sculpter le caractère du son (plus tranchant ou plus rond).

#### 4. Section Master (Droite & Bas)
*   **Mix** : Mélange le son traité (Wet) avec le son d'origine (Dry). Parfait pour de la compression/saturation parallèle.
*   **Output (Sortie)** : Niveau de sortie final.
*   **Delta Monitor** : Un outil puissant qui vous permet d'écouter **uniquement** ce que le plugin ajoute (la distorsion). Très utile pour régler finement l'effet sans être distrait par le son original.
*   **Delta Gain** : Ajuste le volume de ce signal "Delta" pour le rendre plus audible ou éviter qu'il ne sature.
*   **Limiter** : Un limiteur de sécurité en fin de chaîne pour éviter les pics numériques désagréables.

---

# 🇬🇧 USER MANUAL

### Introduction
Steverator is a unique multi-band saturation plugin featuring Steve the Fish. It is designed to add warmth, character, and grit to your audio tracks. Whether you need to fatten up a bass, add bite to vocals, or dirt up drums, Steve is here to serve.

### Interface & Controls

The interface is divided into intuitive sections:

#### 1. Input & Steve (Left)
*   **Input**: Controls the incoming signal level (-24dB to +24dB). Use this to drive the saturation stages harder or softer.
*   **Steve**: Our mascot reacts to the music! He provides visual company during your mixing sessions.
*   **Presets**: A selection of factory presets to get you started quickly.

#### 2. Frequency Bands (Center)
Steverator splits the audio into three parts (Low, Mid, High) for precise processing:

*   **Low Freq & High Freq**: These knobs define the crossover points.
    *   Everything below *Low Freq* is treated by the **LOW** band.
    *   Everything above *High Freq* is treated by the **HIGH** band.
    *   In between lies the **MID** band.

*   **Low Warmth**: Adds specific, round saturation to low frequencies.
*   **High Softness**: Applies smoother saturation to highs to prevent harshness.
*   **Level (Low/High)**: Adjust the output volume of each band to rebalance the spectrum.

#### 3. Global Saturation (Right)
The core of the effect:

*   **Wave (Waveshape)**: Choose the "type" of distortion from a vast collection (Tube, Tape, Diode, Fold, etc.). Each algorithm has its own flavor.
*   **Saturation**: The big knob! Controls the overall intensity of the effect (Drive).
*   **Shape**: Modifies the texture of the distortion. Tweak this to sculpt the character of the sound (sharper or rounder).

#### 4. Master Section (Right & Bottom)
*   **Mix**: Blends the processed (Wet) sound with the original (Dry) sound. Perfect for parallel saturation.
*   **Output**: Final output level.
*   **Delta Monitor**: A powerful tool allowing you to listen **only** to what the plugin is adding (the distortion). Very useful for fine-tuning without being distracted by the dry signal.
*   **Delta Gain**: Adjusts the volume of this "Delta" signal.
*   **Limiter**: A safety limiter at the end of the chain to prevent nasty digital clipping.

---

# 🛠️ DEVELOPER DOCUMENTATION

*(AI Context & Technical Architectures)*

## 🎯 Project Vision
The `steverator` project is a VST3 audio plugin built with JUCE.
Themed around "Le Poisson Steve" (Steve the Fish), this plugin showcases how to structure a modern audio project cleanly using industry-standard practices.

## 🤖 Technical Guide & Architecture

### 🏗 Global Architecture (Model-View-Controller)
The project uses **JUCE** and strictly separates audio processing from the graphical interface:

*   **MODEL (State)**: `AudioProcessorValueTreeState (apvts)`
    *   Located in `PluginProcessor`.
    *   Single source of truth for all parameters.
    *   Handles preset saving/loading (`getStateInformation`).

*   **CONTROLLER (DSP & Logic)**: `PluginProcessor`
    *   **File**: `Source/PluginProcessor.cpp`
    *   **Core**: `processBlock()` is the real-time audio loop. **Critical:** No memory allocation, no locks here.
    *   **Init**: `createParameterLayout()` defines all parameters at startup.

*   **VIEW (Interface)**: `PluginEditor`
    *   **File**: `Source/PluginEditor.cpp`
    *   **Role**: Receives a reference to `PluginProcessor` to link Sliders to Parameters via `Attachments`.
    *   **Rendering**: Uses `CustomLookAndFeel` for vector drawing (procedural knobs, custom fonts).

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
3.  **Assets**: Images (e.g., mascot) and Fonts are in `Assets/` and loaded in `PluginEditor`/`CustomLookAndFeel`.

## 📦 Functional Scale (DSP)

`steverator` is a 3-band audio saturation effect with multi-frequency processing.

**Main Function:**
Apply soft saturation and wave-shaping to the incoming audio signal with independent control of low, mid, and high frequencies.

**Tech Stack:**
*   **Language**: C++
*   **Framework**: JUCE
*   **Plugin Format**: VST3
*   **Platforms**: macOS (Universal), Windows x64

## 📁 File Architecture

```plaintext
steverator/
├── CMakeLists.txt              # 🛠 Build configuration (JUCE VST3 settings)
├── build_and_deploy.sh         # 🚀 MASTER SCRIPT: Builds, signs, and launches standalone
├── Source/
│   ├── PluginProcessor.h/cpp   # 🧠 BRAIN (Controller & DSP) relative to AudioProcessor
│   ├── PluginEditor.h/cpp      # 🎨 FACE (View & UI) relative to AudioProcessorEditor
│   ├── CustomLookAndFeel.h/cpp # 💅 STYLE (vector graphics, fonts, colors)
│
└── Assets/
    ├── steve.png               # 🐟 Mascot image
    ├── indicator.png           # 🟠 Knob position indicator
    └── NanumPenScript-Regular.ttf # 📝 Custom Handwriting Font
```


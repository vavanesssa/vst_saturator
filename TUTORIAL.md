# 📘 VST Saturator — Build & Installation Guide

This guide explains how to set up your environment, build the `steverator` plugin, and install it on your machine.

---

## 🛠 Prerequisites

Before you begin, ensure you have the following installed:

### 1. CMake
CMake is the build system used to generate project files (VS Solution, Xcode Project, Makefiles).
*   **macOS**: `brew install cmake`
*   **Windows**: Download installer from [cmake.org](https://cmake.org/download/)

### 2. C++ Compiler
*   **macOS**: Xcode (install via App Store) + Command Line Tools (`xcode-select --install`).
*   **Windows**: Visual Studio 2022 (Community Edition is free). Ensure "Desktop development with C++" workload is selected.
*   **Linux**: `sudo apt install build-essential` (GCC/Clang).
    *   *Note: JUCE on Linux requires additional libraries:*
        ```bash
        sudo apt install libasound2-dev libjack-jackd2-dev \
            ladspa-sdk libcurl4-openssl-dev libfreetype6-dev \
            libx11-dev libxcomposite-dev libxcursor-dev \
            libxext-dev libxinerama-dev libxrandr-dev \
            libxrender-dev libwebkit2gtk-4.0-dev \
            libglu1-mesa-dev mesa-common-dev
        ```

### 3. Git
*   Required to clone the repository and the JUCE submodule.

---

## 🚀 1. Cloning the Repository

Since this project uses JUCE as a submodule, you must clone it recursively or initialize the submodule after cloning.

**Option A: Clone with submodules (Recommended)**
```bash
git clone --recursive https://github.com/your-username/steverator.git
cd steverator
```

**Option B: If you already cloned without `--recursive`**
```bash
git submodule update --init --recursive
```

---

## 🏗 2. Building the Plugin

We use CMake to build the project. This is platform-agnostic.

### macOS (Terminal)

1.  **Configure the project:**
    ```bash
    # Create a build directory
    mkdir build
    cd build

    # Generate Xcode project files
    cmake .. -G Xcode
    ```

2.  **Build:**
    You can either open the generated `steverator.xcodeproj` in Xcode and click "Build", or run:
    ```bash
    cmake --build . --config Release
    ```

3.  **Locate the Artifact:**
    The compiled VST3 file will be located in:
    `build/steverator_artefacts/Release/vst3/steverator.vst3`

### Windows (PowerShell / Command Prompt)

1.  **Configure the project:**
    ```bash
    mkdir build
    cd build
    cmake ..
    ```

2.  **Build:**
    You can open the generated `steverator.sln` in Visual Studio, or run:
    ```bash
    cmake --build . --config Release
    ```

3.  **Locate the Artifact:**
    `build/steverator_artefacts/Release/vst3/steverator.vst3`

---

## 📦 3. Installation

To use the plugin in your DAW (Ableton, Logic, Reaper, etc.), you need to move the `.vst3` file to the system's plugin folder.

### macOS
Move `steverator.vst3` to:
`/Library/Audio/Plug-Ins/VST3/`
*(Or `~/Library/Audio/Plug-Ins/VST3/` for user-only install)*

### Windows
Move `steverator.vst3` to:
`C:\Program Files\Common Files\VST3\`

---

## 🐞 Troubleshooting

*   **"CMake Error: Could not find JUCE"**:
    *   Did you run `git submodule update --init --recursive`? Check if `libs/JUCE` contains files.
*   **"Architecture mismatch" (macOS)**:
    *   The project is set up to build Universal Binaries (arm64 + x86_64) by default if you are on an M1/M2/M3 mac.
*   **Plugin not showing in DAW**:
    *   Ensure you placed the `.vst3` file in the correct folder.
    *   Rescan plugins in your DAW settings.
    *   Check if you built the `Debug` or `Release` version. Release is recommended for actual use.

---

## 📚 For Developers: Project Structure

*   `CMakeLists.txt`: The build configuration.
*   `Source/`: Contains all C++ code.
    *   `PluginProcessor.cpp/h`: The audio engine (DSP, Parameters).
    *   `PluginEditor.cpp/h`: The GUI.
*   `libs/JUCE`: The JUCE framework source code.

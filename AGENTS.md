# 🤖 AGENTS.md — Instructions for Development Agents

This file contains instructions for Claude (or other AI agents) working on the steverator project.

---

## 📋 Overview

**steverator** is a VST3 audio plugin built with JUCE, themed around "Le Poisson Steve" (Steve the Fish).

**Current Status (January 2026):**
- ✅ Complete 3-band saturation with independent frequency control
- ✅ Full JUCE parameter system with 13 automatable parameters
- ✅ Warm, inviting UI design with mascot image integration
- ✅ Procedural knob and button drawing (no image dependencies for UI)
- ✅ Standalone and VST3 builds available
- ✅ Build hash system for testing verification

**Key constraints:**
- Written in C++17 with JUCE framework
- Builds to VST3 format
- macOS target (Apple Silicon compatible)
- Focus on clarity and maintainability

---

## 🔨 Build & Deploy Workflow

### When the User Requests Changes

If the user asks to modify the plugin (code, UI, images, colors, etc.), always follow this workflow:

1. **Read and understand** the relevant source files (always use Read tool first)
2. **Plan changes** if the task is complex (use EnterPlanMode for implementation decisions)
3. **Make changes** to the code/assets using Edit tool
4. **Build and deploy** using the script
5. **Verify** the changes work by testing the standalone app or in Ableton

### Build Command

After making code changes, always run:

```bash
cd /Users/vava/Documents/GitHub/steverator
./build_and_deploy.sh
```

**This script:**
- ✅ Generates a **unique 5-character build hash** (e.g., `09FA1`)
- ✅ CMake compilation (Release mode) with hash embedded
- ✅ Plugin signing (codesign)
- ✅ Asset copying (steve.png from /Assets/)
- ✅ Cache clearing (Ableton)
- ✅ Deployment to `/Library/Audio/Plug-Ins/VST3/steverator.vst3`
- ✅ Launches standalone app for testing
- ✅ Displays build hash and timestamp

**Important:** The script will output the build hash like this:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✨ BUILD COMPLETE!

🔑 BUILD HASH: 09FA1
📅 TIMESTAMP: 2026-01-13 22:08:18

You should see '09FA1' in BIG ORANGE text at the BOTTOM
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🚀 Launching Standalone...
```

### After Build - Tell the User

**Always include the build hash when telling the user:**

```
✅ Build successful!
🔑 Build hash: 09FA1

For Ableton testing:
1. Restart Ableton (Cmd+Q)
2. Reopen Ableton
3. Rescan plugins: Preferences → File/Folder → Rescan
4. Search for "steverator" and open it
5. Check the bottom-right corner - you should see "09FA1" in orange text
6. If you see a different hash or "DEV", the cache wasn't cleared properly

The standalone app was automatically launched for quick testing.
```

This hash verification ensures the user is testing the fresh build, not a cached version.

---

## 📁 Project Structure

```
steverator/
├── CMakeLists.txt              # JUCE VST3 build configuration
├── build_and_deploy.sh         # Automated build, sign, deploy script ⭐ ALWAYS RUN AFTER CHANGES
├── README.md                   # Full project documentation
├── AGENTS.md                   # This file - agent instructions
├── Assets/                     # Theme assets
│   └── steve.png               # "Steve the Fish" mascot image (220px display)
├── Source/
│   ├── PluginProcessor.h       # Audio plugin interface
│   ├── PluginProcessor.cpp     # Real-time DSP: 3-band saturation, crossovers, effects
│   ├── PluginEditor.h          # UI component declarations
│   ├── PluginEditor.cpp        # UI layout, rendering, parameter binding
│   ├── CustomLookAndFeel.h     # Custom UI styling interface
│   └── CustomLookAndFeel.cpp   # Procedural knob & button drawing
├── libs/
│   └── JUCE/                   # JUCE framework (submodule - DO NOT EDIT)
└── build/                      # Build artifacts (generated - DO NOT EDIT)
```

**Key Implementation Details:**
- No image-based UI elements (all procedurally drawn)
- Color scheme defined in code for easy tweaking
- FlexBox-based responsive layout
- All 13 parameters exposed via APVTS

---

## 🎨 Theme Assets & Color Design

### Asset Management

- **Location:** `/Assets/` directory
- **Current Assets:**
  - `steve.png` - "Steve the Fish" mascot (2.0 MB PNG, displayed at 220px wide)

### Design Philosophy

**Important:** UI elements (knobs, buttons, background) are **all procedurally drawn in C++**.
There are NO image files for UI styling - everything is vector-based and code-drawn.

### Color Scheme (Currently "Le Poisson Steve" - Warm Tones)

The current color palette is defined in the code:

| Element | Color (RGB) | Hex | Purpose |
|---------|-------------|-----|---------|
| Background | (0.93, 0.90, 0.82) | #EDE6D1 | Warm beige base |
| Knob Ring | (0.6, 0.35, 0.1) | #994D1A | Dark brown-orange |
| Knob Arc | Gradient: Golden→Orange | HSV-based | Value indicator |
| Text (Labels) | (0.5, 0.3, 0.1) | #7D4C1A | Dark brown |
| Button Active | (1.0, 0.5, 0.1) | #FF8019 | Bright orange glow |
| Button Inactive | (0.6, 0.35, 0.1) | #994D1A | Subtle brown |

### Updating Theme Assets

To change the mascot image:
1. Replace `/Assets/steve.png` with new image (PNG format)
2. Image will be scaled to 220px wide on left panel
3. Run `./build_and_deploy.sh`
4. Test in Ableton or standalone app

To change colors throughout the UI:
1. Edit color values in:
   - `Source/PluginEditor.cpp` - Background, title, layout colors
   - `Source/CustomLookAndFeel.cpp` - Knob and button colors
2. Colors are defined as `juce::Colour::fromFloatRGBA(R, G, B, A)` with values 0.0-1.0
3. Run `./build_and_deploy.sh`
4. Test the appearance

---

## 💾 Key Files to Know

| File | Purpose | Edit? |
|------|---------|-------|
| `Source/PluginProcessor.cpp` | Audio DSP: saturation, crossovers, parameters | ✅ Yes - Core logic |
| `Source/PluginEditor.cpp` | UI layout, colors, parameter bindings, image loading | ✅ Yes - Layout & appearance |
| `Source/CustomLookAndFeel.cpp` | Knob & button drawing, procedural rendering | ✅ Yes - Styling |
| `CMakeLists.txt` | Build configuration, plugin metadata | ⚠️ Carefully |
| `build_and_deploy.sh` | Automated build & deployment | ⚠️ Carefully |
| `Assets/steve.png` | "Steve the Fish" theme image | ✅ Yes - Replace |
| `README.md` | Project documentation | ✅ Yes - Keep updated |
| `AGENTS.md` | Agent instructions (this file) | ✅ Yes - Keep updated |
| `libs/JUCE/` | JUCE framework (submodule) | ❌ Never edit |
| `build/` | Build artifacts (generated) | ❌ Never edit |

---

## 🐛 Troubleshooting

### Plugin doesn't appear in Ableton

1. Run `./build_and_deploy.sh`
2. Close Ableton completely (Cmd+Q)
3. Reopen Ableton
4. Rescan plugins
5. Search for "steverator2"

### Knob isn't rotating correctly

- Check `CustomLookAndFeel.cpp` for rotation math
- Ensure `knob.png` is square and symmetrical
- Verify `AffineTransform::rotation()` calculations

### Images aren't displaying

- Verify PNG files exist in `/Assets/`
- Run `./build_and_deploy.sh` to copy them
- Check image dimensions (knob should be square, minimum 256x256)
- Clear Ableton cache and restart

### Build fails

- Check compiler warnings
- Ensure JUCE submodule is initialized: `git submodule update --init`
- Verify CMake is installed: `cmake --version`

---

## 📝 Code Style & Conventions

- **Language:** C++17
- **Comments:** Educational, explain the "why" not just "what"
- **Naming:** camelCase for variables, PascalCase for classes
- **Real-time rules:** No allocations in `processBlock()`
- **Dependencies:** Only JUCE, no external libraries

---

## 🎯 Common Tasks

### To change the background color/image

Edit: `Source/PluginEditor.cpp` → `paint()` method

OR replace: `Assets/background.png`

Then run: `./build_and_deploy.sh`

### To change slider behavior

Edit: `Source/PluginEditor.cpp` → `resized()` method

Adjust:
- Slider size calculations
- Position calculations
- Parameter ranges

Then run: `./build_and_deploy.sh`

### To add a new parameter

Edit: `Source/PluginProcessor.cpp` → `createParameterLayout()`

Add new parameter, then:
- Create slider in `PluginEditor.cpp`
- Add attachment
- Adjust layout in `resized()`

Then run: `./build_and_deploy.sh`

### To modify DSP (audio processing)

Edit: `Source/PluginProcessor.cpp` → `processBlock()` method

Keep in mind:
- ⚠️ No memory allocation
- ⚠️ No logging
- ⚠️ No locks/mutexes
- ✅ Fast, real-time safe code

Then run: `./build_and_deploy.sh`

---

## ✅ Checklist Before Asking User to Test

- [ ] Code compiles without errors
- [ ] Build script ran successfully
- [ ] Plugin deployed to `/Library/Audio/Plug-Ins/VST3/steverator2.vst3`
- [ ] All assets (images) are in place
- [ ] User notified to: restart Ableton, rescan plugins
- [ ] Plugin signature is valid (no codesign errors)
- [ ] Ableton caches were cleared

---

## 📚 References

- JUCE Framework: https://juce.com/
- VST3 Specification: https://www.steinberg.net/vst3
- CMake: https://cmake.org/

---

**Last updated:** January 2026

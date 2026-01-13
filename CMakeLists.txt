cmake_minimum_required(VERSION 3.15)

# -----------------------------------------------------------------------------
# 🏷️ Project Definition
# -----------------------------------------------------------------------------
project(steverator VERSION 0.0.1)

# Build Universal Binary (Apple Silicon + Intel) for maximum compatibility
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# -----------------------------------------------------------------------------
# 🧱 JUCE Configuration
# -----------------------------------------------------------------------------
# We add the JUCE subdirectory. This allows us to use JUCE modules and
# cmake functions (like juce_add_plugin).
# The 'libs/JUCE' folder must exist (via git submodule).
add_subdirectory(libs/JUCE)

# Ignore VST3 parameter ID mismatch warning (not applicable for new plugins)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DJUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1")

# Copy assets to build directory
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets
    ${CMAKE_BINARY_DIR}/Assets
    COMMENT "Copying assets to build directory"
)

# -----------------------------------------------------------------------------
# 🎛️ Plugin Definition
# -----------------------------------------------------------------------------
# 'juce_add_plugin' is a helper function from JUCE.
# It sets up the target, links libraries, and handles VST3/AU packaging.
juce_add_plugin(steverator
    # The company name (shows up in DAW)
    COMPANY_NAME "NeiXXa"
    # The plugin code (must be unique 4 chars)
    PLUGIN_CODE "STEV"
    # Manufacturer code (must be unique 4 chars)
    MANUFACTURER_CODE "NXXA"
    # Bundle Identifier (Important for macOS)
    BUNDLE_ID "com.neixxa.steverator"
    # Formats to build
    FORMATS VST3 Standalone
    # Name of the product
    PRODUCT_NAME "Steverator"
    # Plugin Type (Effect, Synth, etc.)
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    # Version
    VERSION "0.0.1"
)

# -----------------------------------------------------------------------------
# 📄 Source Files
# -----------------------------------------------------------------------------
# We list the C++ files that make up our plugin.
target_sources(steverator
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/CustomLookAndFeel.cpp
        Source/CustomLookAndFeel.h
)

# -----------------------------------------------------------------------------
# 📚 Linking Dependencies
# -----------------------------------------------------------------------------
# We link the necessary JUCE modules.
target_link_libraries(steverator
    PRIVATE
        # Base JUCE functionality (Strings, Files, etc.)
        juce::juce_core
        # Events and messaging
        juce::juce_events
        # Graphics and Font rendering
        juce::juce_graphics
        # Image file formats (PNG, JPG)
        juce::juce_data_structures
        # GUI Components (Sliders, Buttons)
        juce::juce_gui_basics
        juce::juce_gui_extra
        # Audio device management and formats
        juce::juce_audio_basics
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
        # DSP module for processing
        juce::juce_dsp
)

# -----------------------------------------------------------------------------
# ⚙️ Compiler Settings
# -----------------------------------------------------------------------------
# We want to use modern C++ (C++17 or 20)
target_compile_features(steverator PUBLIC cxx_std_17)

# Enable strict warnings to help us write better code
if(MSVC)
    target_compile_options(steverator PRIVATE /W4)
else()
    target_compile_options(steverator PRIVATE -Wall -Wextra)
endif()

# -----------------------------------------------------------------------------
# 📁 Binary Output
# -----------------------------------------------------------------------------
# This ensures the VST3 bundle is copied to a convenient location
juce_generate_juce_header(steverator)

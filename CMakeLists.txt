cmake_minimum_required(VERSION 3.22)

project(BusGlue VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE - adjust path as needed for your system
# Option 1: If JUCE is installed via CMake package
find_package(JUCE CONFIG QUIET)

# Option 2: If JUCE is a subdirectory (uncomment and adjust path)
# add_subdirectory(path/to/JUCE)

# If JUCE not found, provide instructions
if(NOT JUCE_FOUND AND NOT TARGET juce::juce_core)
    message(FATAL_ERROR
        "JUCE not found! Please either:\n"
        "1. Install JUCE and set CMAKE_PREFIX_PATH to JUCE's install location\n"
        "2. Add JUCE as a subdirectory by uncommenting the add_subdirectory line in CMakeLists.txt\n"
        "3. Set JUCE_DIR to point to JUCE's cmake directory"
    )
endif()

# Plugin target
juce_add_plugin(BusGlue
    COMPANY_NAME "Ian Fletcher Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE IFAu
    PLUGIN_CODE BuG1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Bus Glue"
    BUNDLE_ID "com.ianfletcheraudio.busglue"
    DESCRIPTION "SSL/API Style Bus Compressor"
)

# Source files
target_sources(BusGlue
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/DSP/DSPUtils.h
        Source/DSP/BusCompressor.cpp
        Source/DSP/BusCompressor.h
        Source/DSP/EnvelopeFollower.cpp
        Source/DSP/EnvelopeFollower.h
        Source/DSP/Saturator.cpp
        Source/DSP/Saturator.h
        Source/UI/LookAndFeel.h
        Source/UI/MeterComponents.h
)

# Include directories
target_include_directories(BusGlue
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode
)

# Compile definitions
target_compile_definitions(BusGlue
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules
target_link_libraries(BusGlue
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

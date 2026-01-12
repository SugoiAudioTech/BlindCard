cmake_minimum_required(VERSION 3.22)

project(BlindCard VERSION 1.0.0)

add_subdirectory(/Users/Mac/JUCE ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(BlindCard
    COMPANY_NAME "SugoiMusicTech"
    PLUGIN_MANUFACTURER_CODE Sugo
    PLUGIN_CODE Blcd
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Blind Card"
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(BlindCard
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Core/Types.h)

target_compile_definitions(BlindCard
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_features(BlindCard PRIVATE cxx_std_17)

target_link_libraries(BlindCard
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

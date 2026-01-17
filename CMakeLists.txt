cmake_minimum_required(VERSION 3.22)

project(BlindCard VERSION 1.0.0)

add_subdirectory(/Users/Mac/JUCE ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(BlindCard
    COMPANY_NAME "Sugoi"
    PLUGIN_MANUFACTURER_CODE Sgoi
    PLUGIN_CODE Blcd
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Blind Card"
    COPY_PLUGIN_AFTER_BUILD TRUE)

# 加入光暈素材為 BinaryData
juce_add_binary_data(BlindCardAssets SOURCES
    assets/glow/glow_warm@1x.png
    assets/glow/glow_warm@2x.png
    assets/glow/glow_cool@1x.png
    assets/glow/glow_cool@2x.png
    assets/glow/compact/glow_warm_compact@1x.png
    assets/glow/compact/glow_warm_compact@2x.png
    assets/glow/compact/glow_cool_compact@1x.png
    assets/glow/compact/glow_cool_compact@2x.png)

target_sources(BlindCard
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Core/Types.h
        Source/Core/BlindCardManager.h
        Source/Core/BlindCardManager.cpp
        Source/UI/CardComponent.h
        Source/UI/CardComponent.cpp
        Source/UI/ColorUtils.h
        Source/UI/DebugPanel.h
        Source/UI/GlowAssets.h)

target_compile_definitions(BlindCard
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_features(BlindCard PRIVATE cxx_std_17)

target_link_libraries(BlindCard
    PRIVATE
        BlindCardAssets
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

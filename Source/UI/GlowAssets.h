#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

namespace blindcard
{

/**
 * 光暈素材管理類別
 * 載入並快取 PNG 貼圖資源
 */
class GlowAssets
{
public:
    static GlowAssets& getInstance()
    {
        static GlowAssets instance;
        return instance;
    }

    /**
     * 取得暖金光暈圖片（翻牌閃爍用）
     * @param use2x 是否使用 @2x 版本
     */
    const juce::Image& getGlowWarm (bool use2x = false) const
    {
        return use2x ? glowWarm2x : glowWarm1x;
    }

    /**
     * 取得白金光暈圖片（選中態用）
     * @param use2x 是否使用 @2x 版本
     */
    const juce::Image& getGlowCool (bool use2x = false) const
    {
        return use2x ? glowCool2x : glowCool1x;
    }

    /**
     * 取得 Compact 模式暖金光暈
     */
    const juce::Image& getGlowWarmCompact (bool use2x = false) const
    {
        return use2x ? glowWarmCompact2x : glowWarmCompact1x;
    }

    /**
     * 取得 Compact 模式白金光暈
     */
    const juce::Image& getGlowCoolCompact (bool use2x = false) const
    {
        return use2x ? glowCoolCompact2x : glowCoolCompact1x;
    }

    /**
     * 檢查素材是否已載入
     */
    bool isLoaded() const { return loaded; }

private:
    GlowAssets()
    {
        loadAssets();
    }

    void loadAssets()
    {
        // 載入 Standard 尺寸素材
        // JUCE BinaryData 將 @ 轉成底線
        glowWarm1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm1x_png,
            static_cast<size_t> (BinaryData::glow_warm1x_pngSize));

        glowWarm2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm2x_png,
            static_cast<size_t> (BinaryData::glow_warm2x_pngSize));

        glowCool1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool1x_png,
            static_cast<size_t> (BinaryData::glow_cool1x_pngSize));

        glowCool2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool2x_png,
            static_cast<size_t> (BinaryData::glow_cool2x_pngSize));

        // 載入 Compact 尺寸素材
        glowWarmCompact1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm_compact1x_png,
            static_cast<size_t> (BinaryData::glow_warm_compact1x_pngSize));

        glowWarmCompact2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm_compact2x_png,
            static_cast<size_t> (BinaryData::glow_warm_compact2x_pngSize));

        glowCoolCompact1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool_compact1x_png,
            static_cast<size_t> (BinaryData::glow_cool_compact1x_pngSize));

        glowCoolCompact2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool_compact2x_png,
            static_cast<size_t> (BinaryData::glow_cool_compact2x_pngSize));

        loaded = glowWarm1x.isValid() && glowCool1x.isValid();
    }

    juce::Image glowWarm1x;
    juce::Image glowWarm2x;
    juce::Image glowCool1x;
    juce::Image glowCool2x;

    juce::Image glowWarmCompact1x;
    juce::Image glowWarmCompact2x;
    juce::Image glowCoolCompact1x;
    juce::Image glowCoolCompact2x;

    bool loaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlowAssets)
};

} // namespace blindcard

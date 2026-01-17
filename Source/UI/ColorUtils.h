#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

namespace blindcard
{

/**
 * HSL 色彩工具函式
 * 用於色相環插值（走最短路徑）
 */
struct ColorUtils
{
    struct HSL
    {
        float h; // 0-360
        float s; // 0-1
        float l; // 0-1
    };

    /**
     * RGB 轉 HSL
     */
    static HSL rgbToHsl (juce::Colour colour)
    {
        float r = colour.getFloatRed();
        float g = colour.getFloatGreen();
        float b = colour.getFloatBlue();

        float maxVal = std::max ({r, g, b});
        float minVal = std::min ({r, g, b});
        float delta = maxVal - minVal;

        HSL hsl;
        hsl.l = (maxVal + minVal) / 2.0f;

        if (delta < 0.0001f)
        {
            hsl.h = 0.0f;
            hsl.s = 0.0f;
        }
        else
        {
            hsl.s = hsl.l > 0.5f
                ? delta / (2.0f - maxVal - minVal)
                : delta / (maxVal + minVal);

            if (maxVal == r)
                hsl.h = std::fmod ((g - b) / delta + 6.0f, 6.0f) * 60.0f;
            else if (maxVal == g)
                hsl.h = ((b - r) / delta + 2.0f) * 60.0f;
            else
                hsl.h = ((r - g) / delta + 4.0f) * 60.0f;
        }

        return hsl;
    }

    /**
     * HSL 轉 RGB
     */
    static juce::Colour hslToRgb (float h, float s, float l, float alpha = 1.0f)
    {
        if (s < 0.0001f)
        {
            return juce::Colour::fromFloatRGBA (l, l, l, alpha);
        }

        auto hueToRgb = [] (float p, float q, float t) -> float
        {
            if (t < 0.0f) t += 1.0f;
            if (t > 1.0f) t -= 1.0f;
            if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
            if (t < 0.5f) return q;
            if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
        };

        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        float hNorm = h / 360.0f;

        float r = hueToRgb (p, q, hNorm + 1.0f / 3.0f);
        float g = hueToRgb (p, q, hNorm);
        float b = hueToRgb (p, q, hNorm - 1.0f / 3.0f);

        return juce::Colour::fromFloatRGBA (r, g, b, alpha);
    }

    static juce::Colour hslToRgb (HSL hsl, float alpha = 1.0f)
    {
        return hslToRgb (hsl.h, hsl.s, hsl.l, alpha);
    }

    /**
     * 計算色相環最短路徑的插值
     * @param fromHue 起始色相 (0-360)
     * @param toHue 目標色相 (0-360)
     * @param t 插值參數 (0-1)
     * @return 插值後的色相 (0-360)
     */
    static float interpolateHueShortestPath (float fromHue, float toHue, float t)
    {
        float diff = toHue - fromHue;

        // 走最短路徑
        if (diff > 180.0f)
            diff -= 360.0f;
        else if (diff < -180.0f)
            diff += 360.0f;

        float result = fromHue + diff * t;

        // 確保結果在 0-360 範圍內
        if (result < 0.0f) result += 360.0f;
        if (result >= 360.0f) result -= 360.0f;

        return result;
    }

    /**
     * HSL 色彩插值（走色相環最短路徑）
     */
    static juce::Colour interpolateHSL (juce::Colour from, juce::Colour to, float t)
    {
        HSL hslFrom = rgbToHsl (from);
        HSL hslTo = rgbToHsl (to);

        float h = interpolateHueShortestPath (hslFrom.h, hslTo.h, t);
        float s = hslFrom.s + (hslTo.s - hslFrom.s) * t;
        float l = hslFrom.l + (hslTo.l - hslFrom.l) * t;
        float a = from.getFloatAlpha() + (to.getFloatAlpha() - from.getFloatAlpha()) * t;

        return hslToRgb (h, s, l, a);
    }
};

/**
 * 簡單的動畫值類別
 * 用於 alpha 透明度等數值的平滑過渡
 */
class AnimatedValue
{
public:
    AnimatedValue (float initialValue = 0.0f)
        : currentValue (initialValue), targetValue (initialValue), startValue (initialValue)
    {
    }

    void setTarget (float target, float durationMs = 150.0f)
    {
        if (std::abs (target - targetValue) < 0.0001f)
            return;

        startValue = currentValue;
        targetValue = target;
        transitionDuration = durationMs;
        elapsedTime = 0.0f;
    }

    void setImmediate (float value)
    {
        currentValue = value;
        targetValue = value;
        startValue = value;
        elapsedTime = transitionDuration;
    }

    /**
     * 更新動畫狀態
     * @param deltaTimeMs 經過的毫秒數
     * @return true 如果值發生變化
     */
    bool update (float deltaTimeMs)
    {
        if (isComplete())
            return false;

        elapsedTime += deltaTimeMs;
        float t = juce::jlimit (0.0f, 1.0f, elapsedTime / transitionDuration);

        // 使用 ease-out 曲線
        float eased = useEaseOut ? easeOut (t) : t;

        float newValue = startValue + (targetValue - startValue) * eased;

        if (std::abs (newValue - currentValue) < 0.0001f)
        {
            currentValue = targetValue;
            return false;
        }

        currentValue = newValue;
        return true;
    }

    float getValue() const { return currentValue; }
    float getTarget() const { return targetValue; }
    bool isComplete() const { return elapsedTime >= transitionDuration; }

    void setEaseOut (bool useEase) { useEaseOut = useEase; }
    void setDuration (float durationMs) { transitionDuration = durationMs; }

private:
    float currentValue;
    float targetValue;
    float startValue;
    float transitionDuration = 150.0f;
    float elapsedTime = 0.0f;
    bool useEaseOut = true;

    static float easeOut (float t)
    {
        return 1.0f - std::pow (1.0f - t, 3.0f);
    }
};

} // namespace blindcard

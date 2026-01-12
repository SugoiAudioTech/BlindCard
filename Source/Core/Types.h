#pragma once

#include <juce_core/juce_core.h>

namespace blindcard
{

// 單輪評分資料
struct RoundData
{
    int rating = 0;          // 1-5 星，0 = 未評分
    juce::String note;       // 文字筆記
};

// 單張卡牌
struct CardSlot
{
    int id = -1;                              // 0-7 卡牌編號
    juce::String realTrackName;               // 真實軌道名稱
    int displayPosition = -1;                 // 洗牌後顯示位置
    bool isRemoved = false;                   // 是否已移除
    juce::Array<RoundData> rounds;            // 各輪記錄

    // 計算平均評分（忽略未評分的輪次）
    float getAverageRating() const
    {
        int count = 0;
        int total = 0;
        for (const auto& r : rounds)
        {
            if (r.rating > 0)
            {
                total += r.rating;
                count++;
            }
        }
        return count > 0 ? static_cast<float>(total) / count : 0.0f;
    }
};

// 遊戲階段
enum class GamePhase
{
    Setup,        // 初始狀態，可編輯軌道名稱
    BlindTesting, // 盲測中
    Revealed      // 已揭曉
};

// 遊戲狀態
struct GameState
{
    GamePhase phase = GamePhase::Setup;
    int totalRounds = 1;                      // 總輪數
    int currentRound = 0;                     // 目前輪次 (0-indexed)
    int selectedCardId = -1;                  // 目前獨奏的卡牌 (-1 = 無)
    juce::Array<CardSlot> cards;              // 最多 8 張卡牌

    static constexpr int MaxCards = 8;
};

} // namespace blindcard

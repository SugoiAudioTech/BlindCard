#pragma once

#include <juce_core/juce_core.h>

namespace blindcard
{

// 單輪評分資料
struct RoundData
{
    int rating = 0;          // 1-5 星，0 = 未評分
    juce::String note;       // 文字筆記
    int guessedTrackId = -1; // 猜測的原始軌道 ID (-1 = 未猜測)
};

// 單張卡牌
struct CardSlot
{
    int id = -1;                              // 0-7 卡牌編號
    juce::String realTrackName;               // 真實軌道名稱
    int displayPosition = -1;                 // 洗牌後顯示位置
    bool isRemoved = false;                   // 是否已移除
    juce::Array<RoundData> rounds;            // 各輪記錄

    // Level Matching
    float measuredLUFS = -100.0f;             // 測量的響度 (LUFS)，-100 = 未測量
    float autoGainDb = 0.0f;                  // 自動增益補償 (dB)
    float manualGainDb = 0.0f;                // 手動增益調整 (dB)

    float getTotalGainDb() const { return autoGainDb + manualGainDb; }
    bool hasLUFSMeasurement() const { return measuredLUFS > -100.0f; }

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

    // 計算猜測正確率（回傳 correctCount 和 totalGuesses）
    std::pair<int, int> getGuessAccuracy() const
    {
        int correct = 0;
        int total = 0;
        for (const auto& r : rounds)
        {
            if (r.guessedTrackId >= 0)  // 有做猜測
            {
                total++;
                if (r.guessedTrackId == id)  // 猜對了
                    correct++;
            }
        }
        return { correct, total };
    }
};

// 遊戲階段
enum class GamePhase
{
    Setup,        // 初始狀態，可編輯軌道名稱
    BlindTesting, // 盲測中
    Revealed      // 已揭曉
};

// 評分模式
enum class RatingMode
{
    Stars,   // 星等評分
    Guess,   // 猜測軌道
    QA       // Q&A 問答模式
};

// Q&A 模式專用狀態
struct QAState
{
    int currentQuestion = 0;              // 當前問題索引 (0-based)
    int targetCardId = -1;                // 當前問題的目標卡牌 ID
    juce::Array<int> askedCardIds;        // 已經問過的卡牌 ID
    juce::Array<bool> answers;            // 每題的答案結果 (true = 正確)

    enum class FeedbackState { None, Correct, Wrong };
    FeedbackState lastFeedback = FeedbackState::None;
    int lastAnsweredCardId = -1;          // 上一次回答選擇的卡牌

    bool isComplete(int maxQuestions) const { return currentQuestion >= maxQuestions; }

    int getCorrectCount() const
    {
        int count = 0;
        for (auto a : answers)
            if (a) count++;
        return count;
    }

    void reset()
    {
        currentQuestion = 0;
        targetCardId = -1;
        askedCardIds.clear();
        answers.clear();
        lastFeedback = FeedbackState::None;
        lastAnsweredCardId = -1;
    }
};

// 遊戲狀態
struct GameState
{
    GamePhase phase = GamePhase::Setup;
    int totalRounds = 1;                      // 總輪數
    int currentRound = 0;                     // 目前輪次 (0-indexed)
    int selectedCardId = -1;                  // 目前獨奏的卡牌 (-1 = 無)
    juce::Array<CardSlot> cards;              // 最多 8 張卡牌
    QAState qaState;                          // Q&A 模式狀態

    static constexpr int MinCards = 2;        // 最少卡牌數（盲測需要對照組）
    static constexpr int MaxCards = 8;        // 最多卡牌數（雙排 4+4 佈局上限）
};

} // namespace blindcard

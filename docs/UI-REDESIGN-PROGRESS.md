# BlindCard UI Redesign Progress

> **Date**: 2026-01-19
> **Branch**: ui-redesign (worktree)
> **Status**: In Progress

## Overview

將原本 React/Next.js 設計的 BlindCard UI 移植到 JUCE C++ 實作。目標是達到與原始設計一致的視覺效果和互動體驗。

## Completed Components

### 1. Theme System
- **ThemeManager** - 單例模式管理深色/淺色主題切換
- **ThemeColors.h** - 定義所有顏色常數
  - 深色主題：黑色背景、紅色主色調、金色強調色
  - 淺色主題：米色背景、深紅主色調
  - 撲克桌顏色：翡翠綠絨布、珊瑚色木框、霓虹紅邊框

### 2. PokerTable (撲克桌)
- **Stadium/Pill Shape** - 橢圓形桌面（corner radius = height/2）
- **多層視覺結構**：
  1. 珊瑚色木框（漸層效果）
  2. 深色邊緣軌道
  3. 霓虹紅發光邊框（多層 glow）
  4. 翡翠綠絨布表面（radial gradient + 紋理 + vignette）
- **ChipStack** - 裝飾性籌碼堆（左右兩側）
- **Card Layout** - 支援 2-8 張卡片佈局（單排/雙排）

### 3. PokerCard (撲克牌)
- **卡片正面** - 黑桃 A 設計 + Track 名稱
- **卡片背面** - 黑底金色花紋圖片（cardback.png）
  - 修復：添加純黑背景防止桌面顏色透過
- **翻牌動畫** - 3D 翻轉效果 + 金色閃光
- **播放指示器** - 綠色發光邊框 + 脈動效果 + 綠點
- **位置標籤** - 右下角金色數字

### 4. ControlPanel (控制面板)
- **Info Row** - Current Tracks 顯示 + Rounds 滑桿 + 回合計數
- **Auto Gain Row** - iOS 風格 toggle 開關
- **Action Buttons** - ChipButton 組件（SHUFFLE, REVEAL, RESET, NEXT）
- **Custom LookAndFeel** - 自訂滑桿和開關樣式

### 5. ChipButton (籌碼按鈕)
- 撲克籌碼造型按鈕
- 多種變體：Red, Blue, Black, Gold
- 條紋邊緣裝飾

### 6. StarRating (星級評分)
- 5 星評分組件
- 點擊動畫（縮放 + 填充）
- 懸停預覽

### 7. HeaderBar (頂部欄)
- Logo + 標題
- 連線狀態指示
- 主題切換按鈕
- 設定/資訊按鈕

### 8. ModeSelector (模式選擇器)
- Stars / Guess / Q&A 三種模式
- 膠囊式切換器設計

### 9. ResultsPanel (結果面板)
- 顯示評分結果
- 支援三種模式的結果呈現

## Known Issues / TODO

### Bugs to Fix
- [ ] Guess 模式下拉選單功能待測試
- [ ] Q&A 模式互動待測試
- [ ] 字體樣式可能需要進一步調整

### Features to Implement
- [ ] 設定對話框
- [ ] 資訊/關於對話框
- [ ] Level Matching UI（音量匹配）
- [ ] 完整的 DAW 連線測試

### UI Polish
- [ ] 動畫微調
- [ ] 響應式佈局優化
- [ ] 深色/淺色主題完整測試

## File Structure

```
Source/UI/
├── Theme/
│   ├── ThemeManager.h/cpp    # 主題管理單例
│   └── ThemeColors.h         # 顏色定義
├── Components/
│   ├── HeaderBar.h/cpp       # 頂部欄
│   ├── ModeSelector.h/cpp    # 模式選擇器
│   ├── PokerTable.h/cpp      # 撲克桌
│   ├── PokerCard.h/cpp       # 撲克牌
│   ├── ChipStack.h/cpp       # 籌碼堆
│   ├── ChipButton.h/cpp      # 籌碼按鈕
│   ├── ControlPanel.h/cpp    # 控制面板
│   ├── ResultsPanel.h/cpp    # 結果面板
│   ├── StarRating.h/cpp      # 星級評分
│   ├── GuessDropdown.h/cpp   # 猜測下拉選單
│   └── QuestionBanner.h/cpp  # Q&A 問題橫幅
└── BlindCardEditor.h/cpp     # 主編輯器組件

assets/
└── cards/
    └── card-back.png         # 卡片背面圖片
```

## Key Fixes Applied

1. **卡片背面透明度問題** - 在繪製圖片前先填充純黑背景
2. **星級評分無法點擊** - PokerCard 高度不足，增加 28px 給控制區域
3. **SHUFFLE 按鈕無效** - 使用 `getCards().size()` 而非 `getRegisteredCount()`
4. **測試卡片初始化** - Standalone 模式自動添加 4 張測試卡片
5. **桌面 vignette 太暗** - 從 0.4 降到 0.15 alpha
6. **字體樣式** - 使用 `Font::getDefaultSansSerifFontName()` 獲得現代感

## Build Commands

```bash
# 在 worktree 目錄執行
cd /Users/Mac/Projects/BlindCard/.worktrees/ui-redesign

# 編譯 Standalone
cmake --build build --target BlindCard_Standalone -j8

# 執行 Standalone
open "build/BlindCard_artefacts/Standalone/Blind Card.app"
```

## Reference Design

原始設計位於主專案的 React/Next.js 實作，主要參考：
- 撲克桌：翡翠綠絨布 + 珊瑚色木框 + 霓虹紅邊框
- 卡片背面：黑底金色花紋
- 整體風格：賭場/撲克主題，深色為主

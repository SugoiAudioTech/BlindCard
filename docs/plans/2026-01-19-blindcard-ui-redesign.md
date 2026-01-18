# BlindCard UI/UX Redesign - Design Document

> **Date:** 2026-01-19
> **Designer:** Vickey & 雅婷姐 (前端設計)
> **Engineer:** Jensen (實作)
> **Status:** Approved, Ready for Implementation

---

## 1. 專案概述

### 目標
將 Vickey & 雅婷姐的前端設計（React/Next.js）轉換為 JUCE C++ 實作，功能 1:1 還原現有 BlindCard。

### 設計來源
- 前端專案：`/Users/Mac/Projects/blind-card-plugin-ui-ux`
- 現有專案：`/Users/Mac/Projects/BlindCard`

### 架構決策
- **方案**：完全重寫 UI 層（方案 A）
- **保留**：BlindCardManager（核心邏輯）、Types.h（資料結構）、PluginProcessor
- **重寫**：PluginEditor、所有 UI 元件
- **複用**：AnimatedValue 動畫類別、部分 ColorUtils

### 視窗規格
- **最小尺寸**：900 × 600 px
- **預設尺寸**：1000 × 700 px
- **最大尺寸**：1400 × 900 px
- **支援拖拉縮放**

---

## 2. 新 UI 元件結構

```
Source/UI/
├── Theme/
│   ├── ThemeManager.h/.cpp      # Light/Dark 主題管理
│   └── ThemeColors.h            # 色彩常數定義
├── Components/
│   ├── HeaderBar.h/.cpp         # Logo + 連線狀態 + 設定
│   ├── ModeSelector.h/.cpp      # Stars/Guess/Q&A 切換
│   ├── PokerTable.h/.cpp        # 撲克桌容器（綠絨布+邊框）
│   ├── PokerCard.h/.cpp         # 單張撲克牌（翻牌動畫）
│   ├── ChipButton.h/.cpp        # 3D 籌碼按鈕
│   ├── ChipStack.h/.cpp         # 裝飾性籌碼堆
│   ├── ControlPanel.h/.cpp      # 左下控制區
│   ├── ResultsPanel.h/.cpp      # 右下結果區
│   ├── GuessDropdown.h/.cpp     # 猜測下拉選單
│   ├── StarRating.h/.cpp        # 5星評分元件
│   └── QuestionBanner.h/.cpp    # Q&A 問題橫幅
├── Animation/
│   └── AnimatedValue.h          # 動畫類別（複用）
└── BlindCardEditor.h/.cpp       # 主編輯器（重寫）
```

---

## 3. 視覺設計系統

### 3.1 色彩系統

#### Dark 主題（主要）
```cpp
// 背景
Background        #0A0A0A    // 深黑底色
Surface           #1F1F1F    // 面板背景
SurfaceAlt        #252525    // 次要面板

// 主色調
Primary           #FF3B4E    // 賭場紅（按鈕、強調）
PrimaryHover      #FF5A6A    // 紅色 hover
Accent            #FFD700    // 金色（星星、發光、標籤）
AccentMuted       #D4AF37    // 暗金（次要）

// 文字
TextPrimary       #F5F5F0    // 主要文字（米白）
TextSecondary     #9CA3AF    // 次要文字（灰）
TextMuted         #6B7280    // 提示文字

// 撲克桌
TableFelt         #1B8B6A    // 綠絨布
TableFeltCenter   #22A77D    // 絨布亮區（漸層中心）
TableWoodLight    #D4A574    // 木紋亮
TableWoodDark     #6B4F38    // 木紋暗
TableRail         #8B5A3C    // 桌緣（紅棕色）
NeonRed           #FF3B4E    // 霓虹紅邊框
NeonGlow          rgba(255,59,78,0.6)  // 紅光暈

// 狀態
Success           #22C55E    // 正確（綠）
Error             #EF4444    // 錯誤（紅）
```

#### Light 主題
```cpp
Background        #F5F0E8    // 暖米色
Surface           #FFFFFF    // 白色面板
Primary           #C41E3A    // 深紅
Accent            #D4AF37    // 暗金
TextPrimary       #1A1A1A    // 深色文字
TableFelt         #2D9B7A    // 亮綠絨布
```

### 3.2 字體
- **主要字體**：系統 Sans-serif（JUCE 預設）
- **Logo**：粗體，18px
- **標題**：Semi-bold，14px
- **內文**：Regular，12-13px
- **卡片數字**：Bold，24px

---

## 4. 元件設計規格

### 4.1 HeaderBar

**佈局結構**
```
┌─────────────────────────────────────────────────────────────┐
│ [Logo]  BlindCard          [Connected ●]    [🌙] [ℹ] [⚙]  │
│         BY SUGOI AUDIO                                      │
└─────────────────────────────────────────────────────────────┘
高度：56px
左右 padding：16px
```

**元素規格**

| 元素 | 規格 |
|------|------|
| Logo | 40×40px 圓角矩形，紅色漸層背景，白色黑桃 ♠ 符號 |
| 標題 | "BlindCard" 紅色 (#FF3B4E)，18px Bold |
| 副標題 | "BY SUGOI AUDIO" 灰色 (#6B7280)，10px，字距加寬 |
| Connected | 膠囊形狀，深灰背景，耳機 icon + 文字 + 金色圓點（脈動動畫） |
| 主題切換 | 44×24px Toggle，月亮/太陽 icon，紅色霓虹 glow 當開啟 |
| Info 按鈕 | 32×32px 圓形，ℹ icon，hover 時亮起 |
| 設定按鈕 | 32×32px 圓形，齒輪 icon，hover 時亮起 |

---

### 4.2 ModeSelector

**佈局結構**
```
            ┌─────────────────────────────────┐
            │  ☆ Stars  │  ◎ Guess  │  ✎ Q&A  │
            └─────────────────────────────────┘
            寬度：自適應（約 280px）
            高度：40px
            位置：置中，Header 下方 16px
```

**狀態樣式**

| 狀態 | 背景 | 文字 | 邊框 |
|------|------|------|------|
| 未選中 | 透明 | #9CA3AF (灰) | 無 |
| 未選中 Hover | rgba(255,255,255,0.05) | #F5F5F0 | 無 |
| 選中 | rgba(255,59,78,0.15) | #FF3B4E (紅) | 1px #FF3B4E |
| 鎖定 (遊戲中) | 同未選中 | 同未選中 | 顯示 🔒 icon |

---

### 4.3 PokerTable

**圖層繪製順序（底到頂）**

1. **木紋外框** (12px 厚度)
   - 漸層：#D4A574 → #6B4F38（左上到右下）
   - 圓角：20px

2. **紅霓虹邊框** (4px 厚度)
   - 顏色：#FF3B4E
   - 外發光：0 0 15px rgba(255,59,78,0.6)
   - 圓角：14px

3. **綠絨布面**
   - 徑向漸層：中心 #22A77D → 邊緣 #1B8B6A
   - 圓角：10px

4. **裝飾籌碼堆**（左右兩側）
   - 左側：5 個籌碼堆疊
   - 右側：4 個籌碼堆疊
   - 顏色：紅黑交替，金色環線

5. **卡片區域**
   - 4 張：單排置中，間距 16px
   - 5-8 張：雙排 4+N，排間距 20px

---

### 4.4 PokerCard

**基本尺寸**
- 預設：140 × 200 px
- 最小（8 張時）：100 × 143 px
- 圓角：8px

**卡片狀態**

| 狀態 | 顯示內容 |
|------|----------|
| 背面（Blind Testing） | 卡背圖片 `04-card-back.png`（不疊加任何文字） |
| 正面（Setup/Revealed） | 花色 + 數字 + Plugin 名稱 |
| 選中 | 金色發光邊框 + 輕微上浮 |
| 播放中 | 音量 icon + 脈動動畫 |

**正面設計**
```
┌─────────────────┐
│ 4               │  ← 左上：數字 (24px Bold)
│ ♠               │  ← 左上：花色符號
│                 │
│       ♠         │  ← 中央：大花色 (48px)
│                 │
│   Plugin A -    │  ← 中央偏下：Plugin 名稱 (12px)
│   Compressor    │
│                 │
│             ♥   │  ← 右下：花色（倒置）
│             4   │  ← 右下：數字（倒置）
└─────────────────┘
背景：#F5F5F0（米白）
花色顏色：♠♣ 黑色，♥♦ 紅色 (#FF3B4E)
```

**Stars 模式附加元素**
- 底部：5 顆星星（未評 #4A4A4A，已評 #FFD700）
- 右下角：金色位置標籤（圓角 4px，背景 #FFD700）

**Guess 模式附加元素**
- 卡片上方：下拉選單「Guess Track... ▼」

---

### 4.5 ChipButton

**3D 籌碼結構**
- 外圈直徑：100%（64px 標準）
- 中圈直徑：75%
- 內圈直徑：50%
- 條紋：12 條米白色裝飾

**顏色變體**

| 變體 | 外圈底色 | 用途 |
|------|----------|------|
| Red | #8B0000 → #CC0000 | Shuffle |
| Gold | #8B7300 → #DAA520 | Next Round |
| Black | #1A1A1A → #333333 | Reset |
| Blue | #1A4A7A → #2E6EB0 | Reveal |

---

### 4.6 ControlPanel

**佈局**
```
┌────────────────────────────────────────────┐
│  ♫ Current Tracks    4    # Rounds    1/4  │
│     ━━━●━━━━━━━━━━        ━━━━●━━━━━━━━━   │
├────────────────────────────────────────────┤
│  🔊 Auto Gain                      (🔴)    │
├────────────────────────────────────────────┤
│        [SHUFFLE]      [RESET]              │
└────────────────────────────────────────────┘
```

**按鈕顯示邏輯**

| 階段 | 顯示按鈕 |
|------|----------|
| Setup | SHUFFLE (紅) + RESET (黑) |
| BlindTesting | REVEAL (藍) + RESET (黑) |
| Revealed | NEXT ROUND (金) + RESET (黑) |

---

### 4.7 ResultsPanel

**Stars 模式 - 評分結果**
- Plugin 名稱 + 星星 + 平均分
- 由高到低排序

**Guess 模式 - 猜測結果**
- 進度：Round X of Y + N/M selected
- Card 列表 + Submit Guesses 按鈕

**Q&A 模式 - 問答結果**
- Score: X/Y correct + 百分比
- 每題正確（綠✓）/ 錯誤（紅✗）

---

### 4.8 QuestionBanner

**位置**：Mode Selector 下方，PokerTable 上方

**視覺規格**
- 寬度：與 PokerTable 同寬
- 高度：48px
- 背景：漸層 rgba(255,59,78,0.15)
- 邊框：1px solid rgba(255,59,78,0.3)

**內容格式**
```
❓ Which card is "Plugin A - Compressor"?    (1/4)
```

**Bug 修復**：現有前端缺少此問題提示，需補上英文版本。

---

## 5. 動畫系統

### 動畫參數

| 動畫類型 | 時長 | 曲線 | 用途 |
|---------|------|------|------|
| 快速過渡 | 150ms | EaseOut | 按鈕 hover、選中狀態 |
| 標準過渡 | 250ms | EaseOut | 面板切換、元素顯隱 |
| 強調動畫 | 400ms | EaseInOut | 翻牌、結果揭曉 |
| 循環動畫 | 1500ms | Linear | 脈動、呼吸燈 |

### 關鍵動畫

**翻牌動畫**
- 0-200ms：rotateY 0° → 90°
- 200ms：切換正反面
- 200-400ms：rotateY 90° → 180°
- 附加：上浮 8px + 金色閃光

**霓虹脈動**
- glow 半徑：10px ↔ 20px
- 週期：1500ms

**星星評分**
- 點擊：scale 1.0 → 1.3 → 1.0
- 填充：從左到右依序亮起

---

## 6. 主題系統

### ThemeManager 架構

```cpp
class ThemeManager : public ChangeBroadcaster {
    enum class Theme { Dark, Light };
    void setTheme(Theme theme);
    void toggleTheme();
    Colour getColour(ColourId id) const;
};
```

### 主題持久化
- 存到 PluginProcessor 的 state
- 插件載入時自動恢復用戶偏好

---

## 7. 實作計劃

### Phase 1：基礎架構
1. ThemeManager + ThemeColors.h
2. 新建 BlindCardEditor 骨架
3. 基本佈局框架

### Phase 2：核心元件
4. HeaderBar
5. ModeSelector
6. PokerTable
7. ChipStack

### Phase 3：卡片系統
8. PokerCard
9. StarRating
10. GuessDropdown
11. 整合卡片到 PokerTable

### Phase 4：控制面板
12. ChipButton
13. ControlPanel
14. ResultsPanel
15. QuestionBanner（含 Bug 修復）

### Phase 5：整合與打磨
16. 連接 BlindCardManager
17. 動畫系統調校
18. Light 主題完善
19. 響應式縮放測試
20. 跨平台測試

---

## 8. 驗收標準

- [ ] 所有現有功能 1:1 還原（Stars/Guess/Q&A 三模式）
- [ ] 視覺與設計稿一致（截圖對照）
- [ ] Light/Dark 主題切換正常
- [ ] 視窗可縮放，最小 900×600，最大 1400×900
- [ ] 2-8 張卡片佈局正確
- [ ] 翻牌動畫流暢 (60fps)
- [ ] macOS (Intel + Apple Silicon) 編譯通過
- [ ] Windows 編譯通過
- [ ] AU / VST3 格式驗證通過

---

## 9. 檔案結構

```
Source/
├── PluginProcessor.h/.cpp          (保留)
├── Core/
│   ├── Types.h                     (保留)
│   └── BlindCardManager.h/.cpp     (保留)
├── UI/
│   ├── Theme/
│   │   ├── ThemeManager.h/.cpp     (新建)
│   │   └── ThemeColors.h           (新建)
│   ├── Components/
│   │   ├── HeaderBar.h/.cpp        (新建)
│   │   ├── ModeSelector.h/.cpp     (新建)
│   │   ├── PokerTable.h/.cpp       (新建)
│   │   ├── PokerCard.h/.cpp        (新建)
│   │   ├── ChipButton.h/.cpp       (新建)
│   │   ├── ChipStack.h/.cpp        (新建)
│   │   ├── ControlPanel.h/.cpp     (新建)
│   │   ├── ResultsPanel.h/.cpp     (新建)
│   │   ├── GuessDropdown.h/.cpp    (新建)
│   │   ├── StarRating.h/.cpp       (新建)
│   │   └── QuestionBanner.h/.cpp   (新建)
│   ├── Animation/
│   │   └── AnimatedValue.h         (複用)
│   └── BlindCardEditor.h/.cpp      (重寫)
├── Assets/
│   └── 04-card-back.png            (從前端專案複製)
└── (刪除舊檔案)
    ├── PluginEditor.h/.cpp
    └── UI/CardComponent.h/.cpp
```

---

*Document generated: 2026-01-19*
*Approved by: CEO*

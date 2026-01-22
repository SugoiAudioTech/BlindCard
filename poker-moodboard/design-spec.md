# BlindCard 撲克牌主題設計規格

> 視覺總監：Vickey Hsieh
> 日期：2026-01-17
> 版本：v1.0

---

## 設計方向

### 核心概念
將 **德州撲克牌桌** 的視覺語言融入音訊插件 UI，創造獨特的品牌識別。

### 選定風格
**深炭黑牌桌** (`02_felt_dark.png`)
- 深沉的絨布質感背景
- 紅色霓虹邊緣光暈
- 金色作為揭曉/啟用的強調色
- 籌碼紋路作為按鈕設計元素

### 設計原則
1. **暗色優先** — 符合音訊軟體使用習慣，減少視覺疲勞
2. **克制的裝飾** — 撲克元素點綴而非堆砌，維持專業感
3. **功能導向** — 視覺服務於操作，不影響使用效率
4. **品牌一致** — 建立可延伸到其他產品的設計語言

---

## 色彩系統

### 背景層級

| Token | Hex | 用途 |
|-------|-----|------|
| `bg-base` | `#0A0A0A` | 最底層背景 |
| `bg-surface` | `#141414` | 面板、Control Bar |
| `bg-elevated` | `#1E1E1E` | 按鈕預設、浮動元件 |
| `bg-hover` | `#252525` | Hover 狀態 |

### 邊框

| Token | Hex | 用途 |
|-------|-----|------|
| `border-subtle` | `#1F1F1F` | 微弱分隔 |
| `border-default` | `#2A2A2A` | 一般邊框 |
| `border-strong` | `#3A3A3A` | 強調邊框 |

### 文字

| Token | Hex | 用途 |
|-------|-----|------|
| `text-primary` | `#F5F5F0` | 主要文字（米白） |
| `text-secondary` | `#A0A0A0` | 次要文字 |
| `text-muted` | `#606060` | 禁用/提示 |

### 強調色 — 紅色系 (Casino Red)

| Token | Hex | 用途 |
|-------|-----|------|
| `red-glow` | `#FF3B4E` | 霓虹發光 |
| `red-primary` | `#E63946` | 警告、Muted 狀態 |
| `red-dark` | `#B82D3A` | Pressed 狀態 |
| `red-subtle` | `#3D1A1E` | 紅色背景提示 |

### 強調色 — 金色系 (Reveal Gold)

| Token | Hex | 用途 |
|-------|-----|------|
| `gold-glow` | `#FFD700` | 發光效果 |
| `gold-primary` | `#D4AF37` | Reveal、啟用狀態 |
| `gold-dark` | `#B8962E` | Pressed 狀態 |
| `gold-subtle` | `#2D2815` | 金色背景提示 |

### 籌碼配色

| Token | Hex | 用途 |
|-------|-----|------|
| `chip-red` | `#8B2D3A` | 按鈕裝飾紋路 |
| `chip-white` | `#F5F5E8` | 按鈕裝飾紋路 |
| `chip-gold` | `#C9A84C` | 邊緣高光 |

---

## 按鈕設計

### StyledButton 狀態

```
┌─────────────────────────────────────────────────────────┐
│  DEFAULT                                                │
│  ┌───────────┐                                          │
│  │           │  背景：#1E1E1E (bg-elevated)             │
│  │  Button   │  邊框：#2A2A2A (border-default)          │
│  │           │  文字：#F5F5F0 (text-primary)            │
│  └───────────┘  圓角：6px                               │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  HOVER                                                  │
│  ┌───────────┐                                          │
│  │           │  背景：#252525 (bg-hover)                │
│  │  Button   │  邊框：#3A3A3A (border-strong)           │
│  │           │  文字：#F5F5F0                           │
│  └───────────┘  過渡：150ms ease-out                    │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  ACTIVE (啟用)                                          │
│  ┌───────────┐                                          │
│  │    ✦      │  背景：#2D2815 (gold-subtle)             │
│  │  Button   │  邊框：#D4AF37 (gold-primary)            │
│  │           │  文字：#D4AF37                           │
│  └───────────┘  發光：0 0 8px rgba(212,175,55,0.4)      │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  WARNING (警告/Muted)                                   │
│  ┌───────────┐                                          │
│  │    ⚠      │  背景：#3D1A1E (red-subtle)              │
│  │  Muted    │  邊框：#E63946 (red-primary)             │
│  │           │  文字：#F5F5F0                           │
│  └───────────┘  發光：0 0 8px rgba(230,57,70,0.4)       │
└─────────────────────────────────────────────────────────┘
```

### 籌碼紋路裝飾（選用）

對於圓形按鈕或 Toggle，可加入籌碼邊緣紋路：

```
       ╭──────────────╮
      ╱ ▪   ▪   ▪   ▪ ╲
     │                  │
     │     BUTTON       │    ▪ = 籌碼邊緣凹槽
     │                  │    顏色交替：chip-red / chip-white
      ╲ ▪   ▪   ▪   ▪ ╱
       ╰──────────────╯
```

---

## ModeToggle 設計

### 結構

```
┌─────────────────────────────────────────┐
│  ┌─────────┐┌─────────┐┌─────────┐      │
│  │  Mode A ││  Mode B ││  Mode C │      │
│  └─────────┘└─────────┘└─────────┘      │
│  ════════════                           │  ← 滑動指示器
└─────────────────────────────────────────┘
```

### 狀態

| 狀態 | 背景 | 文字 | 指示器 |
|------|------|------|--------|
| 未選中 | 透明 | `text-secondary` | — |
| Hover | `bg-hover` | `text-primary` | — |
| 選中 | — | `gold-primary` | `gold-primary` 底線 |

### 動畫
- 指示器滑動：300ms ease-out
- 文字顏色過渡：150ms

---

## Reveal 按鈕設計

### 概念
Reveal = 「亮牌」時刻，是 BlindCard 的核心互動。需要有戲劇性的視覺回饋。

### 狀態流程

```
  ┌──────────┐      點擊      ┌──────────┐      完成      ┌──────────┐
  │  HIDDEN  │  ──────────▶  │ REVEALING│  ──────────▶  │ REVEALED │
  │  (待揭)  │               │  (揭曉中) │               │  (已揭)  │
  └──────────┘               └──────────┘               └──────────┘
       │                          │                          │
       ▼                          ▼                          ▼
   暗色按鈕               金色光芒擴散               金色恆亮狀態
   金色邊框               粒子效果                   滿意的回饋感
```

### HIDDEN 狀態（待揭曉）

```css
background: #1E1E1E;
border: 1px solid #D4AF37;
color: #D4AF37;
box-shadow: 0 0 4px rgba(212, 175, 55, 0.2);
```

### REVEALING 狀態（揭曉中）

```css
/* 背景漸變 */
background: radial-gradient(
  circle at center,
  #D4AF37 0%,
  #2D2815 60%,
  #1E1E1E 100%
);

/* 發光擴散 */
box-shadow:
  0 0 20px rgba(212, 175, 55, 0.6),
  0 0 40px rgba(212, 175, 55, 0.4),
  0 0 60px rgba(212, 175, 55, 0.2);

/* 動畫 */
animation: reveal-pulse 600ms ease-out;
```

### REVEALED 狀態（已揭曉）

```css
background: #2D2815;
border: 1px solid #D4AF37;
color: #D4AF37;
box-shadow: 0 0 12px rgba(212, 175, 55, 0.5);
```

### 動畫 Keyframes

```css
@keyframes reveal-pulse {
  0% {
    transform: scale(1);
    box-shadow: 0 0 0 rgba(212, 175, 55, 0);
  }
  50% {
    transform: scale(1.05);
    box-shadow:
      0 0 30px rgba(212, 175, 55, 0.8),
      0 0 60px rgba(212, 175, 55, 0.4);
  }
  100% {
    transform: scale(1);
    box-shadow: 0 0 12px rgba(212, 175, 55, 0.5);
  }
}
```

---

## 背景質感

### 牌桌絨布漸層

```css
background:
  radial-gradient(
    ellipse 120% 100% at 50% 50%,
    #1A1A1A 0%,
    #0F0F0F 50%,
    #0A0A0A 100%
  );
```

### 絨布紋理（選用）

可疊加細微 noise texture 增加質感：

```css
background-image:
  url('felt-texture.png'),
  radial-gradient(...);
background-blend-mode: overlay;
opacity: 0.03; /* 非常微弱 */
```

### 紅色邊緣光暈（Control Bar 邊框）

```css
border-top: 1px solid #E63946;
box-shadow:
  0 -1px 10px rgba(230, 57, 70, 0.3),
  inset 0 1px 20px rgba(230, 57, 70, 0.05);
```

---

## 元件對應總表

| UI 元件 | 背景 | 邊框 | 文字 | 強調效果 |
|---------|------|------|------|----------|
| 主背景 | `bg-base` | — | — | — |
| Control Bar | `bg-surface` | `red-primary` 頂邊 | — | 紅色光暈 |
| 一般按鈕 | `bg-elevated` | `border-default` | `text-primary` | — |
| 一般按鈕 Hover | `bg-hover` | `border-strong` | `text-primary` | — |
| 啟用按鈕 | `gold-subtle` | `gold-primary` | `gold-primary` | 金色發光 |
| 警告/Muted | `red-subtle` | `red-primary` | `text-primary` | 紅色發光 |
| Reveal 按鈕 | `bg-elevated` | `gold-primary` | `gold-primary` | 金色脈衝 |
| ModeToggle 選中 | — | — | `gold-primary` | 底線滑動 |
| 輸入欄位 | `bg-surface` | `border-default` | `text-primary` | — |

---

## JUCE 實作參考

### ControlBarColors.h 建議

```cpp
namespace PokerTheme
{
    // Background
    const Colour bgBase        { 0xFF0A0A0A };
    const Colour bgSurface     { 0xFF141414 };
    const Colour bgElevated    { 0xFF1E1E1E };
    const Colour bgHover       { 0xFF252525 };

    // Border
    const Colour borderSubtle  { 0xFF1F1F1F };
    const Colour borderDefault { 0xFF2A2A2A };
    const Colour borderStrong  { 0xFF3A3A3A };

    // Text
    const Colour textPrimary   { 0xFFF5F5F0 };
    const Colour textSecondary { 0xFFA0A0A0 };
    const Colour textMuted     { 0xFF606060 };

    // Red Accent
    const Colour redGlow       { 0xFFFF3B4E };
    const Colour redPrimary    { 0xFFE63946 };
    const Colour redDark       { 0xFFB82D3A };
    const Colour redSubtle     { 0xFF3D1A1E };

    // Gold Accent
    const Colour goldGlow      { 0xFFFFD700 };
    const Colour goldPrimary   { 0xFFD4AF37 };
    const Colour goldDark      { 0xFFB8962E };
    const Colour goldSubtle    { 0xFF2D2815 };

    // Chip
    const Colour chipRed       { 0xFF8B2D3A };
    const Colour chipWhite     { 0xFFF5F5E8 };
    const Colour chipGold      { 0xFFC9A84C };
}
```

---

## 參考圖片索引

| 檔案 | 用途 |
|------|------|
| `02_felt_dark.png` | 背景風格參考（選定） |
| `03_chips.png` | 籌碼配色參考 |
| `04_card_back.png` | 撲克牌花紋參考 |
| `07_reveal_glow.png` | Reveal 金光概念 |
| `08_button_states.png` | 按鈕狀態參考 |
| `09_toggle_switch.png` | Toggle 設計參考 |
| `10_reveal_sequence.png` | Reveal 動畫序列 |
| `11_reveal_button.png` | Reveal 按鈕效果 |

---

*此文件由 Sugoi Audio 視覺總監製作，供開發團隊實作參考。*

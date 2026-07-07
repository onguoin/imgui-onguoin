// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "imgui.h"

namespace imgui_onguoin {

struct Palette {
    ImVec4 background;
    ImVec4 sidebar;
    ImVec4 panel;
    ImVec4 surface;
    ImVec4 surfaceRaised;
    ImVec4 accent;
    ImVec4 accentMuted;
    ImVec4 text;
    ImVec4 textMuted;
    ImVec4 success;
    ImVec4 warning;
    ImVec4 danger;
    ImVec4 border;
};

inline constexpr Palette kDefaultPalette{
    ImVec4(0.022f, 0.026f, 0.034f, 1.0f),
    ImVec4(0.016f, 0.019f, 0.026f, 1.0f),
    ImVec4(0.028f, 0.032f, 0.042f, 1.0f),
    ImVec4(0.036f, 0.041f, 0.053f, 1.0f),
    ImVec4(0.060f, 0.070f, 0.095f, 1.0f),
    ImVec4(0.020f, 0.890f, 0.980f, 1.0f),
    ImVec4(0.050f, 0.320f, 0.390f, 1.0f),
    ImVec4(0.955f, 0.965f, 0.985f, 1.0f),
    ImVec4(0.500f, 0.545f, 0.615f, 1.0f),
    ImVec4(0.250f, 0.850f, 0.540f, 1.0f),
    ImVec4(1.000f, 0.650f, 0.250f, 1.0f),
    ImVec4(0.900f, 0.240f, 0.300f, 1.0f),
    ImVec4(0.730f, 0.900f, 1.000f, 0.090f),
};

constexpr const Palette& default_palette() {
    return kDefaultPalette;
}

enum class BackgroundKind {
    Aurora,
    GradientBands,
    Grid,
    Flat,
    Starfield,
};

enum class ThemePreset {
    Onguoin,
    Graphite,
    Spectrum,
};

enum class ThemeFlavor {
    Onguoin,
    Graphite,
    Spectrum,
};

enum class UiTextLanguage {
    English,
    ChineseSimplified,
};

enum class BackgroundComplexity {
    Minimal = 0,
    Balanced,
    Rich,
};

enum class BackgroundMotionCharacter {
    Static = 0,
    Gentle,
    Expressive,
};

enum class BackgroundIntensity {
    Subtle = 0,
    Balanced,
    Vivid,
};

enum class EasingCurve {
    Linear,
    InSine,
    OutSine,
    InOutSine,
    OutCubic,
    InOutCubic,
};

struct PulseMotionStyle {
    float speed = 0.0f;
    float minimumValue = 0.0f;
    float maximumValue = 1.0f;
    float phase = 0.0f;
    EasingCurve curve = EasingCurve::Linear;
};

struct FollowMotionStyle {
    float speed = 0.0f;
    float snapDistance = 0.0f;
    EasingCurve curve = EasingCurve::Linear;
};

struct ProgressMotionStyle {
    float speed = 0.0f;
    float snapDistance = 0.0f;
    EasingCurve curve = EasingCurve::Linear;
};

struct PresenceMotionStyle {
    ProgressMotionStyle enter{};
    ProgressMotionStyle exit{};
};

struct TravelMotionStyle {
    float speed = 0.0f;
    float phase = 0.0f;
};

enum class NoticeTone {
    Info,
    Success,
    Warning,
    Danger,
};

} // namespace imgui_onguoin

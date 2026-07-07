// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_core_types.h"

namespace imgui_onguoin {

enum class BackgroundLayerKind {
    Gradient = 0,
    Ambient,
    Beam,
    Grid,
    Guides,
    StarField,
    CursorAura,
};

using BackgroundLayerMask = unsigned int;

inline constexpr BackgroundLayerMask background_layer_flag(BackgroundLayerKind kind) {
    return 1u << static_cast<unsigned int>(kind);
}

inline constexpr bool background_layer_mask_has(BackgroundLayerMask mask, BackgroundLayerKind kind) {
    return (mask & background_layer_flag(kind)) != 0u;
}

inline constexpr int background_layer_mask_count(BackgroundLayerMask mask) {
    int count = 0;
    while (mask != 0u) {
        count += (mask & 1u) != 0u ? 1 : 0;
        mask >>= 1u;
    }
    return count;
}

struct BackgroundLayerInfo {
    BackgroundLayerKind kind = BackgroundLayerKind::Gradient;
    const char* id = "";
    const char* englishLabel = "";
    const char* chineseLabel = "";
};

struct BackgroundGradientLayer {
    bool enabled = true;
    int bandCount = 28;
    ImVec4 topColor{};
    ImVec4 midColor{};
    ImVec4 bottomColor{};
};

struct BackgroundGlowSpot {
    ImVec2 anchor{};
    float radius = 0.0f;
    float alpha = 0.0f;
    float authenticatedBoostScale = 0.0f;
};

struct BackgroundAmbientLayer {
    bool enabled = true;
    BackgroundGlowSpot primary{};
    BackgroundGlowSpot secondary{};
    BackgroundGlowSpot tertiary{};
};

struct BackgroundBeamLayer {
    bool enabled = true;
    float width = 180.0f;
    float shear = 260.0f;
    float overflow = 80.0f;
    float travelPadding = 520.0f;
    float alpha = 0.027f;
    float unauthenticatedAlphaScale = 0.72f;
    TravelMotionStyle sweepMotion{18.0f, 0.0f};
};

struct BackgroundGridLayer {
    bool enabled = false;
    float spacing = 84.0f;
    float alpha = 0.032f;
};

struct BackgroundGuideLayer {
    bool enabled = true;
    int lineCount = 4;
    float alpha = 0.022f;
    float falloffPerLine = 0.003f;
    float topStart = 0.16f;
    float verticalStep = 0.14f;
    float horizontalInset = 28.0f;
    float authenticatedAlphaScale = 0.12f;
};

struct BackgroundStarFieldLayer {
    bool enabled = false;
    int pointCount = 64;
    float minRadius = 0.7f;
    float maxRadius = 1.6f;
    float alpha = 0.12f;
    PulseMotionStyle twinkleMotion{0.90f, 0.82f, 1.18f, 0.0f, EasingCurve::InOutSine};
    float authenticatedAlphaScale = 0.16f;
};

struct BackgroundStyle {
    BackgroundKind kind = BackgroundKind::Aurora;
    float authenticatedAccentBoost = 0.03f;
    BackgroundGradientLayer gradient{};
    BackgroundAmbientLayer ambient{};
    BackgroundBeamLayer beam{};
    BackgroundGridLayer grid{};
    BackgroundGuideLayer guides{};
    BackgroundStarFieldLayer starField{};
};

struct BackgroundTuning {
    BackgroundIntensity density = BackgroundIntensity::Balanced;
    BackgroundIntensity motion = BackgroundIntensity::Balanced;
};

} // namespace imgui_onguoin

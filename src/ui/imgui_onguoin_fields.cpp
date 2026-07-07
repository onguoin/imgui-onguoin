// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

namespace imgui_onguoin {

namespace {

struct SliderSpectralState {
    float expansionProgress = 0.0f;
    float lastProgress = -1.0f;
    bool pixelBloomActive = false;
};

struct SliderMagneticState {
    int lastValue = 0;
    int lockedTick = 0;
    bool initialized = false;
    bool locked = false;
};

struct SliderTrackGeometry {
    ImVec2 min{};
    ImVec2 max{};
    float width = 1.0f;
    float height = 1.0f;
    float rounding = 0.0f;
};

std::unordered_map<ImGuiID, SliderSpectralState>& slider_spectral_cache() {
    static std::unordered_map<ImGuiID, SliderSpectralState> cache;
    return cache;
}

std::unordered_map<ImGuiID, SliderMagneticState>& slider_magnetic_cache() {
    static std::unordered_map<ImGuiID, SliderMagneticState> cache;
    return cache;
}

float resolve_slider_width(const Theme& theme, SliderFieldStyle style) {
    return style.sliderWidth > 0.0f ? style.sliderWidth : theme.fields.sliderWidth;
}

float resolve_input_width(const Theme& theme, SliderFieldStyle style) {
    return style.inputWidth > 0.0f ? style.inputWidth : theme.fields.valueInputWidth;
}

float resolve_frame_rounding(const Theme& theme, SliderFieldStyle style) {
    return style.frameRounding > 0.0f ? style.frameRounding : theme.fields.frameRounding;
}

SliderTrackGeometry make_slider_track_geometry(ImVec2 min, ImVec2 max, float requestedRounding) {
    SliderTrackGeometry geometry;
    geometry.min = min;
    geometry.max = max;
    geometry.width = std::max(1.0f, max.x - min.x);
    geometry.height = std::max(1.0f, max.y - min.y);
    geometry.rounding = std::clamp(requestedRounding,
                                   0.0f,
                                   std::min(geometry.width, geometry.height) * 0.5f);
    return geometry;
}

SliderTrackGeometry make_slider_track_geometry(ImVec2 min,
                                               ImVec2 max,
                                               const Theme& theme,
                                               SliderFieldStyle style) {
    return make_slider_track_geometry(min, max, resolve_frame_rounding(theme, style) * current_scale());
}

float slider_track_rounded_inset_at_y(const SliderTrackGeometry& geometry, float y) {
    const float radius = std::max(0.0f, geometry.rounding);
    if (radius <= 0.0f) {
        return 0.0f;
    }

    const float topCenter = geometry.min.y + radius;
    const float bottomCenter = geometry.max.y - radius;
    float dy = 0.0f;
    if (y < topCenter) {
        dy = topCenter - y;
    } else if (y > bottomCenter) {
        dy = y - bottomCenter;
    }
    if (dy <= 0.0f) {
        return 0.0f;
    }
    return radius - std::sqrt(std::max(0.0f, radius * radius - dy * dy));
}

void draw_slider_track_background(ImDrawList* drawList,
                                  const SliderTrackGeometry& geometry,
                                  ImVec4 color) {
    drawList->AddRectFilled(geometry.min, geometry.max, to_u32(color), geometry.rounding);
}

float slider_progress_from_value(int value, int minValue, int maxValue) {
    const float range = static_cast<float>(std::max(1, maxValue - minValue));
    return clamp01((static_cast<float>(value) - static_cast<float>(minValue)) / range);
}

float slider_x_from_progress(const SliderTrackGeometry& geometry, float progress) {
    const float pad = std::max(geometry.height * 0.50f, 1.0f);
    return lerp(geometry.min.x + pad, geometry.max.x - pad, clamp01(progress));
}

int slider_value_from_mouse_x(float mouseX, const SliderTrackGeometry& geometry, int minValue, int maxValue) {
    const float pad = std::max(geometry.height * 0.50f, 1.0f);
    const float progress = clamp01((mouseX - (geometry.min.x + pad)) / std::max(1.0f, geometry.width - pad * 2.0f));
    const float value = static_cast<float>(minValue) + progress * static_cast<float>(maxValue - minValue);
    return std::clamp(static_cast<int>(std::lround(value)), minValue, maxValue);
}

int apply_slider_magnetic_resistance(const SliderFieldStyle& style,
                                     ImGuiID id,
                                     int rawValue,
                                     int minValue,
                                     int maxValue) {
    if (style.tickValues == nullptr ||
        style.tickValueCount <= 0 ||
        style.magneticSnapResistance <= 0.0f ||
        style.magneticSnapWindow <= 0) {
        return rawValue;
    }

    SliderMagneticState& state = slider_magnetic_cache()[id];
    if (!state.initialized) {
        state.lastValue = rawValue;
        state.lockedTick = rawValue;
        state.locked = false;
        state.initialized = true;
    }

    const int window = std::max(1, style.magneticSnapWindow);
    const float resistance = std::clamp(style.magneticSnapResistance, 0.0f, 0.96f);
    const int releaseWindow = window + std::max(1, static_cast<int>(std::lround(2.0f + resistance * 4.0f)));

    if (state.locked) {
        const int lockedTick = std::clamp(state.lockedTick, minValue, maxValue);
        if (std::abs(rawValue - lockedTick) <= releaseWindow) {
            state.lastValue = lockedTick;
            return lockedTick;
        }
        state.locked = false;
    }

    int adjusted = rawValue;
    const int lastValue = std::clamp(state.lastValue, minValue, maxValue);
    for (int i = 0; i < style.tickValueCount; ++i) {
        const int tick = std::clamp(style.tickValues[i], minValue, maxValue);
        const bool crossingTick =
            (lastValue < tick && rawValue >= tick) ||
            (lastValue > tick && rawValue <= tick) ||
            rawValue == tick;
        if (!crossingTick) {
            continue;
        }

        const int distance = std::abs(rawValue - tick);
        if (distance > window) {
            continue;
        }

        // High resistance means a real detent: hold at the tick until the mouse
        // moves past the release window. Lower values still behave like a soft pull.
        if (resistance >= 0.70f || distance <= 1) {
            adjusted = tick;
            state.locked = true;
            state.lockedTick = tick;
        } else {
            adjusted = static_cast<int>(std::lround(lerp(static_cast<float>(rawValue),
                                                         static_cast<float>(tick),
                                                         resistance)));
        }
        break;
    }

    adjusted = std::clamp(adjusted, minValue, maxValue);
    state.lastValue = adjusted;
    return adjusted;
}

void draw_slider_decor(ImVec2 min,
                       ImVec2 max,
                       bool hovered,
                       bool active,
                       const Theme& theme,
                       SliderFieldStyle sliderStyle) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const SliderTrackGeometry geometry = make_slider_track_geometry(min, max, theme, sliderStyle);
    InteractionVisualState state;
    state.hovered = hovered;
    state.active = active;

    AccentFrameStyle frameStyle;
    frameStyle.rounding = geometry.rounding;
    frameStyle.idleBorderAlpha = 0.11f;
    frameStyle.hoveredBorderAlpha = 0.25f;
    frameStyle.activeBorderAlpha = 0.46f;
    frameStyle.idleGlowAlpha = 0.105f;
    frameStyle.hoveredGlowAlpha = 0.20f;
    frameStyle.activeGlowAlpha = 0.36f;
    frameStyle.idleThickness = 1.0f;
    frameStyle.activeThickness = 1.6f;

    draw_accent_frame(drawList, geometry.min, geometry.max, theme, state, frameStyle);
}

void draw_slider_custom_border(ImVec2 min,
                               ImVec2 max,
                               bool hovered,
                               bool active,
                               const Theme& theme,
                               SliderFieldStyle sliderStyle) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const SliderTrackGeometry geometry = make_slider_track_geometry(min, max, theme, sliderStyle);
    InteractionVisualState state;
    state.hovered = hovered;
    state.active = active;

    AccentFrameStyle frameStyle;
    frameStyle.rounding = geometry.rounding;
    frameStyle.idleBorderAlpha = 0.42f;
    frameStyle.hoveredBorderAlpha = 0.62f;
    frameStyle.activeBorderAlpha = 0.86f;
    frameStyle.idleGlowAlpha = 0.0f;
    frameStyle.hoveredGlowAlpha = 0.0f;
    frameStyle.activeGlowAlpha = 0.0f;
    frameStyle.idleThickness = 1.05f;
    frameStyle.activeThickness = 1.35f;

    draw_accent_frame(drawList, geometry.min, geometry.max, theme, state, frameStyle);
}

void draw_slider_tick_marks(ImDrawList* drawList,
                            const SliderTrackGeometry& geometry,
                            int minValue,
                            int maxValue,
                            const Theme& theme,
                            const SliderFieldStyle& style) {
    if (style.tickValues == nullptr || style.tickValueCount <= 0) {
        return;
    }

    const float scale = current_scale();
    const float centerY = (geometry.min.y + geometry.max.y) * 0.5f;
    const float radius = style.tickRadius > 0.0f
        ? style.tickRadius * scale
        : std::max(1.2f * scale, geometry.height * 0.105f);
    for (int i = 0; i < style.tickValueCount; ++i) {
        const int tick = std::clamp(style.tickValues[i], minValue, maxValue);
        const float progress = slider_progress_from_value(tick, minValue, maxValue);
        const float x = slider_x_from_progress(geometry, progress);
        if (style.tickGlowScale > 0.0f) {
            ImVec4 glow = style.tickGlowColor.w >= 0.0f ? style.tickGlowColor : style.spectralColorC;
            glow.w = std::clamp(glow.w >= 0.0f ? glow.w : 0.16f, 0.0f, 1.0f);
            if (glow.w > 0.0f) {
                drawList->AddCircleFilled(ImVec2(x, centerY),
                                          radius * std::max(1.0f, 2.1f * style.tickGlowScale),
                                          to_u32(glow),
                                          18);
            }
        }
        drawList->AddCircleFilled(ImVec2(x, centerY),
                                  radius,
                                  to_u32(style.tickColor),
                                  18);
    }
}

void draw_slider_custom_handle(ImDrawList* drawList,
                               const SliderTrackGeometry& geometry,
                               int value,
                               int minValue,
                               int maxValue,
                               bool active,
                               const Theme& theme,
                               const SliderFieldStyle& style) {
    const float scale = current_scale();
    const float progress = slider_progress_from_value(value, minValue, maxValue);
    const float centerX = slider_x_from_progress(geometry, progress);
    const float centerY = (geometry.min.y + geometry.max.y) * 0.5f;
    const float radius = style.handleRadius > 0.0f
        ? style.handleRadius * scale
        : geometry.height * (style.handleShape == SliderFieldHandleShape::Circle ? 0.35f : 0.43f);
    const float resolvedRadius = std::min(radius, geometry.height * 0.44f);

    if (style.handleGlowScale > 0.0f) {
        ImVec4 outerGlow = style.handleGlowColor.w >= 0.0f ? style.handleGlowColor : style.spectralColorC;
        outerGlow.w = std::clamp(outerGlow.w >= 0.0f ? outerGlow.w : (active ? 0.24f : 0.14f), 0.0f, 1.0f);
        if (outerGlow.w > 0.0f) {
            drawList->AddCircleFilled(ImVec2(centerX, centerY),
                                      resolvedRadius * std::max(1.0f, 2.05f * style.handleGlowScale),
                                      to_u32(outerGlow),
                                      32);
        }
    }

    ImVec4 shadow(0.0f, 0.0f, 0.0f, active ? 0.34f : 0.26f);
    drawList->AddCircleFilled(ImVec2(centerX + 0.8f * scale, centerY + 1.0f * scale),
                              resolvedRadius * 1.10f,
                              to_u32(shadow),
                              32);

    ImVec4 fill = style.handleFillColor.w >= 0.0f
        ? style.handleFillColor
        : (active ? ImVec4(0.92f, 0.98f, 1.0f, 1.0f) : ImVec4(0.82f, 0.90f, 0.96f, 1.0f));
    if (active && style.handleFillColor.w >= 0.0f) {
        fill = mix_color(fill, ImVec4(1.0f, 1.0f, 1.0f, fill.w), 0.14f);
    }
    const ImVec4 border = style.handleBorderColor.w >= 0.0f
        ? style.handleBorderColor
        : ImVec4(theme.palette.accent.x, theme.palette.accent.y, theme.palette.accent.z, 0.68f);
    drawList->AddCircleFilled(ImVec2(centerX, centerY), resolvedRadius, to_u32(fill), 32);
    drawList->AddCircle(ImVec2(centerX, centerY),
                        resolvedRadius,
                        to_u32(border),
                        32,
                        std::max(1.0f, 1.0f * scale));
}

bool draw_custom_int_slider(const char* id,
                            int& value,
                            int minValue,
                            int maxValue,
                            const Theme& theme,
                            SliderFieldStyle style,
                            ImVec2* outMin,
                            ImVec2* outMax,
                            bool* outHovered,
                            bool* outActive) {
    const float scale = current_scale();
    const ImVec2 sliderSize(resolve_slider_width(theme, style) * scale, ImGui::GetFrameHeight());
    ImGui::InvisibleButton(id, sliderSize);
    const ImVec2 sliderMin = ImGui::GetItemRectMin();
    const ImVec2 sliderMax = ImGui::GetItemRectMax();
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();
    bool changed = false;

    const SliderTrackGeometry geometry = make_slider_track_geometry(sliderMin, sliderMax, theme, style);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    draw_slider_track_background(drawList, geometry, theme.fields.frameBg);

    if (active) {
        const int rawValue = slider_value_from_mouse_x(ImGui::GetIO().MousePos.x, geometry, minValue, maxValue);
        const int nextValue = apply_slider_magnetic_resistance(style, ImGui::GetID(id), rawValue, minValue, maxValue);
        if (nextValue != value) {
            value = nextValue;
            changed = true;
        }
    } else {
        SliderMagneticState& state = slider_magnetic_cache()[ImGui::GetID(id)];
        state.lastValue = value;
        state.initialized = true;
    }

    if (outMin != nullptr) {
        *outMin = sliderMin;
    }
    if (outMax != nullptr) {
        *outMax = sliderMax;
    }
    if (outHovered != nullptr) {
        *outHovered = hovered;
    }
    if (outActive != nullptr) {
        *outActive = active;
    }

    return changed;
}

bool slider_spectral_has_text(const SliderFieldStyle& style) {
    return style.spectralText != nullptr && style.spectralText[0] != '\0';
}

ImU32 rainbow_slider_color(float seed, float alpha) {
    float hue = std::fmod(seed, 1.0f);
    if (hue < 0.0f) {
        hue += 1.0f;
    }
    return ImColor::HSV(hue, 0.78f, 1.0f, std::clamp(alpha, 0.0f, 1.0f));
}

void draw_slider_sheen_text(ImDrawList* drawList,
                            ImVec2 min,
                            ImVec2 max,
                            const char* text,
                            float intensity,
                            const Theme& theme) {
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    const float scale = current_scale();
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const ImVec2 textPos = centered_text_pos(min, max, textSize, ImVec2(0.0f, -0.5f * scale));
    const ImVec4 base = mix_color(ImVec4(0.58f, 0.30f, 1.0f, 0.92f),
                                  theme.palette.accent,
                                  0.18f);
    drawList->AddText(textPos, to_u32(base), text);

    const float time = static_cast<float>(ImGui::GetTime());
    const float travelSpan = textSize.x + 48.0f * scale;
    const float phase = std::fmod(time * 0.78f, 1.0f);
    const float bandCenter = textPos.x - 24.0f * scale + phase * travelSpan;
    const float softBand = 24.0f * scale;
    const float hardBand = 9.0f * scale;

    drawList->PushClipRect(ImVec2(bandCenter - softBand, textPos.y - 3.0f * scale),
                           ImVec2(bandCenter + softBand, textPos.y + textSize.y + 3.0f * scale),
                           true);
    drawList->AddText(textPos,
                      to_u32(ImVec4(0.54f, 0.95f, 1.0f, 0.22f * intensity)),
                      text);
    drawList->PopClipRect();

    drawList->PushClipRect(ImVec2(bandCenter - hardBand, textPos.y - 3.0f * scale),
                           ImVec2(bandCenter + hardBand, textPos.y + textSize.y + 3.0f * scale),
                           true);
    drawList->AddText(textPos,
                      to_u32(ImVec4(1.0f, 0.97f, 1.0f, 0.82f * intensity)),
                      text);
    drawList->PopClipRect();
}

void draw_slider_rainbow_text(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const char* text,
                              float intensity) {
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    const float scale = current_scale();
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 pos = centered_text_pos(min, max, textSize, ImVec2(0.0f, -0.5f * scale));
    const float time = static_cast<float>(ImGui::GetTime());
    const float pulseAlpha = 0.80f + 0.16f * std::sin(time * 2.4f);
    const float alpha = std::clamp(pulseAlpha * intensity, 0.0f, 1.0f);

    drawList->AddText(ImVec2(pos.x + 0.7f * scale, pos.y + 0.7f * scale),
                      to_u32(ImVec4(0.18f, 0.04f, 0.26f, 0.55f * alpha)),
                      text);

    float cursorX = pos.x;
    for (int i = 0; text[i] != '\0'; ++i) {
        if ((static_cast<unsigned char>(text[i]) & 0x80u) != 0u) {
            drawList->AddText(pos, rainbow_slider_color(time * 0.10f, alpha), text);
            return;
        }

        char glyph[2] = {text[i], '\0'};
        const ImVec2 glyphSize = ImGui::CalcTextSize(glyph);
        const float hueSeed = 0.78f + static_cast<float>(i) * 0.105f + time * 0.115f;
        drawList->AddText(ImVec2(cursorX, pos.y), rainbow_slider_color(hueSeed, alpha), glyph);
        cursorX += glyphSize.x;
    }
}

void draw_slider_value_text(ImDrawList* drawList,
                            const SliderTrackGeometry& geometry,
                            const char* text,
                            const Theme& theme,
                            const SliderFieldStyle& style) {
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    const float scale = current_scale();
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const ImVec2 textPos = centered_text_pos(geometry.min, geometry.max, textSize, ImVec2(0.0f, -0.5f * scale));
    const ImVec4 textColor = style.valueTextColor.w >= 0.0f
        ? style.valueTextColor
        : ImVec4(theme.palette.text.x, theme.palette.text.y, theme.palette.text.z, 0.92f);
    const ImVec4 shadowColor = style.valueTextShadowColor.w >= 0.0f
        ? style.valueTextShadowColor
        : ImVec4(0.0f, 0.0f, 0.0f, 0.38f);

    if (shadowColor.w > 0.0f) {
        drawList->AddText(ImVec2(textPos.x + 0.75f * scale, textPos.y + 0.75f * scale),
                          to_u32(shadowColor),
                          text);
    }
    drawList->AddText(textPos, to_u32(textColor), text);
}

void draw_slider_pixel_bloom(ImDrawList* drawList,
                             const SliderTrackGeometry& geometry,
                             float progress,
                             float expansionProgress,
                             float intensity,
                             const SliderFieldStyle& style) {
    const float scale = current_scale();
    const ImVec2 min = geometry.min;
    const ImVec2 max = geometry.max;
    const float width = geometry.width;
    const float height = geometry.height;
    const float time = static_cast<float>(ImGui::GetTime());
    const float centerX = lerp(min.x + 9.0f * scale, max.x - 9.0f * scale, clamp01(progress));
    const float centerY = (min.y + max.y) * 0.5f;
    const float radialSpanPx = std::max(1.0f,
                                        std::max(std::hypot(centerX - min.x, height * 0.5f),
                                                 std::hypot(max.x - centerX, height * 0.5f)));
    const float front = ease_out_cubic(clamp01(expansionProgress));
    const float initialWaveAlpha = 1.0f - clamp01((expansionProgress - 0.88f) / 0.12f);
    const float rippleCycle = std::fmod(time * 0.38f, 1.0f);
    const float cell = std::max(3.0f * scale, std::floor(4.0f * scale));
    const int columns = std::max(1, static_cast<int>(std::ceil(width / cell)));
    const int rows = std::max(1, static_cast<int>(std::ceil(height / cell)));
    const auto waveStrength = [](float arc, float center, float waveWidth) {
        return clamp01(1.0f - std::fabs(arc - center) / std::max(0.001f, waveWidth));
    };
    auto with_alpha = [](ImVec4 color, float alpha) {
        color.w = alpha;
        return color;
    };
    const ImVec4 baseColor = style.spectralColorA;
    const ImVec4 midColor = style.spectralColorB;
    const ImVec4 highColor = style.spectralColorC;

    drawList->PushClipRect(min, max, true);
    draw_slider_track_background(drawList,
                                 geometry,
                                 with_alpha(baseColor, std::clamp(0.82f * intensity, 0.0f, 0.92f)));

    for (int y = 0; y < rows; ++y) {
        const float cellMinY = min.y + static_cast<float>(y) * cell;
        const float cellMaxY = std::min(cellMinY + cell + 0.15f * scale, max.y);
        const float cy = cellMinY + cell * 0.5f;
        const float roundedInset = slider_track_rounded_inset_at_y(geometry, cy);
        const float rowMinX = min.x + roundedInset;
        const float rowMaxX = max.x - roundedInset;
        for (int x = 0; x < columns; ++x) {
            const float cellMinX = min.x + static_cast<float>(x) * cell;
            const float cellMaxX = std::min(cellMinX + cell + 0.15f * scale, max.x);
            const float cx = cellMinX + cell * 0.5f;
            const float drawMinX = std::max(cellMinX, rowMinX);
            const float drawMaxX = std::min(cellMaxX, rowMaxX);
            if (drawMaxX <= drawMinX) {
                continue;
            }
            const float dx = cx - centerX;
            const float dy = cy - centerY;
            const float expansionArc = std::min(1.0f, std::sqrt(dx * dx + dy * dy) / radialSpanPx);
            const float loopArc = expansionArc;
            const float invasionWidth = 0.34f;
            const float invaded = clamp01((front - expansionArc + invasionWidth) / invasionWidth);

            const float leadBand = waveStrength(expansionArc, front, 0.085f) * initialWaveAlpha;
            const float wakeA = waveStrength(expansionArc, std::max(0.0f, front - 0.155f), 0.120f) * initialWaveAlpha;
            const float wakeB = waveStrength(expansionArc, std::max(0.0f, front - 0.300f), 0.155f) * initialWaveAlpha;
            const float loopA = waveStrength(loopArc, rippleCycle, 0.105f);
            const float loopB = waveStrength(loopArc, std::fmod(rippleCycle + 0.31f, 1.0f), 0.130f);
            const float loopC = waveStrength(loopArc, std::fmod(rippleCycle + 0.62f, 1.0f), 0.150f);

            // Low-res sampled color field: pixel feel without drawing any borders or gaps.
            const float grain = 0.985f + 0.015f * std::sin(static_cast<float>(x) * 0.93f + static_cast<float>(y) * 1.37f);
            const float alpha = (0.30f +
                                 0.24f * invaded +
                                 0.62f * leadBand +
                                 0.32f * wakeA +
                                 0.20f * wakeB +
                                 0.20f * loopA +
                                 0.16f * loopB +
                                 0.13f * loopC) *
                                grain *
                                intensity;

            const float midMix = clamp01(0.18f +
                                         0.42f * invaded +
                                         0.32f * wakeA +
                                         0.22f * loopA +
                                         0.14f * loopB);
            const float highMix = clamp01(0.58f * leadBand +
                                          0.16f * wakeB +
                                          0.22f * loopA +
                                          0.16f * loopB +
                                          0.10f * loopC);
            ImVec4 color = mix_color(baseColor, midColor, midMix);
            color = mix_color(color, highColor, highMix);
            color = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.08f * leadBand);
            color.w = std::clamp(alpha, 0.0f, 0.86f);
            const ImU32 colorU32 = to_u32(color);
            if (roundedInset > 0.0f) {
                const float slice = std::max(1.0f, scale);
                for (float sliceMinY = cellMinY; sliceMinY < cellMaxY; sliceMinY += slice) {
                    const float sliceMaxY = std::min(sliceMinY + slice, cellMaxY);
                    const float sliceCenterY = (sliceMinY + sliceMaxY) * 0.5f;
                    const float sliceInset = slider_track_rounded_inset_at_y(geometry, sliceCenterY);
                    const float sliceMinX = std::max(cellMinX, min.x + sliceInset);
                    const float sliceMaxX = std::min(cellMaxX, max.x - sliceInset);
                    if (sliceMaxX > sliceMinX) {
                        drawList->AddRectFilled(ImVec2(sliceMinX, sliceMinY),
                                                ImVec2(sliceMaxX, sliceMaxY),
                                                colorU32);
                    }
                }
            } else {
                drawList->AddRectFilled(ImVec2(drawMinX, cellMinY),
                                        ImVec2(drawMaxX, cellMaxY),
                                        colorU32);
            }

            const float leadLine = waveStrength(expansionArc, front, 0.052f) * initialWaveAlpha;
            const float wakeLine = waveStrength(expansionArc, std::max(0.0f, front - 0.16f), 0.064f) * initialWaveAlpha;
            const float loopLineA = waveStrength(loopArc, rippleCycle, 0.095f);
            const float loopLineB = waveStrength(loopArc, std::fmod(rippleCycle + 0.31f, 1.0f), 0.116f);
            const float loopLineC = waveStrength(loopArc, std::fmod(rippleCycle + 0.62f, 1.0f), 0.136f);
            const float lineAlpha = std::min(0.40f,
                                             0.26f * leadLine +
                                             0.12f * wakeLine +
                                             0.18f * loopLineA +
                                             0.135f * loopLineB +
                                             0.105f * loopLineC) *
                                    intensity;
            if (lineAlpha > 0.002f) {
                ImVec4 lineColorVec = mix_color(highColor, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.16f);
                lineColorVec.w = lineAlpha;
                const ImU32 lineColor = to_u32(lineColorVec);
                if (roundedInset > 0.0f) {
                    const float slice = std::max(1.0f, scale);
                    for (float sliceMinY = cellMinY; sliceMinY < cellMaxY; sliceMinY += slice) {
                        const float sliceMaxY = std::min(sliceMinY + slice, cellMaxY);
                        const float sliceCenterY = (sliceMinY + sliceMaxY) * 0.5f;
                        const float sliceInset = slider_track_rounded_inset_at_y(geometry, sliceCenterY);
                        const float sliceMinX = std::max(cellMinX, min.x + sliceInset);
                        const float sliceMaxX = std::min(cellMaxX, max.x - sliceInset);
                        if (sliceMaxX > sliceMinX) {
                            drawList->AddRectFilled(ImVec2(sliceMinX, sliceMinY),
                                                    ImVec2(sliceMaxX, sliceMaxY),
                                                    lineColor);
                        }
                    }
                } else {
                    drawList->AddRectFilled(ImVec2(drawMinX, cellMinY),
                                            ImVec2(drawMaxX, cellMaxY),
                                            lineColor);
                }
            }
        }
    }
    drawList->PopClipRect();
}
void draw_slider_spectral_overlay(ImVec2 min,
                                  ImVec2 max,
                                  const char* valueText,
                                  int value,
                                  int minValue,
                                  int maxValue,
                                  bool hovered,
                                  bool active,
                                  const Theme& theme,
                                  const SliderFieldStyle& style,
                                  bool drawBackground,
                                  bool drawText) {
    if (style.spectralEffect == SliderFieldSpectralEffect::None) {
        return;
    }

    const float range = static_cast<float>(std::max(1, maxValue - minValue));
    const float progress = clamp01((static_cast<float>(value) - static_cast<float>(minValue)) / range);
    const float stateBoost = active ? 1.16f : (hovered ? 1.07f : 1.0f);
    const float intensity = std::clamp(style.spectralIntensity * stateBoost, 0.0f, 1.35f);
    const SliderTrackGeometry geometry = make_slider_track_geometry(min, max, theme, style);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (drawBackground && style.spectralEffect == SliderFieldSpectralEffect::PixelBloom) {
        SliderSpectralState& spectralState = slider_spectral_cache()[ImGui::GetID("##slider_spectral")];
        const bool moved = std::fabs(spectralState.lastProgress - progress) > 0.012f;
        if (!spectralState.pixelBloomActive || moved) {
            spectralState.expansionProgress = 0.0f;
        }
        spectralState.pixelBloomActive = true;
        spectralState.lastProgress = progress;
        const float deltaTime = std::clamp(ImGui::GetIO().DeltaTime, 0.0f, 1.0f / 24.0f);
        spectralState.expansionProgress = std::min(1.0f, spectralState.expansionProgress + deltaTime / 1.35f);
        draw_slider_pixel_bloom(drawList, geometry, progress, spectralState.expansionProgress, intensity, style);
    } else if (drawBackground && style.spectralEffect != SliderFieldSpectralEffect::PixelBloom) {
        SliderSpectralState& spectralState = slider_spectral_cache()[ImGui::GetID("##slider_spectral")];
        spectralState.pixelBloomActive = false;
        spectralState.expansionProgress = 0.0f;
        spectralState.lastProgress = progress;
    }

    if (!drawText) {
        return;
    }

    const char* overlayText = slider_spectral_has_text(style) ? style.spectralText : valueText;
    if (overlayText == nullptr || overlayText[0] == '\0') {
        return;
    }

    if (style.spectralEffect == SliderFieldSpectralEffect::TextSheen) {
        draw_slider_sheen_text(drawList, geometry.min, geometry.max, overlayText, intensity, theme);
    } else {
        draw_slider_rainbow_text(drawList, geometry.min, geometry.max, overlayText, intensity);
    }
}

void draw_value_input_decor(ImVec2 min,
                            ImVec2 max,
                            bool hovered,
                            bool active,
                            const Theme& theme) {
    const float scale = current_scale();
    InteractionVisualState state;
    state.hovered = hovered;
    state.active = active;

    AccentFrameStyle style;
    style.rounding = theme.fields.frameRounding * scale;
    style.idleBorderAlpha = 0.09f;
    style.hoveredBorderAlpha = 0.20f;
    style.activeBorderAlpha = 0.38f;
    style.idleGlowAlpha = 0.08f;
    style.hoveredGlowAlpha = 0.08f;
    style.activeGlowAlpha = 0.26f;
    style.idleThickness = 1.0f;
    style.activeThickness = 1.5f;

    draw_accent_frame(ImGui::GetWindowDrawList(), min, max, theme, state, style);
}

void push_field_frame_style(const Theme& theme, SliderFieldStyle style) {
    const float rounding = resolve_frame_rounding(theme, style) * current_scale();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, theme.fields.frameBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, theme.fields.frameBgHovered);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, theme.fields.frameBgActive);
}

void push_slider_grab_style(const Theme& theme) {
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, theme.fields.sliderGrab);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, theme.fields.sliderGrabActive);
}

void pop_field_frame_style() {
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

void draw_field_help_marker(const char* helpText, const Theme& theme) {
    if (helpText != nullptr && helpText[0] != '\0') {
        same_line_help_marker_aligned_to_last_item(helpText, theme);
    }
}

struct AnimatedComboState {
    float openProgress = 0.0f;
    bool wasOpen = false;
};

std::unordered_map<ImGuiID, AnimatedComboState>& animated_combo_cache() {
    static std::unordered_map<ImGuiID, AnimatedComboState> cache;
    return cache;
}

const char* localized_field_text(UiTextLanguage language, const char* english, const char* chinese) {
    return language == UiTextLanguage::ChineseSimplified ? chinese : english;
}

const char* resolve_theme_flavor_field_text(const char* value,
                                            UiTextLanguage language,
                                            const char* english,
                                            const char* chinese) {
    return value != nullptr && value[0] != '\0'
        ? value
        : localized_field_text(language, english, chinese);
}

const char* resolve_background_kind_field_text(const char* value,
                                               UiTextLanguage language,
                                               const char* english,
                                               const char* chinese) {
    return value != nullptr && value[0] != '\0'
        ? value
        : localized_field_text(language, english, chinese);
}

} // namespace

AnimatedComboScope begin_animated_combo(const char* id,
                                        const char* previewValue,
                                        float comboWidth,
                                        const Theme& theme,
                                        SelectFieldStyle style) {
    AnimatedComboScope scope{};
    const float scale = current_scale();
    const float frameRounding = (style.frameRounding > 0.0f ? style.frameRounding : theme.fields.frameRounding) * scale;
    const float popupRounding = (style.popupRounding >= 0.0f ? style.popupRounding : theme.fields.frameRounding) * scale;
    const ImVec2 popupPadding(style.popupPadding.x >= 0.0f ? style.popupPadding.x : theme.fields.selectPopupPadding.x,
                              style.popupPadding.y >= 0.0f ? style.popupPadding.y : theme.fields.selectPopupPadding.y);

    ImGui::SetNextItemWidth(comboWidth);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, frameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, popupRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(popupPadding.x * scale, popupPadding.y * scale));
    scope.styleVarCount = 3;

    const ImGuiID comboId = ImGui::GetID(id);
    AnimatedComboState& anim = animated_combo_cache()[comboId];
    scope.open = ImGui::BeginCombo(id, previewValue != nullptr ? previewValue : "");

    if (scope.open) {
        if (!anim.wasOpen) {
            anim.openProgress = 0.0f;
        }
        const float delta = std::max(0.0f, ImGui::GetIO().DeltaTime);
        anim.openProgress = std::min(1.0f, anim.openProgress + delta / 0.145f);
        const float eased = ease_out_cubic(anim.openProgress);
        const float alpha = std::clamp(0.38f + 0.62f * eased, 0.0f, 1.0f);
        const float slide = (1.0f - eased) * 5.0f * scale;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
        scope.contentStylePushed = true;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slide);
    } else {
        anim.openProgress = 0.0f;
    }

    anim.wasOpen = scope.open;
    return scope;
}

void end_animated_combo(const AnimatedComboScope& scope) {
    if (scope.open) {
        if (scope.contentStylePushed) {
            ImGui::PopStyleVar();
        }
        ImGui::EndCombo();
    }
    if (scope.styleVarCount > 0) {
        ImGui::PopStyleVar(scope.styleVarCount);
    }
}

void sync_int_input_buffer(BufferedInputState inputState, int value) {
    if (inputState.buffer == nullptr || inputState.bufferSize <= 1) {
        return;
    }
    std::snprintf(inputState.buffer, static_cast<std::size_t>(inputState.bufferSize), "%d", value);
}

void sync_float_input_buffer(BufferedInputState inputState, float value, const char* format) {
    if (inputState.buffer == nullptr || inputState.bufferSize <= 1) {
        return;
    }
    std::snprintf(inputState.buffer,
                  static_cast<std::size_t>(inputState.bufferSize),
                  format != nullptr ? format : "%.3f",
                  static_cast<double>(value));
}

void sync_scaled_int_input_buffer(BufferedInputState inputState, int value, float displayScale, const char* format) {
    if (displayScale == 0.0f) {
        return;
    }
    sync_float_input_buffer(inputState,
                            static_cast<float>(value) / displayScale,
                            format);
}

void sync_milliseconds_input_buffer(BufferedInputState inputState, int valueMicroseconds) {
    sync_scaled_int_input_buffer(inputState,
                                 valueMicroseconds,
                                 kMillisecondsDisplayScale,
                                 "%.3f");
}

void sync_float_range_input_buffer(BufferedRangeInputState inputState,
                                   float minValue,
                                   float maxValue,
                                   const char* format) {
    sync_float_input_buffer(inputState.min, minValue, format);
    sync_float_input_buffer(inputState.max, maxValue, format);
}

void sync_scaled_int_range_input_buffer(BufferedRangeInputState inputState,
                                        int minValue,
                                        int maxValue,
                                        float displayScale,
                                        const char* format) {
    if (displayScale == 0.0f) {
        return;
    }
    sync_float_range_input_buffer(inputState,
                                  static_cast<float>(minValue) / displayScale,
                                  static_cast<float>(maxValue) / displayScale,
                                  format);
}

void sync_milliseconds_range_input_buffer(BufferedRangeInputState inputState,
                                          int minValueMicroseconds,
                                          int maxValueMicroseconds) {
    sync_scaled_int_range_input_buffer(inputState,
                                       minValueMicroseconds,
                                       maxValueMicroseconds,
                                       kMillisecondsDisplayScale,
                                       "%.3f");
}

bool draw_int_slider_field(const char* id,
                           const char* label,
                           int& value,
                           int minValue,
                           int maxValue,
                           const char* helpText,
                           BufferedInputState inputState,
                           const Theme& theme,
                           FormLayoutStyle layout,
                           SliderFieldStyle style,
                           float insetX,
                           FieldInteractionState* interaction) {
    if (interaction != nullptr) {
        *interaction = FieldInteractionState{};
    }
    if (inputState.buffer == nullptr || inputState.bufferSize <= 1 || inputState.editing == nullptr) {
        return false;
    }

    value = std::clamp(value, minValue, maxValue);
    if (!*inputState.editing) {
        sync_int_input_buffer(inputState, value);
    }

    const float scale = current_scale();
    bool changed = false;
    ImGui::PushID(id);
    const bool hasLabelColor = style.labelColor.w >= 0.0f;
    if (hasLabelColor) {
        ImGui::PushStyleColor(ImGuiCol_Text, style.labelColor);
    }
    begin_form_row(label, layout, insetX);
    if (hasLabelColor) {
        ImGui::PopStyleColor();
    }

    int sliderValue = value;
    ImVec2 sliderMin{};
    ImVec2 sliderMax{};
    bool sliderHovered = false;
    bool sliderActive = false;
    bool sliderChanged = false;
    if (style.drawCustomSlider) {
        sliderChanged = draw_custom_int_slider("##slider",
                                               sliderValue,
                                               minValue,
                                               maxValue,
                                               theme,
                                               style,
                                               &sliderMin,
                                               &sliderMax,
                                               &sliderHovered,
                                               &sliderActive);
    } else {
        ImGui::SetNextItemWidth(resolve_slider_width(theme, style) * scale);
        push_field_frame_style(theme, style);
        push_slider_grab_style(theme);
        const bool customSliderText = style.spectralEffect != SliderFieldSpectralEffect::None;
        sliderChanged = ImGui::SliderInt("##slider",
                                         &sliderValue,
                                         minValue,
                                         maxValue,
                                         customSliderText ? "" : "%d");
        sliderMin = ImGui::GetItemRectMin();
        sliderMax = ImGui::GetItemRectMax();
        sliderHovered = ImGui::IsItemHovered();
        sliderActive = ImGui::IsItemActive();
        ImGui::PopStyleColor(2);
        pop_field_frame_style();
    }
    char sliderDisplayText[32]{};
    std::snprintf(sliderDisplayText, std::size(sliderDisplayText), "%d", std::clamp(sliderValue, minValue, maxValue));
    draw_slider_spectral_overlay(sliderMin,
                                 sliderMax,
                                 sliderDisplayText,
                                 sliderValue,
                                 minValue,
                                 maxValue,
                                 sliderHovered,
                                 sliderActive,
                                 theme,
                                 style,
                                 true,
                                 !style.drawCustomSlider);
    if (style.drawCustomSlider) {
        const SliderTrackGeometry sliderGeometry = make_slider_track_geometry(sliderMin, sliderMax, theme, style);
        draw_slider_tick_marks(ImGui::GetWindowDrawList(),
                               sliderGeometry,
                               minValue,
                               maxValue,
                               theme,
                               style);
        draw_slider_custom_border(sliderMin, sliderMax, sliderHovered, sliderActive, theme, style);
        draw_slider_custom_handle(ImGui::GetWindowDrawList(),
                                  sliderGeometry,
                                  sliderValue,
                                  minValue,
                                  maxValue,
                                  sliderActive,
                                  theme,
                                  style);
        if (style.spectralEffect == SliderFieldSpectralEffect::None) {
            draw_slider_value_text(ImGui::GetWindowDrawList(),
                                   sliderGeometry,
                                   sliderDisplayText,
                                   theme,
                                   style);
        } else {
            draw_slider_spectral_overlay(sliderMin,
                                         sliderMax,
                                         sliderDisplayText,
                                         sliderValue,
                                         minValue,
                                         maxValue,
                                         sliderHovered,
                                         sliderActive,
                                         theme,
                                         style,
                                         false,
                                         true);
        }
    } else {
        draw_slider_decor(sliderMin, sliderMax, sliderHovered, sliderActive, theme, style);
    }
    if (sliderChanged) {
        value = std::clamp(sliderValue, minValue, maxValue);
        changed = true;
        if (!*inputState.editing) {
            sync_int_input_buffer(inputState, value);
        }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(resolve_input_width(theme, style) * scale);
    const ImGuiInputTextFlags inputFlags =
        ImGuiInputTextFlags_CharsDecimal |
        ImGuiInputTextFlags_EnterReturnsTrue |
        ImGuiInputTextFlags_AutoSelectAll;
    push_field_frame_style(theme, style);
    const bool enterCommitted = ImGui::InputText("##value", inputState.buffer, inputState.bufferSize, inputFlags);
    const ImVec2 inputMin = ImGui::GetItemRectMin();
    const ImVec2 inputMax = ImGui::GetItemRectMax();
    const bool inputHovered = ImGui::IsItemHovered();
    const bool inputActive = ImGui::IsItemActive();
    const bool blurCommitted = ImGui::IsItemDeactivatedAfterEdit();
    pop_field_frame_style();
    draw_value_input_decor(inputMin, inputMax, inputHovered, inputActive, theme);
    if (interaction != nullptr) {
        interaction->sliderHovered = sliderHovered;
        interaction->sliderActive = sliderActive;
        interaction->inputHovered = inputHovered;
        interaction->inputActive = inputActive;
        interaction->hovered = sliderHovered || inputHovered;
        interaction->active = sliderActive || inputActive;
    }
    if (inputActive) {
        *inputState.editing = true;
    }

    if (enterCommitted || blurCommitted) {
        char* end = nullptr;
        const long parsed = std::strtol(inputState.buffer, &end, 10);
        const long clamped = std::clamp(parsed, static_cast<long>(minValue), static_cast<long>(maxValue));
        const int nextValue = (end == inputState.buffer) ? value : static_cast<int>(clamped);
        if (nextValue != value) {
            value = nextValue;
            changed = true;
        }
        *inputState.editing = false;
        sync_int_input_buffer(inputState, value);
    } else if (!inputActive && *inputState.editing) {
        *inputState.editing = false;
        sync_int_input_buffer(inputState, value);
    }

    draw_field_help_marker(helpText, theme);
    ImGui::PopID();
    return changed;
}

bool draw_float_slider_field(const char* id,
                             const char* label,
                             float& value,
                             float minValue,
                             float maxValue,
                             const char* sliderFormat,
                             const char* inputFormat,
                             const char* helpText,
                             BufferedInputState inputState,
                             const Theme& theme,
                             FormLayoutStyle layout,
                             SliderFieldStyle style,
                             float insetX) {
    if (inputState.buffer == nullptr || inputState.bufferSize <= 1 || inputState.editing == nullptr) {
        return false;
    }

    value = std::clamp(value, minValue, maxValue);
    if (!*inputState.editing) {
        sync_float_input_buffer(inputState, value, inputFormat);
    }

    const float scale = current_scale();
    bool changed = false;
    ImGui::PushID(id);
    begin_form_row(label, layout, insetX);

    ImGui::SetNextItemWidth(resolve_slider_width(theme, style) * scale);
    float sliderValue = value;
    push_field_frame_style(theme, style);
    push_slider_grab_style(theme);
    const bool sliderChanged = ImGui::SliderFloat("##slider",
                                                  &sliderValue,
                                                  minValue,
                                                  maxValue,
                                                  sliderFormat != nullptr ? sliderFormat : "%.3f");
    const ImVec2 sliderMin = ImGui::GetItemRectMin();
    const ImVec2 sliderMax = ImGui::GetItemRectMax();
    const bool sliderHovered = ImGui::IsItemHovered();
    const bool sliderActive = ImGui::IsItemActive();
    ImGui::PopStyleColor(2);
    pop_field_frame_style();
    draw_slider_decor(sliderMin, sliderMax, sliderHovered, sliderActive, theme, style);
    if (sliderChanged) {
        value = std::clamp(sliderValue, minValue, maxValue);
        changed = true;
        if (!*inputState.editing) {
            sync_float_input_buffer(inputState, value, inputFormat);
        }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(resolve_input_width(theme, style) * scale);
    const ImGuiInputTextFlags inputFlags =
        ImGuiInputTextFlags_CharsDecimal |
        ImGuiInputTextFlags_EnterReturnsTrue |
        ImGuiInputTextFlags_AutoSelectAll;
    push_field_frame_style(theme, style);
    const bool enterCommitted = ImGui::InputText("##value", inputState.buffer, inputState.bufferSize, inputFlags);
    const ImVec2 inputMin = ImGui::GetItemRectMin();
    const ImVec2 inputMax = ImGui::GetItemRectMax();
    const bool inputHovered = ImGui::IsItemHovered();
    const bool inputActive = ImGui::IsItemActive();
    const bool blurCommitted = ImGui::IsItemDeactivatedAfterEdit();
    pop_field_frame_style();
    draw_value_input_decor(inputMin, inputMax, inputHovered, inputActive, theme);
    if (inputActive) {
        *inputState.editing = true;
    }

    if (enterCommitted || blurCommitted) {
        char* end = nullptr;
        const double parsed = std::strtod(inputState.buffer, &end);
        const double clamped = std::clamp(parsed, static_cast<double>(minValue), static_cast<double>(maxValue));
        const float nextValue = (end == inputState.buffer) ? value : static_cast<float>(clamped);
        if (std::fabs(nextValue - value) > 0.0001f) {
            value = nextValue;
            changed = true;
        }
        *inputState.editing = false;
        sync_float_input_buffer(inputState, value, inputFormat);
    } else if (!inputActive && *inputState.editing) {
        *inputState.editing = false;
        sync_float_input_buffer(inputState, value, inputFormat);
    }

    draw_field_help_marker(helpText, theme);
    ImGui::PopID();
    return changed;
}

bool draw_scaled_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  float displayScale,
                                  const char* sliderFormat,
                                  const char* inputFormat,
                                  const char* helpText,
                                  BufferedInputState inputState,
                                  const Theme& theme,
                                  FormLayoutStyle layout,
                                  SliderFieldStyle style,
                                  float insetX) {
    if (displayScale == 0.0f) {
        return false;
    }

    value = std::clamp(value, minValue, maxValue);
    float displayValue = static_cast<float>(value) / displayScale;
    const bool changed = draw_float_slider_field(id,
                                                 label,
                                                 displayValue,
                                                 static_cast<float>(minValue) / displayScale,
                                                 static_cast<float>(maxValue) / displayScale,
                                                 sliderFormat,
                                                 inputFormat,
                                                 helpText,
                                                 inputState,
                                                 theme,
                                                 layout,
                                                 style,
                                                 insetX);
    value = std::clamp(static_cast<int>(std::lround(static_cast<double>(displayValue) * static_cast<double>(displayScale))),
                       minValue,
                       maxValue);
    return changed;
}

bool draw_milliseconds_slider_field(const char* id,
                                    const char* label,
                                    int& valueMicroseconds,
                                    int minValueMicroseconds,
                                    int maxValueMicroseconds,
                                    const char* helpText,
                                    BufferedInputState inputState,
                                    const Theme& theme,
                                    FormLayoutStyle layout,
                                    SliderFieldStyle style,
                                    float insetX) {
    return draw_scaled_int_slider_field(id,
                                        label,
                                        valueMicroseconds,
                                        minValueMicroseconds,
                                        maxValueMicroseconds,
                                        kMillisecondsDisplayScale,
                                        "%.3f ms",
                                        "%.3f",
                                        helpText,
                                        inputState,
                                        theme,
                                        layout,
                                        style,
                                        insetX);
}

bool draw_float_range_field(const char* id,
                            const char* label,
                            float& minValue,
                            float& maxValue,
                            float rangeMin,
                            float rangeMax,
                            const char* valueFormat,
                            const char* helpText,
                            BufferedRangeInputState inputState,
                            const Theme& theme,
                            FormLayoutStyle layout,
                            SliderFieldStyle style,
                            float insetX) {
    if (inputState.min.buffer == nullptr || inputState.min.bufferSize <= 1 || inputState.min.editing == nullptr ||
        inputState.max.buffer == nullptr || inputState.max.bufferSize <= 1 || inputState.max.editing == nullptr) {
        return false;
    }

    minValue = std::clamp(minValue, rangeMin, rangeMax);
    maxValue = std::clamp(maxValue, minValue, rangeMax);
    if (!*inputState.min.editing) {
        sync_float_input_buffer(inputState.min, minValue, valueFormat);
    }
    if (!*inputState.max.editing) {
        sync_float_input_buffer(inputState.max, maxValue, valueFormat);
    }

    const float scale = current_scale();
    const float sliderHeight = 32.0f * scale;
    const float inputWidth = resolve_input_width(theme, style) * scale;
    const float sliderWidth = resolve_slider_width(theme, style) * scale;
    const float rounding = resolve_frame_rounding(theme, style) * scale;
    bool changed = false;

    ImGui::PushID(id);
    begin_form_row(label, layout, insetX);

    ImGui::SetNextItemWidth(inputWidth);
    push_field_frame_style(theme, style);
    const bool minEnterCommitted = ImGui::InputText("##min_input",
                                                    inputState.min.buffer,
                                                    inputState.min.bufferSize,
                                                    ImGuiInputTextFlags_CharsDecimal |
                                                        ImGuiInputTextFlags_EnterReturnsTrue |
                                                        ImGuiInputTextFlags_AutoSelectAll);
    const ImVec2 minInputMin = ImGui::GetItemRectMin();
    const ImVec2 minInputMax = ImGui::GetItemRectMax();
    const bool minInputHovered = ImGui::IsItemHovered();
    const bool minInputActive = ImGui::IsItemActive();
    const bool minBlurCommitted = ImGui::IsItemDeactivatedAfterEdit();
    pop_field_frame_style();
    draw_value_input_decor(minInputMin, minInputMax, minInputHovered, minInputActive, theme);
    if (minInputActive) {
        *inputState.min.editing = true;
    }
    if (minEnterCommitted || minBlurCommitted) {
        char* end = nullptr;
        const double parsed = std::strtod(inputState.min.buffer, &end);
        const double clamped = std::clamp(parsed, static_cast<double>(rangeMin), static_cast<double>(maxValue));
        const float nextValue = (end == inputState.min.buffer) ? minValue : static_cast<float>(clamped);
        if (std::fabs(nextValue - minValue) > 0.0001f) {
            minValue = nextValue;
            changed = true;
        }
        *inputState.min.editing = false;
        sync_float_input_buffer(inputState.min, minValue, valueFormat);
    } else if (!minInputActive && *inputState.min.editing) {
        *inputState.min.editing = false;
        sync_float_input_buffer(inputState.min, minValue, valueFormat);
    }

    ImGui::SameLine();

    const ImVec2 sliderPos = ImGui::GetCursorScreenPos();
    const ImVec2 sliderSize(sliderWidth, sliderHeight);
    ImGui::InvisibleButton("##range_slider_area", sliderSize);
    const bool sliderHovered = ImGui::IsItemHovered();
    const bool sliderActive = ImGui::IsItemActive();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 sliderMax(sliderPos.x + sliderSize.x, sliderPos.y + sliderSize.y);
    const SliderTrackGeometry sliderGeometry = make_slider_track_geometry(sliderPos, sliderMax, rounding);

    draw_slider_track_background(drawList, sliderGeometry, theme.fields.frameBg);

    const float padding = 12.0f * scale;
    const float trackY = sliderPos.y + sliderHeight * 0.5f;
    const float trackLeft = sliderPos.x + padding;
    const float trackRight = sliderPos.x + sliderSize.x - padding;
    const float trackWidth = std::max(1.0f, trackRight - trackLeft);

    minValue = std::clamp(minValue, rangeMin, rangeMax);
    maxValue = std::clamp(maxValue, minValue, rangeMax);
    const float minNorm = std::clamp((minValue - rangeMin) / std::max(0.0001f, rangeMax - rangeMin), 0.0f, 1.0f);
    const float maxNorm = std::clamp((maxValue - rangeMin) / std::max(0.0001f, rangeMax - rangeMin), 0.0f, 1.0f);
    const float minX = trackLeft + minNorm * trackWidth;
    const float maxX = trackLeft + maxNorm * trackWidth;

    drawList->AddLine(ImVec2(trackLeft, trackY),
                      ImVec2(trackRight, trackY),
                      to_u32(theme.fields.rangeTrack),
                      3.0f * scale);
    if (maxX > minX + 1.0f) {
        drawList->AddLine(ImVec2(minX, trackY),
                          ImVec2(maxX, trackY),
                          to_u32(theme.fields.rangeFill),
                          4.0f * scale);
    }

    static int activeHandle = 0;
    const ImVec2 mousePos = ImGui::GetMousePos();
    const float grabRadius = 8.0f * scale;
    if (sliderActive && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (activeHandle == 0) {
            const float distToMin = std::fabs(mousePos.x - minX);
            const float distToMax = std::fabs(mousePos.x - maxX);
            const float threshold = grabRadius * 2.5f;
            if (distToMin < threshold || distToMax < threshold) {
                activeHandle = (distToMin < distToMax) ? 1 : 2;
            }
        }

        if (activeHandle != 0) {
            const float newNorm = std::clamp((mousePos.x - trackLeft) / trackWidth, 0.0f, 1.0f);
            const float newValue = rangeMin + newNorm * (rangeMax - rangeMin);
            if (activeHandle == 1 && newValue <= maxValue) {
                if (std::fabs(newValue - minValue) > 0.0001f) {
                    minValue = newValue;
                    changed = true;
                }
            } else if (activeHandle == 2 && newValue >= minValue) {
                if (std::fabs(newValue - maxValue) > 0.0001f) {
                    maxValue = newValue;
                    changed = true;
                }
            }
        }
    } else {
        activeHandle = 0;
    }

    const bool minHandleHovered = std::fabs(mousePos.x - minX) < grabRadius * 1.5f &&
                                  std::fabs(mousePos.y - trackY) < sliderHeight * 0.5f && sliderHovered;
    const bool maxHandleHovered = std::fabs(mousePos.x - maxX) < grabRadius * 1.5f &&
                                  std::fabs(mousePos.y - trackY) < sliderHeight * 0.5f && sliderHovered;
    const ImVec2 minHandleCenter(minX, trackY);
    const ImVec2 maxHandleCenter(maxX, trackY);
    const bool minActive = activeHandle == 1;
    const bool maxActive = activeHandle == 2;

    if (minActive || minHandleHovered) {
        drawList->AddCircleFilled(minHandleCenter,
                                  grabRadius * 1.4f,
                                  to_u32(ImVec4(theme.palette.accent.x,
                                                theme.palette.accent.y,
                                                theme.palette.accent.z,
                                                theme.fields.rangeHandleHaloAlpha)));
    }
    drawList->AddCircleFilled(minHandleCenter,
                              grabRadius,
                              to_u32(ImVec4(theme.palette.accent.x,
                                            theme.palette.accent.y,
                                            theme.palette.accent.z,
                                            minActive ? theme.fields.rangeHandleFillActiveAlpha
                                                      : theme.fields.rangeHandleFillAlpha)));
    drawList->AddCircle(minHandleCenter,
                        grabRadius,
                        to_u32(ImVec4(theme.fields.rangeHandleRing.x,
                                      theme.fields.rangeHandleRing.y,
                                      theme.fields.rangeHandleRing.z,
                                      minActive ? theme.fields.rangeHandleRingActiveAlpha
                                                : (minHandleHovered ? theme.fields.rangeHandleRingHoverAlpha
                                                                    : theme.fields.rangeHandleRingIdleAlpha))),
                        0,
                        1.5f * scale);

    if (maxActive || maxHandleHovered) {
        drawList->AddCircleFilled(maxHandleCenter,
                                  grabRadius * 1.4f,
                                  to_u32(ImVec4(theme.palette.accent.x,
                                                theme.palette.accent.y,
                                                theme.palette.accent.z,
                                                theme.fields.rangeHandleHaloAlpha)));
    }
    drawList->AddCircleFilled(maxHandleCenter,
                              grabRadius,
                              to_u32(ImVec4(theme.palette.accent.x,
                                            theme.palette.accent.y,
                                            theme.palette.accent.z,
                                            maxActive ? theme.fields.rangeHandleFillActiveAlpha
                                                      : theme.fields.rangeHandleFillAlpha)));
    drawList->AddCircle(maxHandleCenter,
                        grabRadius,
                        to_u32(ImVec4(theme.fields.rangeHandleRing.x,
                                      theme.fields.rangeHandleRing.y,
                                      theme.fields.rangeHandleRing.z,
                                      maxActive ? theme.fields.rangeHandleRingActiveAlpha
                                                : (maxHandleHovered ? theme.fields.rangeHandleRingHoverAlpha
                                                                    : theme.fields.rangeHandleRingIdleAlpha))),
                        0,
                        1.5f * scale);

    if (style.showRangeValueLabels) {
        char minLabel[32];
        char maxLabel[32];
        std::snprintf(minLabel, sizeof(minLabel), valueFormat != nullptr ? valueFormat : "%.3f", static_cast<double>(minValue));
        std::snprintf(maxLabel, sizeof(maxLabel), valueFormat != nullptr ? valueFormat : "%.3f", static_cast<double>(maxValue));
        const ImVec2 minLabelSize = ImGui::CalcTextSize(minLabel);
        const ImVec2 maxLabelSize = ImGui::CalcTextSize(maxLabel);
        drawList->AddText(ImVec2(minX - minLabelSize.x * 0.5f, sliderPos.y + sliderHeight + 4.0f * scale),
                          to_u32(ImVec4(theme.palette.text.x,
                                        theme.palette.text.y,
                                        theme.palette.text.z,
                                        theme.fields.rangeValueTextAlpha)),
                          minLabel);
        drawList->AddText(ImVec2(maxX - maxLabelSize.x * 0.5f, sliderPos.y - minLabelSize.y - 4.0f * scale),
                          to_u32(ImVec4(theme.palette.text.x,
                                        theme.palette.text.y,
                                        theme.palette.text.z,
                                        theme.fields.rangeValueTextAlpha)),
                          maxLabel);
    }

    InteractionVisualState sliderState;
    sliderState.hovered = sliderHovered;
    sliderState.active = sliderActive;

    AccentFrameStyle sliderFrameStyle;
    sliderFrameStyle.rounding = sliderGeometry.rounding;
    sliderFrameStyle.idleBorderAlpha = 0.11f;
    sliderFrameStyle.hoveredBorderAlpha = 0.25f;
    sliderFrameStyle.activeBorderAlpha = 0.46f;
    sliderFrameStyle.idleGlowAlpha = 0.105f;
    sliderFrameStyle.hoveredGlowAlpha = 0.20f;
    sliderFrameStyle.activeGlowAlpha = 0.36f;
    sliderFrameStyle.idleThickness = 1.0f;
    sliderFrameStyle.activeThickness = 1.6f;
    draw_accent_frame(drawList, sliderGeometry.min, sliderGeometry.max, theme, sliderState, sliderFrameStyle);

    ImGui::SameLine();

    ImGui::SetNextItemWidth(inputWidth);
    push_field_frame_style(theme, style);
    const bool maxEnterCommitted = ImGui::InputText("##max_input",
                                                    inputState.max.buffer,
                                                    inputState.max.bufferSize,
                                                    ImGuiInputTextFlags_CharsDecimal |
                                                        ImGuiInputTextFlags_EnterReturnsTrue |
                                                        ImGuiInputTextFlags_AutoSelectAll);
    const ImVec2 maxInputMin = ImGui::GetItemRectMin();
    const ImVec2 maxInputMax = ImGui::GetItemRectMax();
    const bool maxInputHovered = ImGui::IsItemHovered();
    const bool maxInputActive = ImGui::IsItemActive();
    const bool maxBlurCommitted = ImGui::IsItemDeactivatedAfterEdit();
    pop_field_frame_style();
    draw_value_input_decor(maxInputMin, maxInputMax, maxInputHovered, maxInputActive, theme);
    if (maxInputActive) {
        *inputState.max.editing = true;
    }
    if (maxEnterCommitted || maxBlurCommitted) {
        char* end = nullptr;
        const double parsed = std::strtod(inputState.max.buffer, &end);
        const double clamped = std::clamp(parsed, static_cast<double>(minValue), static_cast<double>(rangeMax));
        const float nextValue = (end == inputState.max.buffer) ? maxValue : static_cast<float>(clamped);
        if (std::fabs(nextValue - maxValue) > 0.0001f) {
            maxValue = nextValue;
            changed = true;
        }
        *inputState.max.editing = false;
        sync_float_input_buffer(inputState.max, maxValue, valueFormat);
    } else if (!maxInputActive && *inputState.max.editing) {
        *inputState.max.editing = false;
        sync_float_input_buffer(inputState.max, maxValue, valueFormat);
    }

    draw_field_help_marker(helpText, theme);
    ImGui::PopID();
    return changed;
}

bool draw_scaled_int_range_field(const char* id,
                                 const char* label,
                                 int& minValue,
                                 int& maxValue,
                                 int rangeMin,
                                 int rangeMax,
                                 float displayScale,
                                 const char* valueFormat,
                                 const char* helpText,
                                 BufferedRangeInputState inputState,
                                 const Theme& theme,
                                 FormLayoutStyle layout,
                                 SliderFieldStyle style,
                                 float insetX) {
    if (displayScale == 0.0f) {
        return false;
    }

    minValue = std::clamp(minValue, rangeMin, rangeMax);
    maxValue = std::clamp(maxValue, minValue, rangeMax);
    float displayMinValue = static_cast<float>(minValue) / displayScale;
    float displayMaxValue = static_cast<float>(maxValue) / displayScale;
    const bool changed = draw_float_range_field(id,
                                                label,
                                                displayMinValue,
                                                displayMaxValue,
                                                static_cast<float>(rangeMin) / displayScale,
                                                static_cast<float>(rangeMax) / displayScale,
                                                valueFormat,
                                                helpText,
                                                inputState,
                                                theme,
                                                layout,
                                                style,
                                                insetX);
    minValue = std::clamp(static_cast<int>(std::lround(static_cast<double>(displayMinValue) * static_cast<double>(displayScale))),
                          rangeMin,
                          rangeMax);
    maxValue = std::clamp(static_cast<int>(std::lround(static_cast<double>(displayMaxValue) * static_cast<double>(displayScale))),
                          minValue,
                          rangeMax);
    return changed;
}

bool draw_milliseconds_range_field(const char* id,
                                   const char* label,
                                   int& minValueMicroseconds,
                                   int& maxValueMicroseconds,
                                   int rangeMinMicroseconds,
                                   int rangeMaxMicroseconds,
                                   const char* helpText,
                                   BufferedRangeInputState inputState,
                                   const Theme& theme,
                                   FormLayoutStyle layout,
                                   SliderFieldStyle style,
                                   float insetX) {
    return draw_scaled_int_range_field(id,
                                       label,
                                       minValueMicroseconds,
                                       maxValueMicroseconds,
                                       rangeMinMicroseconds,
                                       rangeMaxMicroseconds,
                                       kMillisecondsDisplayScale,
                                       "%.3f",
                                       helpText,
                                       inputState,
                                        theme,
                                        layout,
                                        style,
                                        insetX);
}

bool draw_theme_flavor_field(const char* id,
                             ThemeFlavor currentFlavor,
                             const ThemeFlavorFieldData& data,
                             const Theme& theme,
                             const std::function<void(ThemeFlavor)>& activateFlavor,
                             FormLayoutStyle layout,
                             SelectFieldStyle style,
                             float insetX) {
    int optionCount = 0;
    const ThemeFlavorInfo* options = theme_flavor_infos(&optionCount);
    const char* label = resolve_theme_flavor_field_text(
        data.text.label,
        data.language,
        "Theme flavor",
        "\xE4\xB8\xBB\xE9\xA2\x98\xE9\xA3\x8E\xE6\xA0\xBC");
    const char* previewValue = theme_flavor_label(currentFlavor, data.language);
    const char* helpText = resolve_theme_flavor_field_text(
        data.text.helpText,
        data.language,
        "Controls the palette, surface contrast, and overall visual tone used by the shared imgui-onguoin UI kit.",
        "\xE6\x8E\xA7\xE5\x88\xB6\x20imgui-onguoin\x20\xE5\x85\xB1\xE7\x94\xA8\x20UI\x20\xE5\xA5\x97\xE4\xBB\xB6\xE7\x9A\x84\xE9\x85\x8D\xE8\x89\xB2\xE3\x80\x81\xE8\xA1\xA8\xE9\x9D\xA2\xE5\xB1\x82\xE6\xAC\xA1\xE5\x92\x8C\xE6\x95\xB4\xE4\xBD\x93\xE8\xA7\x86\xE8\xA7\x89\xE6\xB0\x94\xE8\xB4\xA8\xE3\x80\x82");

    return draw_select_field_from_list(
        id,
        label,
        previewValue,
        helpText,
        options,
        optionCount,
        [&](const ThemeFlavorInfo& info) {
            return theme_flavor_label(info, data.language);
        },
        [&](const ThemeFlavorInfo& info) {
            return currentFlavor == info.flavor;
        },
        [&](const ThemeFlavorInfo& info) {
            if (activateFlavor) {
                activateFlavor(info.flavor);
            }
        },
        theme,
        layout,
        style,
        insetX);
}

bool draw_background_kind_field(const char* id,
                                BackgroundKind currentKind,
                                const BackgroundKindFieldData& data,
                                const Theme& theme,
                                const std::function<void(BackgroundKind)>& activateKind,
                                FormLayoutStyle layout,
                                SelectFieldStyle style,
                                float insetX) {
    int optionCount = 0;
    const BackgroundKindInfo* options = background_kind_infos(&optionCount);
    const char* label = resolve_background_kind_field_text(
        data.text.label,
        data.language,
        "Background style",
        "\xE8\x83\x8C\xE6\x99\xAF\xE6\xA0\xB7\xE5\xBC\x8F");
    const char* previewValue = background_kind_label(currentKind, data.language);
    const char* helpText = resolve_background_kind_field_text(
        data.text.helpText,
        data.language,
        "Switches the large-scale backdrop renderer independently from the main theme so future styles stay easy to extend.",
        "\xE7\x8B\xAC\xE7\xAB\x8B\xE5\x88\x87\xE6\x8D\xA2\xE5\xA4\xA7\xE8\x83\x8C\xE6\x99\xAF\xE7\xBB\x98\xE5\x88\xB6\xE6\xA0\xB7\xE5\xBC\x8F\xEF\xBC\x8C\xE4\xB8\x8D\xE5\x92\x8C\xE4\xB8\xBB\xE4\xB8\xBB\xE9\xA2\x98\xE5\xBC\xBA\xE8\x80\xA6\xE5\x90\x88\xEF\xBC\x8C\xE6\x96\xB9\xE4\xBE\xBF\xE5\x90\x8E\xE7\xBB\xAD\xE7\xBB\xA7\xE7\xBB\xAD\xE6\x89\xA9\xE5\xB1\x95\xE6\x96\xB0\xE7\x9A\x84\xE8\x83\x8C\xE6\x99\xAF\xE6\x96\xB9\xE6\xA1\x88\xE3\x80\x82");

    return draw_select_field_from_list(
        id,
        label,
        previewValue,
        helpText,
        options,
        optionCount,
        [&](const BackgroundKindInfo& info) {
            return background_kind_label(info, data.language);
        },
        [&](const BackgroundKindInfo& info) {
            return currentKind == info.kind;
        },
        [&](const BackgroundKindInfo& info) {
            if (activateKind) {
                activateKind(info.kind);
            }
        },
        theme,
        layout,
        style,
        insetX);
}

} // namespace imgui_onguoin

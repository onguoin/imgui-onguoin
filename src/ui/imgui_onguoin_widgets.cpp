// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace imgui_onguoin {

namespace {

const char* safe_text(const char* text) {
    return text != nullptr ? text : "";
}

ImVec4 apply_alpha(ImVec4 color, float alpha) {
    color.w *= alpha;
    return color;
}

ImVec4 resolve_color(ImVec4 value, ImVec4 fallback) {
    return value.x >= 0.0f ? value : fallback;
}

StickVisualizerPoint clamp_stick_point(StickVisualizerPoint point) {
    point.x = std::clamp(point.x, -1.0f, 1.0f);
    point.y = std::clamp(point.y, -1.0f, 1.0f);
    return point;
}

ImVec2 stick_point_to_screen(ImVec2 center, float radius, StickVisualizerPoint point) {
    const StickVisualizerPoint clamped = clamp_stick_point(point);
    return ImVec2(center.x + clamped.x * radius,
                  center.y - clamped.y * radius);
}

float sector_visual_degrees(int sectorIndex) {
    return -90.0f + 45.0f * static_cast<float>(sectorIndex);
}

float row_pulse(bool selected) {
    return selected ? (0.55f + 0.45f * std::sin(static_cast<float>(ImGui::GetTime()) * 5.4f)) : 0.0f;
}

constexpr float kPi = 3.1415926535f;

struct OptionToggleNeonState {
    float hoverProgress = 0.0f;
    float activationProgress = 0.0f;
    bool initialized = false;
};

std::unordered_map<ImGuiID, OptionToggleNeonState>& option_toggle_neon_cache() {
    static std::unordered_map<ImGuiID, OptionToggleNeonState> cache;
    return cache;
}

ImVec2 rotated_circle_point(ImVec2 center, float radius, float radians, float rotation) {
    const float angle = radians + rotation;
    return ImVec2(center.x + std::cos(angle) * radius,
                  center.y + std::sin(angle) * radius);
}

ImVec4 neon_capsule_flow_color(const NeonCapsuleGlowStyle& style, float phase) {
    ImVec4 colorA = resolve_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    ImVec4 colorB = resolve_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    ImVec4 colorC = resolve_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
    phase = std::fmod(phase, 1.0f);
    if (phase < 0.0f) {
        phase += 1.0f;
    }

    if (phase < 1.0f / 3.0f) {
        return mix_color(colorA, colorB, phase * 3.0f);
    }
    if (phase < 2.0f / 3.0f) {
        return mix_color(colorB, colorC, (phase - 1.0f / 3.0f) * 3.0f);
    }
    return mix_color(colorC, colorA, (phase - 2.0f / 3.0f) * 3.0f);
}

NeonCapsuleGlowStyle neon_capsule_style_from_option_toggle(const OptionToggleStyle& style) {
    NeonCapsuleGlowStyle result;
    result.colorA = style.flowColorA;
    result.colorB = style.flowColorB;
    result.colorC = style.flowColorC;
    result.intensity = style.flowIntensity;
    result.speed = style.flowSpeed;
    result.glowScale = style.flowGlowScale;
    result.idleFlowAlpha = style.flowIdleAlpha;
    return result;
}

ImVec4 option_toggle_flow_color(const OptionToggleStyle& style, float phase) {
    return neon_capsule_flow_color(neon_capsule_style_from_option_toggle(style), phase);
}

void draw_neon_flow_capsule(ImDrawList* drawList,
                            ImVec2 min,
                            ImVec2 max,
                            float rounding,
                            const OptionToggleStyle& style,
                            float hoverProgress,
                            float activationProgress) {
    draw_neon_capsule_glow(drawList,
                           min,
                           max,
                           rounding,
                           neon_capsule_style_from_option_toggle(style),
                           hoverProgress,
                           activationProgress);
}

void draw_neon_flow_text(ImDrawList* drawList,
                         ImVec2 min,
                         ImVec2 max,
                         const char* text,
                         const Theme& theme,
                         const OptionToggleStyle& style,
                         float hoverProgress,
                         float activationProgress,
                         ImVec2 textOffset) {
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    const float presence = std::clamp(std::max(activationProgress, hoverProgress * 0.64f) * style.flowIntensity,
                                      0.0f,
                                      1.0f);
    const float scale = current_scale();
    const float time = static_cast<float>(ImGui::GetTime());
    const float speed = std::max(0.02f, style.flowSpeed);
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const ImVec2 textPos = centered_text_pos(min, max, textSize, textOffset);
    const float travelWidth = textSize.x + 56.0f * scale;
    const float bandWidth = (22.0f + 22.0f * activationProgress) * scale;

    const float active = std::clamp(activationProgress, 0.0f, 1.0f);
    const float hoverOnly = hoverProgress * (1.0f - active);
    const float baseAlpha = std::clamp(0.50f + 0.46f * active + 0.34f * hoverOnly,
                                       0.0f,
                                       1.0f);
    ImVec4 idleText = theme.palette.textMuted;
    idleText.w = std::max(idleText.w, 0.60f);
    const ImVec4 hoverText(1.00f, 0.72f, 0.60f, baseAlpha);
    const ImVec4 enabledText(0.94f, 0.98f, 1.0f, baseAlpha);
    const ImVec4 readyText = mix_color(idleText, hoverText, std::clamp(hoverOnly, 0.0f, 1.0f));
    const ImVec4 baseText = mix_color(readyText, enabledText, active);
    drawList->AddText(textPos,
                      to_u32(baseText),
                      text);

    if (presence <= 0.002f) {
        return;
    }

    for (int band = 0; band < 3; ++band) {
        const float phase = std::fmod(time * speed * 0.72f + static_cast<float>(band) * 0.34f, 1.0f);
        const float centerX = textPos.x - 28.0f * scale + phase * travelWidth;
        ImVec4 color = option_toggle_flow_color(style, phase + static_cast<float>(band) * 0.17f);
        color = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.46f + 0.16f * activationProgress);
        color.w = (0.12f + 0.26f * activationProgress + 0.07f * hoverProgress) * presence;

        drawList->PushClipRect(ImVec2(centerX - bandWidth, textPos.y - 3.0f * scale),
                               ImVec2(centerX + bandWidth, textPos.y + textSize.y + 3.0f * scale),
                               true);
        drawList->AddText(textPos, to_u32(color), text);
        drawList->PopClipRect();
    }

    if (activationProgress > 0.02f) {
        ImVec4 soft = mix_color(option_toggle_flow_color(style, time * speed * 0.20f),
                                theme.palette.text,
                                0.58f);
        soft.w = 0.13f * activationProgress * presence;
        drawList->AddText(ImVec2(textPos.x + 0.5f * scale, textPos.y + 0.5f * scale),
                          to_u32(soft),
                          text);
    }
}


void draw_rotation_direction_arrow(ImDrawList* drawList,
                                   ImVec2 center,
                                   float radius,
                                   bool clockwise,
                                   float rotation,
                                   ImU32 color,
                                   float thickness,
                                   float headSize) {
    constexpr int kSegments = 30;
    const float start = clockwise ? 3.78f : -0.64f;
    const float sweep = clockwise ? 4.76f : -4.76f;

    drawList->PathClear();
    for (int i = 0; i <= kSegments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kSegments);
        drawList->PathLineTo(rotated_circle_point(center, radius, start + sweep * t, rotation));
    }
    drawList->PathStroke(color, 0, thickness);

    const float endAngle = start + sweep + rotation;
    const ImVec2 tip(center.x + std::cos(endAngle) * radius,
                     center.y + std::sin(endAngle) * radius);
    const float tangentAngle = endAngle + (clockwise ? kPi * 0.5f : -kPi * 0.5f);
    const ImVec2 dir(std::cos(tangentAngle), std::sin(tangentAngle));
    const ImVec2 perp(-dir.y, dir.x);
    const ImVec2 base(tip.x - dir.x * headSize, tip.y - dir.y * headSize);
    drawList->AddTriangleFilled(tip,
                                ImVec2(base.x + perp.x * headSize * 0.62f,
                                       base.y + perp.y * headSize * 0.62f),
                                ImVec2(base.x - perp.x * headSize * 0.62f,
                                       base.y - perp.y * headSize * 0.62f),
                                color);
}

struct HelpMarkerAnimationState {
    float presence = 0.0f;
    ImVec2 lastMin = ImVec2(0.0f, 0.0f);
    ImVec2 lastMax = ImVec2(0.0f, 0.0f);
    bool hasRect = false;
};

struct ConfigExchangeAnimationState {
    float presence = 0.0f;
    bool inputActive = false;
};

struct DirectionToggleAnimationState {
    float knobProgress = 0.0f;
    bool visualClockwise = true;
    bool lastClockwise = true;
    bool initialized = false;
};

struct HelpMarkerOverlayCommand {
    Theme theme{};
    std::string text;
    ImVec2 pos = ImVec2(0.0f, 0.0f);
    ImVec2 size = ImVec2(0.0f, 0.0f);
    ImFont* font = nullptr;
    float glyphFontSize = 0.0f;
    float textFontSize = 0.0f;
    float scale = 1.0f;
    float presence = 0.0f;
    bool hovered = false;
    bool active = false;
};

struct HelpMarkerOverlayLayout {
    ImVec2 islandMin = ImVec2(0.0f, 0.0f);
    ImVec2 islandMax = ImVec2(0.0f, 0.0f);
    ImVec2 iconMax = ImVec2(0.0f, 0.0f);
    ImVec2 textPos = ImVec2(0.0f, 0.0f);
    float islandRounding = 0.0f;
    float currentWrapWidth = 1.0f;
    float textAlpha = 0.0f;
    float widthProgress = 0.0f;
    float glowProgress = 0.0f;
};

std::unordered_map<ImGuiID, HelpMarkerAnimationState>& help_marker_animation_cache() {
    static std::unordered_map<ImGuiID, HelpMarkerAnimationState> cache;
    return cache;
}

std::unordered_map<ImGuiID, ConfigExchangeAnimationState>& config_exchange_animation_cache() {
    static std::unordered_map<ImGuiID, ConfigExchangeAnimationState> cache;
    return cache;
}

std::unordered_map<ImGuiID, DirectionToggleAnimationState>& direction_toggle_animation_cache() {
    static std::unordered_map<ImGuiID, DirectionToggleAnimationState> cache;
    return cache;
}

std::vector<HelpMarkerOverlayCommand>& help_marker_overlay_queue() {
    static std::vector<HelpMarkerOverlayCommand> queue;
    return queue;
}

int& help_marker_overlay_queue_frame() {
    static int frame = -1;
    return frame;
}

void prepare_help_marker_overlay_queue() {
    const int frame = ImGui::GetFrameCount();
    int& queuedFrame = help_marker_overlay_queue_frame();
    if (queuedFrame != frame) {
        help_marker_overlay_queue().clear();
        queuedFrame = frame;
    }
}

ImVec2 calc_wrapped_text_size(ImFont* font, float fontSize, float wrapWidth, const char* text) {
    if (font == nullptr || text == nullptr || text[0] == '\0') {
        return ImVec2(0.0f, 0.0f);
    }
    return font->CalcTextSizeA(fontSize, 100000.0f, std::max(1.0f, wrapWidth), text);
}

ImVec2 calc_unwrapped_text_size(ImFont* font, float fontSize, const char* text) {
    if (font == nullptr || text == nullptr || text[0] == '\0') {
        return ImVec2(0.0f, 0.0f);
    }
    return font->CalcTextSizeA(fontSize, 100000.0f, 0.0f, text);
}

ImVec4 help_marker_fill_color(const Theme& theme, bool hovered, bool active) {
    const Palette& palette = theme.palette;
    const HelpMarkerWidgetTokens& helpTokens = theme.widgets.helpMarker;
    return active
        ? palette.surfaceRaised
        : (hovered ? mix_color(palette.surfaceRaised, palette.accentMuted, 0.36f)
                   : helpTokens.collapsedFill);
}

ImVec4 help_marker_border_color(const Theme& theme, bool hovered) {
    const Palette& palette = theme.palette;
    const HelpMarkerWidgetTokens& helpTokens = theme.widgets.helpMarker;
    return hovered
        ? ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.72f)
        : ImVec4(palette.border.x, palette.border.y, palette.border.z, helpTokens.collapsedBorderAlpha);
}

HelpMarkerOverlayLayout make_help_marker_overlay_layout(const HelpMarkerOverlayCommand& command) {
    const HelpMarkerWidgetTokens& helpTokens = command.theme.widgets.helpMarker;
    const float scale = command.scale;
    const float rounding = helpTokens.helpMarkerRounding * scale;
    const ImVec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
    const ImVec2 viewportMax(viewportPos.x + ImGui::GetMainViewport()->WorkSize.x,
                             viewportPos.y + ImGui::GetMainViewport()->WorkSize.y);
    const float viewportMargin = 10.0f * scale;
    const float collapsedW = std::max(command.size.x, helpTokens.tooltipCollapsedSize * scale);
    const float collapsedH = std::max(command.size.y, helpTokens.tooltipCollapsedSize * scale);
    const float paddingX = helpTokens.tooltipPaddingX * scale;
    const float paddingY = helpTokens.tooltipPaddingY * scale;
    const float textInsetX = collapsedW + helpTokens.tooltipIconTextGap * scale;
    const float availableW = std::max(collapsedW, viewportMax.x - command.pos.x - viewportMargin);
    const float maxExpandedW = std::max(collapsedW,
                                        std::min(helpTokens.tooltipExpandedMaxWidth * scale, availableW));
    const float minExpandedW = std::min(std::max(collapsedW, helpTokens.tooltipExpandedMinWidth * scale),
                                        maxExpandedW);
    const float maxTextWrapWidth = std::max(1.0f, maxExpandedW - textInsetX - paddingX);
    const float preferredWrapWidth = std::min(helpTokens.tooltipWrapWidth * ImGui::GetFontSize(),
                                              maxTextWrapWidth);
    ImVec2 textSize = calc_wrapped_text_size(command.font,
                                             command.textFontSize,
                                             preferredWrapWidth,
                                             command.text.c_str());
    const float expandedW = std::clamp(textInsetX + textSize.x + paddingX,
                                       minExpandedW,
                                       maxExpandedW);
    const float finalWrapWidth = std::max(1.0f, expandedW - textInsetX - paddingX);
    textSize = calc_wrapped_text_size(command.font,
                                      command.textFontSize,
                                      finalWrapWidth,
                                      command.text.c_str());
    const float expandedH = std::max(collapsedH, textSize.y + paddingY * 2.0f);

    const float widthProgress = ease(EasingCurve::OutCubic,
                                     std::clamp((command.presence - 0.01f) / 0.78f, 0.0f, 1.0f));
    const float heightProgress = ease(EasingCurve::OutCubic,
                                      std::clamp((command.presence - 0.16f) / 0.84f, 0.0f, 1.0f));
    const float textProgress = ease(EasingCurve::OutCubic,
                                    std::clamp((command.presence - 0.42f) / 0.58f, 0.0f, 1.0f));
    const float islandW = collapsedW + (expandedW - collapsedW) * widthProgress;
    const float islandH = collapsedH + (expandedH - collapsedH) * heightProgress;
    const float currentWrapWidth = std::max(1.0f, islandW - textInsetX - paddingX);
    const ImVec2 currentTextSize = calc_wrapped_text_size(command.font,
                                                          command.textFontSize,
                                                          currentWrapWidth,
                                                          command.text.c_str());
    const float availableTextHeight = std::max(0.0f, islandH - paddingY * 2.0f);
    const float textWidthGate = std::clamp((islandW - textInsetX - 4.0f * scale) /
                                               std::max(1.0f, expandedW - textInsetX - 4.0f * scale),
                                           0.0f,
                                           1.0f);
    const float textHeightGate = std::clamp(availableTextHeight / std::max(1.0f, currentTextSize.y),
                                            0.0f,
                                            1.0f);

    HelpMarkerOverlayLayout layout;
    layout.islandMin = command.pos;
    layout.islandMax = ImVec2(command.pos.x + islandW, command.pos.y + islandH);
    layout.iconMax = ImVec2(command.pos.x + collapsedW, command.pos.y + collapsedH);
    layout.islandRounding = std::min(rounding, std::min(islandW, islandH) * 0.5f);
    layout.currentWrapWidth = currentWrapWidth;
    layout.textAlpha = textProgress * textWidthGate * textHeightGate;
    layout.widthProgress = widthProgress;
    layout.glowProgress = std::clamp((command.presence - 0.12f) / 0.88f, 0.0f, 1.0f);
    layout.textPos = ImVec2(command.pos.x + textInsetX,
                            command.pos.y + paddingY +
                                std::max(0.0f, (availableTextHeight - currentTextSize.y) * 0.5f) +
                                helpTokens.tooltipTextOffsetY * scale);
    return layout;
}

void draw_help_marker_collapsed_visual(ImDrawList* drawList,
                                       ImFont* font,
                                       float glyphFontSize,
                                       ImVec2 pos,
                                       ImVec2 size,
                                       const Theme& theme,
                                       bool hovered,
                                       bool active,
                                       float scale) {
    const Palette& palette = theme.palette;
    const HelpMarkerWidgetTokens& helpTokens = theme.widgets.helpMarker;
    const ImVec2 max(pos.x + size.x, pos.y + size.y);
    const float rounding = helpTokens.helpMarkerRounding * scale;
    for (int layer = 3; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 3.0f;
        const float offset = (1.5f + 2.0f * t) * scale;
        const float alpha = (hovered ? helpTokens.hoveredGlowAlpha : helpTokens.collapsedGlowAlpha) * t;
        drawList->AddRect(ImVec2(pos.x - offset, pos.y - offset),
                          ImVec2(max.x + offset, max.y + offset),
                          to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, alpha)),
                          rounding + offset,
                          0,
                          1.0f * scale);
    }

    drawList->AddRectFilled(pos, max, to_u32(help_marker_fill_color(theme, hovered, active)), rounding);
    drawList->AddRect(pos, max, to_u32(help_marker_border_color(theme, hovered)), rounding, 0, 1.15f * scale);
    drawList->AddRect(ImVec2(pos.x + 1.0f * scale, pos.y + 1.0f * scale),
                      ImVec2(max.x - 1.0f * scale, max.y - 1.0f * scale),
                      to_u32(ImVec4(1.0f,
                                    1.0f,
                                    1.0f,
                                    hovered ? helpTokens.hoveredInnerHighlightAlpha
                                            : helpTokens.collapsedInnerHighlightAlpha)),
                      std::max(0.0f, rounding - 1.0f * scale),
                      0,
                      1.0f * scale);

    const ImVec2 q = calc_unwrapped_text_size(font, glyphFontSize, "?");
    drawList->AddText(font,
                      glyphFontSize,
                      centered_text_pos(pos,
                                        max,
                                        q,
                                        ImVec2(0.0f, helpTokens.helpMarkerGlyphOffsetY * scale)),
                      to_u32(hovered ? palette.accent : palette.text),
                      "?");
}

void draw_help_marker_overlay_command(const HelpMarkerOverlayCommand& command) {
    const Theme& theme = command.theme;
    const Palette& palette = theme.palette;
    const HelpMarkerWidgetTokens& helpTokens = theme.widgets.helpMarker;
    const float scale = command.scale;
    const HelpMarkerOverlayLayout layout = make_help_marker_overlay_layout(command);
    ImDrawList* foreground = ImGui::GetForegroundDrawList();

    for (int layer = 5; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 5.0f;
        const float offset = (1.8f + 5.8f * t) * scale;
        const int shadowAlpha = static_cast<int>((8.0f + 38.0f * layout.glowProgress) * t * t);
        if (shadowAlpha <= 0) {
            continue;
        }
        foreground->AddRectFilled(ImVec2(layout.islandMin.x + offset,
                                         layout.islandMin.y + offset + 3.0f * scale),
                                  ImVec2(layout.islandMax.x + offset * 0.12f,
                                         layout.islandMax.y + offset + 5.0f * scale),
                                  IM_COL32(0, 0, 0, shadowAlpha),
                                  layout.islandRounding + offset * 0.35f);
    }

    for (int layer = 4; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 4.0f;
        const float offset = (0.8f + 2.0f * t) * scale;
        foreground->AddRect(ImVec2(layout.islandMin.x - offset, layout.islandMin.y - offset),
                            ImVec2(layout.islandMax.x + offset, layout.islandMax.y + offset),
                            to_u32(ImVec4(palette.accent.x,
                                          palette.accent.y,
                                          palette.accent.z,
                                          (command.hovered ? helpTokens.hoveredGlowAlpha
                                                           : helpTokens.collapsedGlowAlpha) * t +
                                              0.075f * layout.glowProgress * t)),
                            layout.islandRounding + offset,
                            0,
                            1.0f * scale);
    }

    const ImVec4 fill = help_marker_fill_color(theme, command.hovered, command.active);
    const ImVec4 border = help_marker_border_color(theme, command.hovered);
    const ImVec4 islandFill = mix_color(fill, ImVec4(0.0f, 0.0f, 0.0f, 0.90f), layout.widthProgress * 0.85f);
    foreground->AddRectFilled(layout.islandMin,
                              layout.islandMax,
                              to_u32(islandFill),
                              layout.islandRounding);
    foreground->AddRect(layout.islandMin,
                        layout.islandMax,
                        to_u32(ImVec4(border.x,
                                      border.y,
                                      border.z,
                                      std::min(0.92f, border.w + 0.22f * layout.glowProgress))),
                        layout.islandRounding,
                        0,
                        1.1f * scale);
    foreground->AddRect(ImVec2(layout.islandMin.x + 1.0f * scale, layout.islandMin.y + 1.0f * scale),
                        ImVec2(layout.islandMax.x - 1.0f * scale, layout.islandMax.y - 1.0f * scale),
                        to_u32(ImVec4(1.0f,
                                      1.0f,
                                      1.0f,
                                      command.hovered ? helpTokens.hoveredInnerHighlightAlpha
                                                      : helpTokens.collapsedInnerHighlightAlpha)),
                        std::max(0.0f, layout.islandRounding - 1.0f * scale),
                        0,
                        1.0f * scale);

    foreground->PushClipRect(layout.islandMin, layout.islandMax, true);
    const ImVec2 q = calc_unwrapped_text_size(command.font, command.glyphFontSize, "?");
    foreground->AddText(command.font,
                        command.glyphFontSize,
                        centered_text_pos(layout.islandMin,
                                          layout.iconMax,
                                          q,
                                          ImVec2(0.0f, helpTokens.helpMarkerGlyphOffsetY * scale)),
                        to_u32(command.hovered ? palette.accent : palette.text),
                        "?");

    if (layout.textAlpha > 0.002f) {
        foreground->AddText(command.font,
                            command.textFontSize,
                            layout.textPos,
                            to_u32(ImVec4(0.93f, 0.95f, 0.98f, 0.94f * layout.textAlpha)),
                            command.text.c_str(),
                            nullptr,
                            layout.currentWrapWidth);
    }
    foreground->PopClipRect();
}

const char* resolve_background_summary_text(const char* value, const char* fallback) {
    return value != nullptr && value[0] != '\0' ? value : fallback;
}

const char* resolve_theme_summary_text(const char* value, const char* fallback) {
    return value != nullptr && value[0] != '\0' ? value : fallback;
}

const char* localized_theme_summary_fallback(UiTextLanguage language, const char* english, const char* chinese) {
    return language == UiTextLanguage::ChineseSimplified ? chinese : english;
}

const char* localized_background_summary_fallback(UiTextLanguage language, const char* english, const char* chinese) {
    return language == UiTextLanguage::ChineseSimplified ? chinese : english;
}

const char* localized_theme_selection_summary_fallback(UiTextLanguage language, const char* english, const char* chinese) {
    return language == UiTextLanguage::ChineseSimplified ? chinese : english;
}

const char* localized_background_preview_fallback(UiTextLanguage language, const char* english, const char* chinese) {
    return language == UiTextLanguage::ChineseSimplified ? chinese : english;
}

float resolve_theme_selection_label_width(UiTextLanguage language, float fallbackScaleWidth) {
    if (fallbackScaleWidth >= 0.0f) {
        return fallbackScaleWidth;
    }
    return language == UiTextLanguage::English ? 132.0f : 86.0f;
}

float resolve_background_summary_label_width(UiTextLanguage language, float fallbackScaleWidth) {
    if (fallbackScaleWidth >= 0.0f) {
        return fallbackScaleWidth;
    }
    return language == UiTextLanguage::English ? 108.0f : 86.0f;
}

BackgroundStyle resolve_background_summary_style(const BackgroundSummaryData& data) {
    if (data.style != nullptr) {
        return *data.style;
    }
    return make_background_style(data.kind);
}

BackgroundStyle resolve_background_preview_style(const BackgroundPreviewData& data) {
    BackgroundStyle style = data.style != nullptr
        ? *data.style
        : make_background_style(data.kind);
    if (data.tuning != nullptr) {
        style = apply_background_tuning(style, *data.tuning);
    }
    return style;
}

void draw_preview_sweep(ImDrawList* drawList,
                        ImVec2 min,
                        ImVec2 max,
                        const Theme& theme,
                        float alpha) {
    const float scale = current_scale();
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    if (width <= 1.0f || height <= 1.0f || alpha <= 0.0f) {
        return;
    }

    const float bandWidth = std::max(38.0f * scale, width * 0.14f);
    const float skew = height * 0.70f;
    const float sweepSpan = width + bandWidth * 2.0f;
    const float sweep = travel(theme.motion.flowLine, sweepSpan) - bandWidth;
    const float x0 = min.x + sweep;
    const float x1 = x0 + bandWidth;
    const ImVec4 accent = theme.palette.accent;
    const ImU32 faint = to_u32(ImVec4(accent.x, accent.y, accent.z, alpha * 0.10f));
    const ImU32 soft = to_u32(ImVec4(accent.x, accent.y, accent.z, alpha * 0.22f));
    const ImU32 strong = to_u32(ImVec4(accent.x, accent.y, accent.z, alpha * 0.34f));

    const ImVec2 outer[] = {
        ImVec2(x0 - bandWidth * 0.28f, min.y),
        ImVec2(x1, min.y),
        ImVec2(x1 + skew, max.y),
        ImVec2(x0 + skew, max.y),
    };
    const ImVec2 inner[] = {
        ImVec2(x0 + bandWidth * 0.10f, min.y),
        ImVec2(x1 + bandWidth * 0.12f, min.y),
        ImVec2(x1 + skew + bandWidth * 0.06f, max.y),
        ImVec2(x0 + skew + bandWidth * 0.08f, max.y),
    };
    const ImVec2 core[] = {
        ImVec2(x0 + bandWidth * 0.28f, min.y),
        ImVec2(x1 - bandWidth * 0.04f, min.y),
        ImVec2(x1 + skew - bandWidth * 0.12f, max.y),
        ImVec2(x0 + skew + bandWidth * 0.18f, max.y),
    };
    drawList->AddConvexPolyFilled(outer, 4, faint);
    drawList->AddConvexPolyFilled(inner, 4, soft);
    drawList->AddConvexPolyFilled(core, 4, strong);
}

void draw_background_preview_name_pill(ImDrawList* drawList,
                                       ImVec2 min,
                                       ImVec2 max,
                                       const BackgroundPreviewData& data,
                                       const Theme& theme,
                                       float inset,
                                       float pulse) {
    const float scale = current_scale();
    const char* themeName = theme_flavor_label(data.flavor, data.language);
    const char* backgroundName = background_kind_label(data.kind, data.language);
    const std::string text = std::string(safe_text(themeName)) + " / " + safe_text(backgroundName);
    const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    const float paddingX = 10.0f * scale;
    const float pillHeight = std::max(22.0f * scale, textSize.y + 8.0f * scale);
    const float maxPillWidth = std::max(0.0f, (max.x - min.x) - inset * 2.0f);
    const float pillWidth = std::min(maxPillWidth, textSize.x + paddingX * 2.0f);
    if (pillWidth <= 1.0f || pillHeight <= 1.0f) {
        return;
    }

    const ImVec2 pillMin(min.x + inset, min.y + inset);
    const ImVec2 pillMax(pillMin.x + pillWidth, pillMin.y + pillHeight);
    const Palette& palette = theme.palette;
    const ImVec4 fill = mix_color(palette.sidebar, palette.surfaceRaised, 0.34f);
    InteractionVisualState state;
    CapsuleStyle style;
    style.rounding = pillHeight * 0.5f;
    style.fill = ImVec4(fill.x, fill.y, fill.z, 0.56f);
    style.textColor = ImVec4(palette.text.x, palette.text.y, palette.text.z, 0.92f);
    style.frame.rounding = style.rounding;
    style.frame.color = palette.accent;
    style.frame.idleBorderAlpha = 0.18f + 0.05f * pulse;
    style.frame.idleGlowAlpha = 0.020f + 0.018f * pulse;
    style.frame.idleThickness = 1.0f;
    draw_capsule(drawList, pillMin, pillMax, text.c_str(), theme, state, style);
}

std::string make_background_capability_summary(const BackgroundSummaryData& data) {
    const BackgroundStyle style = resolve_background_summary_style(data);
    const int enabledCount = enabled_background_layer_count(style);
    const int dynamicCount = background_layer_mask_count(
        enabled_background_layers(style) & background_dynamic_layers(data.kind));
    return std::to_string(enabledCount) + " layers, " + std::to_string(dynamicCount) + " dynamic";
}

std::string make_background_layer_summary(const BackgroundSummaryData& data) {
    const BackgroundStyle style = resolve_background_summary_style(data);
    const BackgroundLayerMask enabledLayers = enabled_background_layers(style);
    std::string text;
    int layerCount = 0;
    const BackgroundLayerInfo* layers = background_layer_infos(&layerCount);
    const char* dynamicSuffix = resolve_background_summary_text(
        data.text.dynamicSuffix,
        localized_background_summary_fallback(data.language, " (dynamic)", "\xEF\xBC\x88\xE5\x8A\xA8\xE6\x80\x81\xEF\xBC\x89"));
    const char* separator = ", ";

    for (int index = 0; index < layerCount; ++index) {
        const BackgroundLayerInfo& layer = layers[index];
        if (!background_layer_mask_has(enabledLayers, layer.kind)) {
            continue;
        }
        if (!text.empty()) {
            text += separator;
        }
        text += background_layer_label(layer, data.language);
        if (background_layer_is_dynamic(data.kind, layer.kind)) {
            text += dynamicSuffix;
        }
    }

    if (!text.empty()) {
        return text;
    }
    return resolve_background_summary_text(
        data.text.noLayersText,
        localized_background_summary_fallback(data.language, "No shared layers declared", "\xE6\x9C\xAA\xE5\xA3\xB0\xE6\x98\x8E\xE5\x85\xB1\xE4\xBA\xAB\xE5\xB1\x82"));
}

const char* theme_motion_summary_text(const ThemeSummaryData& data) {
    return theme_flavor_has_expressive_motion(data.flavor)
        ? resolve_theme_summary_text(
              data.text.expressiveMotion,
              localized_theme_summary_fallback(data.language, "Expressive motion", "\xE6\x9B\xB4\xE6\x9C\x89\xE8\xA1\xA8\xE7\x8E\xB0\xE5\x8A\x9B\xE7\x9A\x84\xE5\x8A\xA8\xE6\x95\x88"))
        : resolve_theme_summary_text(
              data.text.restrainedMotion,
              localized_theme_summary_fallback(data.language, "Restrained motion", "\xE6\x9B\xB4\xE5\x85\x8B\xE5\x88\xB6\xE7\x9A\x84\xE5\x8A\xA8\xE6\x95\x88"));
}

const char* resolve_theme_selection_summary_text(const char* value, const char* fallback) {
    return value != nullptr && value[0] != '\0' ? value : fallback;
}

std::string make_theme_selection_preset_text(const ThemeSelectionSummaryData& data) {
    int presetCount = 0;
    const ThemePresetInfo* presets = theme_preset_infos(&presetCount);
    for (int index = 0; index < presetCount; ++index) {
        const ThemePresetInfo& preset = presets[index];
        if (theme_selection_matches_preset(data.selection, preset.preset)) {
            return theme_preset_label(preset, data.language);
        }
    }
    return resolve_theme_selection_summary_text(
        data.text.customPreset,
        localized_theme_selection_summary_fallback(data.language, "Custom", "\xE8\x87\xAA\xE5\xAE\x9A\xE4\xB9\x89"));
}

std::string make_theme_selection_mode_text(const ThemeSelectionSummaryData& data) {
    int presetCount = 0;
    const ThemePresetInfo* presets = theme_preset_infos(&presetCount);
    const char* suffix = resolve_theme_selection_summary_text(
        data.text.matchedPresetSuffix,
        localized_theme_selection_summary_fallback(data.language, "preset-aligned", "\xE9\xA2\x84\xE8\xAE\xBE\xE7\xBB\x84\xE5\x90\x88"));
    for (int index = 0; index < presetCount; ++index) {
        const ThemePresetInfo& preset = presets[index];
        if (theme_selection_matches_preset(data.selection, preset.preset)) {
            return std::string(theme_preset_label(preset, data.language)) + " " + suffix;
        }
    }
    return resolve_theme_selection_summary_text(
        data.text.customPreset,
        localized_theme_selection_summary_fallback(data.language, "Custom", "\xE8\x87\xAA\xE5\xAE\x9A\xE4\xB9\x89"));
}

} // namespace

void draw_page_hero(const PageHeroText& hero, const Theme& theme) {
    const Palette& palette = theme.palette;
    const SpacingTokens& spacing = theme.spacing;
    const HeroWidgetTokens& heroTokens = theme.widgets.hero;
    const float scale = current_scale();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;

    if (hero.eyebrow != nullptr && hero.eyebrow[0] != '\0') {
        ImGui::TextDisabled("%s", hero.eyebrow);
        ImGui::Dummy(ImVec2(0.0f, spacing.heroLeadGap * scale));
    }

    const ImVec2 titleAnchor = ImGui::GetCursorScreenPos();
    drawList->AddRectFilled(titleAnchor,
                            ImVec2(titleAnchor.x + spacing.heroAccentWidth * scale, titleAnchor.y + heroTokens.heroAccentHeight * scale),
                            to_u32(palette.accent),
                            2.0f * scale);

    if (hero.status != nullptr && hero.status[0] != '\0') {
        const ImVec2 statusSize = measure_text_block(hero.status).size;
        drawList->AddText(trailing_text_pos(start,
                                            ImVec2(start.x + width, titleAnchor.y + heroTokens.heroAccentHeight * scale),
                                            statusSize,
                                            0.0f,
                                            ImVec2(0.0f, heroTokens.heroStatusOffsetY * scale)),
                          to_u32(palette.accent),
                          hero.status);
    }

    ImGui::SetCursorScreenPos(ImVec2(titleAnchor.x + heroTokens.heroAccentInset * scale,
                                     titleAnchor.y + heroTokens.heroTitleOffsetY * scale));
    ImGui::SetWindowFontScale(heroTokens.heroTitleScale);
    ImGui::TextUnformatted(safe_text(hero.title));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Dummy(ImVec2(0.0f, 2.0f * scale));
    ImGui::PushStyleColor(ImGuiCol_Text, palette.textMuted);
    ImGui::TextWrapped("%s", safe_text(hero.subtitle));
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, spacing.heroTailGap * scale));

    const ImVec2 linePos = ImGui::GetCursorScreenPos();
    draw_flow_line(drawList, linePos, ImVec2(linePos.x + width, linePos.y), theme, 0.20f);
    ImGui::Dummy(ImVec2(0.0f, spacing.heroFlowGap * scale));
}

void draw_footer_signature(ImDrawList* drawList,
                           ImVec2 min,
                           float width,
                           const char* text,
                           const Theme& theme,
                           FooterSignatureStyle style) {
    const FooterSignatureWidgetTokens& footerTokens = theme.widgets.footerSignature;
    const float scale = current_scale();
    ImFont* font = ImGui::GetFont();
    const float fontScale = style.fontScale >= 0.0f ? style.fontScale : footerTokens.fontScale;
    const float fontSize = ImGui::GetFontSize() * fontScale;
    const float lineWidth = (style.lineWidth >= 0.0f ? style.lineWidth : footerTokens.lineWidth) * scale;
    const float gap = (style.gap >= 0.0f ? style.gap : footerTokens.gap) * scale;
    const char* resolvedText = safe_text(text);
    const ImVec2 textSize = font->CalcTextSizeA(fontSize, 100000.0f, 0.0f, resolvedText);
    const float groupWidth = lineWidth + gap + textSize.x;
    const float x = min.x + std::max(0.0f, (width - groupWidth) * 0.5f);
    const float y = min.y;
    const float midY = y + textSize.y * 0.5f + 0.5f * scale;
    const ImVec4 lineColor = style.lineColor.x >= 0.0f
        ? apply_alpha(style.lineColor, style.alpha)
        : ImVec4(theme.palette.accent.x,
                 theme.palette.accent.y,
                 theme.palette.accent.z,
                 footerTokens.lineAlpha * style.alpha);
    const ImVec4 textColor = style.textColor.x >= 0.0f
        ? apply_alpha(style.textColor, style.alpha)
        : ImVec4(theme.palette.textMuted.x,
                 theme.palette.textMuted.y,
                 theme.palette.textMuted.z,
                 style.alpha);

    drawList->AddLine(ImVec2(x, midY),
                      ImVec2(x + lineWidth, midY),
                      to_u32(lineColor),
                      1.2f * scale);
    drawList->AddText(font,
                      fontSize,
                      ImVec2(x + lineWidth + gap, y),
                      to_u32(textColor),
                      resolvedText);
}

float draw_identity_capsule(const IdentityCapsuleData& data,
                            const Theme& theme,
                            IdentityCapsuleStyle style) {
    const Palette& palette = theme.palette;
    const IdentityCapsuleWidgetTokens& tokens = theme.widgets.identityCapsule;
    const SurfaceTokens& surfaces = theme.surfaces;
    const float scale = current_scale();
    ImFont* font = ImGui::GetFont();
    const float baseFontSize = ImGui::GetFontSize();
    const float labelFontScale = style.labelFontScale >= 0.0f ? style.labelFontScale : tokens.labelFontScale;
    const float valueFontScale = style.valueFontScale >= 0.0f ? style.valueFontScale : tokens.valueFontScale;
    const float labelFontSize = baseFontSize * labelFontScale;
    const float valueFontSize = baseFontSize * valueFontScale;
    const float horizontalPadding = (style.horizontalPadding >= 0.0f ? style.horizontalPadding : tokens.horizontalPadding) * scale;
    const float textGap = (style.textGap >= 0.0f ? style.textGap : tokens.textGap) * scale;
    const float height = (style.height >= 0.0f ? style.height : tokens.height) * scale;
    const float minWidth = (style.minWidth >= 0.0f ? style.minWidth : tokens.minWidth) * scale;
    const float maxWidth = (style.maxWidth >= 0.0f ? style.maxWidth : tokens.maxWidth) * scale;
    const float rounding = (style.rounding >= 0.0f ? style.rounding : tokens.rounding) * scale;
    const float textOffsetY = (style.textOffsetY >= 0.0f ? style.textOffsetY : tokens.textOffsetY) * scale;
    const char* label = safe_text(data.label);
    const char* value = safe_text(data.value);
    const bool hasLabel = label[0] != '\0';
    const ImVec2 labelSize = font->CalcTextSizeA(labelFontSize, 100000.0f, 0.0f, label);
    const ImVec2 valueSize = font->CalcTextSizeA(valueFontSize, 100000.0f, 0.0f, value);
    const float contentWidth = valueSize.x + (hasLabel ? labelSize.x + textGap : 0.0f);
    const float width = std::clamp(contentWidth + horizontalPadding * 2.0f,
                                   minWidth,
                                   maxWidth);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max(min.x + width, min.y + height);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec4 baseFill = mix_color(palette.background, surfaces.frameBg, 0.42f);

    const ImVec4 fillColor = style.fillColor.x >= 0.0f
        ? style.fillColor
        : ImVec4(baseFill.x, baseFill.y, baseFill.z, 0.96f);
    const ImVec4 borderColor = style.borderColor.x >= 0.0f
        ? style.borderColor
        : ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, tokens.borderAlpha);
    const ImVec4 innerBorderColor = style.innerBorderColor.x >= 0.0f
        ? style.innerBorderColor
        : ImVec4(1.0f, 1.0f, 1.0f, tokens.innerBorderAlpha);
    const ImVec4 labelColor = style.labelColor.x >= 0.0f
        ? style.labelColor
        : ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, tokens.labelAlpha);
    const ImVec4 valueColor = style.valueColor.x >= 0.0f
        ? style.valueColor
        : ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, tokens.valueAlpha);

    drawList->AddRectFilled(min, max, to_u32(fillColor), rounding);
    drawList->AddRect(min, max, to_u32(borderColor), rounding, 0, 1.0f * scale);
    drawList->AddRect(ImVec2(min.x + 1.0f * scale, min.y + 1.0f * scale),
                      ImVec2(max.x - 1.0f * scale, max.y - 1.0f * scale),
                      to_u32(innerBorderColor),
                      std::max(0.0f, rounding - 1.0f * scale),
                      0,
                      1.0f * scale);

    const float labelY = min.y + (height - labelSize.y) * 0.5f + textOffsetY;
    const float valueY = min.y + (height - valueSize.y) * 0.5f + textOffsetY;
    const float labelX = hasLabel ? min.x + horizontalPadding : min.x + (width - valueSize.x) * 0.5f;
    if (hasLabel) {
        drawList->AddText(font, labelFontSize, ImVec2(labelX, labelY), to_u32(labelColor), label);
    }
    drawList->AddText(font,
                      valueFontSize,
                      ImVec2(hasLabel ? labelX + labelSize.x + textGap : labelX, valueY),
                      to_u32(valueColor),
                      value);

    ImGui::Dummy(ImVec2(width, height));
    return width;
}

WidgetFrameLayout draw_compact_overlay_shell(ImDrawList* drawList,
                                             ImVec2 rootPos,
                                             ImVec2 windowSize,
                                             const CompactOverlayShellData& data,
                                             const Theme& theme,
                                             CompactOverlayShellStyle style) {
    const Palette& palette = theme.palette;
    const CompactOverlayShellWidgetTokens& tokens = theme.widgets.compactOverlayShell;
    const float scale = current_scale();
    const float frameInset = (style.frameInset >= 0.0f ? style.frameInset : tokens.frameInset) * scale;
    const float frameRounding = (style.frameRounding >= 0.0f ? style.frameRounding : tokens.frameRounding) * scale;
    const float frameBorderThickness = (style.frameBorderThickness >= 0.0f ? style.frameBorderThickness : tokens.frameBorderThickness) * scale;
    const float frameGlowAlpha = style.frameGlowAlpha >= 0.0f ? style.frameGlowAlpha : tokens.frameGlowAlpha;
    const float headerInsetX = (style.headerInsetX >= 0.0f ? style.headerInsetX : tokens.headerInsetX) * scale;
    const float headerTitleOffsetY = (style.headerTitleOffsetY >= 0.0f ? style.headerTitleOffsetY : tokens.headerTitleOffsetY) * scale;
    const float headerValueOffsetY = (style.headerValueOffsetY >= 0.0f ? style.headerValueOffsetY : tokens.headerValueOffsetY) * scale;
    const float headerGap = (style.headerGap >= 0.0f ? style.headerGap : tokens.headerGap) * scale;
    const float headerDividerY = (style.headerDividerY >= 0.0f ? style.headerDividerY : tokens.headerDividerY) * scale;
    const float centerStatusOffsetY = (style.centerStatusOffsetY >= 0.0f ? style.centerStatusOffsetY : tokens.centerStatusOffsetY) * scale;
    const float centerStatusPaddingX = (style.centerStatusPaddingX >= 0.0f ? style.centerStatusPaddingX : tokens.centerStatusPaddingX) * scale;
    const float centerStatusPaddingY = (style.centerStatusPaddingY >= 0.0f ? style.centerStatusPaddingY : tokens.centerStatusPaddingY) * scale;
    const float centerStatusRounding = (style.centerStatusRounding >= 0.0f ? style.centerStatusRounding : tokens.centerStatusRounding) * scale;
    const float centerStatusFillAlpha = style.centerStatusFillAlpha >= 0.0f ? style.centerStatusFillAlpha : tokens.centerStatusFillAlpha;
    const float centerStatusBorderAlpha = style.centerStatusBorderAlpha >= 0.0f ? style.centerStatusBorderAlpha : tokens.centerStatusBorderAlpha;
    const ImVec2 frameMin(rootPos.x + frameInset, rootPos.y + frameInset);
    const ImVec2 frameMax(rootPos.x + windowSize.x - frameInset, rootPos.y + windowSize.y - frameInset);

    const ImVec4 frameColor = style.frameColor.x >= 0.0f ? style.frameColor : palette.accent;
    const ImVec4 compactText = mix_color(palette.textMuted, palette.accent, 0.18f);
    const ImVec4 titleDefaultColor(compactText.x, compactText.y, compactText.z, 0.92f);
    const ImVec4 statusLabelDefaultColor(compactText.x, compactText.y, compactText.z, 0.78f);
    const ImVec4 titleColor = style.titleColor.x >= 0.0f ? style.titleColor : titleDefaultColor;
    const ImVec4 statusLabelColor = style.statusLabelColor.x >= 0.0f ? style.statusLabelColor : statusLabelDefaultColor;
    const ImVec4 statusValueColor = style.statusValueColor.x >= 0.0f ? style.statusValueColor : palette.accent;
    const ImVec4 centerStatusColor = style.centerStatusColor.x >= 0.0f ? style.centerStatusColor : palette.accent;
    const ImVec4 dividerColor = style.dividerColor.x >= 0.0f ? style.dividerColor : palette.accent;

    drawList->AddRect(frameMin,
                      frameMax,
                      to_u32(frameColor),
                      frameRounding,
                      0,
                      frameBorderThickness);
    draw_glow_rect(drawList, frameMin, frameMax, frameRounding, theme, frameGlowAlpha);

    const char* title = safe_text(data.title);
    const char* statusLabel = safe_text(data.statusLabel);
    const char* statusValue = safe_text(data.statusValue);
    const char* centerStatusText = safe_text(data.centerStatusText);
    drawList->AddText(ImVec2(frameMin.x + headerInsetX, frameMin.y + headerTitleOffsetY),
                      to_u32(titleColor),
                      title);

    const ImVec2 statusValueSize = ImGui::CalcTextSize(statusValue);
    const ImVec2 statusLabelSize = ImGui::CalcTextSize(statusLabel);
    const float statusValueX = frameMax.x - headerInsetX - statusValueSize.x;
    const float statusLabelX = statusValueX - headerGap - statusLabelSize.x;
    drawList->AddText(ImVec2(statusLabelX, frameMin.y + headerValueOffsetY),
                      to_u32(statusLabelColor),
                      statusLabel);
    drawList->AddText(ImVec2(statusValueX, frameMin.y + headerValueOffsetY),
                      to_u32(statusValueColor),
                      statusValue);
    if (centerStatusText[0] != '\0') {
        const ImVec2 centerStatusTextSize = ImGui::CalcTextSize(centerStatusText);
        const ImVec2 centerStatusSize(centerStatusTextSize.x + centerStatusPaddingX * 2.0f,
                                      centerStatusTextSize.y + centerStatusPaddingY * 2.0f);
        const float preferredCenterX = frameMin.x + (frameMax.x - frameMin.x) * 0.5f;
        const float titleRight = frameMin.x + headerInsetX + ImGui::CalcTextSize(title).x + headerGap;
        const float statusLeft = statusLabelX - headerGap;
        float centerStatusMinX = preferredCenterX - centerStatusSize.x * 0.5f;
        centerStatusMinX = std::max(centerStatusMinX, titleRight);
        centerStatusMinX = std::min(centerStatusMinX, statusLeft - centerStatusSize.x);
        const ImVec2 centerStatusMin(centerStatusMinX, frameMin.y + centerStatusOffsetY);
        const ImVec2 centerStatusMax(centerStatusMin.x + centerStatusSize.x,
                                     centerStatusMin.y + centerStatusSize.y);
        drawList->AddRectFilled(centerStatusMin,
                                centerStatusMax,
                                to_u32(ImVec4(palette.panel.x, palette.panel.y, palette.panel.z, centerStatusFillAlpha)),
                                centerStatusRounding);
        drawList->AddRect(centerStatusMin,
                          centerStatusMax,
                          to_u32(ImVec4(centerStatusColor.x, centerStatusColor.y, centerStatusColor.z, centerStatusBorderAlpha)),
                          centerStatusRounding);
        drawList->AddText(ImVec2(centerStatusMin.x + centerStatusPaddingX,
                                 centerStatusMin.y + centerStatusPaddingY),
                          to_u32(centerStatusColor),
                          centerStatusText);
    }
    drawList->AddLine(ImVec2(frameMin.x + headerInsetX, frameMin.y + headerDividerY),
                      ImVec2(frameMax.x - headerInsetX, frameMin.y + headerDividerY),
                      to_u32(dividerColor),
                      frameBorderThickness);

    WidgetFrameLayout layout;
    layout.min = frameMin;
    layout.max = frameMax;
    return layout;
}

void draw_compact_input_matrix(ImVec2 frameMin,
                               ImVec2 frameMax,
                               const CompactInputMatrixData& data,
                               const Theme& theme,
                               CompactInputMatrixStyle style) {
    const Palette& palette = theme.palette;
    const CompactInputMatrixWidgetTokens& tokens = theme.widgets.compactInputMatrix;
    const float scale = current_scale();
    const float horizontalInset = (style.horizontalInset >= 0.0f ? style.horizontalInset : tokens.horizontalInset) * scale;
    const float topRowY = frameMin.y + (style.topRowY >= 0.0f ? style.topRowY : tokens.topRowY) * scale;
    const float bottomRowY = frameMin.y + (style.bottomRowY >= 0.0f ? style.bottomRowY : tokens.bottomRowY) * scale;
    const float keyHeight = (style.keyHeight >= 0.0f ? style.keyHeight : tokens.keyHeight) * scale;
    const float keyGap = (style.keyGap >= 0.0f ? style.keyGap : tokens.keyGap) * scale;
    const float sideWidthRatio = style.sideWidthRatio >= 0.0f ? style.sideWidthRatio : tokens.sideWidthRatio;
    const float sideWidthMin = (style.sideWidthMin >= 0.0f ? style.sideWidthMin : tokens.sideWidthMin) * scale;
    const float sideWidthMax = (style.sideWidthMax >= 0.0f ? style.sideWidthMax : tokens.sideWidthMax) * scale;

    const float innerWidth = std::max(0.0f, frameMax.x - frameMin.x - horizontalInset * 2.0f);
    const float sideWidth = std::clamp(innerWidth * sideWidthRatio, sideWidthMin, sideWidthMax);
    const float wasdWidth = std::max(0.0f, innerWidth - sideWidth - keyGap);
    const float wasdKeyWidth = std::max(0.0f, (wasdWidth - keyGap * 2.0f) / 3.0f);
    const ImVec2 sideKeySize(sideWidth, keyHeight);
    const ImVec2 wasdKeySize(wasdKeyWidth, keyHeight);

    const float sideX = frameMin.x + horizontalInset;
    const float wasdX = sideX + sideWidth + keyGap;

    const auto drawKey = [&](const CompactInputMatrixKey& key, ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(pos);
        ImGui::Dummy(size);

        const ImVec4 accent = resolve_color(key.accentColor, palette.accent);
        const ImVec4 inactiveBorder = mix_color(palette.textMuted, accent, tokens.inactiveBorderAccentMix);
        const ImVec4 inactiveText = mix_color(palette.textMuted, accent, tokens.inactiveTextAccentMix);
        const ImVec4 fill = key.active
            ? ImVec4(accent.x, accent.y, accent.z, tokens.activeFillAlpha)
            : ImVec4(palette.panel.x, palette.panel.y, palette.panel.z, tokens.inactiveFillAlpha);
        const ImVec4 textColor = key.active
            ? ImVec4(accent.x, accent.y, accent.z, tokens.activeTextAlpha)
            : ImVec4(inactiveText.x, inactiveText.y, inactiveText.z, tokens.inactiveTextAlpha);
        const ImVec4 borderColor = key.active
            ? ImVec4(accent.x, accent.y, accent.z, 1.0f)
            : ImVec4(inactiveBorder.x, inactiveBorder.y, inactiveBorder.z, 1.0f);

        InteractionVisualState state;
        state.selected = key.active;
        CapsuleStyle keyStyle;
        keyStyle.rounding = theme.radii.control;
        keyStyle.fill = fill;
        keyStyle.textColor = textColor;
        AccentFrameStyle frameStyle;
        frameStyle.rounding = theme.radii.control;
        frameStyle.color = borderColor;
        frameStyle.idleBorderAlpha = tokens.inactiveBorderAlpha;
        frameStyle.hoveredBorderAlpha = tokens.inactiveBorderAlpha;
        frameStyle.activeBorderAlpha = tokens.activeBorderAlpha;
        frameStyle.selectedBorderAlpha = tokens.activeBorderAlpha;
        frameStyle.idleGlowAlpha = 0.0f;
        frameStyle.hoveredGlowAlpha = 0.0f;
        frameStyle.activeGlowAlpha = tokens.activeGlowAlpha;
        frameStyle.selectedGlowAlpha = tokens.activeGlowAlpha;
        frameStyle.idleThickness = tokens.inactiveBorderThickness;
        frameStyle.activeThickness = tokens.activeBorderThickness;
        frameStyle.selectedThickness = tokens.activeBorderThickness;
        keyStyle.frame = frameStyle;

        draw_capsule(ImGui::GetWindowDrawList(),
                     pos,
                     ImVec2(pos.x + size.x, pos.y + size.y),
                     safe_text(key.label),
                     theme,
                     state,
                     keyStyle);
    };

    drawKey(data.topLeft, ImVec2(sideX, topRowY), sideKeySize);
    drawKey(data.bottomLeft, ImVec2(sideX, bottomRowY), sideKeySize);
    drawKey(data.topCenter, ImVec2(wasdX + wasdKeyWidth + keyGap, topRowY), wasdKeySize);
    drawKey(data.bottomCenterLeft, ImVec2(wasdX, bottomRowY), wasdKeySize);
    drawKey(data.bottomCenter, ImVec2(wasdX + wasdKeyWidth + keyGap, bottomRowY), wasdKeySize);
    drawKey(data.bottomCenterRight, ImVec2(wasdX + (wasdKeyWidth + keyGap) * 2.0f, bottomRowY), wasdKeySize);
}

void draw_compact_rhythm_capsule(ImDrawList* drawList,
                                 ImVec2 min,
                                 ImVec2 size,
                                 const CompactRhythmCapsuleData& data,
                                 const Theme& theme,
                                 CompactRhythmCapsuleStyle style) {
    if (drawList == nullptr || size.x <= 1.0f || size.y <= 1.0f) {
        return;
    }

    const Palette& palette = theme.palette;
    const float scale = current_scale();
    const ImVec2 max(min.x + size.x, min.y + size.y);
    const float rounding = (style.rounding >= 0.0f ? style.rounding : size.y * 0.5f);
    const float borderThickness = (style.borderThickness >= 0.0f ? style.borderThickness : 2.2f) * scale;
    const float pulseBaseRadius = (style.pulseRadius >= 0.0f ? style.pulseRadius : 7.0f) * scale;
    const float innerPaddingX = (style.innerPaddingX >= 0.0f ? style.innerPaddingX : 18.0f) * scale;
    const float blockWidth = (style.blockWidth >= 0.0f ? style.blockWidth : 7.0f) * scale;
    const float blockHeight = (style.blockHeight >= 0.0f ? style.blockHeight : 12.0f) * scale;
    const float blockGap = (style.blockGap >= 0.0f ? style.blockGap : 4.0f) * scale;
    const float blockTravelSpeed = (style.blockTravelSpeed >= 0.0f ? style.blockTravelSpeed : 72.0f) * scale;
    const float waveformSpikeMinHeight = (style.waveformSpikeMinHeight >= 0.0f ? style.waveformSpikeMinHeight : 4.0f) * scale;
    const float waveformSpikeMaxHeight = (style.waveformSpikeMaxHeight >= 0.0f ? style.waveformSpikeMaxHeight : 17.0f) * scale;
    const float waveformSpikeThickness = (style.waveformSpikeThickness >= 0.0f ? style.waveformSpikeThickness : 1.35f) * scale;
    const float speed01 = std::clamp(data.speed01, 0.0f, 1.0f);
    const ImVec4 fillColor = resolve_color(style.fillColor, ImVec4(0.018f, 0.026f, 0.040f, 0.92f));
    const ImVec4 borderColor = resolve_color(style.borderColor, ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.46f));
    const ImVec4 pulseColor = resolve_color(style.pulseColor, palette.accent);
    const ImVec4 blockColor = resolve_color(style.blockColor, palette.accent);
    const ImVec4 waveformCoreColor = resolve_color(style.waveformCoreColor, ImVec4(0.82f, 1.0f, 0.96f, 0.92f));
    const ImVec4 waveformGlowColor = resolve_color(style.waveformGlowColor, blockColor);

    const ImVec2 capsuleMin(min.x, min.y);
    const ImVec2 capsuleMax(max.x, max.y);
    drawList->AddRectFilled(capsuleMin, capsuleMax, to_u32(fillColor), rounding);
    drawList->AddRect(capsuleMin, capsuleMax, to_u32(borderColor), rounding, 0, borderThickness);
    drawList->AddRect(ImVec2(capsuleMin.x + borderThickness, capsuleMin.y + borderThickness),
                      ImVec2(capsuleMax.x - borderThickness, capsuleMax.y - borderThickness),
                      to_u32(ImVec4(1.0f, 1.0f, 1.0f, 0.012f)),
                      std::max(0.0f, rounding - borderThickness),
                      0,
                      1.0f * scale);

    const float heartbeatHz = 1.15f + speed01 * 5.6f;
    const float pulseWave = 0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * heartbeatHz * 6.2831853f);
    const float pulseEase = pulseWave * pulseWave * (3.0f - 2.0f * pulseWave);
    const float pulseRadius = pulseBaseRadius * (1.0f + (0.10f + speed01 * 0.22f) * pulseEase);
    const ImVec2 pulseCenter(capsuleMin.x + rounding, capsuleMin.y + size.y * 0.5f);
    const float glowRadius = pulseRadius * (2.2f + speed01 * 1.2f) * std::max(0.2f, style.pulseGlowScale);
    drawList->AddCircleFilled(pulseCenter,
                              glowRadius,
                              to_u32(ImVec4(pulseColor.x, pulseColor.y, pulseColor.z, (0.045f + 0.090f * speed01) * pulseEase)),
                              48);
    drawList->AddCircleFilled(pulseCenter,
                              pulseRadius,
                              to_u32(ImVec4(pulseColor.x, pulseColor.y, pulseColor.z, 0.92f)),
                              48);
    drawList->AddCircle(pulseCenter,
                        pulseRadius + 0.9f * scale,
                        to_u32(ImVec4(1.0f, 1.0f, 1.0f, 0.18f + 0.20f * pulseEase)),
                        48,
                        1.1f * scale);

    const ImVec2 clipMin(capsuleMin.x + rounding + innerPaddingX - 8.0f * scale, capsuleMin.y + borderThickness + 1.0f * scale);
    const ImVec2 clipMax(capsuleMax.x - innerPaddingX, capsuleMax.y - borderThickness - 1.0f * scale);
    drawList->PushClipRect(clipMin, clipMax, true);
    const float centerY = capsuleMin.y + size.y * 0.5f;
    const float idleAlpha = 0.38f + speed01 * 0.26f;
    const float idleStep = std::max(2.6f * scale, blockWidth + blockGap);
    const float idlePhase = std::fmod(static_cast<float>(ImGui::GetTime()) * (18.0f + 48.0f * speed01) * scale,
                                      idleStep);
    for (float x = clipMin.x - idlePhase; x < clipMax.x; x += idleStep) {
        const float local = (x - clipMin.x) / std::max(1.0f, clipMax.x - clipMin.x);
        const float noiseA = 0.5f + 0.5f * std::sin(local * 55.0f + static_cast<float>(ImGui::GetTime()) * 5.4f);
        const float noiseB = 0.5f + 0.5f * std::sin(local * 137.0f - static_cast<float>(ImGui::GetTime()) * 8.2f);
        const float h = (waveformSpikeMinHeight * 0.62f + waveformSpikeMaxHeight * 0.46f * noiseA * noiseB) * (0.70f + 0.30f * speed01);
        drawList->AddLine(ImVec2(x, centerY - h * 1.16f),
                          ImVec2(x, centerY + h * 1.16f),
                          to_u32(ImVec4(waveformGlowColor.x, waveformGlowColor.y, waveformGlowColor.z, idleAlpha * 0.72f)),
                          std::max(1.0f, waveformSpikeThickness * 2.65f));
        drawList->AddLine(ImVec2(x, centerY - h),
                          ImVec2(x, centerY + h),
                          to_u32(ImVec4(waveformCoreColor.x, waveformCoreColor.y, waveformCoreColor.z, idleAlpha)),
                          std::max(1.0f, waveformSpikeThickness));
    }

    const int blockCount = std::min(std::max(data.blockCount, 0), 64);
    for (int i = 0; i < blockCount; ++i) {
        const CompactRhythmBlock& block = data.blocks[i];
        const float age = std::max(0.0f, block.ageSeconds);
        const float travel = age * (blockTravelSpeed * (0.65f + speed01 * 1.35f));
        const float baseX = pulseCenter.x + pulseBaseRadius + innerPaddingX + static_cast<float>(i % 10) * (blockWidth + blockGap);
        const float x = baseX + travel;
        if (x > clipMax.x || x + blockWidth < clipMin.x) {
            continue;
        }

        const float fade = std::clamp(1.0f - age / 1.05f, 0.0f, 1.0f);
        const float strength = std::clamp(block.strength, 0.18f, 1.0f);
        const float hash = std::sin((static_cast<float>(i) + block.laneOffset * 7.0f) * 12.9898f + age * 31.0f) * 43758.5453f;
        const float noise = hash - std::floor(hash);
        const float spikeHeight = std::clamp(waveformSpikeMinHeight +
                                             (waveformSpikeMaxHeight - waveformSpikeMinHeight) * (0.28f + 0.72f * noise) * strength,
                                             waveformSpikeMinHeight,
                                             waveformSpikeMaxHeight);
        const float phase = static_cast<float>(i % 4) * 0.35f;
        const float upperScale = 0.70f + 0.30f * std::sin(age * 9.0f + phase);
        const float lowerScale = 0.72f + 0.28f * std::cos(age * 8.0f + phase * 1.7f);
        const float alpha = (0.24f + 0.62f * strength) * fade;
        const ImVec2 topA(x, centerY - 1.0f * scale);
        const ImVec2 topB(x + blockWidth * 0.5f, centerY - spikeHeight * upperScale);
        const ImVec2 topC(x + blockWidth, centerY - 1.0f * scale);
        const ImVec2 bottomA(x, centerY + 1.0f * scale);
        const ImVec2 bottomB(x + blockWidth * 0.5f, centerY + spikeHeight * lowerScale);
        const ImVec2 bottomC(x + blockWidth, centerY + 1.0f * scale);

        drawList->AddTriangleFilled(topA,
                                    topB,
                                    topC,
                                    to_u32(ImVec4(waveformGlowColor.x, waveformGlowColor.y, waveformGlowColor.z, alpha * 0.55f * style.waveformGlowScale)));
        drawList->AddTriangleFilled(bottomA,
                                    bottomB,
                                    bottomC,
                                    to_u32(ImVec4(waveformGlowColor.x, waveformGlowColor.y, waveformGlowColor.z, alpha * 0.55f * style.waveformGlowScale)));
        drawList->AddLine(ImVec2(x + blockWidth * 0.5f, centerY - spikeHeight * upperScale * 0.82f),
                          ImVec2(x + blockWidth * 0.5f, centerY + spikeHeight * lowerScale * 0.82f),
                          to_u32(ImVec4(waveformCoreColor.x, waveformCoreColor.y, waveformCoreColor.z, alpha)),
                          waveformSpikeThickness);
    }
    drawList->PopClipRect();
}

void draw_stick_visualizer(const StickVisualizerData& data,
                           ImVec2 size,
                           const Theme& theme,
                           StickVisualizerStyle style) {
    const Palette& palette = theme.palette;
    const StickVisualizerWidgetTokens& tokens = theme.widgets.stickVisualizer;
    const float scale = current_scale();
    const float rounding = style.rounding >= 0.0f ? style.rounding : tokens.frameRounding * scale;
    const float radiusRatio = style.radiusRatio >= 0.0f ? style.radiusRatio : tokens.radiusRatio;
    const float centerYRatio = style.centerYRatio >= 0.0f ? style.centerYRatio : tokens.centerYRatio;
    const ImVec4 targetColor = resolve_color(style.targetColor, palette.warning);
    const ImVec4 currentColor = resolve_color(style.currentColor, palette.accent);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 max(pos.x + size.x, pos.y + size.y);

    drawList->AddRectFilled(pos, max, to_u32(palette.surface), rounding);
    draw_glow_rect(drawList, pos, max, rounding, theme, tokens.glowAlpha);

    const float radius = std::min(size.x, size.y) * radiusRatio;
    const ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * centerYRatio);
    const float outerDeadZoneRadius = radius * std::clamp(data.outerDeadZoneRatio, 0.0f, 0.95f);
    const float effectiveRadius = std::max(2.0f, radius - outerDeadZoneRadius);
    const float deadZoneRadius = radius * std::clamp(data.innerDeadZoneRatio, 0.0f, 1.0f);

    drawList->AddCircleFilled(center, radius, to_u32(ImVec4(0.010f, 0.020f, 0.040f, 1.0f)), 96);
    if (outerDeadZoneRadius > 0.5f) {
        drawList->AddCircleFilled(center,
                                  radius,
                                  to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.065f)),
                                  96);
        drawList->AddCircleFilled(center, effectiveRadius, to_u32(ImVec4(0.010f, 0.020f, 0.040f, 1.0f)), 96);
    }
    drawList->AddCircle(center,
                        radius,
                        to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.18f)),
                        96,
                        1.25f);

    const float halfDiagonal = std::clamp(data.diagonalRangeDegrees, 1.0f, 89.0f) * 0.5f;
    for (int sectorIndex : {1, 3, 5, 7}) {
        const float centerDegrees = sector_visual_degrees(sectorIndex);
        const float start = centerDegrees - halfDiagonal;
        const float end = centerDegrees + halfDiagonal;
        draw_wedge(drawList,
                   center,
                   deadZoneRadius,
                   effectiveRadius,
                   start,
                   end,
                   to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.085f)));
        draw_arc(drawList,
                 center,
                 effectiveRadius,
                 start,
                 end,
                 to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.28f)),
                 1.5f);
        drawList->AddLine(center,
                          point_on_circle(center, effectiveRadius, start),
                          to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.18f)),
                          1.0f);
        drawList->AddLine(center,
                          point_on_circle(center, effectiveRadius, end),
                          to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.18f)),
                          1.0f);
    }

    drawList->AddCircle(center,
                        effectiveRadius,
                        to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.52f)),
                        96,
                        2.0f);
    drawList->AddCircle(center,
                        deadZoneRadius,
                        to_u32(ImVec4(palette.warning.x, palette.warning.y, palette.warning.z, 0.18f)),
                        64,
                        1.0f);
    for (int i = 0; i < 8; ++i) {
        const float angle = -67.5f + 45.0f * static_cast<float>(i);
        drawList->AddLine(center,
                          point_on_circle(center, effectiveRadius, angle),
                          to_u32(ImVec4(palette.border.x, palette.border.y, palette.border.z, 1.25f)),
                          1.0f);
    }

    const ImVec2 target = stick_point_to_screen(center, radius, data.target);
    const ImVec2 current = stick_point_to_screen(center, radius, data.current);
    drawList->AddLine(center,
                      target,
                      to_u32(ImVec4(targetColor.x, targetColor.y, targetColor.z, 0.65f)),
                      1.5f);
    drawList->AddCircleFilled(target,
                              tokens.targetPointRadius * scale,
                              to_u32(targetColor),
                              24);
    drawList->AddLine(center,
                      current,
                      to_u32(ImVec4(currentColor.x, currentColor.y, currentColor.z, 0.78f)),
                      2.0f);
    drawList->AddCircleFilled(current,
                              tokens.currentPointRadius * scale,
                              to_u32(currentColor),
                              32);
    drawList->AddCircle(current,
                        tokens.currentRingRadius * scale,
                        to_u32(ImVec4(currentColor.x, currentColor.y, currentColor.z, 0.55f)),
                        32,
                        1.5f);

    ImGui::SetCursorScreenPos(ImVec2(pos.x + 16.0f * scale, pos.y + 14.0f * scale));
    ImGui::TextDisabled("%s", safe_text(data.title));
    if (data.statusLabel != nullptr && data.statusLabel[0] != '\0' &&
        data.statusValue != nullptr && data.statusValue[0] != '\0') {
        const ImVec2 afterTitlePos = ImGui::GetCursorScreenPos();
        const ImVec2 pillSize = status_pill_size(data.statusLabel, data.statusValue, theme);
        const float pillX = pos.x + size.x - 16.0f * scale - pillSize.x;
        const float pillY = pos.y + 14.0f * scale +
            std::max(0.0f, (ImGui::GetTextLineHeightWithSpacing() - pillSize.y) * 0.5f);
        if (pillX > pos.x + 16.0f * scale + ImGui::CalcTextSize(safe_text(data.title)).x + 10.0f * scale) {
            ImGui::SetCursorScreenPos(ImVec2(pillX, pillY));
            draw_status_pill(data.statusLabel,
                             data.statusValue,
                             resolve_color(data.statusColor, palette.accent),
                             theme);
            ImGui::SetCursorScreenPos(afterTitlePos);
        }
    }

    const float infoTop = pos.y + size.y - 58.0f * scale;
    drawList->AddRectFilled(ImVec2(pos.x + tokens.infoPanelInsetX * scale, infoTop - 4.0f * scale),
                            ImVec2(pos.x + size.x - tokens.infoPanelInsetX * scale,
                                   pos.y + size.y - tokens.infoPanelBottomInset * scale),
                            to_u32(ImVec4(0.012f, 0.024f, 0.046f, 0.72f)),
                            tokens.infoPanelRounding * scale);
    drawList->AddRect(ImVec2(pos.x + tokens.infoPanelInsetX * scale, infoTop - 4.0f * scale),
                      ImVec2(pos.x + size.x - tokens.infoPanelInsetX * scale,
                             pos.y + size.y - tokens.infoPanelBottomInset * scale),
                      to_u32(palette.border),
                      tokens.infoPanelRounding * scale);

    ImGui::SetCursorScreenPos(ImVec2(pos.x + 24.0f * scale, infoTop + 2.0f * scale));
    ImGui::TextUnformatted(safe_text(data.sectorText));
    ImGui::SetCursorScreenPos(ImVec2(pos.x + 24.0f * scale, infoTop + 25.0f * scale));
    ImGui::TextDisabled("%s", safe_text(data.targetText));

    const float currentX = std::max(pos.x + size.x * 0.52f, pos.x + 260.0f * scale);
    ImGui::SetCursorScreenPos(ImVec2(currentX, infoTop + 25.0f * scale));
    ImGui::TextDisabled("%s", safe_text(data.currentText));

    ImGui::SetCursorScreenPos(pos);
    ImGui::Dummy(size);
}

void draw_key_binding_row(ImDrawList* drawList,
                          ImVec2 min,
                          ImVec2 size,
                          const KeyBindingRowData& data,
                          const Theme& theme,
                          KeyBindingRowStyle style) {
    const Palette& palette = theme.palette;
    const KeyBindingRowWidgetTokens& tokens = theme.widgets.keyBindingRow;
    const float scale = current_scale();
    const float rounding = style.rounding >= 0.0f ? style.rounding : tokens.rowRounding * scale;
    const float keyPillWidth = style.keyPillWidth >= 0.0f ? style.keyPillWidth : tokens.keyPillWidth * scale;
    const ImVec4 accentColor = resolve_color(style.accentColor, palette.accent);
    const ImVec4 baseFillColor = resolve_color(style.baseFillColor, palette.surface);
    const ImVec4 raisedFillColor = resolve_color(style.raisedFillColor, palette.surfaceRaised);
    const float pulse = row_pulse(data.selected);
    const ImVec4 fill = data.blocked
        ? ImVec4(0.020f, 0.030f, 0.050f, 1.0f)
        : (data.selected
            ? mix_color(raisedFillColor, accentColor, 0.20f + 0.10f * pulse)
            : mix_color(baseFillColor, raisedFillColor, data.hovered ? 0.60f : 0.20f));
    const ImVec4 border = data.selected
        ? ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.70f)
        : ImVec4(1.0f, 1.0f, 1.0f, data.hovered ? 0.14f : 0.07f);
    const ImVec4 textColor = data.blocked ? palette.textMuted : palette.text;
    const ImVec4 descColor = data.blocked
        ? ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.68f)
        : palette.textMuted;
    const ImVec2 max(min.x + size.x, min.y + size.y);

    drawList->AddRectFilled(min, max, to_u32(fill), rounding);
    drawList->AddRect(min,
                      max,
                      to_u32(border),
                      rounding,
                      0,
                      data.selected ? 1.8f * scale : 1.0f * scale);
    if (data.selected) {
        draw_glow_rect(drawList, min, max, rounding, theme, tokens.rowGlowAlpha);
    }

    drawList->AddText(ImVec2(min.x + 16.0f * scale, min.y + 14.0f * scale),
                      to_u32(textColor),
                      safe_text(data.title));
    drawList->AddText(nullptr,
                      0.0f,
                      ImVec2(min.x + 16.0f * scale, min.y + 40.0f * scale),
                      to_u32(descColor),
                      safe_text(data.description),
                      nullptr,
                      size.x - 250.0f * scale);

    const ImVec2 pillMin(max.x - keyPillWidth - tokens.keyPillInsetX * scale, min.y + 16.0f * scale);
    const ImVec2 pillMax(max.x - tokens.keyPillInsetX * scale, pillMin.y + tokens.keyPillHeight * scale);
    const ImVec4 pillFill = data.selected
        ? ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.22f)
        : ImVec4(0.028f, 0.040f, 0.070f, data.blocked ? 0.55f : 1.0f);
    drawList->AddRectFilled(pillMin, pillMax, to_u32(pillFill), tokens.keyPillRounding * scale);
    drawList->AddRect(pillMin,
                      pillMax,
                      to_u32(data.selected
                                 ? ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.70f)
                                 : ImVec4(1.0f, 1.0f, 1.0f, data.blocked ? 0.05f : 0.10f)),
                      tokens.keyPillRounding * scale);

    const ImVec2 keySize = ImGui::CalcTextSize(safe_text(data.keyText));
    drawList->AddText(ImVec2(pillMin.x + (keyPillWidth - keySize.x) * 0.5f, pillMin.y + 8.0f * scale),
                      to_u32(data.blocked ? palette.textMuted : palette.text),
                      safe_text(data.keyText));

    const ImVec2 hintSize = ImGui::CalcTextSize(safe_text(data.hintText));
    drawList->AddText(ImVec2(pillMin.x + keyPillWidth - hintSize.x, min.y + 60.0f * scale),
                      to_u32(data.selected ? accentColor : descColor),
                      safe_text(data.hintText));
}

void draw_notice_toast(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 size,
                       const NoticeToastData& data,
                       const Theme& theme,
                       NoticeToastStyle style) {
    const Palette& palette = theme.palette;
    const NoticeToastWidgetTokens& tokens = theme.widgets.noticeToast;
    const float scale = current_scale();
    const float rounding = style.rounding >= 0.0f ? style.rounding : tokens.rounding * scale;
    const ImVec4 accentColor = resolve_color(style.accentColor, palette.accent);
    const float alpha = clamp01(data.alpha);
    const ImVec2 max(min.x + size.x, min.y + size.y);

    for (int layer = 5; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 5.0f;
        const float offset = (2.0f + t * 8.0f) * scale;
        const float layerAlpha = alpha * (0.025f + 0.030f * (1.0f - t));
        drawList->AddRectFilled(ImVec2(min.x + offset, min.y + offset + 3.0f * scale),
                                ImVec2(max.x + offset, max.y + offset + 3.0f * scale),
                                to_u32(ImVec4(0.0f, 0.0f, 0.0f, layerAlpha)),
                                rounding + offset * 0.25f);
    }

    draw_static_glow_rect(drawList, min, max, rounding, theme, tokens.glowAlpha * alpha, 1.2f * scale);
    drawList->AddRectFilled(min,
                            max,
                            to_u32(ImVec4(0.024f, 0.030f, 0.044f, 0.96f * alpha)),
                            rounding);
    drawList->AddRectFilled(ImVec2(min.x + 1.0f * scale, min.y + 1.0f * scale),
                            ImVec2(max.x - 1.0f * scale, max.y - 1.0f * scale),
                            to_u32(ImVec4(0.040f, 0.050f, 0.068f, 0.54f * alpha)),
                            std::max(0.0f, rounding - 1.0f * scale));
    drawList->AddRect(min,
                      max,
                      to_u32(ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.45f * alpha)),
                      rounding,
                      0,
                      1.2f * scale);

    const float progressH = std::max(1.8f, tokens.progressHeight * scale);
    const float progressInsetX = std::max(14.0f, tokens.progressInsetX * scale);
    const float progressY = min.y + 1.1f * scale;
    const ImVec2 progressMin(min.x + progressInsetX, progressY);
    const ImVec2 progressMax(max.x - progressInsetX, progressY + progressH);
    const float progressW = progressMax.x - progressMin.x;
    drawList->AddRectFilled(progressMin,
                            progressMax,
                            to_u32(ImVec4(0.05f, 0.33f, 0.40f, 0.14f * alpha)),
                            progressH * 0.5f);
    if (data.dismissProgress > 0.001f) {
        const float fillWidth = progressW * clamp01(data.dismissProgress);
        const ImVec2 fillMax(progressMin.x + fillWidth, progressMax.y);
        const float progressR = progressH * 0.5f;
        for (int layer = 3; layer >= 1; --layer) {
            const float t = static_cast<float>(layer) / 3.0f;
            const float expandX = (0.6f + 1.5f * t) * scale;
            const float expandY = (0.7f + 1.7f * t) * scale;
            const float glowAlpha = (0.045f + 0.030f * (1.0f - t)) * alpha;
            drawList->AddRectFilled(ImVec2(progressMin.x - expandX, progressMin.y - expandY),
                                    ImVec2(fillMax.x + expandX, progressMax.y + expandY),
                                    to_u32(ImVec4(0.10f, 0.82f, 0.94f, glowAlpha)),
                                    progressR + expandY);
        }
        drawList->AddRectFilled(progressMin,
                                fillMax,
                                to_u32(ImVec4(0.08f, 0.72f, 0.86f, 0.78f * alpha)),
                                progressR);
        const float coreInset = std::max(0.25f, 0.35f * scale);
        if (fillWidth > coreInset * 2.0f) {
            drawList->AddRectFilled(ImVec2(progressMin.x + coreInset, progressMin.y + coreInset),
                                    ImVec2(fillMax.x - coreInset, progressMax.y - coreInset),
                                    to_u32(ImVec4(0.58f, 0.98f, 1.0f, 0.54f * alpha)),
                                    std::max(0.0f, progressR - coreInset));
        }
    }

    const ImVec2 iconCenter(min.x + 34.0f * scale, min.y + 34.0f * scale);
    drawList->AddCircleFilled(iconCenter,
                              tokens.iconRadius * scale,
                              to_u32(ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.16f * alpha)),
                              32);
    drawList->AddCircle(iconCenter,
                        tokens.iconRadius * scale,
                        to_u32(ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.66f * alpha)),
                        32,
                        1.4f * scale);
    const ImVec2 markSize = ImGui::CalcTextSize(safe_text(data.iconText));
    drawList->AddText(ImVec2(iconCenter.x - markSize.x * 0.5f,
                             iconCenter.y - markSize.y * 0.5f - 0.5f * scale),
                      to_u32(ImVec4(accentColor.x, accentColor.y, accentColor.z, alpha)),
                      safe_text(data.iconText));

    ImFont* font = ImGui::GetFont();
    const float titleSize = ImGui::GetFontSize() * 0.96f;
    const float bodySize = ImGui::GetFontSize() * 0.84f;
    const float footerSize = ImGui::GetFontSize() * 0.70f;
    const float textX = min.x + 60.0f * scale;
    const float textW = max.x - textX - 18.0f * scale;
    drawList->AddText(font,
                      titleSize,
                      ImVec2(textX, min.y + 18.0f * scale),
                      to_u32(ImVec4(palette.text.x, palette.text.y, palette.text.z, alpha)),
                      safe_text(data.title));
    drawList->AddText(font,
                      bodySize,
                      ImVec2(textX, min.y + 44.0f * scale),
                      to_u32(ImVec4(accentColor.x, accentColor.y, accentColor.z, alpha)),
                      safe_text(data.body),
                      nullptr,
                      textW);
    drawList->AddText(font,
                      footerSize,
                      ImVec2(textX, max.y - 24.0f * scale),
                      to_u32(ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.82f * alpha)),
                      safe_text(data.footer),
                      nullptr,
                      textW);
}

void draw_dynamic_island(ImDrawList* drawList,
                         ImVec2 displaySize,
                         const DynamicIslandData& data,
                         const Theme& theme,
                         DynamicIslandStyle style) {
    const Palette& palette = theme.palette;
    const DynamicIslandWidgetTokens& tokens = theme.widgets.dynamicIsland;
    const float scale = current_scale();
    const float expanded = clamp01(data.expanded);
    const ImVec4 accentColor = resolve_color(style.accentColor, palette.accent);
    ImFont* font = ImGui::GetFont();
    const float normalFontSize = ImGui::GetFontSize() * 0.86f;
    const float titleFontSize = ImGui::GetFontSize() * 0.88f;
    const float bodyFontSize = ImGui::GetFontSize() * 0.80f;
    const float footerFontSize = ImGui::GetFontSize() * 0.68f;

    const ImVec2 collapsedTextSize = font->CalcTextSizeA(normalFontSize, 100000.0f, 0.0f, safe_text(data.collapsedText));
    const ImVec2 titleTextSize = font->CalcTextSizeA(titleFontSize, 100000.0f, 0.0f, safe_text(data.title));
    const ImVec2 bodyTextSize = font->CalcTextSizeA(bodyFontSize, 100000.0f, 0.0f, safe_text(data.body));
    const ImVec2 footerTextSize = font->CalcTextSizeA(footerFontSize, 100000.0f, 0.0f, safe_text(data.footer));

    const float collapsedW = std::clamp(collapsedTextSize.x + 58.0f * scale,
                                        tokens.collapsedMinWidth * scale,
                                        displaySize.x - 44.0f * scale);
    const float collapsedH = tokens.collapsedHeight * scale;
    const float expandedTextW = std::max({titleTextSize.x, bodyTextSize.x, footerTextSize.x});
    const float expandedW = std::clamp(expandedTextW + 62.0f * scale,
                                       std::max(collapsedW, tokens.expandedMinWidth * scale),
                                       std::min(tokens.expandedMaxWidth * scale, displaySize.x - 44.0f * scale));
    const float expandedH = tokens.expandedHeight * scale;
    const float islandW = collapsedW + (expandedW - collapsedW) * expanded;
    const float islandH = collapsedH + (expandedH - collapsedH) * expanded;
    const ImVec2 min((displaySize.x - islandW) * 0.5f, tokens.topInset * scale);
    const ImVec2 max(min.x + islandW, min.y + islandH);
    const float rounding = (collapsedH * 0.5f) + ((24.0f * scale) - collapsedH * 0.5f) * expanded;

    for (int layer = 6; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 5.0f;
        const float offset = (1.8f + 7.8f * t) * scale;
        const int alpha = static_cast<int>((18.0f + 30.0f * expanded) * t * t);
        drawList->AddRectFilled(ImVec2(min.x + offset, min.y + offset + 4.0f * scale),
                                ImVec2(max.x - offset * 0.16f, max.y + offset + 6.0f * scale),
                                IM_COL32(0, 0, 0, alpha),
                                rounding + offset * 0.30f);
    }

    for (int layer = 7; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 7.0f;
        const float expandInset = (0.7f + 1.85f * static_cast<float>(layer)) * scale;
        const int alpha = static_cast<int>((28.0f + 28.0f * expanded) * (1.0f - t * 0.62f));
        drawList->AddRect(ImVec2(min.x - expandInset, min.y - expandInset),
                          ImVec2(max.x + expandInset, max.y + expandInset),
                          to_u32(ImVec4(accentColor.x, accentColor.y, accentColor.z, static_cast<float>(alpha) / 255.0f)),
                          rounding + expandInset,
                          0,
                          std::max(1.0f, 1.05f * scale));
    }

    drawList->AddRectFilled(min, max, IM_COL32(0, 0, 0, 255), rounding);
    drawList->AddRect(ImVec2(min.x + 0.5f * scale, min.y + 0.5f * scale),
                      ImVec2(max.x - 0.5f * scale, max.y - 0.5f * scale),
                      to_u32(ImVec4(0.16f, 0.48f, 1.0f, 0.56f + 0.18f * expanded)),
                      rounding,
                      0,
                      1.15f * scale);
    drawList->AddRect(ImVec2(min.x + 1.2f * scale, min.y + 1.2f * scale),
                      ImVec2(max.x - 1.2f * scale, max.y - 1.2f * scale),
                      to_u32(ImVec4(0.35f, 0.68f, 1.0f, 0.16f + 0.10f * expanded)),
                      std::max(0.0f, rounding - 1.2f * scale),
                      0,
                      1.0f * scale);

    const float collapsedAlpha = std::clamp(1.0f - expanded * 1.35f, 0.0f, 1.0f);
    if (collapsedAlpha > 0.002f) {
        const ImVec2 textPos(min.x + (islandW - collapsedTextSize.x) * 0.5f,
                             min.y + (islandH - collapsedTextSize.y) * 0.5f - 0.5f * scale);
        drawList->AddText(font,
                          normalFontSize,
                          textPos,
                          to_u32(ImVec4(0.96f, 0.97f, 0.99f, 0.95f * collapsedAlpha)),
                          safe_text(data.collapsedText));
    }

    const float expandedAlpha = std::clamp((expanded - 0.20f) / 0.80f, 0.0f, 1.0f);
    if (expandedAlpha > 0.002f) {
        const float padX = 24.0f * scale;
        const float textW = islandW - padX * 2.0f;
        drawList->AddText(font,
                          titleFontSize,
                          ImVec2(min.x + padX, min.y + 18.0f * scale),
                          to_u32(ImVec4(0.98f, 0.98f, 1.00f, 0.97f * expandedAlpha)),
                          safe_text(data.title));
        drawList->AddText(font,
                          bodyFontSize,
                          ImVec2(min.x + padX, min.y + 44.0f * scale),
                          to_u32(ImVec4(0.88f, 0.90f, 0.94f, 0.91f * expandedAlpha)),
                          safe_text(data.body),
                          nullptr,
                          textW);
        drawList->AddText(font,
                          footerFontSize,
                          ImVec2(min.x + padX, min.y + 88.0f * scale),
                          to_u32(ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.86f * expandedAlpha)),
                          safe_text(data.footer),
                          nullptr,
                          textW);
    }
}

ConfigExchangeControlResult draw_config_exchange_control(const char* id,
                                                         const ConfigExchangeControlData& data,
                                                         const Theme& theme,
                                                         ConfigExchangeControlStyle style) {
    ConfigExchangeControlResult result;
    if (data.buffer == nullptr || data.bufferSize <= 1) {
        return result;
    }

    const Palette& palette = theme.palette;
    const ConfigExchangeWidgetTokens& tokens = theme.widgets.configExchange;
    const float scale = current_scale();
    const float collapsedSize = (style.collapsedSize > 0.0f ? style.collapsedSize : tokens.collapsedSize) * scale;
    const float height = (style.height > 0.0f ? style.height : tokens.height) * scale;
    const float expandedWidth = std::max(collapsedSize,
                                         (style.expandedWidth > 0.0f ? style.expandedWidth : tokens.expandedWidth) * scale);
    const float rounding = (style.rounding > 0.0f ? style.rounding : tokens.rounding) * scale;
    const ImVec4 accentColor = resolve_color(style.accentColor, palette.accent);
    const ImVec2 layoutSize(collapsedSize, height);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 layoutMax(min.x + layoutSize.x, min.y + layoutSize.y);

    ImGui::PushID(id);
    ImGui::InvisibleButton("##config_exchange_hit", layoutSize);
    const ImGuiID animId = ImGui::GetID("##config_exchange_anim");
    const bool hitHovered = ImGui::IsItemHovered();
    const bool hitActive = ImGui::IsItemActive();

    ConfigExchangeAnimationState& anim = config_exchange_animation_cache()[animId];
    const bool inputWasActive = anim.inputActive;
    const float probeExpanded = ease_out_cubic(anim.presence);
    const float probeWidth = collapsedSize + (expandedWidth - collapsedSize) * probeExpanded;
    const bool expandedHovered = anim.presence > 0.02f &&
        ImGui::IsMouseHoveringRect(min, ImVec2(min.x + probeWidth, min.y + height), true);
    const bool targetExpanded = hitHovered || hitActive || expandedHovered || anim.inputActive;
    anim.presence = advance_presence(anim.presence,
                                     targetExpanded,
                                     theme.motion.bindingIslandPresence);
    const float expanded = ease_out_cubic(anim.presence);
    const float width = collapsedSize + (expandedWidth - collapsedSize) * expanded;
    const ImVec2 max(min.x + width, min.y + height);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float glowAlpha = tokens.glowAlpha + (tokens.hoveredGlowAlpha - tokens.glowAlpha) * expanded;
    if (glowAlpha > 0.001f) {
        draw_static_glow_rect(drawList, min, max, rounding, theme, glowAlpha, 1.0f);
    }

    const ImVec4 fillBase = mix_color(palette.background, palette.surfaceRaised, 0.50f);
    const float fillAlpha = tokens.fillAlpha + (tokens.expandedFillAlpha - tokens.fillAlpha) * expanded;
    drawList->AddRectFilled(min,
                            max,
                            to_u32(ImVec4(fillBase.x, fillBase.y, fillBase.z, fillAlpha)),
                            rounding);
    drawList->AddRect(min,
                      max,
                      to_u32(ImVec4(accentColor.x,
                                    accentColor.y,
                                    accentColor.z,
                                    tokens.borderAlpha + (tokens.hoveredBorderAlpha - tokens.borderAlpha) * expanded)),
                      rounding,
                      0,
                      1.05f * scale);
    drawList->AddRect(ImVec2(min.x + 1.0f * scale, min.y + 1.0f * scale),
                      ImVec2(max.x - 1.0f * scale, max.y - 1.0f * scale),
                      to_u32(ImVec4(1.0f, 1.0f, 1.0f, 0.022f + 0.040f * expanded)),
                      std::max(0.0f, rounding - 1.0f * scale),
                      0,
                      1.0f * scale);

    const float iconX = min.x + tokens.iconInsetX * scale;
    const float iconY = min.y + height * 0.5f;
    const float lineLength = tokens.iconLineLength * scale;
    const float lineGap = tokens.iconLineGap * scale;
    const ImU32 iconColor = to_u32(ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.78f + 0.18f * expanded));
    drawList->AddLine(ImVec2(iconX, iconY - lineGap),
                      ImVec2(iconX + lineLength, iconY - lineGap),
                      iconColor,
                      1.45f * scale);
    drawList->AddLine(ImVec2(iconX + lineLength * 0.32f, iconY),
                      ImVec2(iconX + lineLength, iconY),
                      iconColor,
                      1.45f * scale);
    drawList->AddLine(ImVec2(iconX + lineLength * 0.56f, iconY + lineGap),
                      ImVec2(iconX + lineLength, iconY + lineGap),
                      iconColor,
                      1.45f * scale);

    const float inputAlpha = std::clamp((expanded - 0.24f) / 0.76f, 0.0f, 1.0f);
    bool inputHovered = false;
    bool inputActive = false;
    if (inputAlpha > 0.01f) {
        const float inputX = min.x + collapsedSize + tokens.inputLeadGap * scale;
        const float inputW = std::max(20.0f * scale, width - collapsedSize - (tokens.inputLeadGap + tokens.inputTrailingPadding) * scale);
        const ImVec2 inputMin(inputX, min.y + tokens.inputInsetY * scale);
        const ImVec2 inputMax(inputX + inputW, max.y - tokens.inputInsetY * scale);
        const float inputRounding = std::max(0.0f, rounding - 2.0f * scale);
        drawList->AddRectFilled(inputMin,
                                inputMax,
                                to_u32(ImVec4(palette.background.x,
                                              palette.background.y,
                                              palette.background.z,
                                              tokens.inputFillAlpha * inputAlpha)),
                                inputRounding);
        drawList->AddRect(inputMin,
                          inputMax,
                          to_u32(ImVec4(accentColor.x,
                                        accentColor.y,
                                        accentColor.z,
                                        tokens.inputBorderAlpha * inputAlpha)),
                          inputRounding,
                          0,
                          1.0f * scale);
        drawList->AddRect(ImVec2(inputMin.x + 1.0f * scale, inputMin.y + 1.0f * scale),
                          ImVec2(inputMax.x - 1.0f * scale, inputMax.y - 1.0f * scale),
                          to_u32(ImVec4(1.0f, 1.0f, 1.0f, tokens.inputInnerHighlightAlpha * inputAlpha)),
                          std::max(0.0f, inputRounding - 1.0f * scale),
                          0,
                          1.0f * scale);

        const float inputPaddingX = tokens.inputPaddingX * scale;
        const float inputHeight = inputMax.y - inputMin.y;
        const float inputFramePaddingY = std::max(0.0f, (inputHeight - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::SetCursorScreenPos(ImVec2(inputMin.x + inputPaddingX, inputMin.y));
        ImGui::SetNextItemWidth(std::max(1.0f, inputW - inputPaddingX * 2.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, inputFramePaddingY));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, std::max(0.0f, rounding - 3.0f * scale));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(palette.text.x, palette.text.y, palette.text.z, inputAlpha));
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.58f * inputAlpha));
        result.edited = ImGui::InputTextWithHint("##config_exchange_input",
                                                 safe_text(data.placeholder),
                                                 data.buffer,
                                                 data.bufferSize,
                                                 ImGuiInputTextFlags_AutoSelectAll |
                                                     ImGuiInputTextFlags_EnterReturnsTrue);
        inputHovered = ImGui::IsItemHovered();
        inputActive = ImGui::IsItemActive();
        result.committed = result.edited || ImGui::IsItemDeactivatedAfterEdit();
        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(2);

        const float emphasisAlpha = inputActive
            ? tokens.inputActiveBorderAlpha
            : (inputHovered ? tokens.inputHoveredBorderAlpha : tokens.inputBorderAlpha);
        drawList->AddRect(inputMin,
                          inputMax,
                          to_u32(ImVec4(accentColor.x,
                                        accentColor.y,
                                        accentColor.z,
                                        emphasisAlpha * inputAlpha)),
                          inputRounding,
                          0,
                          (inputActive ? 1.45f : 1.15f) * scale);
    }

    result.hovered = hitHovered || inputHovered;
    result.active = hitActive || inputActive || inputWasActive;
    result.expanded = expanded > 0.08f;
    anim.inputActive = inputActive;
    if (result.active) {
        anim.presence = std::max(anim.presence, 0.92f);
    }

    ImGui::SetCursorScreenPos(ImVec2(layoutMax.x, layoutMax.y));
    ImGui::PopID();
    return result;
}

void draw_notice_banner(const char* text,
                        NoticeTone tone,
                        float width,
                        const Theme& theme,
                        NoticeBannerStyle style) {
    const float scale = current_scale();
    const NoticeWidgetTokens& noticeTokens = theme.widgets.notice;
    const ImVec2 padding(style.padding.x >= 0.0f ? style.padding.x : noticeTokens.noticeBannerPadding.x,
                         style.padding.y >= 0.0f ? style.padding.y : noticeTokens.noticeBannerPadding.y);
    const float rounding = style.rounding >= 0.0f ? style.rounding : noticeTokens.noticeBannerRounding;
    const float spacingAfter = style.spacingAfter >= 0.0f ? style.spacingAfter : noticeTokens.noticeBannerSpacingAfter;
    const float padX = padding.x * scale;
    const float padY = padding.y * scale;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const TextBlockMetrics textMetrics = measure_text_block(text, std::max(1.0f, width - padX * 2.0f));
    const float height = textMetrics.size.y + padY * 2.0f;
    ImVec2 max(pos.x + width, pos.y + height);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const NoticeToneStyle& toneStyle = notice_tone_style(theme, tone);

    NoticeSurfaceStyle surfaceStyle;
    surfaceStyle.rounding = rounding * scale;
    surfaceStyle.fill = toneStyle.fill;
    surfaceStyle.border = toneStyle.border;
    surfaceStyle.textColor = toneStyle.text;
    surfaceStyle.padding = ImVec2(padX, padY);
    draw_notice_surface(drawList,
                        pos,
                        max,
                        text,
                        width - padX * 2.0f,
                        surfaceStyle);
    ImGui::Dummy(ImVec2(0.0f, height + spacingAfter * scale));
}

int draw_two_option_segmented_control(const char* id,
                                      const SegmentedOption& first,
                                      const SegmentedOption& second,
                                      int selectedIndex,
                                      const Theme& theme,
                                      SegmentedControlStyle style) {
    const Palette& palette = theme.palette;
    const SegmentedControlWidgetTokens& tokens = theme.widgets.segmentedControl;
    const float scale = current_scale();
    const float gap = (style.gap >= 0.0f ? style.gap : tokens.gap) * scale;
    const float height = (style.height >= 0.0f ? style.height : tokens.height) * scale;
    const float rounding = (style.rounding >= 0.0f ? style.rounding : tokens.rounding) * scale;

    ImVec2 firstSize = style.firstSize;
    if (firstSize.x < 0.0f) {
        firstSize.x = std::max(42.0f * scale, ImGui::CalcTextSize(safe_text(first.label)).x + 18.0f * scale);
    }
    if (firstSize.y < 0.0f) {
        firstSize.y = height;
    }
    ImVec2 secondSize = style.secondSize;
    if (secondSize.x < 0.0f) {
        secondSize.x = std::max(42.0f * scale, ImGui::CalcTextSize(safe_text(second.label)).x + 18.0f * scale);
    }
    if (secondSize.y < 0.0f) {
        secondSize.y = height;
    }

    const ImVec4 baseFillColor = style.baseFillColor.x >= 0.0f
        ? style.baseFillColor
        : ImVec4(0.031f, 0.045f, 0.078f, tokens.idleFillAlpha);
    const ImVec4 hoveredFillColor = style.hoveredFillColor.x >= 0.0f
        ? style.hoveredFillColor
        : ImVec4(0.055f, 0.075f, 0.125f, tokens.hoveredFillAlpha);
    const ImVec4 selectedFillColor = style.selectedFillColor.x >= 0.0f
        ? style.selectedFillColor
        : palette.accent;
    const ImVec4 baseTextColor = style.baseTextColor.x >= 0.0f ? style.baseTextColor : palette.text;
    const ImVec4 selectedTextColor = style.selectedTextColor.x >= 0.0f ? style.selectedTextColor : palette.text;
    const ImVec4 accentColor = style.accentColor.x >= 0.0f ? style.accentColor : palette.accent;

    auto drawOption = [&](const char* optionId,
                          const char* label,
                          ImVec2 size,
                          bool selected) {
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::PushID(optionId != nullptr ? optionId : label);
        const bool pressed = ImGui::InvisibleButton("##segmented_option", size);
        const bool hovered = ImGui::IsItemHovered();
        const bool held = ImGui::IsItemActive();

        InteractionVisualState state;
        state.hovered = hovered;
        state.active = held;
        state.selected = selected;

        CapsuleStyle capsuleStyle;
        capsuleStyle.rounding = rounding;
        capsuleStyle.fill = selected ? selectedFillColor : (hovered ? hoveredFillColor : baseFillColor);
        capsuleStyle.textColor = selected ? selectedTextColor : baseTextColor;

        AccentFrameStyle frameStyle;
        frameStyle.rounding = rounding;
        frameStyle.color = accentColor;
        frameStyle.idleBorderAlpha = tokens.idleBorderAlpha;
        frameStyle.hoveredBorderAlpha = selected ? tokens.selectedBorderAlpha : 0.0f;
        frameStyle.activeBorderAlpha = selected ? tokens.selectedBorderAlpha : 0.0f;
        frameStyle.selectedBorderAlpha = tokens.selectedBorderAlpha;
        frameStyle.idleGlowAlpha = 0.0f;
        frameStyle.hoveredGlowAlpha = 0.0f;
        frameStyle.activeGlowAlpha = 0.0f;
        frameStyle.selectedGlowAlpha = 0.0f;
        frameStyle.idleThickness = 1.0f;
        frameStyle.activeThickness = 1.0f;
        frameStyle.selectedThickness = 1.0f;
        capsuleStyle.frame = frameStyle;

        draw_capsule(ImGui::GetWindowDrawList(),
                     pos,
                     ImVec2(pos.x + size.x, pos.y + size.y),
                     safe_text(label),
                     theme,
                     state,
                     capsuleStyle);
        ImGui::PopID();
        return pressed;
    };

    ImGui::PushID(id);
    ImGui::BeginGroup();
    const bool firstPressed = drawOption(first.id, first.label, firstSize, selectedIndex == 0);
    ImGui::SameLine(0.0f, gap);
    const bool secondPressed = drawOption(second.id, second.label, secondSize, selectedIndex == 1);
    ImGui::EndGroup();
    ImGui::PopID();

    if (firstPressed) {
        return 0;
    }
    if (secondPressed) {
        return 1;
    }
    return selectedIndex;
}

int draw_three_option_segmented_control(const char* id,
                                        const SegmentedOption& first,
                                        const SegmentedOption& second,
                                        const SegmentedOption& third,
                                        int selectedIndex,
                                        const Theme& theme,
                                        SegmentedControlStyle style) {
    const Palette& palette = theme.palette;
    const SegmentedControlWidgetTokens& tokens = theme.widgets.segmentedControl;
    const float scale = current_scale();
    const float gap = (style.gap >= 0.0f ? style.gap : tokens.gap) * scale;
    const float height = (style.height >= 0.0f ? style.height : tokens.height) * scale;
    const float rounding = (style.rounding >= 0.0f ? style.rounding : tokens.rounding) * scale;
    const float borderThickness = (style.borderThickness >= 0.0f ? style.borderThickness : 2.0f) * scale;

    auto resolveSize = [&](ImVec2 size, const char* label) {
        if (size.x < 0.0f) {
            size.x = std::max(52.0f * scale, ImGui::CalcTextSize(safe_text(label)).x + 20.0f * scale);
        }
        if (size.y < 0.0f) {
            size.y = height;
        }
        return size;
    };
    const ImVec2 firstSize = resolveSize(style.firstSize, first.label);
    const ImVec2 secondSize = resolveSize(style.secondSize, second.label);
    const ImVec2 thirdSize = resolveSize(style.thirdSize, third.label);

    const ImVec4 baseFillColor = style.baseFillColor.x >= 0.0f
        ? style.baseFillColor
        : ImVec4(0.031f, 0.045f, 0.078f, tokens.idleFillAlpha);
    const ImVec4 hoveredFillColor = style.hoveredFillColor.x >= 0.0f
        ? style.hoveredFillColor
        : ImVec4(0.055f, 0.075f, 0.125f, tokens.hoveredFillAlpha);
    const ImVec4 selectedFillColor = style.selectedFillColor.x >= 0.0f
        ? style.selectedFillColor
        : palette.accent;
    const ImVec4 middleFillColor = style.middleFillColor.x >= 0.0f
        ? style.middleFillColor
        : ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.18f);
    const ImVec4 borderColor = style.borderColor.x >= 0.0f
        ? style.borderColor
        : ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.58f);
    const ImVec4 baseTextColor = style.baseTextColor.x >= 0.0f ? style.baseTextColor : palette.textMuted;
    const ImVec4 selectedTextColor = style.selectedTextColor.x >= 0.0f ? style.selectedTextColor : palette.text;
    const ImVec4 accentColor = style.accentColor.x >= 0.0f ? style.accentColor : palette.accent;

    auto drawOption = [&](const SegmentedOption& option, ImVec2 size, bool selected, bool middleSelected) {
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::PushID(option.id != nullptr ? option.id : option.label);
        const bool pressed = ImGui::InvisibleButton("##segmented_option", size);
        const bool hovered = ImGui::IsItemHovered();
        const bool held = ImGui::IsItemActive();

        InteractionVisualState state;
        state.hovered = hovered;
        state.active = held;
        state.selected = selected;

        CapsuleStyle capsuleStyle;
        capsuleStyle.rounding = rounding;
        capsuleStyle.fill = selected
            ? (middleSelected ? middleFillColor : selectedFillColor)
            : (hovered ? hoveredFillColor : baseFillColor);
        capsuleStyle.textColor = selected ? selectedTextColor : baseTextColor;

        AccentFrameStyle frameStyle;
        frameStyle.rounding = rounding;
        frameStyle.color = selected ? accentColor : borderColor;
        frameStyle.idleBorderAlpha = selected ? 0.82f : 0.52f;
        frameStyle.hoveredBorderAlpha = selected ? 0.96f : 0.70f;
        frameStyle.activeBorderAlpha = selected ? 1.0f : 0.78f;
        frameStyle.selectedBorderAlpha = 1.0f;
        frameStyle.idleGlowAlpha = 0.0f;
        frameStyle.hoveredGlowAlpha = 0.0f;
        frameStyle.activeGlowAlpha = 0.0f;
        frameStyle.selectedGlowAlpha = 0.0f;
        frameStyle.idleThickness = borderThickness;
        frameStyle.activeThickness = borderThickness;
        frameStyle.selectedThickness = borderThickness;
        capsuleStyle.frame = frameStyle;

        draw_capsule(ImGui::GetWindowDrawList(),
                     pos,
                     ImVec2(pos.x + size.x, pos.y + size.y),
                     safe_text(option.label),
                     theme,
                     state,
                     capsuleStyle);
        ImGui::PopID();
        return pressed;
    };

    ImGui::PushID(id);
    ImGui::BeginGroup();
    const bool firstPressed = drawOption(first, firstSize, selectedIndex == 0, false);
    ImGui::SameLine(0.0f, gap);
    const bool secondPressed = drawOption(second, secondSize, selectedIndex == 1, true);
    ImGui::SameLine(0.0f, gap);
    const bool thirdPressed = drawOption(third, thirdSize, selectedIndex == 2, false);
    ImGui::EndGroup();
    ImGui::PopID();

    if (firstPressed) {
        return 0;
    }
    if (secondPressed) {
        return 1;
    }
    if (thirdPressed) {
        return 2;
    }
    return selectedIndex;
}

WindowChromeControlsResult draw_window_chrome_controls(const char* id,
                                                       const WindowChromeControlsData& data,
                                                       const Theme& theme,
                                                       WindowChromeControlsStyle style) {
    const Palette& palette = theme.palette;
    const WindowChromeControlsWidgetTokens& tokens = theme.widgets.windowChromeControls;
    const float scale = current_scale();
    const float gap = (style.gap >= 0.0f ? style.gap : tokens.gap) * scale;
    const float buttonHeight = tokens.buttonHeight * scale;
    const ImVec2 minimizeSize(style.minimizeButtonSize.x >= 0.0f ? style.minimizeButtonSize.x : tokens.minimizeWidth * scale,
                              style.minimizeButtonSize.y >= 0.0f ? style.minimizeButtonSize.y : buttonHeight);
    const ImVec2 closeSize(style.closeButtonSize.x >= 0.0f ? style.closeButtonSize.x : tokens.closeWidth * scale,
                           style.closeButtonSize.y >= 0.0f ? style.closeButtonSize.y : buttonHeight);

    WindowChromeControlsResult result;
    result.selectedIndex = data.selectedIndex;

    ImGui::PushID(id);
    if (data.showLanguageSegment) {
        result.selectedIndex = draw_two_option_segmented_control("language_segment",
                                                                 data.firstOption,
                                                                 data.secondOption,
                                                                 data.selectedIndex,
                                                                 theme,
                                                                 style.languageSegment);
        ImGui::SameLine(0.0f, gap);
    }

    const ImVec4 minimizeAccent = style.minimizeAccentColor.x >= 0.0f
        ? style.minimizeAccentColor
        : palette.textMuted;
    const ImVec4 closeAccent = style.closeAccentColor.x >= 0.0f
        ? style.closeAccentColor
        : palette.danger;

    result.minimizePressed = draw_outline_action_button("minimize",
                                                        "-",
                                                        minimizeSize,
                                                        minimizeAccent,
                                                        theme,
                                                        style.buttonFillAlpha,
                                                        style.buttonBorderAlpha);
    ImGui::SameLine(0.0f, gap);
    result.closePressed = draw_outline_action_button("close",
                                                     "X",
                                                     closeSize,
                                                     closeAccent,
                                                     theme,
                                                     style.buttonFillAlpha,
                                                     style.buttonBorderAlpha);
    ImGui::PopID();
    return result;
}

void draw_status_pill(const char* label,
                      const char* value,
                      ImVec4 valueColor,
                      const Theme& theme) {
    const Palette& palette = theme.palette;
    const SurfaceTokens& surfaces = theme.surfaces;
    const StatusPillWidgetTokens& statusTokens = theme.widgets.statusPill;
    const float scale = current_scale();
    const ImVec2 size = status_pill_size(label, value, theme);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 max(pos.x + size.x, pos.y + size.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    StatusPillStyle style;
    style.rounding = 3.0f * scale;
    style.fill = surfaces.statusPillBg;
    style.labelColor = ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.76f);
    style.valueColor = valueColor;
    style.indicatorColor = ImVec4(valueColor.x, valueColor.y, valueColor.z, 0.68f);
    style.baselineColor = ImVec4(palette.border.x, palette.border.y, palette.border.z, 0.16f);
    style.indicatorInsetY = statusTokens.statusPillIndicatorInsetY * scale;
    style.indicatorWidth = statusTokens.statusPillIndicatorWidth * scale;
    style.baselineInsetStart = statusTokens.statusPillLineInsetStart * scale;
    style.baselineInsetEnd = statusTokens.statusPillLineInsetEnd * scale;
    style.labelLead = statusTokens.statusPillLabelLead * scale;
    style.valueInset = statusTokens.statusPillValueInset * scale;
    draw_status_pill_surface(drawList, pos, max, safe_text(label), safe_text(value), style);
    ImGui::Dummy(size);
}

ImVec2 status_pill_size(const char* label,
                        const char* value,
                        const Theme& theme) {
    const StatusPillWidgetTokens& statusTokens = theme.widgets.statusPill;
    const float scale = current_scale();
    return ImVec2(pill_width_for_text(safe_text(label),
                                      safe_text(value),
                                      statusTokens.statusPillHorizontalPadding * scale,
                                      statusTokens.statusPillMinWidth * scale,
                                      statusTokens.statusPillValueGap * scale),
                  statusTokens.statusPillHeight * scale);
}

WidgetFrameLayout draw_status_pill_group(const StatusPillData* items,
                                         int count,
                                         const Theme& theme,
                                         StatusPillGroupStyle style) {
    const Palette& palette = theme.palette;
    const SurfaceTokens& surfaces = theme.surfaces;
    const StatusPillWidgetTokens& pillTokens = theme.widgets.statusPill;
    const StatusPillGroupWidgetTokens& groupTokens = theme.widgets.statusPillGroup;
    const float scale = current_scale();
    const float gap = (style.gap >= 0.0f ? style.gap : groupTokens.gap) * scale;
    const float rowGap = (style.rowGap >= 0.0f ? style.rowGap : groupTokens.rowGap) * scale;
    const float wrapWidth = style.wrapWidth >= 0.0f ? style.wrapWidth * scale : -1.0f;
    const float pillHeight = pillTokens.statusPillHeight * scale;
    const float horizontalPadding = pillTokens.statusPillHorizontalPadding * scale;
    const float minimumWidth = pillTokens.statusPillMinWidth * scale;
    const float valueGap = pillTokens.statusPillValueGap * scale;
    const float rounding = 3.0f * scale;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    WidgetFrameLayout layout;
    layout.min = origin;
    layout.max = origin;

    if (items == nullptr || count <= 0) {
        return layout;
    }

    StatusPillStyle pillStyle;
    pillStyle.rounding = rounding;
    pillStyle.fill = surfaces.statusPillBg;
    pillStyle.labelColor = ImVec4(palette.textMuted.x, palette.textMuted.y, palette.textMuted.z, 0.76f);
    pillStyle.indicatorInsetY = pillTokens.statusPillIndicatorInsetY * scale;
    pillStyle.indicatorWidth = pillTokens.statusPillIndicatorWidth * scale;
    pillStyle.baselineInsetStart = pillTokens.statusPillLineInsetStart * scale;
    pillStyle.baselineInsetEnd = pillTokens.statusPillLineInsetEnd * scale;
    pillStyle.labelLead = pillTokens.statusPillLabelLead * scale;
    pillStyle.valueInset = pillTokens.statusPillValueInset * scale;

    float cursorX = origin.x;
    float cursorY = origin.y;
    float rowStartX = origin.x;
    float rowMaxHeight = pillHeight;
    float contentMaxX = origin.x;
    float contentMaxY = origin.y;

    for (int index = 0; index < count; ++index) {
        const StatusPillData& item = items[index];
        const char* label = safe_text(item.label);
        const char* value = safe_text(item.value);
        const float pillWidth = pill_width_for_text(label,
                                                    value,
                                                    horizontalPadding,
                                                    minimumWidth,
                                                    valueGap);

        if (style.layout == StatusPillGroupLayout::Wrap &&
            index > 0 &&
            wrapWidth > 0.0f &&
            cursorX + pillWidth > rowStartX + wrapWidth) {
            cursorX = rowStartX;
            cursorY += rowMaxHeight + rowGap;
        } else if (style.layout == StatusPillGroupLayout::Vertical && index > 0) {
            cursorX = rowStartX;
            cursorY += rowMaxHeight + rowGap;
        }

        const ImVec2 min(cursorX, cursorY);
        const ImVec2 max(cursorX + pillWidth, cursorY + pillHeight);
        const ImVec4 resolvedValueColor = resolve_color(item.valueColor, palette.text);
        pillStyle.valueColor = resolvedValueColor;
        pillStyle.indicatorColor = ImVec4(resolvedValueColor.x, resolvedValueColor.y, resolvedValueColor.z, 0.68f);
        pillStyle.baselineColor = ImVec4(palette.border.x, palette.border.y, palette.border.z, 0.16f);
        draw_status_pill_surface(drawList, min, max, label, value, pillStyle);

        contentMaxX = std::max(contentMaxX, max.x);
        contentMaxY = std::max(contentMaxY, max.y);

        if (style.layout == StatusPillGroupLayout::Vertical) {
            continue;
        }

        cursorX = max.x + gap;
    }

    layout.max = ImVec2(contentMaxX, contentMaxY);
    ImGui::Dummy(ImVec2(std::max(0.0f, layout.max.x - layout.min.x),
                        std::max(0.0f, layout.max.y - layout.min.y)));
    return layout;
}

WidgetFrameLayout draw_status_info_list(const StatusInfoRowData* items,
                                        int count,
                                        const Theme& theme,
                                        StatusInfoListStyle style) {
    const StatusInfoListWidgetTokens& tokens = theme.widgets.statusInfoList;
    const float scale = current_scale();
    const float rowGap = (style.rowGap >= 0.0f ? style.rowGap : tokens.rowGap) * scale;
    const float labelWidth = style.labelWidth >= 0.0f ? style.labelWidth : 102.0f * scale;
    const ImVec2 origin = ImGui::GetCursorScreenPos();

    WidgetFrameLayout layout;
    layout.min = origin;
    layout.max = origin;

    if (items == nullptr || count <= 0) {
        return layout;
    }

    float maxRowWidth = 0.0f;
    float totalHeight = 0.0f;
    for (int index = 0; index < count; ++index) {
        const StatusInfoRowData& item = items[index];
        const char* label = safe_text(item.label);
        const char* value = safe_text(item.value);
        const ImVec2 labelSize = ImGui::CalcTextSize(label);
        const ImVec2 valueSize = ImGui::CalcTextSize(value);
        const float rowHeight = std::max(labelSize.y, valueSize.y);

        draw_status_row(label,
                        value,
                        resolve_color(item.valueColor, theme.palette.text),
                        labelWidth);

        totalHeight += rowHeight;
        if (index > 0) {
            totalHeight += rowGap;
        }
        maxRowWidth = std::max(maxRowWidth, labelWidth + valueSize.x);

        if (index + 1 < count) {
            ImGui::Dummy(ImVec2(0.0f, rowGap));
        }
    }

    layout.max = ImVec2(origin.x + maxRowWidth, origin.y + totalHeight);
    return layout;
}

WidgetFrameLayout draw_theme_summary(const ThemeSummaryData& data,
                                     const Theme& theme,
                                     ThemeSummaryStyle style) {
    const char* toneLabel = resolve_theme_summary_text(
        data.text.toneLabel,
        localized_theme_summary_fallback(data.language, "Tone", "\xE4\xB8\xBB\xE9\xA2\x98\xE6\xB0\x94\xE8\xB4\xA8"));
    const char* backgroundLabel = resolve_theme_summary_text(
        data.text.backgroundLabel,
        localized_theme_summary_fallback(data.language, "Preferred background", "\xE6\x8E\xA8\xE8\x8D\x90\xE8\x83\x8C\xE6\x99\xAF"));
    const char* motionLabel = resolve_theme_summary_text(
        data.text.motionLabel,
        localized_theme_summary_fallback(data.language, "Motion", "\xE5\x8A\xA8\xE6\x95\x88\xE9\xA3\x8E\xE6\xA0\xBC"));
    const BackgroundKind preferredBackground = default_background_kind_for_flavor(data.flavor);
    const char* toneValue = theme_flavor_tone(data.flavor, data.language);
    const char* backgroundValue = background_kind_label(preferredBackground, data.language);
    const char* motionValue = theme_motion_summary_text(data);

    StatusInfoRowData rows[] = {
        {toneLabel, toneValue, theme.palette.textMuted},
        {backgroundLabel, backgroundValue, theme.palette.text},
        {motionLabel, motionValue, theme.palette.textMuted},
    };

    StatusInfoListStyle listStyle;
    listStyle.labelWidth = style.labelWidth;
    listStyle.rowGap = style.rowGap;
    return draw_status_info_list(rows,
                                 static_cast<int>(std::size(rows)),
                                 theme,
                                 listStyle);
}

WidgetFrameLayout draw_theme_selection_summary(const ThemeSelectionSummaryData& data,
                                               const Theme& theme,
                                               ThemeSelectionSummaryStyle style) {
    const char* presetLabel = resolve_theme_selection_summary_text(
        data.text.presetLabel,
        localized_theme_selection_summary_fallback(data.language, "Preset", "\xE5\x8C\xB9\xE9\x85\x8D\xE9\xA2\x84\xE8\xAE\xBE"));
    const char* modeLabel = resolve_theme_selection_summary_text(
        data.text.modeLabel,
        localized_theme_selection_summary_fallback(data.language, "Selection mode", "\xE9\x80\x89\xE6\x8B\xA9\xE6\xA8\xA1\xE5\xBC\x8F"));
    const std::string presetValue = make_theme_selection_preset_text(data);
    const std::string modeValue = make_theme_selection_mode_text(data);

    StatusInfoRowData rows[] = {
        {presetLabel, presetValue.c_str(), theme.palette.text},
        {modeLabel, modeValue.c_str(), theme.palette.textMuted},
    };

    StatusInfoListStyle listStyle;
    listStyle.labelWidth = style.labelWidth;
    listStyle.rowGap = style.rowGap;
    return draw_status_info_list(rows,
                                 static_cast<int>(std::size(rows)),
                                 theme,
                                 listStyle);
}

WidgetFrameLayout draw_background_summary(const BackgroundSummaryData& data,
                                          const Theme& theme,
                                          BackgroundSummaryStyle style) {
    const char* shellLabel = resolve_background_summary_text(
        data.text.shellLabel,
        localized_background_summary_fallback(data.language, "Shell fit", "\xE9\x80\x82\xE9\x85\x8D\xE5\xBB\xBA\xE8\xAE\xAE"));
    const char* characterLabel = resolve_background_summary_text(
        data.text.characterLabel,
        localized_background_summary_fallback(data.language, "Character", "\xE8\x83\x8C\xE6\x99\xAF\xE6\xB0\x94\xE8\xB4\xA8"));
    const char* capabilityLabel = resolve_background_summary_text(
        data.text.capabilityLabel,
        localized_background_summary_fallback(data.language, "Capability", "\xE8\x83\xBD\xE5\x8A\x9B\xE6\x91\x98\xE8\xA6\x81"));
    const char* layersLabel = resolve_background_summary_text(
        data.text.layersLabel,
        localized_background_summary_fallback(data.language, "Layers", "\xE5\xB1\x82\xE7\xBB\x84\xE6\x88\x90"));
    const bool recommended = background_recommended_for_authenticated_shell(data.kind);
    const char* shellValue = recommended
        ? resolve_background_summary_text(
              data.text.shellRecommended,
              localized_background_summary_fallback(data.language, "Recommended for main shell", "\xE6\x9B\xB4\xE9\x80\x82\xE5\x90\x88\xE4\xB8\xBB\xE7\x95\x8C\xE9\x9D\xA2"))
        : resolve_background_summary_text(
              data.text.shellMinimal,
              localized_background_summary_fallback(data.language, "Best for minimal/flat shell", "\xE6\x9B\xB4\xE9\x80\x82\xE5\x90\x88\xE6\x9E\x81\xE7\xAE\x80\x2F\xE7\xBA\xAF\xE5\xB9\xB3\xE7\x95\x8C\xE9\x9D\xA2"));

    const char* characterValue = background_character(data.kind, data.language);
    const std::string capabilityValue = make_background_capability_summary(data);
    const std::string layerValue = make_background_layer_summary(data);
    StatusInfoRowData rows[] = {
        {shellLabel, shellValue, theme.palette.textMuted},
        {characterLabel, characterValue, theme.palette.text},
        {capabilityLabel, capabilityValue.c_str(), theme.palette.textMuted},
        {layersLabel, layerValue.c_str(), theme.palette.text},
    };

    StatusInfoListStyle listStyle;
    listStyle.labelWidth = style.labelWidth;
    listStyle.rowGap = style.rowGap;
    return draw_status_info_list(rows,
                                 static_cast<int>(std::size(rows)),
                                 theme,
                                 listStyle);
}

WidgetFrameLayout draw_background_preview(const BackgroundPreviewData& data,
                                          const Theme& theme,
                                          BackgroundPreviewStyle style) {
    const float scale = current_scale();
    const BackgroundStyle previewBackground = resolve_background_preview_style(data);
    Theme previewTheme = theme;
    previewTheme.background = previewBackground;

    const ImVec2 size(style.size.x >= 0.0f ? style.size.x * scale : 0.0f,
                      style.size.y >= 0.0f ? style.size.y * scale : 136.0f * scale);
    const float availableWidth = ImGui::GetContentRegionAvail().x;
    const float previewWidth = size.x > 0.0f ? std::min(size.x, availableWidth) : availableWidth;
    const ImVec2 previewSize(std::max(0.0f, previewWidth), size.y);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max(min.x + previewSize.x, min.y + previewSize.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float pulse = animate(theme.motion.glowPulse);
    const float accentGlowAlpha = 0.05f + 0.05f * pulse;

    SurfaceStyle surfaceStyle = style.surface;
    if (surfaceStyle.rounding < 0.0f) {
        surfaceStyle.rounding = theme.radii.surface;
    }
    if (surfaceStyle.glowAlpha < 0.0f) {
        surfaceStyle.glowAlpha = theme.surfaces.cardGlowAlpha * 0.56f;
    }
    draw_surface_card(drawList, min, max, theme, surfaceStyle);
    draw_glow_rect(drawList, min, max, surfaceStyle.rounding, theme, accentGlowAlpha * 0.72f);

    const float previewInset = (style.previewInset >= 0.0f ? style.previewInset : 1.0f) * scale;
    const ImVec2 previewMin(min.x + previewInset, min.y + previewInset);
    const ImVec2 previewMax(max.x - previewInset, max.y - previewInset);

    drawList->PushClipRect(previewMin, previewMax, true);
    draw_app_background(drawList,
                        previewMin,
                        previewMax,
                        data.authenticated,
                        previewTheme);

    drawList->AddRectFilledMultiColor(
        previewMin,
        previewMax,
        to_u32(ImVec4(previewTheme.palette.background.x,
                      previewTheme.palette.background.y,
                      previewTheme.palette.background.z,
                      0.030f + 0.015f * pulse)),
        to_u32(ImVec4(previewTheme.palette.background.x,
                      previewTheme.palette.background.y,
                      previewTheme.palette.background.z,
                      0.000f)),
        to_u32(ImVec4(previewTheme.palette.panel.x,
                      previewTheme.palette.panel.y,
                      previewTheme.palette.panel.z,
                      0.090f + 0.030f * pulse)),
        to_u32(ImVec4(previewTheme.palette.sidebar.x,
                      previewTheme.palette.sidebar.y,
                      previewTheme.palette.sidebar.z,
                      0.060f + 0.020f * pulse)));

    draw_ambient_glow(drawList,
                      ImVec2(previewMin.x + 20.0f * scale, previewMin.y + 18.0f * scale),
                      30.0f * scale,
                      previewTheme,
                      0.06f + 0.04f * pulse);
    draw_ambient_glow(drawList,
                      ImVec2(previewMax.x - 32.0f * scale, previewMax.y - 28.0f * scale),
                      38.0f * scale,
                      previewTheme,
                      0.04f + 0.03f * pulse);
    draw_preview_sweep(drawList,
                       ImVec2(previewMin.x, previewMin.y + previewSize.y * 0.18f),
                       ImVec2(previewMax.x, previewMax.y - previewSize.y * 0.18f),
                       previewTheme,
                       0.014f + 0.020f * pulse);
    const float overlayInset = (style.overlayInset >= 0.0f ? style.overlayInset : 10.0f) * scale;
    draw_background_preview_name_pill(drawList,
                                      previewMin,
                                      previewMax,
                                      data,
                                      previewTheme,
                                      overlayInset,
                                      pulse);
    drawList->PopClipRect();

    ImGui::Dummy(previewSize);

    WidgetFrameLayout layout;
    layout.min = min;
    layout.max = max;
    return layout;
}

bool draw_theme_selection_section(const char* id,
                                  const ThemeSelectionSectionData& data,
                                  const Theme& theme,
                                  const std::function<void(ThemeFlavor)>& activateFlavor,
                                  const std::function<void(BackgroundKind)>& activateBackgroundKind,
                                  ThemeSelectionSectionStyle style) {
    ImGui::PushID(id);

    const FormLayoutStyle layout = style.layout.preferredColumnX >= 0.0f ||
        style.layout.insetX >= 0.0f ||
        style.layout.nestedInsetX >= 0.0f ||
        style.layout.minimumLabelGap >= 0.0f ||
        style.layout.rowSpacing >= 0.0f ||
        style.layout.sectionSpacing >= 0.0f ||
        style.layout.footerSpacing >= 0.0f
        ? style.layout
        : make_form_layout_style(theme, data.language, FormLayoutPreset::Standard);
    const float insetX = style.insetX >= 0.0f
        ? style.insetX
        : layout.insetX;

    ThemeFlavorFieldData flavorField;
    flavorField.language = data.language;
    flavorField.text = data.themeFlavorFieldText;

    bool changed = draw_theme_flavor_field(
        "theme_flavor",
        data.selection.flavor,
        flavorField,
        theme,
        activateFlavor,
        layout,
        style.field,
        insetX);

    add_vertical_space(style.fieldGap);

    BackgroundKindFieldData backgroundField;
    backgroundField.language = data.language;
    backgroundField.text = data.backgroundKindFieldText;
    changed |= draw_background_kind_field(
        "background_kind",
        data.selection.backgroundKind,
        backgroundField,
        theme,
        activateBackgroundKind,
        layout,
        style.field,
        insetX);

    if (data.showBackgroundPreview) {
        add_vertical_space(style.previewGap);
        set_scaled_cursor_x(insetX);

        BackgroundPreviewData backgroundPreview;
        backgroundPreview.flavor = data.selection.flavor;
        backgroundPreview.kind = data.selection.backgroundKind;
        backgroundPreview.style = data.backgroundStyle;
        backgroundPreview.language = data.language;
        backgroundPreview.authenticated = data.previewAuthenticated;
        backgroundPreview.text = data.backgroundPreviewText;
        draw_background_preview(backgroundPreview, theme, style.backgroundPreview);
    }

    if (!data.showDetails) {
        ImGui::PopID();
        return changed;
    }

    add_vertical_space(style.summaryStartGap);
    set_scaled_cursor_x(insetX);

    ThemeSummaryData themeSummary;
    themeSummary.flavor = data.selection.flavor;
    themeSummary.language = data.language;
    themeSummary.text = data.themeSummaryText;
    const float scale = current_scale();

    ThemeSummaryStyle themeSummaryStyle = style.themeSummary;
    if (themeSummaryStyle.labelWidth < 0.0f) {
        themeSummaryStyle.labelWidth = resolve_theme_selection_label_width(data.language, -1.0f) * scale;
    }
    if (themeSummaryStyle.rowGap < 0.0f) {
        themeSummaryStyle.rowGap = 6.0f;
    }
    draw_theme_summary(themeSummary, theme, themeSummaryStyle);

    add_vertical_space(style.summaryGap);
    set_scaled_cursor_x(insetX);

    ThemeSelectionSummaryData selectionSummary;
    selectionSummary.selection = data.selection;
    selectionSummary.language = data.language;
    selectionSummary.text = data.selectionSummaryText;
    ThemeSelectionSummaryStyle selectionSummaryStyle = style.selectionSummary;
    if (selectionSummaryStyle.labelWidth < 0.0f) {
        selectionSummaryStyle.labelWidth = resolve_theme_selection_label_width(data.language, -1.0f) * scale;
    }
    if (selectionSummaryStyle.rowGap < 0.0f) {
        selectionSummaryStyle.rowGap = 6.0f;
    }
    draw_theme_selection_summary(selectionSummary, theme, selectionSummaryStyle);

    add_vertical_space(style.summaryGap);
    set_scaled_cursor_x(insetX);

    BackgroundSummaryData backgroundSummary;
    backgroundSummary.kind = data.selection.backgroundKind;
    backgroundSummary.style = data.backgroundStyle;
    backgroundSummary.language = data.language;
    backgroundSummary.text = data.backgroundSummaryText;
    BackgroundSummaryStyle backgroundSummaryStyle = style.backgroundSummary;
    if (backgroundSummaryStyle.labelWidth < 0.0f) {
        backgroundSummaryStyle.labelWidth = resolve_background_summary_label_width(data.language, -1.0f) * scale;
    }
    if (backgroundSummaryStyle.rowGap < 0.0f) {
        backgroundSummaryStyle.rowGap = 6.0f;
    }
    draw_background_summary(backgroundSummary, theme, backgroundSummaryStyle);

    ImGui::PopID();
    return changed;
}

WidgetFrameLayout draw_signal_pill_group(const SignalPillData* items,
                                         int count,
                                         bool live,
                                         const Theme& theme,
                                         SignalPillGroupStyle style) {
    const SignalPillGroupWidgetTokens& tokens = theme.widgets.signalPillGroup;
    const SignalPillWidgetTokens& pillTokens = theme.widgets.signalPill;
    const float scale = current_scale();
    const float gap = (style.gap >= 0.0f ? style.gap : tokens.gap) * scale;
    const float rowGap = (style.rowGap >= 0.0f ? style.rowGap : tokens.rowGap) * scale;
    const float wrapWidth = style.wrapWidth >= 0.0f ? style.wrapWidth * scale : -1.0f;
    const float minWidth = pillTokens.minWidth * scale;
    const float horizontalPadding = pillTokens.horizontalPadding * scale;
    const float pillHeight = pillTokens.height * scale;
    const ImVec2 origin = ImGui::GetCursorScreenPos();

    WidgetFrameLayout layout;
    layout.min = origin;
    layout.max = origin;

    if (items == nullptr || count <= 0) {
        return layout;
    }

    float cursorX = origin.x;
    float cursorY = origin.y;
    float contentMaxX = origin.x;
    float contentMaxY = origin.y;

    for (int index = 0; index < count; ++index) {
        const SignalPillData& item = items[index];
        const char* label = safe_text(item.label);
        const float width = std::max(minWidth, ImGui::CalcTextSize(label).x + horizontalPadding);

        if (style.layout == SignalPillGroupLayout::Wrap &&
            index > 0 &&
            wrapWidth > 0.0f &&
            cursorX + width > origin.x + wrapWidth) {
            cursorX = origin.x;
            cursorY += pillHeight + rowGap;
        }

        ImGui::SetCursorScreenPos(ImVec2(cursorX, cursorY));
        draw_signal_pill(label, item.active, live, theme);

        contentMaxX = std::max(contentMaxX, cursorX + width);
        contentMaxY = std::max(contentMaxY, cursorY + pillHeight);
        cursorX += width + gap;
    }

    layout.max = ImVec2(contentMaxX, contentMaxY);
    ImGui::SetCursorScreenPos(origin);
    ImGui::Dummy(ImVec2(std::max(0.0f, layout.max.x - layout.min.x),
                        std::max(0.0f, layout.max.y - layout.min.y)));
    return layout;
}

void draw_status_row(const char* label, const char* value, ImVec4 valueColor, float labelWidth) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", safe_text(label));
    ImGui::SameLine(labelWidth);
    ImGui::PushStyleColor(ImGuiCol_Text, valueColor);
    ImGui::TextUnformatted(safe_text(value));
    ImGui::PopStyleColor();
}

void draw_signal_pill(const char* label,
                      bool active,
                      bool live,
                      const Theme& theme) {
    const Palette& palette = theme.palette;
    const SignalPillWidgetTokens& tokens = theme.widgets.signalPill;
    const float scale = current_scale();
    const float width = std::max(tokens.minWidth * scale,
                                 ImGui::CalcTextSize(safe_text(label)).x + tokens.horizontalPadding * scale);
    const ImVec2 size(width, tokens.height * scale);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec4 offColor = live ? ImVec4(0.035f, 0.050f, 0.085f, 1.0f) : ImVec4(0.180f, 0.040f, 0.060f, 1.0f);
    const ImVec4 onColor = live ? palette.accent : palette.danger;
    const float pulse = active ? imgui_onguoin::animate(theme.motion.signalPulse) : 0.0f;
    InteractionVisualState state;
    state.active = active;
    CapsuleStyle style;
    style.rounding = theme.radii.control;
    style.fill = active ? mix_color(onColor, palette.text, 0.08f * pulse) : offColor;
    style.textColor = live ? palette.text : ImVec4(1.0f, 0.70f, 0.75f, 1.0f);
    AccentFrameStyle frameStyle;
    frameStyle.rounding = theme.radii.control;
    frameStyle.color = active ? palette.accent : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    frameStyle.idleBorderAlpha = 0.06f;
    frameStyle.hoveredBorderAlpha = 0.06f;
    frameStyle.activeBorderAlpha = 0.70f;
    frameStyle.idleGlowAlpha = 0.0f;
    frameStyle.hoveredGlowAlpha = 0.0f;
    frameStyle.activeGlowAlpha = 0.18f;
    frameStyle.idleThickness = 1.0f;
    frameStyle.activeThickness = 1.6f;
    style.frame = frameStyle;
    draw_capsule(drawList, pos, ImVec2(pos.x + size.x, pos.y + size.y), safe_text(label), theme, state, style);
    ImGui::Dummy(size);
}

void draw_state_box(const char* label,
                    bool active,
                    ImVec4 accent,
                    ImVec2 size,
                    const Theme& theme) {
    const Palette& palette = theme.palette;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec4 border = active
        ? ImVec4(accent.x, accent.y, accent.z, 1.0f)
        : ImVec4(palette.text.x, palette.text.y, palette.text.z, 1.0f);

    InteractionVisualState state;
    state.selected = active;
    CapsuleStyle style;
    style.rounding = theme.radii.control;
    style.fill = active
        ? ImVec4(accent.x, accent.y, accent.z, 0.50f)
        : ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.textColor = palette.text;
    AccentFrameStyle frameStyle;
    frameStyle.rounding = theme.radii.control;
    frameStyle.color = border;
    frameStyle.idleBorderAlpha = border.w;
    frameStyle.hoveredBorderAlpha = border.w;
    frameStyle.activeBorderAlpha = border.w;
    frameStyle.selectedBorderAlpha = border.w;
    frameStyle.idleGlowAlpha = 0.0f;
    frameStyle.hoveredGlowAlpha = 0.0f;
    frameStyle.activeGlowAlpha = 0.0f;
    frameStyle.selectedGlowAlpha = 0.0f;
    frameStyle.idleThickness = 1.2f;
    frameStyle.activeThickness = 1.2f;
    frameStyle.selectedThickness = 1.2f;
    style.frame = frameStyle;
    draw_capsule(drawList, pos, ImVec2(pos.x + size.x, pos.y + size.y), label, theme, state, style);
    ImGui::Dummy(size);
}

bool draw_outline_action_button(const char* id,
                                const char* label,
                                ImVec2 size,
                                ImVec4 accent,
                                const Theme& theme,
                                float fillAlpha,
                                float borderAlpha,
                                ImVec4 textColor) {
    const Palette& palette = theme.palette;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::PushID(id);
    const bool pressed = ImGui::InvisibleButton("##outline_action", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    InteractionVisualState state;
    state.hovered = hovered;
    state.active = held;

    CapsuleStyle style;
    style.rounding = theme.radii.button;
    style.fill = ImVec4(accent.x,
                        accent.y,
                        accent.z,
                        held ? 0.50f : (hovered ? std::max(fillAlpha, 0.18f) : fillAlpha));
    style.textColor = textColor.x >= 0.0f ? textColor : palette.text;
    AccentFrameStyle frameStyle;
    frameStyle.rounding = theme.radii.button;
    frameStyle.color = accent;
    frameStyle.idleBorderAlpha = borderAlpha;
    frameStyle.hoveredBorderAlpha = 0.82f;
    frameStyle.activeBorderAlpha = 0.95f;
    frameStyle.idleGlowAlpha = 0.0f;
    frameStyle.hoveredGlowAlpha = 0.0f;
    frameStyle.activeGlowAlpha = 0.0f;
    frameStyle.idleThickness = 1.2f;
    frameStyle.activeThickness = 1.6f;
    style.frame = frameStyle;
    draw_capsule(drawList, pos, ImVec2(pos.x + size.x, pos.y + size.y), label, theme, state, style);
    ImGui::PopID();
    return pressed;
}

bool draw_primary_action_button(const char* id,
                                const char* label,
                                ImVec2 size,
                                bool enabled,
                                const Theme& theme) {
    const Palette& palette = theme.palette;
    const SurfaceTokens& surfaces = theme.surfaces;
    const float scale = current_scale();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::PushID(id);
    const bool pressed = ImGui::InvisibleButton("##primary_action", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    InteractionVisualState state;
    state.hovered = hovered;
    state.active = held;
    state.enabled = enabled;

    const float pulse = imgui_onguoin::animate(theme.motion.primaryActionPulse);
    const float activeBlend = enabled ? (held ? 0.55f : (hovered ? 0.42f : 0.26f)) : 0.08f;
    const ImVec2 min = pos;
    const ImVec2 max(pos.x + size.x, pos.y + size.y);
    const float rounding = theme.radii.button * scale;
    const ImVec4 fill = enabled
        ? mix_color(palette.surfaceRaised, palette.accentMuted, activeBlend)
        : ImVec4(0.035f, 0.040f, 0.052f, 0.92f);
    const ImVec4 border = enabled
        ? ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, hovered ? 0.72f : (0.42f + 0.10f * pulse))
        : ImVec4(palette.border.x, palette.border.y, palette.border.z, 0.16f);

    InteractiveSurfaceStyle style;
    style.rounding = rounding;
    style.fill = fill;
    style.textColor = enabled ? palette.text : palette.textMuted;
    style.textOffset = ImVec2(0.0f, -0.5f * scale);
    style.topHighlightAlpha = enabled ? surfaces.buttonTopHighlightAlpha : surfaces.buttonTopHighlightDisabledAlpha;
    style.topHighlightHeightFraction = 0.45f;
    style.topLineInset = 18.0f;
    style.topLineOffsetY = 1.4f;
    style.topLineColor = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, enabled ? 0.22f : 0.05f);
    AccentFrameStyle frameStyle;
    frameStyle.rounding = rounding;
    frameStyle.color = border;
    frameStyle.idleBorderAlpha = border.w;
    frameStyle.hoveredBorderAlpha = border.w;
    frameStyle.activeBorderAlpha = border.w;
    frameStyle.idleGlowAlpha = enabled ? theme.widgets.action.primaryGlowIdleAlpha : 0.0f;
    frameStyle.hoveredGlowAlpha = enabled ? theme.widgets.action.primaryGlowHoverAlpha : 0.0f;
    frameStyle.activeGlowAlpha = enabled ? theme.widgets.action.primaryGlowHoverAlpha : 0.0f;
    frameStyle.idleThickness = 1.2f;
    frameStyle.activeThickness = 1.7f;
    style.frame = frameStyle;
    draw_interactive_surface(drawList, min, max, label, theme, state, style);

    ImGui::PopID();
    return pressed && enabled;
}

bool draw_option_toggle_button(const char* id,
                               const char* label,
                               bool& value,
                               float width,
                               const Theme& theme,
                               OptionToggleStyle styleOptions) {
    const Palette& palette = theme.palette;
    const ToggleWidgetTokens& toggleTokens = theme.widgets.toggle;
    const float scale = current_scale();
    const ImVec2 size(width, toggleTokens.height * scale);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool neonFlow = styleOptions.visualEffect == OptionToggleVisualEffect::NeonFlow;

    ImGui::PushID(id);
    const ImGuiID widgetId = ImGui::GetID("##option_toggle");
    const bool pressed = ImGui::InvisibleButton("##option_toggle", size);
    const bool hovered = ImGui::IsItemHovered();
    if (pressed) {
        value = !value;
    }

    float neonHoverProgress = 0.0f;
    float neonActivationProgress = value ? 1.0f : 0.0f;
    if (neonFlow) {
        OptionToggleNeonState& neonState = option_toggle_neon_cache()[widgetId];
        if (!neonState.initialized) {
            neonState.activationProgress = value ? 1.0f : 0.0f;
            neonState.hoverProgress = hovered ? 1.0f : 0.0f;
            neonState.initialized = true;
        }

        const float deltaTime = std::clamp(ImGui::GetIO().DeltaTime, 0.0f, 1.0f / 20.0f);
        neonState.hoverProgress = move_toward(neonState.hoverProgress,
                                              hovered ? 1.0f : 0.0f,
                                              deltaTime / 0.18f);
        neonState.activationProgress = move_toward(neonState.activationProgress,
                                                   value ? 1.0f : 0.0f,
                                                   deltaTime / (value ? 0.45f : 0.22f));
        neonHoverProgress = neonState.hoverProgress;
        neonActivationProgress = neonState.activationProgress;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 max(pos.x + size.x, pos.y + size.y);
    const float rounding = neonFlow ? size.y * 0.5f : theme.radii.button * 0.92f * scale;
    const ImVec4 defaultTopFill = value
        ? (hovered ? ImVec4(0.050f, 0.073f, 0.086f, 1.0f) : ImVec4(0.040f, 0.055f, 0.067f, 1.0f))
        : (hovered ? ImVec4(0.049f, 0.056f, 0.074f, 1.0f) : ImVec4(0.034f, 0.039f, 0.052f, 1.0f));
    ImVec4 topFill = value && styleOptions.selectedFillColor.x >= 0.0f
        ? styleOptions.selectedFillColor
        : defaultTopFill;
    if (neonFlow) {
        const ImVec4 inactiveFill = hovered
            ? ImVec4(0.010f, 0.021f, 0.054f, 0.98f)
            : ImVec4(0.010f, 0.013f, 0.024f, 0.96f);
        const ImVec4 activeFill = styleOptions.selectedFillColor.x >= 0.0f
            ? styleOptions.selectedFillColor
            : ImVec4(0.014f, 0.033f, 0.126f, 0.98f);
        topFill = mix_color(inactiveFill, activeFill, neonActivationProgress);
    }
    InteractionVisualState state;
    state.hovered = hovered;
    state.selected = value;

    InteractiveSurfaceStyle style;
    style.rounding = rounding;
    style.fill = topFill;
    if (neonFlow) {
        style.textColor = value
            ? (styleOptions.selectedTextColor.x >= 0.0f
                   ? styleOptions.selectedTextColor
                   : ImVec4(0.90f, 0.96f, 1.0f, 1.0f))
            : (hovered
                   ? ImVec4(0.92f, 0.96f, 1.0f, 0.92f)
                   : palette.textMuted);
    } else {
        style.textColor = value
            ? (styleOptions.selectedTextColor.x >= 0.0f ? styleOptions.selectedTextColor : palette.accent)
            : palette.textMuted;
    }
    style.textOffset = ImVec2(0.0f, -0.5f * scale);
    AccentFrameStyle frameStyle;
    frameStyle.rounding = rounding;
    frameStyle.color = value
        ? (styleOptions.selectedBorderColor.x >= 0.0f ? styleOptions.selectedBorderColor : palette.accent)
        : (styleOptions.borderColor.x >= 0.0f ? styleOptions.borderColor : palette.border);
    frameStyle.idleBorderAlpha = 0.11f;
    frameStyle.hoveredBorderAlpha = value ? 0.72f : 0.28f;
    frameStyle.activeBorderAlpha = value ? 0.72f : 0.28f;
    frameStyle.selectedBorderAlpha = styleOptions.selectedBorderAlpha >= 0.0f ? styleOptions.selectedBorderAlpha : 0.48f;
    frameStyle.idleGlowAlpha = 0.0f;
    frameStyle.hoveredGlowAlpha = value ? toggleTokens.toggleGlowHoverAlpha : 0.0f;
    frameStyle.activeGlowAlpha = value ? toggleTokens.toggleGlowHoverAlpha : 0.0f;
    frameStyle.selectedGlowAlpha = toggleTokens.toggleGlowIdleAlpha;
    frameStyle.idleThickness = 1.0f;
    frameStyle.activeThickness = 1.4f;
    frameStyle.selectedThickness = 1.4f;
    if (neonFlow) {
        frameStyle.color = value
            ? option_toggle_flow_color(styleOptions, static_cast<float>(ImGui::GetTime()) * styleOptions.flowSpeed)
            : palette.border;
        frameStyle.idleBorderAlpha = 0.055f;
        frameStyle.hoveredBorderAlpha = value ? 0.12f : 0.10f;
        frameStyle.activeBorderAlpha = value ? 0.14f : 0.10f;
        frameStyle.selectedBorderAlpha = 0.13f;
        frameStyle.idleGlowAlpha = 0.0f;
        frameStyle.hoveredGlowAlpha = 0.0f;
        frameStyle.activeGlowAlpha = 0.0f;
        frameStyle.selectedGlowAlpha = 0.0f;
        frameStyle.idleThickness = 1.0f;
        frameStyle.activeThickness = 1.0f;
        frameStyle.selectedThickness = 1.0f;
    }
    style.frame = frameStyle;
    if (value && !neonFlow) {
        const float pulse = imgui_onguoin::animate(theme.motion.togglePulse);
        style.innerPulseColor = palette.accent;
        style.innerPulseAlpha = (hovered ? 0.22f : 0.14f) * pulse;
    }
    draw_interactive_surface(drawList, pos, max, neonFlow ? "" : label, theme, state, style);
    if (neonFlow) {
        draw_neon_flow_capsule(drawList,
                               pos,
                               max,
                               rounding,
                               styleOptions,
                               neonHoverProgress,
                               neonActivationProgress);
        draw_neon_flow_text(drawList,
                            pos,
                            max,
                            label,
                            theme,
                            styleOptions,
                            neonHoverProgress,
                            neonActivationProgress,
                            style.textOffset);
    }

    ImGui::PopID();
    return pressed;
}

bool draw_option_toggle_button(const char* id,
                               const char* label,
                               bool& value,
                               const Theme& theme,
                               OptionToggleStyle style) {
    const float resolvedWidth = style.width > 0.0f ? style.width : 220.0f * current_scale();
    if (style.helpText != nullptr && style.helpText[0] != '\0') {
        ImGui::BeginGroup();
        const bool changed = draw_option_toggle_button(id, label, value, resolvedWidth, theme, style);
        same_line_help_marker_aligned_to_last_item(style.helpText, theme);
        ImGui::EndGroup();
        return changed;
    }
    return draw_option_toggle_button(id, label, value, resolvedWidth, theme, style);
}

bool draw_direction_toggle_button(const char* id,
                                  const char* clockwiseLabel,
                                  const char* counterclockwiseLabel,
                                  bool& clockwise,
                                  const Theme& theme,
                                  DirectionToggleStyle style) {
    const Palette& palette = theme.palette;
    const DirectionToggleWidgetTokens& tokens = theme.widgets.directionToggle;
    const float scale = current_scale();
    const float width = style.width > 0.0f ? style.width : 220.0f * scale;
    const float height = (style.height > 0.0f ? style.height : tokens.height) * scale;
    const float knobDiameter = std::min(height - 4.0f * scale,
                                        (style.knobDiameter > 0.0f ? style.knobDiameter : tokens.knobDiameter) * scale);
    const float paddingX = tokens.horizontalPadding * scale;
    const ImVec2 size(width, height);

    if (style.helpText != nullptr && style.helpText[0] != '\0') {
        ImGui::BeginGroup();
    }

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::PushID(id);
    const ImGuiID stateId = ImGui::GetID("##direction_toggle_state");
    const bool pressed = ImGui::InvisibleButton("##direction_toggle", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    if (pressed) {
        clockwise = !clockwise;
    }

    DirectionToggleAnimationState& anim = direction_toggle_animation_cache()[stateId];
    if (!anim.initialized) {
        anim.knobProgress = clockwise ? 0.0f : 1.0f;
        anim.visualClockwise = clockwise;
        anim.lastClockwise = clockwise;
        anim.initialized = true;
    }
    if (anim.lastClockwise != clockwise) {
        anim.visualClockwise = !clockwise;
        anim.lastClockwise = clockwise;
    }

    const float targetProgress = clockwise ? 0.0f : 1.0f;
    anim.knobProgress = follow_value(anim.knobProgress, targetProgress, tokens.knobFollow);
    if (std::fabs(anim.knobProgress - targetProgress) < 0.006f) {
        anim.knobProgress = targetProgress;
        anim.visualClockwise = clockwise;
    }

    const ImVec2 max(pos.x + size.x, pos.y + size.y);
    const float rounding = height * 0.5f;
    const bool moving = anim.visualClockwise != clockwise || std::fabs(anim.knobProgress - targetProgress) > 0.006f;
    const float pulseValue = moving ? 1.0f : imgui_onguoin::animate(theme.motion.togglePulse);
    const float activeMix = hovered ? 0.17f : 0.105f;
    const ImVec4 trackFill = mix_color(palette.surfaceRaised,
                                       palette.accentMuted,
                                       activeMix);
    const ImVec4 trackBorder = ImVec4(palette.accent.x,
                                      palette.accent.y,
                                      palette.accent.z,
                                      (hovered ? 0.28f : tokens.trackBorderAlpha) + (moving ? 0.035f : 0.0f));

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float glowAlpha = hovered
        ? tokens.hoverGlowAlpha
        : (moving ? tokens.selectedGlowAlpha * 0.75f : tokens.selectedGlowAlpha * 0.38f * pulseValue);
    if (glowAlpha > 0.0f) {
        draw_static_glow_rect(drawList, pos, max, rounding, theme, glowAlpha, 1.0f * scale);
    }
    drawList->AddRectFilled(pos,
                            max,
                            to_u32(ImVec4(trackFill.x, trackFill.y, trackFill.z, tokens.trackFillAlpha)),
                            rounding);
    drawList->AddRect(pos,
                      max,
                      to_u32(trackBorder),
                      rounding,
                      0,
                      (held ? 1.55f : 1.15f) * scale);
    draw_top_highlight(drawList,
                       pos,
                       max,
                       rounding,
                       hovered ? 0.030f : 0.018f,
                       0.42f);

    const char* label = clockwise ? safe_text(clockwiseLabel) : safe_text(counterclockwiseLabel);
    const ImVec2 textSize = measure_text_block(label).size;
    const float textInset = tokens.textInset * scale;
    const ImVec2 textMin(pos.x + textInset, pos.y);
    const ImVec2 textMax(max.x - textInset, max.y);
    const ImVec4 textColor = mix_color(palette.textMuted, palette.text, hovered ? 0.54f : 0.42f);
    drawList->PushClipRect(ImVec2(pos.x + textInset, pos.y),
                           ImVec2(max.x - textInset, max.y),
                           true);
    drawList->AddText(centered_text_pos(textMin,
                                        textMax,
                                        textSize,
                                        ImVec2(0.0f, -0.5f * scale)),
                      to_u32(textColor),
                      label);
    drawList->PopClipRect();

    const float travel = std::max(0.0f, width - knobDiameter - paddingX * 2.0f);
    const float knobMinX = pos.x + paddingX + travel * anim.knobProgress;
    const ImVec2 knobMin(knobMinX, pos.y + (height - knobDiameter) * 0.5f);
    const ImVec2 knobMax(knobMin.x + knobDiameter, knobMin.y + knobDiameter);
    const ImVec2 knobCenter((knobMin.x + knobMax.x) * 0.5f, (knobMin.y + knobMax.y) * 0.5f);
    const float knobRadius = knobDiameter * 0.5f;
    const ImVec4 knobFill = mix_color(palette.surface,
                                      palette.surfaceRaised,
                                      hovered ? 0.54f : 0.42f);
    const ImVec4 knobBorder = ImVec4(palette.accent.x,
                                     palette.accent.y,
                                     palette.accent.z,
                                     hovered ? 0.52f : tokens.knobBorderAlpha);
    drawList->AddCircleFilled(ImVec2(knobCenter.x + 0.0f, knobCenter.y + 1.6f * scale),
                              knobRadius,
                              to_u32(ImVec4(0.0f, 0.0f, 0.0f, tokens.knobShadowAlpha)),
                              48);
    drawList->AddCircleFilled(knobCenter, knobRadius, to_u32(knobFill), 48);
    drawList->AddCircle(knobCenter, knobRadius, to_u32(knobBorder), 48, 1.25f * scale);
    drawList->AddCircle(knobCenter,
                        knobRadius - 3.0f * scale,
                        to_u32(ImVec4(palette.text.x, palette.text.y, palette.text.z, 0.035f)),
                        48,
                        1.0f * scale);

    const float transitionOrigin = clockwise ? 1.0f : 0.0f;
    const float transitionDistance = std::fabs(anim.knobProgress - transitionOrigin);
    const float spinDirection = clockwise ? -1.0f : 1.0f;
    const float arrowRotation = moving ? transitionDistance * kPi * 2.0f * spinDirection : 0.0f;
    const ImVec4 arrowColor = mix_color(palette.textMuted, palette.accent, hovered ? 0.42f : 0.30f);
    draw_rotation_direction_arrow(drawList,
                                  knobCenter,
                                  tokens.arrowRadius * scale,
                                  anim.visualClockwise,
                                  arrowRotation,
                                  to_u32(arrowColor),
                                  tokens.arrowStroke * scale,
                                  tokens.arrowHeadSize * scale);

    ImGui::PopID();

    if (style.helpText != nullptr && style.helpText[0] != '\0') {
        same_line_help_marker_aligned_to_last_item(style.helpText, theme);
        ImGui::EndGroup();
    }

    return pressed;
}

bool draw_option_toggle_button_with_help(const char* id,
                                          const char* label,
                                          bool& value,
                                         const char* helpText,
                                         float width,
                                         const Theme& theme) {
    OptionToggleStyle style;
    style.width = width;
    style.helpText = helpText;
    return draw_option_toggle_button(id, label, value, theme, style);
}

void draw_help_marker(const char* text, const Theme& theme) {
    const HelpMarkerWidgetTokens& helpTokens = theme.widgets.helpMarker;
    const float scale = current_scale();
    const float sizeValue = helpTokens.helpMarkerSize * scale;
    const ImVec2 size(sizeValue, sizeValue);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::PushID(static_cast<const void*>(text));
    ImGui::PushID(static_cast<int>(pos.x * 1000.0f));
    ImGui::PushID(static_cast<int>(pos.y * 1000.0f));
    const ImGuiID markerId = ImGui::GetID("##help");
    ImGui::InvisibleButton("##help", size);
    prepare_help_marker_overlay_queue();
    std::unordered_map<ImGuiID, HelpMarkerAnimationState>& animationCache = help_marker_animation_cache();
    HelpMarkerAnimationState& animation = animationCache[markerId];
    const bool itemVisible = ImGui::IsItemVisible();
    const bool active = ImGui::IsItemActive();
    const bool popupOpen = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
    const bool anotherItemActive = ImGui::IsAnyItemActive() && !active;
    const bool blockHover = popupOpen || anotherItemActive;
    const bool itemHovered = !blockHover && ImGui::IsItemHovered();
    const bool expandedHovered = !blockHover &&
        animation.hasRect &&
        ImGui::IsMouseHoveringRect(animation.lastMin, animation.lastMax, false);
    const bool hovered = itemHovered || expandedHovered;
    float& presence = animation.presence;
    const bool hasText = text != nullptr && text[0] != '\0';
    presence = follow_value(presence,
                            hovered && hasText ? 1.0f : 0.0f,
                            helpTokens.tooltipFollowSpeed,
                            EasingCurve::OutCubic);
    if (!itemVisible && presence <= 0.002f) {
        animation.hasRect = false;
        ImGui::PopID();
        ImGui::PopID();
        ImGui::PopID();
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImFont* font = ImGui::GetFont();
    const float glyphFontSize = ImGui::GetFontSize();
    const float fontSize = ImGui::GetFontSize() * 0.82f;

    if (!hasText || presence <= 0.002f) {
        draw_help_marker_collapsed_visual(drawList,
                                          font,
                                          glyphFontSize,
                                          pos,
                                          size,
                                          theme,
                                          hovered,
                                          active,
                                          scale);
        animation.lastMin = pos;
        animation.lastMax = ImVec2(pos.x + size.x, pos.y + size.y);
        animation.hasRect = itemVisible;
    } else {
        HelpMarkerOverlayCommand command;
        command.theme = theme;
        command.text = text;
        command.pos = pos;
        command.size = size;
        command.font = font;
        command.glyphFontSize = glyphFontSize;
        command.textFontSize = fontSize;
        command.scale = scale;
        command.presence = presence;
        command.hovered = hovered;
        command.active = active;
        const HelpMarkerOverlayLayout layout = make_help_marker_overlay_layout(command);
        animation.lastMin = layout.islandMin;
        animation.lastMax = layout.islandMax;
        animation.hasRect = true;
        help_marker_overlay_queue().push_back(std::move(command));
    }

    ImGui::PopID();
    ImGui::PopID();
    ImGui::PopID();
}

void same_line_help_marker_aligned_to_last_item(const char* text, const Theme& theme) {
    const float itemCenterY = (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f;
    const float scale = current_scale();
    ImGui::SameLine(0.0f, theme.widgets.helpMarker.helpInlineGap * scale);

    const ImVec2 cursor = ImGui::GetCursorScreenPos();
    const float markerSize = theme.widgets.helpMarker.helpMarkerSize * scale;
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, itemCenterY - markerSize * 0.5f));
    draw_help_marker(text, theme);
}

void flush_help_marker_overlays() {
    prepare_help_marker_overlay_queue();
    std::vector<HelpMarkerOverlayCommand>& queue = help_marker_overlay_queue();
    for (const HelpMarkerOverlayCommand& command : queue) {
        draw_help_marker_overlay_command(command);
    }
    queue.clear();
}

} // namespace imgui_onguoin

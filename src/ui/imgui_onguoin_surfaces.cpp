// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace imgui_onguoin {

namespace {

ImVec2 resolve_vec2(ImVec2 value, ImVec2 fallback) {
    return ImVec2(value.x >= 0.0f ? value.x : fallback.x,
                  value.y >= 0.0f ? value.y : fallback.y);
}

float resolve_metric(float value, float fallback) {
    return value >= 0.0f ? value : fallback;
}

float clamp_metric(float value, float minValue, float maxValue) {
    if (minValue >= 0.0f) {
        value = std::max(value, minValue);
    }
    if (maxValue >= 0.0f) {
        value = std::min(value, maxValue);
    }
    return value;
}

struct AutoPanelFrame {
    ImGuiID id = 0;
    float minY = 0.0f;
    float contentStartY = 0.0f;
    float bottomPadding = 0.0f;
    float minHeight = -1.0f;
    float maxHeight = -1.0f;
    float cachedHeight = 0.0f;
    bool hasCachedHeight = false;
    bool contentSubmitted = false;
    bool freezeHeightWhileParentScrolling = false;
};

struct AutoPanelBeginState {
    AutoPanelFrame frame{};
    float height = 0.0f;
};

std::unordered_map<ImGuiID, float>& auto_panel_height_cache() {
    static std::unordered_map<ImGuiID, float> cache;
    return cache;
}

std::vector<AutoPanelFrame>& auto_panel_stack() {
    static std::vector<AutoPanelFrame> stack;
    return stack;
}

AutoPanelBeginState prepare_auto_panel(const char* id,
                                       float fallbackHeight,
                                       float minHeight,
                                       float maxHeight,
                                       float bottomPadding,
                                       bool freezeHeightWhileParentScrolling) {
    const ImGuiID panelId = ImGui::GetID(id);
    const float resolvedFallbackHeight = resolve_metric(fallbackHeight, 240.0f);
    const auto cachedHeightIt = auto_panel_height_cache().find(panelId);
    const bool hasCachedHeight = cachedHeightIt != auto_panel_height_cache().end();
    const float measuredHeight = hasCachedHeight
        ? cachedHeightIt->second
        : resolvedFallbackHeight;

    AutoPanelBeginState state;
    state.height = clamp_metric(measuredHeight, minHeight, maxHeight);
    state.frame.id = panelId;
    state.frame.minY = ImGui::GetCursorScreenPos().y;
    state.frame.bottomPadding = bottomPadding * current_scale();
    state.frame.minHeight = minHeight;
    state.frame.maxHeight = maxHeight;
    state.frame.cachedHeight = measuredHeight;
    state.frame.hasCachedHeight = hasCachedHeight;
    state.frame.freezeHeightWhileParentScrolling = freezeHeightWhileParentScrolling;
    return state;
}

} // namespace

SurfaceStyle make_surface_style(const Theme& theme,
                                float rounding,
                                float borderBoost,
                                float glowAlpha) {
    SurfaceStyle style;
    style.rounding = rounding >= 0.0f ? rounding : theme.radii.surface;
    style.glowAlpha = glowAlpha;
    style.borderBoost = borderBoost;
    return style;
}

SurfacePanelStyle make_surface_panel_style(const Theme& theme,
                                           float rounding,
                                           float borderBoost,
                                           bool topHighlight,
                                           float topHighlightAlpha,
                                           float topHighlightHeight,
                                           ImVec2 padding,
                                           ImGuiWindowFlags childFlags) {
    SurfacePanelStyle style;
    style.surface = make_surface_style(theme, rounding, borderBoost);
    style.padding = padding;
    style.topHighlight = topHighlight;
    style.topHighlightAlpha = topHighlightAlpha;
    style.topHighlightHeight = topHighlightHeight;
    style.childFlags = childFlags;
    return style;
}

TitledPanelStyle make_titled_panel_style(const Theme& theme,
                                         float rounding,
                                         float borderBoost,
                                         ImVec2 padding,
                                         ImVec2 titleOffset,
                                         float titleSpacing,
                                         ImGuiWindowFlags childFlags) {
    TitledPanelStyle style;
    style.panel = make_surface_panel_style(theme,
                                           rounding,
                                           borderBoost,
                                           false,
                                           0.018f,
                                           0.34f,
                                           padding,
                                           childFlags);
    style.titleOffset = titleOffset;
    style.titleSpacing = titleSpacing;
    return style;
}

void draw_surface_card(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 max,
                       const Theme& theme,
                       SurfaceStyle style) {
    const Palette& palette = theme.palette;
    const SurfaceTokens& surfaces = theme.surfaces;
    const float rounding = style.rounding > 0.0f ? style.rounding : theme.radii.surface;
    const float glowAlpha = style.glowAlpha >= 0.0f ? style.glowAlpha : surfaces.cardGlowAlpha;
    const InteractionVisualState frameState{false, false, false, true};
    AccentFrameStyle frameStyle;
    frameStyle.rounding = rounding;
    frameStyle.color = palette.border;
    frameStyle.idleBorderAlpha = palette.border.w * style.borderBoost;
    frameStyle.hoveredBorderAlpha = frameStyle.idleBorderAlpha;
    frameStyle.activeBorderAlpha = frameStyle.idleBorderAlpha;
    frameStyle.selectedBorderAlpha = frameStyle.idleBorderAlpha;
    frameStyle.idleGlowAlpha = glowAlpha;
    frameStyle.hoveredGlowAlpha = glowAlpha;
    frameStyle.activeGlowAlpha = glowAlpha;
    frameStyle.selectedGlowAlpha = glowAlpha;
    frameStyle.idleThickness = 1.0f;
    frameStyle.activeThickness = 1.0f;
    frameStyle.selectedThickness = 1.0f;

    drawList->AddRectFilled(min, max, to_u32(palette.surface), rounding);
    draw_accent_frame(drawList, min, max, theme, frameState, frameStyle);
    drawList->AddRect(ImVec2(min.x + 1.0f, min.y + 1.0f),
                      ImVec2(max.x - 1.0f, max.y - 1.0f),
                      to_u32(ImVec4(1.0f, 1.0f, 1.0f, surfaces.innerHighlightAlpha * style.borderBoost)),
                      std::max(0.0f, rounding - 1.0f),
                      0,
                      1.0f);
}

bool begin_surface_panel(const char* id,
                         ImVec2 size,
                         const Theme& theme,
                         SurfacePanelStyle style) {
    style.padding = resolve_vec2(style.padding, theme.layout.surfacePanelPadding);
    const float scale = current_scale();
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max(min.x + size.x, min.y + size.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    draw_surface_card(drawList, min, max, theme, style.surface);

    if (style.topHighlight) {
        const float inset = 1.0f * scale;
        drawList->AddRectFilled(ImVec2(min.x + inset, min.y + inset),
                                ImVec2(max.x - inset, min.y + size.y * style.topHighlightHeight),
                                to_u32(ImVec4(1.0f, 1.0f, 1.0f, style.topHighlightAlpha)),
                                std::max(0.0f, style.surface.rounding - inset),
                                ImDrawFlags_RoundCornersTop);
    }

    ImGui::SetCursorScreenPos(ImVec2(min.x + style.padding.x * scale, min.y + style.padding.y * scale));
    const ImVec2 childSize(std::max(0.0f, size.x - style.padding.x * 2.0f * scale),
                           std::max(0.0f, size.y - style.padding.y * 2.0f * scale));
    return ImGui::BeginChild(id,
                             childSize,
                             style.childLayoutFlags,
                             style.childFlags | ImGuiWindowFlags_NoBackground);
}

void end_surface_panel() {
    ImGui::EndChild();
}

bool begin_auto_surface_panel(const char* id,
                              float width,
                              const Theme& theme,
                              AutoSurfacePanelStyle style) {
    style.panel.padding = resolve_vec2(style.panel.padding, theme.layout.surfacePanelPadding);
    style.bottomPadding = resolve_metric(style.bottomPadding, style.panel.padding.y);
    if (style.measureHeightWhenClipped) {
        style.panel.childLayoutFlags &= ~(ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);
        style.panel.childLayoutFlags |= ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize;
    }

    const float scale = current_scale();
    AutoPanelBeginState state = prepare_auto_panel(id,
                                                   style.fallbackHeight,
                                                   style.minHeight,
                                                   style.maxHeight,
                                                   style.bottomPadding,
                                                   style.freezeHeightWhileParentScrolling);
    const bool open = begin_surface_panel(id,
                                          ImVec2(width, state.height * scale),
                                          theme,
                                          style.panel);
    state.frame.contentStartY = ImGui::GetCursorScreenPos().y;
    state.frame.contentSubmitted = open;
    auto_panel_stack().push_back(state.frame);
    return open;
}

bool begin_auto_titled_panel(const char* id,
                             const char* title,
                             float width,
                             const Theme& theme,
                             AutoPanelStyle style) {
    style.titled.panel.padding = resolve_vec2(style.titled.panel.padding, theme.layout.titledPanelPadding);
    style.bottomPadding = resolve_metric(style.bottomPadding, style.titled.panel.padding.y);
    if (style.measureHeightWhenClipped) {
        style.titled.panel.childLayoutFlags &= ~(ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);
        style.titled.panel.childLayoutFlags |= ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize;
    }
    const float scale = current_scale();
    AutoPanelBeginState state = prepare_auto_panel(id,
                                                   style.fallbackHeight,
                                                   style.minHeight,
                                                   style.maxHeight,
                                                   style.bottomPadding,
                                                   style.freezeHeightWhileParentScrolling);
    const bool open = begin_titled_panel(id,
                                         title,
                                         ImVec2(width, state.height * scale),
                                         theme,
                                         style.titled);
    state.frame.contentStartY = ImGui::GetCursorScreenPos().y;
    state.frame.contentSubmitted = open;
    auto_panel_stack().push_back(state.frame);
    return open;
}

void end_auto_panel() {
    std::vector<AutoPanelFrame>& stack = auto_panel_stack();
    if (stack.empty()) {
        end_surface_panel();
        return;
    }

    const AutoPanelFrame frame = stack.back();
    stack.pop_back();

    const float contentBottomY = std::max(ImGui::GetCursorScreenPos().y,
                                          frame.contentStartY);
    const float measuredHeight = contentBottomY - frame.minY + frame.bottomPadding;
    const ImGuiIO& io = ImGui::GetIO();
    const float nextHeight = clamp_metric(measuredHeight / current_scale(),
                                          frame.minHeight,
                                          frame.maxHeight);
    const bool contentHeightChanged = std::fabs(nextHeight - frame.cachedHeight) > 8.0f;
    const bool freezeHeightNow = frame.hasCachedHeight &&
        !contentHeightChanged &&
        frame.freezeHeightWhileParentScrolling &&
        (ImGui::IsMouseDown(ImGuiMouseButton_Left) ||
         io.MouseWheel != 0.0f ||
         io.MouseWheelH != 0.0f);
    if (frame.contentSubmitted && !freezeHeightNow) {
        auto_panel_height_cache()[frame.id] = nextHeight;
    }
    end_surface_panel();
}

void draw_bottom_dock_frame(ImDrawList* drawList,
                            ImVec2 min,
                            ImVec2 max,
                            const Theme& theme,
                            float rounding) {
    const Palette& palette = theme.palette;
    const SurfaceTokens& surfaces = theme.surfaces;
    const float scale = current_scale();
    const float dockRounding = rounding >= 0.0f ? rounding : theme.radii.dock;
    const float pulse = imgui_onguoin::animate(theme.motion.dockPulse);

    for (int layer = 5; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 5.0f;
        const float off = (2.0f + 2.5f * t) * scale;
        const int alpha = static_cast<int>(18.0f * t * t);
        drawList->AddRectFilled(ImVec2(min.x + off, min.y + 5.0f * scale + off),
                                ImVec2(max.x - off, max.y + 6.0f * scale + off),
                                IM_COL32(0, 0, 0, alpha),
                                dockRounding + off * 0.35f);
    }

    for (int layer = 4; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 4.0f;
        const float off = (1.6f + 2.6f * t) * scale;
        const float alpha = (0.020f + 0.050f * (1.0f - t)) * pulse;
        drawList->AddRect(ImVec2(min.x - off, min.y - off),
                          ImVec2(max.x + off, max.y + off),
                          to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, alpha)),
                          dockRounding + off,
                          0,
                          1.0f * scale);
    }

    drawList->AddRectFilled(min, max, to_u32(surfaces.dockOuter), dockRounding);
    drawList->AddRectFilled(ImVec2(min.x + 1.0f * scale, min.y + 1.0f * scale),
                            ImVec2(max.x - 1.0f * scale, max.y - 1.0f * scale),
                            to_u32(surfaces.dockInner),
                            std::max(0.0f, dockRounding - 1.0f * scale));

    const float topY = min.y + 1.0f * scale;
    drawList->AddLine(ImVec2(min.x + 18.0f * scale, topY),
                      ImVec2(max.x - 18.0f * scale, topY),
                      to_u32(ImVec4(1.0f, 1.0f, 1.0f, 0.055f)),
                      1.0f * scale);

    const ImVec2 innerMin(min.x + 1.0f * scale, min.y + 1.0f * scale);
    const ImVec2 innerMax(max.x - 1.0f * scale, max.y - 1.0f * scale);
    drawList->AddRect(min,
                      max,
                      to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.105f + 0.085f * pulse)),
                      dockRounding,
                      0,
                      1.0f * scale);
    drawList->AddRect(innerMin,
                      innerMax,
                      to_u32(ImVec4(1.0f, 1.0f, 1.0f, 0.020f)),
                      std::max(0.0f, dockRounding - 1.0f * scale),
                      0,
                      1.0f * scale);

    drawList->AddLine(ImVec2(min.x + 22.0f * scale, max.y - 1.5f * scale),
                      ImVec2(max.x - 22.0f * scale, max.y - 1.5f * scale),
                      to_u32(ImVec4(0.0f, 0.0f, 0.0f, 0.34f)),
                      1.0f * scale);
}

} // namespace imgui_onguoin

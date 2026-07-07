// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

namespace imgui_onguoin {

namespace {

ImVec2 resolve_vec2(ImVec2 value, ImVec2 fallback) {
    return ImVec2(value.x >= 0.0f ? value.x : fallback.x,
                  value.y >= 0.0f ? value.y : fallback.y);
}

float resolve_metric(float value, float fallback) {
    return value >= 0.0f ? value : fallback;
}

} // namespace

bool begin_titled_panel(const char* id,
                        const char* title,
                        ImVec2 size,
                        const Theme& theme,
                        TitledPanelStyle style) {
    style.panel.padding = resolve_vec2(style.panel.padding, theme.layout.titledPanelPadding);
    style.titleOffset = resolve_vec2(style.titleOffset, theme.layout.titledPanelTitleOffset);
    style.titleSpacing = resolve_metric(style.titleSpacing, theme.layout.titledPanelTitleSpacing);
    const bool open = begin_surface_panel(id, size, theme, style.panel);
    const float scale = current_scale();
    ImGui::SetCursorPos(ImVec2(style.titleOffset.x * scale, style.titleOffset.y * scale));
    ImGui::TextDisabled("%s", title);
    ImGui::Dummy(ImVec2(0.0f, style.titleSpacing * scale));
    return open;
}

} // namespace imgui_onguoin

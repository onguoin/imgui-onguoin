// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_effects.h"

namespace imgui_onguoin {

SurfaceStyle make_surface_style(const Theme& theme,
                                float rounding = -1.0f,
                                float borderBoost = 1.0f,
                                float glowAlpha = -1.0f);
inline SurfaceStyle make_surface_style(float rounding = -1.0f,
                                       float borderBoost = 1.0f,
                                       float glowAlpha = -1.0f) {
    return make_surface_style(default_theme(), rounding, borderBoost, glowAlpha);
}
inline SurfaceStyle make_surface_style(const Palette& palette,
                                       float rounding = -1.0f,
                                       float borderBoost = 1.0f,
                                       float glowAlpha = -1.0f) {
    return make_surface_style(make_theme(palette), rounding, borderBoost, glowAlpha);
}

SurfacePanelStyle make_surface_panel_style(const Theme& theme,
                                           float rounding = -1.0f,
                                           float borderBoost = 1.0f,
                                           bool topHighlight = false,
                                           float topHighlightAlpha = 0.018f,
                                           float topHighlightHeight = 0.34f,
                                           ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                           ImGuiWindowFlags childFlags =
                                               ImGuiWindowFlags_NoScrollbar |
                                               ImGuiWindowFlags_NoScrollWithMouse);
inline SurfacePanelStyle make_surface_panel_style(float rounding = -1.0f,
                                                  float borderBoost = 1.0f,
                                                  bool topHighlight = false,
                                                  float topHighlightAlpha = 0.018f,
                                                  float topHighlightHeight = 0.34f,
                                                  ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                                  ImGuiWindowFlags childFlags =
                                                      ImGuiWindowFlags_NoScrollbar |
                                                      ImGuiWindowFlags_NoScrollWithMouse) {
    return make_surface_panel_style(default_theme(),
                                    rounding,
                                    borderBoost,
                                    topHighlight,
                                    topHighlightAlpha,
                                    topHighlightHeight,
                                    padding,
                                    childFlags);
}
inline SurfacePanelStyle make_surface_panel_style(const Palette& palette,
                                                  float rounding = -1.0f,
                                                  float borderBoost = 1.0f,
                                                  bool topHighlight = false,
                                                  float topHighlightAlpha = 0.018f,
                                                  float topHighlightHeight = 0.34f,
                                                  ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                                  ImGuiWindowFlags childFlags =
                                                      ImGuiWindowFlags_NoScrollbar |
                                                      ImGuiWindowFlags_NoScrollWithMouse) {
    return make_surface_panel_style(make_theme(palette),
                                    rounding,
                                    borderBoost,
                                    topHighlight,
                                    topHighlightAlpha,
                                    topHighlightHeight,
                                    padding,
                                    childFlags);
}

TitledPanelStyle make_titled_panel_style(const Theme& theme,
                                         float rounding = -1.0f,
                                         float borderBoost = 1.0f,
                                         ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                         ImVec2 titleOffset = ImVec2(-1.0f, -1.0f),
                                         float titleSpacing = -1.0f,
                                         ImGuiWindowFlags childFlags =
                                             ImGuiWindowFlags_NoScrollbar |
                                             ImGuiWindowFlags_NoScrollWithMouse);
inline TitledPanelStyle make_titled_panel_style(float rounding = -1.0f,
                                                float borderBoost = 1.0f,
                                                ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                                ImVec2 titleOffset = ImVec2(-1.0f, -1.0f),
                                                float titleSpacing = -1.0f,
                                                ImGuiWindowFlags childFlags =
                                                    ImGuiWindowFlags_NoScrollbar |
                                                    ImGuiWindowFlags_NoScrollWithMouse) {
    return make_titled_panel_style(default_theme(),
                                   rounding,
                                   borderBoost,
                                   padding,
                                   titleOffset,
                                   titleSpacing,
                                   childFlags);
}
inline TitledPanelStyle make_titled_panel_style(const Palette& palette,
                                                float rounding = -1.0f,
                                                float borderBoost = 1.0f,
                                                ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                                ImVec2 titleOffset = ImVec2(-1.0f, -1.0f),
                                                float titleSpacing = -1.0f,
                                                ImGuiWindowFlags childFlags =
                                                    ImGuiWindowFlags_NoScrollbar |
                                                    ImGuiWindowFlags_NoScrollWithMouse) {
    return make_titled_panel_style(make_theme(palette),
                                   rounding,
                                   borderBoost,
                                   padding,
                                   titleOffset,
                                   titleSpacing,
                                   childFlags);
}

void draw_surface_card(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 max,
                       const Theme& theme,
                       SurfaceStyle style = {});
inline void draw_surface_card(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const Theme& theme,
                              float rounding,
                              float borderBoost = 1.0f,
                              float glowAlpha = -1.0f) {
    draw_surface_card(drawList,
                      min,
                      max,
                      theme,
                      make_surface_style(theme, rounding, borderBoost, glowAlpha));
}
inline void draw_surface_card(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              SurfaceStyle style = {}) {
    draw_surface_card(drawList, min, max, default_theme(), style);
}
inline void draw_surface_card(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              float rounding,
                              float borderBoost = 1.0f,
                              float glowAlpha = -1.0f) {
    draw_surface_card(drawList,
                      min,
                      max,
                      default_theme(),
                      make_surface_style(default_theme(), rounding, borderBoost, glowAlpha));
}
inline void draw_surface_card(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const Palette& palette,
                              SurfaceStyle style = {}) {
    draw_surface_card(drawList, min, max, make_theme(palette), style);
}
inline void draw_surface_card(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const Palette& palette,
                              float rounding,
                              float borderBoost = 1.0f,
                              float glowAlpha = -1.0f) {
    const Theme theme = make_theme(palette);
    draw_surface_card(drawList,
                      min,
                      max,
                      theme,
                      make_surface_style(theme, rounding, borderBoost, glowAlpha));
}

bool begin_surface_panel(const char* id,
                         ImVec2 size,
                         const Theme& theme,
                         SurfacePanelStyle style = {});
inline bool begin_surface_panel(const char* id,
                                ImVec2 size,
                                const Theme& theme,
                                float rounding,
                                float borderBoost = 1.0f,
                                bool topHighlight = false,
                                float topHighlightAlpha = 0.018f,
                                float topHighlightHeight = 0.34f,
                                ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                ImGuiWindowFlags childFlags =
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoScrollWithMouse) {
    return begin_surface_panel(id,
                               size,
                               theme,
                               make_surface_panel_style(theme,
                                                        rounding,
                                                        borderBoost,
                                                        topHighlight,
                                                        topHighlightAlpha,
                                                        topHighlightHeight,
                                                        padding,
                                                        childFlags));
}
inline bool begin_surface_panel(const char* id,
                                ImVec2 size,
                                SurfacePanelStyle style = {}) {
    return begin_surface_panel(id, size, default_theme(), style);
}
inline bool begin_surface_panel(const char* id,
                                ImVec2 size,
                                float rounding,
                                float borderBoost = 1.0f,
                                bool topHighlight = false,
                                float topHighlightAlpha = 0.018f,
                                float topHighlightHeight = 0.34f,
                                ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                ImGuiWindowFlags childFlags =
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoScrollWithMouse) {
    return begin_surface_panel(id,
                               size,
                               default_theme(),
                               make_surface_panel_style(default_theme(),
                                                        rounding,
                                                        borderBoost,
                                                        topHighlight,
                                                        topHighlightAlpha,
                                                        topHighlightHeight,
                                                        padding,
                                                        childFlags));
}
inline bool begin_surface_panel(const char* id,
                                ImVec2 size,
                                const Palette& palette,
                                SurfacePanelStyle style = {}) {
    return begin_surface_panel(id, size, make_theme(palette), style);
}
inline bool begin_surface_panel(const char* id,
                                ImVec2 size,
                                const Palette& palette,
                                float rounding,
                                float borderBoost = 1.0f,
                                bool topHighlight = false,
                                float topHighlightAlpha = 0.018f,
                                float topHighlightHeight = 0.34f,
                                ImVec2 padding = ImVec2(-1.0f, -1.0f),
                                ImGuiWindowFlags childFlags =
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoScrollWithMouse) {
    const Theme theme = make_theme(palette);
    return begin_surface_panel(id,
                               size,
                               theme,
                               make_surface_panel_style(theme,
                                                        rounding,
                                                        borderBoost,
                                                        topHighlight,
                                                        topHighlightAlpha,
                                                        topHighlightHeight,
                                                        padding,
                                                        childFlags));
}

bool begin_titled_panel(const char* id,
                        const char* title,
                        ImVec2 size,
                        const Theme& theme,
                        TitledPanelStyle style = {});
inline bool begin_titled_panel(const char* id,
                               const char* title,
                               ImVec2 size,
                               const Theme& theme,
                               float rounding,
                               float borderBoost = 1.0f,
                               ImVec2 padding = ImVec2(-1.0f, -1.0f),
                               ImVec2 titleOffset = ImVec2(-1.0f, -1.0f),
                               float titleSpacing = -1.0f,
                               ImGuiWindowFlags childFlags =
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse) {
    return begin_titled_panel(id,
                              title,
                              size,
                              theme,
                              make_titled_panel_style(theme,
                                                      rounding,
                                                      borderBoost,
                                                      padding,
                                                      titleOffset,
                                                      titleSpacing,
                                                      childFlags));
}
inline bool begin_titled_panel(const char* id,
                               const char* title,
                               ImVec2 size,
                               TitledPanelStyle style = {}) {
    return begin_titled_panel(id, title, size, default_theme(), style);
}
inline bool begin_titled_panel(const char* id,
                               const char* title,
                               ImVec2 size,
                               float rounding,
                               float borderBoost = 1.0f,
                               ImVec2 padding = ImVec2(-1.0f, -1.0f),
                               ImVec2 titleOffset = ImVec2(-1.0f, -1.0f),
                               float titleSpacing = -1.0f,
                               ImGuiWindowFlags childFlags =
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse) {
    return begin_titled_panel(id,
                              title,
                              size,
                              default_theme(),
                              make_titled_panel_style(default_theme(),
                                                      rounding,
                                                      borderBoost,
                                                      padding,
                                                      titleOffset,
                                                      titleSpacing,
                                                      childFlags));
}
inline bool begin_titled_panel(const char* id,
                               const char* title,
                               ImVec2 size,
                               const Palette& palette,
                               TitledPanelStyle style = {}) {
    return begin_titled_panel(id, title, size, make_theme(palette), style);
}
inline bool begin_titled_panel(const char* id,
                               const char* title,
                               ImVec2 size,
                               const Palette& palette,
                               float rounding,
                               float borderBoost = 1.0f,
                               ImVec2 padding = ImVec2(-1.0f, -1.0f),
                               ImVec2 titleOffset = ImVec2(-1.0f, -1.0f),
                               float titleSpacing = -1.0f,
                               ImGuiWindowFlags childFlags =
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse) {
    const Theme theme = make_theme(palette);
    return begin_titled_panel(id,
                              title,
                              size,
                              theme,
                              make_titled_panel_style(theme,
                                                      rounding,
                                                      borderBoost,
                                                      padding,
                                                      titleOffset,
                                                      titleSpacing,
                                                      childFlags));
}

bool begin_auto_titled_panel(const char* id,
                             const char* title,
                             float width,
                             const Theme& theme,
                             AutoPanelStyle style = {});
bool begin_auto_surface_panel(const char* id,
                              float width,
                              const Theme& theme,
                              AutoSurfacePanelStyle style = {});
inline bool begin_auto_surface_panel(const char* id,
                                     float width,
                                     AutoSurfacePanelStyle style = {}) {
    return begin_auto_surface_panel(id, width, default_theme(), style);
}
inline bool begin_auto_surface_panel(const char* id,
                                     float width,
                                     const Palette& palette,
                                     AutoSurfacePanelStyle style = {}) {
    return begin_auto_surface_panel(id, width, make_theme(palette), style);
}
inline bool begin_auto_titled_panel(const char* id,
                                    const char* title,
                                    float width,
                                    AutoPanelStyle style = {}) {
    return begin_auto_titled_panel(id, title, width, default_theme(), style);
}
inline bool begin_auto_titled_panel(const char* id,
                                    const char* title,
                                    float width,
                                    const Palette& palette,
                                    AutoPanelStyle style = {}) {
    return begin_auto_titled_panel(id, title, width, make_theme(palette), style);
}
void end_auto_panel();

void end_surface_panel();
void draw_bottom_dock_frame(ImDrawList* drawList,
                            ImVec2 min,
                            ImVec2 max,
                            const Theme& theme,
                            float rounding = -1.0f);
inline void draw_bottom_dock_frame(ImDrawList* drawList,
                                   ImVec2 min,
                                   ImVec2 max,
                                   float rounding = -1.0f) {
    draw_bottom_dock_frame(drawList, min, max, default_theme(), rounding);
}
inline void draw_bottom_dock_frame(ImDrawList* drawList,
                                   ImVec2 min,
                                   ImVec2 max,
                                   const Palette& palette,
                                   float rounding = -1.0f) {
    draw_bottom_dock_frame(drawList, min, max, make_theme(palette), rounding);
}

} // namespace imgui_onguoin

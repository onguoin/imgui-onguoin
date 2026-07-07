// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_math.h"
#include "ui/imgui_onguoin_theme.h"

namespace imgui_onguoin {

void draw_glow_rect(ImDrawList* drawList,
                    ImVec2 min,
                    ImVec2 max,
                    float rounding,
                    const Theme& theme,
                    float alpha = 0.18f);
inline void draw_glow_rect(ImDrawList* drawList,
                           ImVec2 min,
                           ImVec2 max,
                           float rounding,
                           float alpha = 0.18f) {
    draw_glow_rect(drawList, min, max, rounding, default_theme(), alpha);
}
inline void draw_glow_rect(ImDrawList* drawList,
                           ImVec2 min,
                           ImVec2 max,
                           float rounding,
                           const Palette& palette,
                           float alpha = 0.18f) {
    draw_glow_rect(drawList, min, max, rounding, make_theme(palette), alpha);
}

void draw_flow_line(ImDrawList* drawList,
                    ImVec2 from,
                    ImVec2 to,
                    const Theme& theme,
                    float alpha = 0.14f);
inline void draw_flow_line(ImDrawList* drawList,
                           ImVec2 from,
                           ImVec2 to,
                           float alpha = 0.14f) {
    draw_flow_line(drawList, from, to, default_theme(), alpha);
}
inline void draw_flow_line(ImDrawList* drawList,
                           ImVec2 from,
                           ImVec2 to,
                           const Palette& palette,
                           float alpha = 0.14f) {
    draw_flow_line(drawList, from, to, make_theme(palette), alpha);
}

void draw_static_glow_rect(ImDrawList* drawList,
                           ImVec2 min,
                           ImVec2 max,
                           float rounding,
                           const Theme& theme,
                           float alpha = 0.18f,
                           float thickness = 1.0f);
inline void draw_static_glow_rect(ImDrawList* drawList,
                                  ImVec2 min,
                                  ImVec2 max,
                                  float rounding,
                                  float alpha = 0.18f,
                                  float thickness = 1.0f) {
    draw_static_glow_rect(drawList, min, max, rounding, default_theme(), alpha, thickness);
}
inline void draw_static_glow_rect(ImDrawList* drawList,
                                  ImVec2 min,
                                  ImVec2 max,
                                  float rounding,
                                  const Palette& palette,
                                  float alpha = 0.18f,
                                  float thickness = 1.0f) {
    draw_static_glow_rect(drawList, min, max, rounding, make_theme(palette), alpha, thickness);
}

void draw_neon_capsule_glow(ImDrawList* drawList,
                            ImVec2 min,
                            ImVec2 max,
                            float rounding,
                            NeonCapsuleGlowStyle style,
                            float hoverProgress,
                            float activationProgress);

void draw_top_highlight(ImDrawList* drawList,
                        ImVec2 min,
                        ImVec2 max,
                        float rounding,
                        float alpha,
                        float heightFraction = 0.45f);
void draw_text_block(ImDrawList* drawList,
                     ImVec2 min,
                     ImVec2 padding,
                     const char* text,
                     ImU32 color,
                     float wrapWidth = 0.0f);
void draw_accent_frame(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 max,
                       const Theme& theme,
                       const InteractionVisualState& state,
                       AccentFrameStyle style = {});
inline void draw_accent_frame(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const InteractionVisualState& state,
                              AccentFrameStyle style = {}) {
    draw_accent_frame(drawList, min, max, default_theme(), state, style);
}
inline void draw_accent_frame(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const Palette& palette,
                              const InteractionVisualState& state,
                              AccentFrameStyle style = {}) {
    draw_accent_frame(drawList, min, max, make_theme(palette), state, style);
}

void draw_ambient_glow(ImDrawList* drawList,
                       ImVec2 center,
                       float radius,
                       const Theme& theme,
                       float alphaScale = 1.0f);
inline void draw_ambient_glow(ImDrawList* drawList,
                              ImVec2 center,
                              float radius,
                              float alphaScale = 1.0f) {
    draw_ambient_glow(drawList, center, radius, default_theme(), alphaScale);
}
inline void draw_ambient_glow(ImDrawList* drawList,
                              ImVec2 center,
                              float radius,
                              const Palette& palette,
                              float alphaScale = 1.0f) {
    draw_ambient_glow(drawList, center, radius, make_theme(palette), alphaScale);
}

void draw_cursor_aura(ImDrawList* drawList,
                      ImVec2 center,
                      float radius,
                      const Theme& theme,
                      float alphaScale = 1.0f);
inline void draw_cursor_aura(ImDrawList* drawList,
                             ImVec2 center,
                             float radius,
                             float alphaScale = 1.0f) {
    draw_cursor_aura(drawList, center, radius, default_theme(), alphaScale);
}

void draw_circle_badge(ImDrawList* drawList,
                       ImVec2 center,
                       float radius,
                       ImU32 fillColor,
                       ImU32 borderColor,
                       int segments = 28,
                       float borderThickness = 1.0f);
ImVec2 point_on_circle(ImVec2 center, float radius, float visualDegrees);
void draw_arc(ImDrawList* drawList,
              ImVec2 center,
              float radius,
              float startDegrees,
              float endDegrees,
              ImU32 color,
              float thickness,
              int segments = 36);
void draw_wedge(ImDrawList* drawList,
                ImVec2 center,
                float innerRadius,
                float outerRadius,
                float startDegrees,
                float endDegrees,
                ImU32 color,
                int segments = 18);
void draw_capsule(ImDrawList* drawList,
                  ImVec2 min,
                  ImVec2 max,
                  const char* text,
                  const Theme& theme,
                  const InteractionVisualState& state,
                  CapsuleStyle style);
void draw_interactive_surface(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const char* text,
                              const Theme& theme,
                              const InteractionVisualState& state,
                              InteractiveSurfaceStyle style);
void draw_notice_surface(ImDrawList* drawList,
                         ImVec2 min,
                         ImVec2 max,
                         const char* text,
                         float wrapWidth,
                         NoticeSurfaceStyle style);
void draw_status_pill_surface(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const char* label,
                              const char* value,
                              StatusPillStyle style);
void draw_select_frame(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 max,
                       bool hovered,
                       bool open,
                       const Theme& theme,
                       float rounding);

} // namespace imgui_onguoin

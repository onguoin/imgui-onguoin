// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_types.h"

namespace imgui_onguoin {

ImU32 to_u32(ImVec4 value);
ImVec4 mix_color(ImVec4 a, ImVec4 b, float t);
float clamp01(float value);
float lerp(float a, float b, float t);
float move_toward(float current, float target, float maxStep);
float ease_out_cubic(float value);
float ease(EasingCurve curve, float value);
float follow_factor(float speed, float deltaTime, EasingCurve curve = EasingCurve::Linear);
inline float follow_factor(float speed, EasingCurve curve = EasingCurve::Linear) {
    return follow_factor(speed, ImGui::GetIO().DeltaTime, curve);
}
float follow_factor(const FollowMotionStyle& motion, float deltaTime);
inline float follow_factor(const FollowMotionStyle& motion) {
    return follow_factor(motion, ImGui::GetIO().DeltaTime);
}
float follow_value(float current,
                   float target,
                   float speed,
                   float deltaTime,
                   EasingCurve curve = EasingCurve::Linear);
inline float follow_value(float current,
                          float target,
                          float speed,
                          EasingCurve curve = EasingCurve::Linear) {
    return follow_value(current, target, speed, ImGui::GetIO().DeltaTime, curve);
}
float follow_value(float current,
                   float target,
                   const FollowMotionStyle& motion,
                   float deltaTime);
inline float follow_value(float current,
                          float target,
                          const FollowMotionStyle& motion) {
    return follow_value(current, target, motion, ImGui::GetIO().DeltaTime);
}
float advance_progress(float current,
                       float target,
                       const ProgressMotionStyle& motion,
                       float deltaTime);
inline float advance_progress(float current,
                              float target,
                              const ProgressMotionStyle& motion) {
    return advance_progress(current, target, motion, ImGui::GetIO().DeltaTime);
}
float advance_presence(float current,
                       bool visible,
                       const PresenceMotionStyle& motion,
                       float deltaTime);
inline float advance_presence(float current,
                              bool visible,
                              const PresenceMotionStyle& motion) {
    return advance_presence(current, visible, motion, ImGui::GetIO().DeltaTime);
}
float animate(const PulseMotionStyle& motion, float timeSeconds);
float animate(const PulseMotionStyle& motion);
float travel(const TravelMotionStyle& motion, float span, float timeSeconds);
float travel(const TravelMotionStyle& motion, float span);
float pulse(float speed, float minValue = 0.0f, float maxValue = 1.0f, float phase = 0.0f);
float current_scale();
float emphasized_alpha(const InteractionVisualState& state,
                       float idleAlpha,
                       float hoveredAlpha,
                       float activeAlpha,
                       float selectedAlpha = -1.0f);
float emphasized_thickness(const InteractionVisualState& state,
                           float idleThickness,
                           float activeThickness,
                           float selectedThickness = -1.0f);
TextBlockMetrics measure_text_block(const char* text, float wrapWidth = 0.0f);
ImVec2 centered_text_pos(ImVec2 min, ImVec2 max, ImVec2 textSize, ImVec2 offset = ImVec2(0.0f, 0.0f));
ImVec2 leading_text_pos(ImVec2 min,
                        ImVec2 max,
                        ImVec2 textSize,
                        float inset = 0.0f,
                        ImVec2 offset = ImVec2(0.0f, 0.0f));
ImVec2 trailing_text_pos(ImVec2 min,
                         ImVec2 max,
                         ImVec2 textSize,
                         float inset = 0.0f,
                         ImVec2 offset = ImVec2(0.0f, 0.0f));
float pill_width_for_text(const char* label,
                          const char* value,
                          float horizontalPadding,
                          float minimumWidth = 0.0f,
                          float gap = 0.0f);

} // namespace imgui_onguoin

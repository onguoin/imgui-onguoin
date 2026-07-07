// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#include <algorithm>
#include <cmath>

namespace imgui_onguoin {

ImU32 to_u32(ImVec4 value) {
    return ImGui::ColorConvertFloat4ToU32(value);
}

ImVec4 mix_color(ImVec4 a, ImVec4 b, float t) {
    t = clamp01(t);
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t);
}

float clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float lerp(float a, float b, float t) {
    return a + (b - a) * clamp01(t);
}

float move_toward(float current, float target, float maxStep) {
    const float step = std::max(0.0f, maxStep);
    if (current < target) {
        return std::min(current + step, target);
    }
    return std::max(current - step, target);
}

float ease(EasingCurve curve, float value) {
    value = clamp01(value);
    switch (curve) {
    case EasingCurve::Linear:
        return value;
    case EasingCurve::InSine:
        return 1.0f - std::cos((value * 3.1415926535f) * 0.5f);
    case EasingCurve::OutSine:
        return std::sin((value * 3.1415926535f) * 0.5f);
    case EasingCurve::InOutSine:
        return -(std::cos(3.1415926535f * value) - 1.0f) * 0.5f;
    case EasingCurve::OutCubic: {
        const float inv = 1.0f - value;
        return 1.0f - inv * inv * inv;
    }
    case EasingCurve::InOutCubic:
        return value < 0.5f
            ? 4.0f * value * value * value
            : 1.0f - std::pow(-2.0f * value + 2.0f, 3.0f) * 0.5f;
    }

    return value;
}

float ease_out_cubic(float value) {
    return ease(EasingCurve::OutCubic, value);
}

float follow_factor(float speed, float deltaTime, EasingCurve curve) {
    return ease(curve, clamp01(speed * deltaTime));
}

float follow_factor(const FollowMotionStyle& motion, float deltaTime) {
    return follow_factor(motion.speed, deltaTime, motion.curve);
}

float follow_value(float current,
                   float target,
                   float speed,
                   float deltaTime,
                   EasingCurve curve) {
    const float factor = follow_factor(speed, deltaTime, curve);
    return current + (target - current) * factor;
}

float follow_value(float current,
                   float target,
                   const FollowMotionStyle& motion,
                   float deltaTime) {
    const float next = follow_value(current, target, motion.speed, deltaTime, motion.curve);
    return std::fabs(target - next) <= motion.snapDistance ? target : next;
}

float advance_progress(float current,
                       float target,
                       const ProgressMotionStyle& motion,
                       float deltaTime) {
    const float next = move_toward(current,
                                   target,
                                   std::max(0.0f, motion.speed) * std::max(0.0f, deltaTime));
    return std::fabs(target - next) <= motion.snapDistance ? target : next;
}

float advance_presence(float current,
                       bool visible,
                       const PresenceMotionStyle& motion,
                       float deltaTime) {
    return advance_progress(current,
                            visible ? 1.0f : 0.0f,
                            visible ? motion.enter : motion.exit,
                            deltaTime);
}

float animate(const PulseMotionStyle& motion, float timeSeconds) {
    const float waveform = 0.5f + 0.5f * std::sin(timeSeconds * motion.speed + motion.phase);
    const float shaped = ease(motion.curve, waveform);
    return lerp(motion.minimumValue, motion.maximumValue, shaped);
}

float animate(const PulseMotionStyle& motion) {
    return animate(motion, static_cast<float>(ImGui::GetTime()));
}

float travel(const TravelMotionStyle& motion, float span, float timeSeconds) {
    if (span <= 0.0f) {
        return 0.0f;
    }
    return std::fmod(timeSeconds * motion.speed + motion.phase, span);
}

float travel(const TravelMotionStyle& motion, float span) {
    return travel(motion, span, static_cast<float>(ImGui::GetTime()));
}

float pulse(float speed, float minValue, float maxValue, float phase) {
    const float waveform = 0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * speed + phase);
    return lerp(minValue, maxValue, waveform);
}

float current_scale() {
    return std::max(0.60f, ImGui::GetIO().FontGlobalScale);
}

float emphasized_alpha(const InteractionVisualState& state,
                       float idleAlpha,
                       float hoveredAlpha,
                       float activeAlpha,
                       float selectedAlpha) {
    const float resolvedSelected = selectedAlpha >= 0.0f ? selectedAlpha : activeAlpha;
    if (!state.enabled) {
        return idleAlpha;
    }
    if (state.active) {
        return activeAlpha;
    }
    if (state.selected) {
        return resolvedSelected;
    }
    if (state.hovered) {
        return hoveredAlpha;
    }
    return idleAlpha;
}

float emphasized_thickness(const InteractionVisualState& state,
                           float idleThickness,
                           float activeThickness,
                           float selectedThickness) {
    const float resolvedSelected = selectedThickness >= 0.0f ? selectedThickness : activeThickness;
    if (!state.enabled) {
        return idleThickness;
    }
    if (state.active) {
        return activeThickness;
    }
    if (state.selected) {
        return resolvedSelected;
    }
    return idleThickness;
}

TextBlockMetrics measure_text_block(const char* text, float wrapWidth) {
    TextBlockMetrics metrics;
    metrics.wrapWidth = wrapWidth;
    metrics.size = ImGui::CalcTextSize(text != nullptr ? text : "",
                                       nullptr,
                                       false,
                                       wrapWidth > 0.0f ? wrapWidth : -1.0f);
    return metrics;
}

ImVec2 centered_text_pos(ImVec2 min, ImVec2 max, ImVec2 textSize, ImVec2 offset) {
    return ImVec2(min.x + ((max.x - min.x) - textSize.x) * 0.5f + offset.x,
                  min.y + ((max.y - min.y) - textSize.y) * 0.5f + offset.y);
}

ImVec2 leading_text_pos(ImVec2 min, ImVec2 max, ImVec2 textSize, float inset, ImVec2 offset) {
    return ImVec2(min.x + inset + offset.x,
                  min.y + ((max.y - min.y) - textSize.y) * 0.5f + offset.y);
}

ImVec2 trailing_text_pos(ImVec2 min, ImVec2 max, ImVec2 textSize, float inset, ImVec2 offset) {
    return ImVec2(max.x - inset - textSize.x + offset.x,
                  min.y + ((max.y - min.y) - textSize.y) * 0.5f + offset.y);
}

float pill_width_for_text(const char* label,
                          const char* value,
                          float horizontalPadding,
                          float minimumWidth,
                          float gap) {
    const ImVec2 labelSize = ImGui::CalcTextSize(label != nullptr ? label : "");
    const ImVec2 valueSize = ImGui::CalcTextSize(value != nullptr ? value : "");
    return std::max(minimumWidth, labelSize.x + valueSize.x + horizontalPadding * 2.0f + gap);
}

} // namespace imgui_onguoin

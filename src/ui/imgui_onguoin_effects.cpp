// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace imgui_onguoin {

namespace {

constexpr float kPi = 3.1415926535f;

ImVec4 resolve_neon_color(ImVec4 value, ImVec4 fallback) {
    return value.x >= 0.0f ? value : fallback;
}

ImVec4 neon_capsule_flow_color(const NeonCapsuleGlowStyle& style, float phase) {
    ImVec4 colorA = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    ImVec4 colorB = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    ImVec4 colorC = resolve_neon_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
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

void append_line_points(std::vector<ImVec2>& points, ImVec2 from, ImVec2 to, float step) {
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    const float length = std::sqrt(dx * dx + dy * dy);
    const int segments = std::max(1, static_cast<int>(std::ceil(length / std::max(1.0f, step))));
    if (points.empty()) {
        points.push_back(from);
    }
    for (int i = 1; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        points.push_back(ImVec2(lerp(from.x, to.x, t), lerp(from.y, to.y, t)));
    }
}

void append_arc_points(std::vector<ImVec2>& points,
                       ImVec2 center,
                       float radius,
                       float startRadians,
                       float endRadians,
                       int segments) {
    segments = std::max(2, segments);
    for (int i = 1; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float angle = lerp(startRadians, endRadians, t);
        points.push_back(ImVec2(center.x + std::cos(angle) * radius,
                                center.y + std::sin(angle) * radius));
    }
}

std::vector<ImVec2> make_neon_capsule_outline(ImVec2 min, ImVec2 max, float rounding, float scale) {
    std::vector<ImVec2> points;
    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);
    const float radius = std::clamp(rounding, 0.0f, std::min(width, height) * 0.5f);
    const float step = std::max(0.55f, 0.85f * scale);
    const int arcSegments = std::max(32, static_cast<int>(std::ceil(radius * 3.2f)));

    append_line_points(points, ImVec2(min.x + radius, min.y), ImVec2(max.x - radius, min.y), step);
    append_arc_points(points, ImVec2(max.x - radius, min.y + radius), radius, -kPi * 0.5f, 0.0f, arcSegments);
    append_line_points(points, ImVec2(max.x, min.y + radius), ImVec2(max.x, max.y - radius), step);
    append_arc_points(points, ImVec2(max.x - radius, max.y - radius), radius, 0.0f, kPi * 0.5f, arcSegments);
    append_line_points(points, ImVec2(max.x - radius, max.y), ImVec2(min.x + radius, max.y), step);
    append_arc_points(points, ImVec2(min.x + radius, max.y - radius), radius, kPi * 0.5f, kPi, arcSegments);
    append_line_points(points, ImVec2(min.x, max.y - radius), ImVec2(min.x, min.y + radius), step);
    append_arc_points(points, ImVec2(min.x + radius, min.y + radius), radius, kPi, kPi * 1.5f, arcSegments);
    if (!points.empty()) {
        points.push_back(points.front());
    }
    return points;
}

float circular_distance01(float a, float b) {
    float distance = std::fabs(a - b);
    return std::min(distance, 1.0f - distance);
}

float smooth_band_strength(float distance, float width) {
    const float local = clamp01(1.0f - distance / std::max(0.001f, width));
    return local * local * (3.0f - 2.0f * local);
}

float perimeter_band(float t, float center, float width) {
    return smooth_band_strength(circular_distance01(t, center), width);
}

float smoothstep01(float edge0, float edge1, float value) {
    const float t = clamp01((value - edge0) / std::max(0.001f, edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

ImVec4 neon_capsule_edge_color(const NeonCapsuleGlowStyle& style, float t, float phase) {
    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const ImVec4 warm = resolve_neon_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
    const float shimmer = 0.025f * std::sin((phase + t) * kPi * 2.0f);
    const float rightCyan = perimeter_band(t, 0.245f + shimmer, 0.235f);
    const float topCyan = perimeter_band(t, 0.045f + shimmer * 0.35f, 0.180f);
    const float leftViolet = perimeter_band(t, 0.755f - shimmer, 0.245f);
    const float bottomViolet = perimeter_band(t, 0.525f - shimmer * 0.45f, 0.250f);
    const float warmRim = perimeter_band(t, 0.430f + shimmer * 0.25f, 0.115f);

    const float cyanWeight = 0.22f + rightCyan * 1.30f + topCyan * 0.48f;
    const float violetWeight = 0.22f + leftViolet * 1.08f + bottomViolet * 0.92f;
    const float warmWeight = warmRim * 0.24f;
    const float total = std::max(0.001f, cyanWeight + violetWeight + warmWeight);
    return ImVec4((cyan.x * cyanWeight + violet.x * violetWeight + warm.x * warmWeight) / total,
                  (cyan.y * cyanWeight + violet.y * violetWeight + warm.y * warmWeight) / total,
                  (cyan.z * cyanWeight + violet.z * violetWeight + warm.z * warmWeight) / total,
                  1.0f);
}

ImVec4 neon_capsule_web_edge_color(const NeonCapsuleGlowStyle& style,
                                   ImVec2 point,
                                   ImVec2 min,
                                   ImVec2 max,
                                   float phase,
                                   bool coreLine) {
    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const ImVec4 warm = resolve_neon_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);
    const float x = clamp01((point.x - min.x) / width);
    const float y = clamp01((point.y - min.y) / height);
    const float shimmer = 0.035f * std::sin((phase + x * 0.31f + y * 0.19f) * kPi * 2.0f);
    const float cyanMix = smoothstep01(0.26f + shimmer, 0.86f + shimmer * 0.25f, x);
    ImVec4 color = mix_color(violet, cyan, cyanMix);

    const float bottomViolet = smoothstep01(0.42f, 1.0f, y) * (1.0f - smoothstep01(0.72f, 1.0f, x) * 0.32f);
    const float rightCyan = smoothstep01(0.68f, 1.0f, x);
    const float topCyan = 1.0f - smoothstep01(0.0f, 0.34f, y);
    const float topWarm = topCyan * (1.0f - smoothstep01(0.62f, 1.0f, x));
    color = mix_color(color, violet, 0.46f * bottomViolet);
    color = mix_color(color, cyan, 0.38f * rightCyan);
    color = mix_color(color, cyan, 0.48f * topCyan);
    color = mix_color(color, warm, 0.035f * topWarm);
    if (coreLine) {
        const float topLight = (1.0f - smoothstep01(0.0f, 0.28f, y)) * (0.35f + 0.65f * x);
        const float rightLight = smoothstep01(0.55f, 1.0f, x);
        const float bottomRightLight = bottomViolet * smoothstep01(0.58f, 1.0f, x);
        const float whiteMix = 0.055f + 0.38f * std::clamp(std::max(std::max(topLight, rightLight), bottomRightLight),
                                                          0.0f,
                                                          1.0f);
        color = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), whiteMix);
    }
    color.w = 1.0f;
    return color;
}

ImVec4 neon_capsule_hover_edge_color(const NeonCapsuleGlowStyle& style,
                                     ImVec2 point,
                                     ImVec2 min,
                                     ImVec2 max,
                                     float phase,
                                     bool coreLine) {
    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const ImVec4 warm = resolve_neon_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);
    const float x = clamp01((point.x - min.x) / width);
    const float y = clamp01((point.y - min.y) / height);
    const float shimmer = 0.030f * std::sin((phase + x * 0.24f - y * 0.18f) * kPi * 2.0f);
    const float violetMix = smoothstep01(0.60f + shimmer, 1.0f + shimmer * 0.28f, x);
    ImVec4 color = mix_color(cyan, violet, violetMix);

    const float topCyan = 1.0f - smoothstep01(0.0f, 0.36f, y);
    const float leftTeal = 1.0f - smoothstep01(0.0f, 0.28f, x);
    const float rightViolet = smoothstep01(0.62f, 1.0f, x);
    const float bottomViolet = smoothstep01(0.52f, 1.0f, y) * smoothstep01(0.28f, 1.0f, x);
    color = mix_color(color, cyan, 0.44f * topCyan + 0.24f * leftTeal);
    color = mix_color(color, violet, 0.30f * rightViolet * (1.0f - topCyan * 0.55f) + 0.36f * bottomViolet);
    color = mix_color(color, warm, 0.030f * bottomViolet);
    if (coreLine) {
        const float topLight = topCyan * (0.66f + 0.34f * (1.0f - x));
        const float rightLight = rightViolet * 0.70f;
        const float whiteMix = 0.035f + 0.145f * std::clamp(std::max(topLight, rightLight), 0.0f, 1.0f);
        color = mix_color(color, ImVec4(0.92f, 1.0f, 1.0f, 1.0f), whiteMix);
    }
    color.w = 1.0f;
    return color;
}

float capsule_inset_at_y(ImVec2 min, ImVec2 max, float rounding, float y) {
    const float radius = std::clamp(rounding, 0.0f, std::min(max.x - min.x, max.y - min.y) * 0.5f);
    if (radius <= 0.0f) {
        return 0.0f;
    }

    const float topCenter = min.y + radius;
    const float bottomCenter = max.y - radius;
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

float rounded_rect_signed_distance(ImVec2 point, ImVec2 min, ImVec2 max, float rounding) {
    const ImVec2 center((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
    const ImVec2 halfSize((max.x - min.x) * 0.5f, (max.y - min.y) * 0.5f);
    const float radius = std::clamp(rounding, 0.0f, std::min(halfSize.x, halfSize.y));
    const float qx = std::fabs(point.x - center.x) - (halfSize.x - radius);
    const float qy = std::fabs(point.y - center.y) - (halfSize.y - radius);
    const float outsideX = std::max(qx, 0.0f);
    const float outsideY = std::max(qy, 0.0f);
    const float outsideDistance = std::sqrt(outsideX * outsideX + outsideY * outsideY);
    const float insideDistance = std::min(std::max(qx, qy), 0.0f);
    return outsideDistance + insideDistance - radius;
}

void draw_neon_capsule_inner_volume(ImDrawList* drawList,
                                    ImVec2 min,
                                    ImVec2 max,
                                    float rounding,
                                    const NeonCapsuleGlowStyle& style,
                                    float presence,
                                    float phase,
                                    float scale) {
    if (presence <= 0.002f) {
        return;
    }

    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const float pulse = 0.5f + 0.5f * std::sin(phase * kPi * 2.0f);
    const float rowStep = std::max(1.0f, scale);
    const ImVec4 deepBlue(0.016f, 0.046f, 0.515f, 1.0f);

    const float inset = 1.25f * scale;
    const ImVec2 innerMin(min.x + inset, min.y + inset);
    const ImVec2 innerMax(max.x - inset, max.y - inset);
    const float innerWidth = std::max(1.0f, innerMax.x - innerMin.x);
    const float innerHeight = std::max(1.0f, innerMax.y - innerMin.y);
    const float innerRounding = std::max(0.0f, rounding - inset);
    drawList->AddRectFilled(innerMin,
                            innerMax,
                            to_u32(ImVec4(0.010f, 0.026f, 0.410f, 0.360f * presence)),
                            innerRounding);

    for (float y = innerMin.y; y < innerMax.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.35f * scale, innerMax.y);
        const float cy = (y + y2) * 0.5f;
        const float rowInset = capsule_inset_at_y(innerMin, innerMax, innerRounding, cy);
        const float rowMinX = innerMin.x + rowInset;
        const float rowMaxX = innerMax.x - rowInset;
        if (rowMaxX <= rowMinX) {
            continue;
        }

        const float yNorm = clamp01((cy - innerMin.y) / innerHeight);
        const float bottom = std::pow(clamp01((yNorm - 0.38f) / 0.62f), 1.75f);
        const float top = std::pow(clamp01((0.42f - yNorm) / 0.42f), 1.90f);
        const float verticalEdge = std::pow(clamp01(std::fabs(yNorm - 0.5f) * 2.0f), 2.10f);
        const float rowWidth = rowMaxX - rowMinX;
        const float leftEdgeWidth = rowWidth * 0.125f;
        const float rightEdgeWidth = rowWidth * 0.150f;
        const float leftEdgeX = std::min(rowMinX + leftEdgeWidth, rowMaxX);
        const float rightEdgeX = std::max(rowMaxX - rightEdgeWidth, rowMinX);

        ImVec4 leftColor = mix_color(violet, deepBlue, 0.70f);
        ImVec4 centerColor = deepBlue;
        ImVec4 rightColor = mix_color(cyan, deepBlue, 0.52f);
        leftColor.w = presence * (0.020f + 0.052f * bottom + 0.010f * top);
        centerColor.w = presence * (0.248f + 0.074f * verticalEdge);
        rightColor.w = presence * (0.036f + 0.088f * (0.82f + 0.18f * pulse) + 0.018f * top);
        drawList->AddRectFilledMultiColor(ImVec2(rowMinX, y),
                                          ImVec2(leftEdgeX, y2),
                                          to_u32(leftColor),
                                          to_u32(centerColor),
                                          to_u32(centerColor),
                                          to_u32(leftColor));
        if (rightEdgeX > leftEdgeX) {
            drawList->AddRectFilledMultiColor(ImVec2(leftEdgeX, y),
                                              ImVec2(rightEdgeX, y2),
                                              to_u32(centerColor),
                                              to_u32(centerColor),
                                              to_u32(centerColor),
                                              to_u32(centerColor));
        }
        drawList->AddRectFilledMultiColor(ImVec2(rightEdgeX, y),
                                          ImVec2(rowMaxX, y2),
                                          to_u32(centerColor),
                                          to_u32(rightColor),
                                          to_u32(rightColor),
                                          to_u32(centerColor));

        const float leftGlowWidth = rowWidth * 0.090f;
        const float rightGlowWidth = rowWidth * 0.145f;
        ImVec4 leftGlow = violet;
        leftGlow.w = presence * (0.018f + 0.046f * bottom);
        ImVec4 transparentLeft = leftGlow;
        transparentLeft.w = 0.0f;
        drawList->AddRectFilledMultiColor(ImVec2(rowMinX, y),
                                          ImVec2(std::min(rowMinX + leftGlowWidth, rowMaxX), y2),
                                          to_u32(leftGlow),
                                          to_u32(transparentLeft),
                                          to_u32(transparentLeft),
                                          to_u32(leftGlow));

        ImVec4 rightGlow = cyan;
        rightGlow.w = presence * (0.026f + 0.064f * (0.84f + 0.18f * pulse));
        ImVec4 transparentRight = rightGlow;
        transparentRight.w = 0.0f;
        drawList->AddRectFilledMultiColor(ImVec2(std::max(rowMaxX - rightGlowWidth, rowMinX), y),
                                          ImVec2(rowMaxX, y2),
                                          to_u32(transparentRight),
                                          to_u32(rightGlow),
                                          to_u32(rightGlow),
                                          to_u32(transparentRight));
    }
}

void draw_neon_capsule_core_depth(ImDrawList* drawList,
                                  ImVec2 min,
                                  ImVec2 max,
                                  float rounding,
                                  float presence,
                                  float scale) {
    if (presence <= 0.002f) {
        return;
    }

    const float insetX = std::max(2.0f * scale, (max.y - min.y) * 0.18f);
    const float insetY = std::max(2.0f * scale, (max.y - min.y) * 0.18f);
    if (max.x - min.x <= insetX * 2.0f || max.y - min.y <= insetY * 2.0f) {
        return;
    }

    const ImVec2 innerMin(min.x + insetX, min.y + insetY);
    const ImVec2 innerMax(max.x - insetX, max.y - insetY);
    const float innerRounding = std::max(0.0f, rounding - insetY);
    const float alpha = std::clamp(0.068f * presence, 0.0f, 0.10f);
    drawList->AddRectFilled(innerMin,
                            innerMax,
                            to_u32(ImVec4(0.006f, 0.014f, 0.070f, alpha)),
                            innerRounding);
}

void draw_neon_capsule_center_volume(ImDrawList* drawList,
                                     ImVec2 min,
                                     ImVec2 max,
                                     float rounding,
                                     const NeonCapsuleGlowStyle& style,
                                     float presence,
                                     float phase,
                                     float scale) {
    if (presence <= 0.002f) {
        return;
    }

    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const float height = std::max(1.0f, max.y - min.y);
    const float insetX = std::max(2.0f * scale, height * 0.26f);
    const float insetY = std::max(2.0f * scale, height * 0.30f);
    if (max.x - min.x <= insetX * 2.0f || max.y - min.y <= insetY * 2.0f) {
        return;
    }

    const ImVec2 innerMin(min.x + insetX, min.y + insetY);
    const ImVec2 innerMax(max.x - insetX, max.y - insetY);
    const float innerRounding = std::max(0.0f, rounding - insetY);
    const float pulse = 0.5f + 0.5f * std::sin((phase + 0.18f) * kPi * 2.0f);
    ImVec4 left(0.010f, 0.020f, 0.230f, 0.0f);
    ImVec4 center(0.025f, 0.052f, 0.365f, presence * (0.052f + 0.022f * pulse));
    ImVec4 right = mix_color(cyan, ImVec4(0.020f, 0.045f, 0.360f, 1.0f), 0.62f);
    right.w = presence * (0.066f + 0.026f * pulse);

    const float split = lerp(innerMin.x, innerMax.x, 0.62f);
    drawList->AddRectFilledMultiColor(innerMin,
                                      ImVec2(split, innerMax.y),
                                      to_u32(left),
                                      to_u32(center),
                                      to_u32(center),
                                      to_u32(left));
    drawList->AddRectFilledMultiColor(ImVec2(split, innerMin.y),
                                      innerMax,
                                      to_u32(center),
                                      to_u32(right),
                                      to_u32(right),
                                      to_u32(center));
}

void draw_neon_capsule_inner_rim(ImDrawList* drawList,
                                 ImVec2 min,
                                 ImVec2 max,
                                 float rounding,
                                 const NeonCapsuleGlowStyle& style,
                                 float presence,
                                 float phase,
                                 float scale) {
    if (presence <= 0.002f) {
        return;
    }

    const float height = std::max(1.0f, max.y - min.y);
    const float rimWidth = std::min(height * 0.145f, 4.6f * scale);
    const float rowStep = std::max(0.75f, 0.80f * scale);
    if (rimWidth <= 0.25f) {
        return;
    }

    for (float y = min.y; y < max.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.18f * scale, max.y);
        const float cy = (y + y2) * 0.5f;
        const float rowInset = capsule_inset_at_y(min, max, rounding, cy);
        const float rowLeft = min.x + rowInset;
        const float rowRight = max.x - rowInset;
        if (rowRight <= rowLeft) {
            continue;
        }

        const float yNorm = clamp01((cy - min.y) / height);
        const float bottomBoost = smoothstep01(0.42f, 1.0f, yNorm);
        const float topBoost = 1.0f - smoothstep01(0.0f, 0.42f, yNorm);
        const float leftWidth = std::min(rimWidth * (0.72f + 0.22f * bottomBoost), (rowRight - rowLeft) * 0.46f);
        const float rightWidth = std::min(rimWidth * (0.92f + 0.18f * topBoost), (rowRight - rowLeft) * 0.50f);

        if (leftWidth > 0.3f) {
            ImVec4 edge = neon_capsule_web_edge_color(style, ImVec2(rowLeft, cy), min, max, phase, true);
            edge.w = std::clamp(presence * (0.150f + 0.105f * bottomBoost), 0.0f, 0.34f);
            ImVec4 transparent = edge;
            transparent.w = 0.0f;
            drawList->AddRectFilledMultiColor(ImVec2(rowLeft, y),
                                              ImVec2(rowLeft + leftWidth, y2),
                                              to_u32(edge),
                                              to_u32(transparent),
                                              to_u32(transparent),
                                              to_u32(edge));
        }

        if (rightWidth > 0.3f) {
            ImVec4 edge = neon_capsule_web_edge_color(style, ImVec2(rowRight, cy), min, max, phase, true);
            edge.w = std::clamp(presence * (0.210f + 0.145f * topBoost), 0.0f, 0.44f);
            ImVec4 transparent = edge;
            transparent.w = 0.0f;
            drawList->AddRectFilledMultiColor(ImVec2(rowRight - rightWidth, y),
                                              ImVec2(rowRight, y2),
                                              to_u32(transparent),
                                              to_u32(edge),
                                              to_u32(edge),
                                              to_u32(transparent));
        }
    }
}

void draw_neon_capsule_horizontal_rim(ImDrawList* drawList,
                                      ImVec2 min,
                                      ImVec2 max,
                                      float rounding,
                                      const NeonCapsuleGlowStyle& style,
                                      float presence,
                                      float phase,
                                      float scale) {
    if (presence <= 0.002f) {
        return;
    }

    const float height = std::max(1.0f, max.y - min.y);
    const float bandHeight = std::min(height * 0.18f, 5.2f * scale);
    const float rowStep = std::max(0.75f, 0.80f * scale);
    if (bandHeight <= 0.25f) {
        return;
    }

    auto drawBand = [&](float bandMinY, float bandMaxY, bool bottom) {
        for (float y = bandMinY; y < bandMaxY; y += rowStep) {
            const float y2 = std::min(y + rowStep + 0.18f * scale, bandMaxY);
            const float cy = (y + y2) * 0.5f;
            const float local = bottom
                ? clamp01((cy - bandMinY) / std::max(0.001f, bandMaxY - bandMinY))
                : clamp01((bandMaxY - cy) / std::max(0.001f, bandMaxY - bandMinY));
            const float alphaShape = local * local * (3.0f - 2.0f * local);
            if (alphaShape <= 0.004f) {
                continue;
            }

            const float rowInset = capsule_inset_at_y(min, max, rounding, cy);
            const float rowLeft = min.x + rowInset;
            const float rowRight = max.x - rowInset;
            if (rowRight <= rowLeft) {
                continue;
            }

            ImVec4 left = neon_capsule_web_edge_color(style, ImVec2(rowLeft, cy), min, max, phase, true);
            ImVec4 right = neon_capsule_web_edge_color(style, ImVec2(rowRight, cy), min, max, phase, true);
            ImVec4 center = mix_color(left, right, 0.58f);
            center = mix_color(center, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), bottom ? 0.10f : 0.13f);

            const float alpha = presence * alphaShape * (bottom ? 0.42f : 0.38f);
            left.w = std::clamp(alpha * (bottom ? 1.04f : 0.82f), 0.0f, 0.52f);
            center.w = std::clamp(alpha * (bottom ? 0.78f : 0.74f), 0.0f, 0.46f);
            right.w = std::clamp(alpha * (bottom ? 0.82f : 1.08f), 0.0f, 0.58f);

            const float width = rowRight - rowLeft;
            const float splitA = rowLeft + width * 0.42f;
            const float splitB = rowLeft + width * 0.68f;
            drawList->AddRectFilledMultiColor(ImVec2(rowLeft, y),
                                              ImVec2(splitA, y2),
                                              to_u32(left),
                                              to_u32(center),
                                              to_u32(center),
                                              to_u32(left));
            drawList->AddRectFilledMultiColor(ImVec2(splitA, y),
                                              ImVec2(splitB, y2),
                                              to_u32(center),
                                              to_u32(center),
                                              to_u32(center),
                                              to_u32(center));
            drawList->AddRectFilledMultiColor(ImVec2(splitB, y),
                                              ImVec2(rowRight, y2),
                                              to_u32(center),
                                              to_u32(right),
                                              to_u32(right),
                                              to_u32(center));
        }
    };

    drawBand(min.y, std::min(min.y + bandHeight, max.y), false);
    drawBand(std::max(min.y, max.y - bandHeight), max.y, true);
}

void draw_neon_capsule_sdf_glow(ImDrawList* drawList,
                                ImVec2 min,
                                ImVec2 max,
                                float rounding,
                                const NeonCapsuleGlowStyle& style,
                                float activePresence,
                                float hoverPresence,
                                float phase,
                                float glowScale,
                                float scale) {
    const float presence = std::clamp(activePresence + hoverPresence * 0.48f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const float height = std::max(1.0f, max.y - min.y);
    const float outerRange = std::min(height * 0.26f, 6.8f * scale) * glowScale;
    const float innerRange = std::min(height * 0.24f, 6.4f * scale);
    if (outerRange <= 0.25f || innerRange <= 0.25f) {
        return;
    }

    const ImVec2 fieldMin(min.x - outerRange, min.y - outerRange);
    const ImVec2 fieldMax(max.x + outerRange, max.y + outerRange);
    const float rowStep = std::max(0.46f, 0.52f * scale);
    const float colStep = std::max(0.46f, 0.56f * scale);

    for (float y = fieldMin.y; y < fieldMax.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.18f * scale, fieldMax.y);
        for (float x = fieldMin.x; x < fieldMax.x; x += colStep) {
            const float x2 = std::min(x + colStep + 0.18f * scale, fieldMax.x);
            const ImVec2 sample((x + x2) * 0.5f, (y + y2) * 0.5f);
            const float distance = rounded_rect_signed_distance(sample, min, max, rounding);
            const bool outside = distance >= 0.0f;
            const float range = outside ? outerRange : innerRange;
            const float normalized = std::fabs(distance) / range;
            if (normalized >= 1.0f) {
                continue;
            }

            const float falloff = outside
                ? std::pow(1.0f - normalized, 2.35f)
                : std::pow(1.0f - normalized, 2.05f);
            if (falloff <= 0.003f) {
                continue;
            }

            const float xNorm = clamp01((sample.x - min.x) / std::max(1.0f, max.x - min.x));
            const float yNorm = clamp01((sample.y - min.y) / std::max(1.0f, max.y - min.y));
            const float rightBoost = smoothstep01(0.66f, 1.0f, xNorm);
            const float leftBoost = 1.0f - smoothstep01(0.0f, 0.24f, xNorm);
            const float bottomBoost = smoothstep01(0.52f, 1.0f, yNorm);
            float alpha = presence * falloff;
            if (outside) {
                alpha *= 0.070f + 0.250f * rightBoost + 0.092f * leftBoost + 0.105f * bottomBoost;
            } else {
                alpha *= 0.024f + 0.086f * rightBoost + 0.045f * leftBoost + 0.042f * bottomBoost;
            }

            if (hoverPresence > activePresence) {
                alpha *= outside ? 0.68f : 0.30f;
            }
            if (alpha <= 0.002f) {
                continue;
            }

            const bool hoverPalette = hoverPresence > activePresence;
            ImVec4 color = hoverPalette
                ? neon_capsule_hover_edge_color(style, sample, min, max, phase, false)
                : neon_capsule_web_edge_color(style, sample, min, max, phase, false);
            color.w = std::clamp(alpha, 0.0f, outside ? 0.30f : 0.14f);
            drawList->AddRectFilled(ImVec2(x, y), ImVec2(x2, y2), to_u32(color));
        }
    }
}

void draw_neon_capsule_row_blur(ImDrawList* drawList,
                                ImVec2 min,
                                ImVec2 max,
                                float rounding,
                                const NeonCapsuleGlowStyle& style,
                                float presence,
                                float phase,
                                float glowScale,
                                float scale,
                                bool hoverPalette) {
    if (presence <= 0.002f) {
        return;
    }

    const float height = std::max(1.0f, max.y - min.y);
    const float maxExpand = std::min(height * 0.20f, 5.4f * scale) * glowScale;
    const float rowStep = std::max(0.8f, 0.82f * scale);
    constexpr int kBands = 5;
    for (int band = kBands; band >= 1; --band) {
        const float t0 = static_cast<float>(band - 1) / static_cast<float>(kBands);
        const float t1 = static_cast<float>(band) / static_cast<float>(kBands);
        const float innerExpand = maxExpand * t0;
        const float outerExpand = maxExpand * t1;
        const ImVec2 innerMin(min.x - innerExpand, min.y - innerExpand);
        const ImVec2 innerMax(max.x + innerExpand, max.y + innerExpand);
        const ImVec2 outerMin(min.x - outerExpand, min.y - outerExpand);
        const ImVec2 outerMax(max.x + outerExpand, max.y + outerExpand);
        const float innerRounding = rounding + innerExpand;
        const float outerRounding = rounding + outerExpand;
        const float falloff = std::pow(1.0f - t0, 2.15f);
        if (falloff <= 0.004f) {
            continue;
        }

        for (float y = outerMin.y; y < outerMax.y; y += rowStep) {
            const float y2 = std::min(y + rowStep + 0.18f * scale, outerMax.y);
            const float cy = (y + y2) * 0.5f;
            const float outerInset = capsule_inset_at_y(outerMin, outerMax, outerRounding, cy);
            const float outerLeft = outerMin.x + outerInset;
            const float outerRight = outerMax.x - outerInset;
            if (outerRight <= outerLeft) {
                continue;
            }

            const bool intersectsInner = cy >= innerMin.y && cy <= innerMax.y;
            float innerLeft = outerRight;
            float innerRight = outerLeft;
            if (intersectsInner) {
                const float innerInset = capsule_inset_at_y(innerMin, innerMax, innerRounding, cy);
                innerLeft = innerMin.x + innerInset;
                innerRight = innerMax.x - innerInset;
            }

            const float yNorm = clamp01((cy - min.y) / height);
            const float bottomBoost = smoothstep01(0.42f, 1.0f, yNorm);
            if (intersectsInner && innerLeft > outerLeft + 0.2f) {
                ImVec4 color = hoverPalette
                    ? neon_capsule_hover_edge_color(style, ImVec2(min.x, cy), min, max, phase, false)
                    : neon_capsule_web_edge_color(style, ImVec2(min.x, cy), min, max, phase, false);
                color.w = std::clamp(presence * falloff * (0.050f + 0.060f * bottomBoost), 0.0f, 0.18f);
                ImVec4 transparent = color;
                transparent.w = 0.0f;
                drawList->AddRectFilledMultiColor(ImVec2(outerLeft, y),
                                                  ImVec2(innerLeft, y2),
                                                  to_u32(transparent),
                                                  to_u32(color),
                                                  to_u32(color),
                                                  to_u32(transparent));
            }

            if (intersectsInner && outerRight > innerRight + 0.2f) {
                ImVec4 color = hoverPalette
                    ? neon_capsule_hover_edge_color(style, ImVec2(max.x, cy), min, max, phase, false)
                    : neon_capsule_web_edge_color(style, ImVec2(max.x, cy), min, max, phase, false);
                color.w = std::clamp(presence * falloff * (0.122f + 0.145f * (1.0f - bottomBoost * 0.18f)), 0.0f, 0.34f);
                ImVec4 transparent = color;
                transparent.w = 0.0f;
                drawList->AddRectFilledMultiColor(ImVec2(innerRight, y),
                                                  ImVec2(outerRight, y2),
                                                  to_u32(color),
                                                  to_u32(transparent),
                                                  to_u32(transparent),
                                                  to_u32(color));
            }

            if (!intersectsInner) {
                const float sampleY = cy < min.y ? min.y : max.y;
                const float topBias = cy < min.y ? 0.34f : 1.0f;
                ImVec4 left = hoverPalette
                    ? neon_capsule_hover_edge_color(style,
                                                   ImVec2(min.x + (max.x - min.x) * 0.20f, sampleY),
                                                   min,
                                                   max,
                                                   phase,
                                                   false)
                    : neon_capsule_web_edge_color(style,
                                                  ImVec2(min.x + (max.x - min.x) * 0.20f, sampleY),
                                                  min,
                                                  max,
                                                  phase,
                                                  false);
                ImVec4 right = hoverPalette
                    ? neon_capsule_hover_edge_color(style,
                                                   ImVec2(min.x + (max.x - min.x) * 0.84f, sampleY),
                                                   min,
                                                   max,
                                                   phase,
                                                   false)
                    : neon_capsule_web_edge_color(style,
                                                  ImVec2(min.x + (max.x - min.x) * 0.84f, sampleY),
                                                  min,
                                                  max,
                                                  phase,
                                                  false);
                left.w = std::clamp(presence * falloff * 0.052f * topBias, 0.0f, 0.13f);
                right.w = std::clamp(presence * falloff * 0.112f * topBias, 0.0f, 0.22f);
                drawList->AddRectFilledMultiColor(ImVec2(outerLeft, y),
                                                  ImVec2(outerRight, y2),
                                                  to_u32(left),
                                                  to_u32(right),
                                                  to_u32(right),
                                                  to_u32(left));
            }
        }
    }
}

void draw_neon_capsule_edge_bloom(ImDrawList* drawList,
                                  const std::vector<ImVec2>& outline,
                                  ImVec2 min,
                                  ImVec2 max,
                                  const NeonCapsuleGlowStyle& style,
                                  float active,
                                  float hover,
                                  float travel,
                                  float glowScale,
                                  float scale) {
    const int segmentCount = static_cast<int>(outline.size()) - 1;
    if (segmentCount <= 0) {
        return;
    }

    const float presence = std::clamp(active + hover * 0.45f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    for (int pass = 0; pass < 3; ++pass) {
        const float thickness = (pass == 0 ? 4.8f : (pass == 1 ? 3.0f : 1.85f)) * scale * glowScale;
        const float alphaBase = presence * (pass == 0 ? 0.072f : (pass == 1 ? 0.245f : 0.920f));
        if (alphaBase <= 0.002f) {
            continue;
        }

        for (int i = 0; i < segmentCount; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(std::max(1, segmentCount));
            const ImVec2 mid((outline[i].x + outline[i + 1].x) * 0.5f,
                             (outline[i].y + outline[i + 1].y) * 0.5f);
            const float x = clamp01((mid.x - min.x) / std::max(1.0f, max.x - min.x));
            const float y = clamp01((mid.y - min.y) / std::max(1.0f, max.y - min.y));
            const float rightCyan = smoothstep01(0.64f, 1.0f, x);
            const float leftViolet = 1.0f - smoothstep01(0.0f, 0.34f, x);
            const float bottomViolet = smoothstep01(0.42f, 1.0f, y);
            const float topCyan = (1.0f - smoothstep01(0.0f, 0.36f, y)) * smoothstep01(0.42f, 1.0f, x);
            const float sideStrength = std::clamp(0.58f + rightCyan * 0.38f + leftViolet * 0.24f +
                                                      bottomViolet * 0.36f + topCyan * 0.18f,
                                                  0.0f,
                                                  1.12f);
            if (sideStrength <= 0.004f) {
                continue;
            }

            ImVec4 color = neon_capsule_web_edge_color(style, mid, min, max, travel + t * 0.12f, pass == 2);
            if (pass == 2) {
                color = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.18f);
            }
            color.w = std::clamp(alphaBase * sideStrength, 0.0f, pass == 2 ? 0.96f : 0.44f);
            drawList->AddLine(outline[i], outline[i + 1], to_u32(color), thickness);
        }
    }
}

void draw_neon_capsule_ambient_aura(ImDrawList* drawList,
                                    ImVec2 min,
                                    ImVec2 max,
                                    float rounding,
                                    const NeonCapsuleGlowStyle& style,
                                    float active,
                                    float hover,
                                    float phase,
                                    float glowScale,
                                    float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence * 0.50f + hoverPresence * 0.92f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const float height = std::max(1.0f, max.y - min.y);
    const float width = std::max(1.0f, max.x - min.x);
    const bool hoverPalette = hoverPresence > activePresence;
    const float maxExpand = std::min(height * (hoverPalette ? 0.36f : 0.20f), (hoverPalette ? 10.8f : 5.6f) * scale) *
                            std::max(0.35f, glowScale);
    if (maxExpand <= 0.25f) {
        return;
    }

    const float rowStep = std::max(0.70f, 0.76f * scale);
    constexpr int kLayers = 5;
    for (int layer = kLayers; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / static_cast<float>(kLayers);
        const float expand = maxExpand * t;
        const float feather = 1.0f - t;
        const ImVec2 auraMin(min.x - expand, min.y - expand);
        const ImVec2 auraMax(max.x + expand, max.y + expand);
        const float auraRounding = rounding + expand;
            const float layerAlpha = presence * (hoverPalette ? 0.022f : 0.022f) * (0.35f + feather * feather * 1.75f);

        for (float y = auraMin.y; y < auraMax.y; y += rowStep) {
            const float y2 = std::min(y + rowStep + 0.18f * scale, auraMax.y);
            const float cy = (y + y2) * 0.5f;
            const float inset = capsule_inset_at_y(auraMin, auraMax, auraRounding, cy);
            const float leftX = auraMin.x + inset;
            const float rightX = auraMax.x - inset;
            if (rightX <= leftX + 0.5f) {
                continue;
            }

            const float yNorm = clamp01((cy - min.y) / height);
            const float topBias = 1.0f - smoothstep01(0.0f, 0.38f, yNorm);
            const float bottomBias = smoothstep01(0.52f, 1.0f, yNorm);
            const float verticalSoft = hoverPalette
                ? std::clamp(0.72f + topBias * 0.02f + bottomBias * 0.82f, 0.0f, 1.42f)
                : std::clamp(0.72f + topBias * 0.32f + bottomBias * 0.18f, 0.0f, 1.18f);
            const float leftSampleX = min.x + width * 0.18f;
            const float rightSampleX = min.x + width * 0.84f;
            ImVec4 left = hoverPalette
                ? neon_capsule_hover_edge_color(style, ImVec2(leftSampleX, cy), min, max, phase + t * 0.04f, false)
                : neon_capsule_web_edge_color(style, ImVec2(leftSampleX, cy), min, max, phase + t * 0.04f, false);
            ImVec4 right = hoverPalette
                ? neon_capsule_hover_edge_color(style, ImVec2(rightSampleX, cy), min, max, phase + t * 0.04f, false)
                : neon_capsule_web_edge_color(style, ImVec2(rightSampleX, cy), min, max, phase + t * 0.04f, false);

            if (hoverPalette) {
                left = mix_color(left, ImVec4(0.05f, 0.60f, 0.76f, 1.0f), 0.34f + 0.10f * topBias);
                right = mix_color(right, ImVec4(0.62f, 0.12f, 0.90f, 1.0f), 0.32f + 0.26f * bottomBias);
            } else {
                left = mix_color(left, ImVec4(0.78f, 0.28f, 1.0f, 1.0f), 0.26f + 0.16f * bottomBias);
                right = mix_color(right, ImVec4(0.12f, 0.82f, 1.0f, 1.0f), 0.28f + 0.18f * topBias);
            }

            left.w = std::clamp(layerAlpha * verticalSoft, 0.0f, hoverPalette ? 0.070f : 0.070f);
            right.w = std::clamp(layerAlpha * verticalSoft, 0.0f, hoverPalette ? 0.072f : 0.075f);
            drawList->AddRectFilledMultiColor(ImVec2(leftX, y),
                                              ImVec2(rightX, y2),
                                              to_u32(left),
                                              to_u32(right),
                                              to_u32(right),
                                              to_u32(left));
        }
    }

    if (hoverPalette) {
        const float bandHeight = std::min(height * 0.30f, 10.0f * scale) * std::max(0.55f, glowScale);
        const float bandTop = max.y - 0.12f * scale;
        const float bandBottom = max.y + bandHeight;
        const float bandLeft = min.x + rounding * 0.18f;
        const float bandRight = max.x - rounding * 0.18f;
        if (bandRight > bandLeft + 1.0f && bandBottom > bandTop + 0.5f) {
            const float rowStep = std::max(0.62f, 0.70f * scale);
            for (float y = bandTop; y < bandBottom; y += rowStep) {
                const float y2 = std::min(y + rowStep + 0.16f * scale, bandBottom);
                const float t = clamp01((y - bandTop) / std::max(0.001f, bandBottom - bandTop));
                const float falloff = std::pow(1.0f - t, 1.55f);
                if (falloff <= 0.004f) {
                    continue;
                }
                ImVec4 left(0.06f, 0.48f, 0.62f, 0.205f * hoverPresence * falloff);
                ImVec4 center(0.08f, 0.40f, 0.88f, 0.330f * hoverPresence * falloff);
                ImVec4 right(0.48f, 0.12f, 0.86f, 0.270f * hoverPresence * falloff);
                const float splitA = lerp(bandLeft, bandRight, 0.34f);
                const float splitB = lerp(bandLeft, bandRight, 0.68f);
                drawList->AddRectFilledMultiColor(ImVec2(bandLeft, y),
                                                  ImVec2(splitA, y2),
                                                  to_u32(left),
                                                  to_u32(center),
                                                  to_u32(center),
                                                  to_u32(left));
                drawList->AddRectFilledMultiColor(ImVec2(splitA, y),
                                                  ImVec2(splitB, y2),
                                                  to_u32(center),
                                                  to_u32(center),
                                                  to_u32(center),
                                                  to_u32(center));
                drawList->AddRectFilledMultiColor(ImVec2(splitB, y),
                                                  ImVec2(bandRight, y2),
                                                  to_u32(center),
                                                  to_u32(right),
                                                  to_u32(right),
                                                  to_u32(center));
            }
        }
    }
}

void draw_neon_capsule_css_halo(ImDrawList* drawList,
                                 ImVec2 min,
                                 ImVec2 max,
                                 float rounding,
                                 const NeonCapsuleGlowStyle& style,
                                float presence,
                                float phase,
                                float glowScale,
                                float scale) {
    if (presence <= 0.002f) {
        return;
    }

    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);
    const float extent = std::min(height * 0.105f, 2.85f * scale) * glowScale;
    if (extent <= 0.25f) {
        return;
    }

    const ImVec2 outerMin(min.x - extent, min.y - extent);
    const ImVec2 outerMax(max.x + extent, max.y + extent);
    const float outerRounding = rounding + extent;
    const float rowStep = std::max(1.0f, scale);

    for (float y = outerMin.y; y < outerMax.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.25f * scale, outerMax.y);
        const float cy = (y + y2) * 0.5f;
        const float outerInset = capsule_inset_at_y(outerMin, outerMax, outerRounding, cy);
        const float outerLeft = outerMin.x + outerInset;
        const float outerRight = outerMax.x - outerInset;
        if (outerRight <= outerLeft) {
            continue;
        }

        const bool intersectsInnerY = cy >= min.y && cy <= max.y;
        if (intersectsInnerY) {
            const float innerInset = capsule_inset_at_y(min, max, rounding, cy);
            const float innerLeft = min.x + innerInset;
            const float innerRight = max.x - innerInset;
            const float yNorm = clamp01((cy - min.y) / height);
            const float edgeAlpha = presence * (0.100f + 0.052f * smoothstep01(0.48f, 1.0f, yNorm));

            if (innerLeft > outerLeft + 0.3f) {
                ImVec4 innerColor = neon_capsule_web_edge_color(style,
                                                                 ImVec2(innerLeft, cy),
                                                                 min,
                                                                 max,
                                                                 phase,
                                                                 false);
                innerColor.w = edgeAlpha * (0.58f + 0.22f * smoothstep01(0.52f, 1.0f, yNorm));
                ImVec4 transparent = innerColor;
                transparent.w = 0.0f;
                drawList->AddRectFilledMultiColor(ImVec2(outerLeft, y),
                                                  ImVec2(innerLeft, y2),
                                                  to_u32(transparent),
                                                  to_u32(innerColor),
                                                  to_u32(innerColor),
                                                  to_u32(transparent));
            }

            if (outerRight > innerRight + 0.3f) {
                ImVec4 innerColor = neon_capsule_web_edge_color(style,
                                                                 ImVec2(innerRight, cy),
                                                                 min,
                                                                 max,
                                                                 phase,
                                                                 false);
                innerColor.w = edgeAlpha * 1.55f;
                ImVec4 transparent = innerColor;
                transparent.w = 0.0f;
                drawList->AddRectFilledMultiColor(ImVec2(innerRight, y),
                                                  ImVec2(outerRight, y2),
                                                  to_u32(innerColor),
                                                  to_u32(transparent),
                                                  to_u32(transparent),
                                                  to_u32(innerColor));
            }
            continue;
        }

        const float distanceY = cy < min.y ? (min.y - cy) : (cy - max.y);
        const float fade = smooth_band_strength(distanceY / extent, 1.0f);
        if (fade <= 0.003f) {
            continue;
        }

        const float sampleY = cy < min.y ? min.y : max.y;
        ImVec4 leftColor = neon_capsule_web_edge_color(style,
                                                       ImVec2(outerLeft + (outerRight - outerLeft) * 0.18f, sampleY),
                                                       min,
                                                       max,
                                                       phase,
                                                       false);
        ImVec4 rightColor = neon_capsule_web_edge_color(style,
                                                        ImVec2(outerLeft + (outerRight - outerLeft) * 0.82f, sampleY),
                                                        min,
                                                        max,
                                                        phase,
                                                        false);
        const float topBias = cy < min.y ? 0.64f : 1.0f;
        leftColor.w = presence * fade * 0.046f * topBias;
        rightColor.w = presence * fade * 0.125f * topBias;
        drawList->AddRectFilledMultiColor(ImVec2(outerLeft, y),
                                          ImVec2(outerRight, y2),
                                          to_u32(leftColor),
                                          to_u32(rightColor),
                                          to_u32(rightColor),
                                          to_u32(leftColor));
    }
}

void draw_neon_capsule_css_outline(ImDrawList* drawList,
                                   const std::vector<ImVec2>& outline,
                                   ImVec2 min,
                                   ImVec2 max,
                                   const NeonCapsuleGlowStyle& style,
                                   float active,
                                   float hover,
                                   float phase,
                                   float glowScale,
                                   float scale,
                                   bool corePasses) {
    const int segmentCount = static_cast<int>(outline.size()) - 1;
    if (segmentCount <= 0) {
        return;
    }

    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.86f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const int passBegin = corePasses ? 3 : 0;
    const int passEnd = corePasses ? 5 : 3;
    for (int pass = passBegin; pass < passEnd; ++pass) {
        const float thickness =
            pass == 0 ? 1.45f * scale * glowScale :
            pass == 1 ? 1.10f * scale * glowScale :
            pass == 2 ? 0.82f * scale :
            pass == 3 ? 1.32f * scale :
                        0.78f * scale;
        const float alphaBase =
            pass == 0 ? (0.0028f * activePresence + 0.0007f * hoverPresence) :
            pass == 1 ? (0.0100f * activePresence + 0.0024f * hoverPresence) :
            pass == 2 ? (0.0380f * activePresence + 0.0060f * hoverPresence) :
            pass == 3 ? (0.260f * activePresence + 0.026f * hoverPresence) :
                        (0.720f * activePresence + 0.088f * hoverPresence);
        if (alphaBase <= 0.002f) {
            continue;
        }

        for (int i = 0; i < segmentCount; ++i) {
            const ImVec2 mid((outline[i].x + outline[i + 1].x) * 0.5f,
                             (outline[i].y + outline[i + 1].y) * 0.5f);
            const float x = clamp01((mid.x - min.x) / std::max(1.0f, max.x - min.x));
            const float y = clamp01((mid.y - min.y) / std::max(1.0f, max.y - min.y));
            const float rightCyan = smoothstep01(0.60f, 1.0f, x);
            const float leftViolet = 1.0f - smoothstep01(0.0f, 0.36f, x);
            const float topEdge = 1.0f - smoothstep01(0.0f, 0.30f, y);
            const float bottomEdge = smoothstep01(0.48f, 1.0f, y);
            const bool hoverPalette = hoverPresence > activePresence;
            const float sideBoost = std::clamp(0.62f + rightCyan * 0.50f + leftViolet * 0.42f +
                                                   topEdge * (hoverPalette ? 0.13f : (0.34f + 0.12f * activePresence)) +
                                                   bottomEdge * (hoverPalette ? 0.62f : (0.48f + 0.36f * activePresence)),
                                               0.0f,
                                               1.36f);

            const bool enabledCore = pass >= 3 && activePresence > 0.04f;
            ImVec4 color = hoverPalette
                ? neon_capsule_hover_edge_color(style, mid, min, max, phase + x * 0.06f, enabledCore)
                : neon_capsule_web_edge_color(style, mid, min, max, phase + x * 0.06f, enabledCore);
            if (pass >= 3) {
                const float localWhite = std::clamp(std::max(std::max(rightCyan, topEdge * (0.30f + 0.70f * x)),
                                                              std::max(bottomEdge * 0.58f, leftViolet * 0.52f)),
                                                     0.0f,
                                                     1.0f);
                const float whiteMix = (pass == 4 ? 0.36f : 0.16f) * activePresence * localWhite;
                color = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), whiteMix);
            }
            color.w = std::clamp(alphaBase * sideBoost, 0.0f, pass >= 4 ? 1.0f : 0.52f);
            drawList->AddLine(outline[i], outline[i + 1], to_u32(color), thickness);
        }
    }
}

void draw_neon_capsule_border_band(ImDrawList* drawList,
                                   ImVec2 min,
                                   ImVec2 max,
                                   float rounding,
                                   const NeonCapsuleGlowStyle& style,
                                   float active,
                                   float hover,
                                   float phase,
                                   float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.82f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const float height = std::max(1.0f, max.y - min.y);
    const float borderWidth = (1.10f + 1.10f * activePresence + 0.52f * hoverPresence) * scale;
    if (borderWidth <= 0.25f || height <= borderWidth * 2.0f) {
        return;
    }

    const ImVec2 innerMin(min.x + borderWidth, min.y + borderWidth);
    const ImVec2 innerMax(max.x - borderWidth, max.y - borderWidth);
    const float innerRounding = std::max(0.0f, rounding - borderWidth);
    const float rowStep = std::max(0.72f, 0.74f * scale);
    const bool core = activePresence > 0.04f;

    auto drawGradientBand = [&](ImVec2 a, ImVec2 b, float yNorm) {
        if (b.x <= a.x || b.y <= a.y) {
            return;
        }

        const float width = std::max(1.0f, max.x - min.x);
        const float x0 = clamp01((a.x - min.x) / width);
        const float x1 = clamp01((b.x - min.x) / width);
        const bool hoverPalette = hoverPresence > activePresence;
        ImVec4 left = hoverPalette
            ? neon_capsule_hover_edge_color(style, ImVec2(a.x, min.y + yNorm * height), min, max, phase, core)
            : neon_capsule_web_edge_color(style, ImVec2(a.x, min.y + yNorm * height), min, max, phase, core);
        ImVec4 right = hoverPalette
            ? neon_capsule_hover_edge_color(style, ImVec2(b.x, min.y + yNorm * height), min, max, phase + 0.04f, core)
            : neon_capsule_web_edge_color(style, ImVec2(b.x, min.y + yNorm * height), min, max, phase + 0.04f, core);
        const float topEdge = 1.0f - smoothstep01(0.0f, 0.32f, yNorm);
        const float bottomEdge = smoothstep01(0.48f, 1.0f, yNorm);
        const float sideBoost = std::clamp(0.70f + topEdge * (0.15f + 0.22f * activePresence) +
                                               bottomEdge * (0.40f + 0.24f * hoverPresence + 0.28f * activePresence),
                                           0.0f,
                                           1.32f);
        float alpha = (0.34f * hoverPresence + 0.94f * activePresence) * sideBoost;
        if (!core && hoverPresence > 0.002f) {
            alpha *= 1.0f - 0.52f * topEdge * hoverPresence;
        }
        left.w = std::clamp(alpha * (0.88f + 0.18f * (1.0f - x0)), 0.0f, 0.86f);
        right.w = std::clamp(alpha * (0.92f + 0.20f * x1), 0.0f, 0.92f);
        if (core && x1 > 0.68f) {
            const float rightLight = smoothstep01(0.68f, 1.0f, x1) * activePresence;
            right = mix_color(right, ImVec4(0.76f, 0.94f, 1.0f, right.w), 0.38f * rightLight);
            right.w = std::clamp(right.w + 0.080f * rightLight, 0.0f, 0.98f);
        }
        if (core) {
            const float hot = std::clamp(std::max(topEdge * (0.35f + 0.65f * x1),
                                                  bottomEdge * (1.0f - x0 * 0.28f)),
                                         0.0f,
                                         1.0f);
            left = mix_color(left, ImVec4(1.0f, 0.86f, 1.0f, left.w), 0.24f * hot);
            right = mix_color(right, ImVec4(0.92f, 0.98f, 1.0f, right.w), 0.30f * hot);
        }
        drawList->AddRectFilledMultiColor(a,
                                          b,
                                          to_u32(left),
                                          to_u32(right),
                                          to_u32(right),
                                          to_u32(left));
    };

    for (float y = min.y; y < max.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.16f * scale, max.y);
        const float cy = (y + y2) * 0.5f;
        const float yNorm = clamp01((cy - min.y) / height);
        const float outerInset = capsule_inset_at_y(min, max, rounding, cy);
        const float outerLeft = min.x + outerInset;
        const float outerRight = max.x - outerInset;
        if (outerRight <= outerLeft) {
            continue;
        }

        if (cy < innerMin.y || cy > innerMax.y) {
            drawGradientBand(ImVec2(outerLeft, y), ImVec2(outerRight, y2), yNorm);
            continue;
        }

        const float innerInset = capsule_inset_at_y(innerMin, innerMax, innerRounding, cy);
        const float innerLeft = innerMin.x + innerInset;
        const float innerRight = innerMax.x - innerInset;
        if (innerLeft > outerLeft + 0.2f) {
            drawGradientBand(ImVec2(outerLeft, y), ImVec2(innerLeft, y2), yNorm);
        }
        if (outerRight > innerRight + 0.2f) {
            drawGradientBand(ImVec2(innerRight, y), ImVec2(outerRight, y2), yNorm);
        }
    }
}

void draw_neon_capsule_css_shell(ImDrawList* drawList,
                                 ImVec2 min,
                                 ImVec2 max,
                                 float rounding,
                                 const NeonCapsuleGlowStyle& style,
                                 float active,
                                 float hover,
                                 float phase,
                                 float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.86f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);
    const float shellWidth = (1.92f + 0.78f * activePresence + 0.34f * hoverPresence) * scale;
    if (height <= shellWidth * 2.0f || width <= shellWidth * 2.0f) {
        return;
    }

    const bool hoverPalette = hoverPresence > activePresence;
    const ImVec2 innerMin(min.x + shellWidth, min.y + shellWidth);
    const ImVec2 innerMax(max.x - shellWidth, max.y - shellWidth);
    const float innerRounding = std::max(0.0f, rounding - shellWidth);
    const float rowStep = std::max(0.46f, 0.52f * scale);

    for (float y = min.y; y < max.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.20f * scale, max.y);
        const float cy = (y + y2) * 0.5f;
        const float yNorm = clamp01((cy - min.y) / height);
        const float outerInset = capsule_inset_at_y(min, max, rounding, cy);
        const float outerLeft = min.x + outerInset;
        const float outerRight = max.x - outerInset;
        if (outerRight <= outerLeft) {
            continue;
        }

        const bool hasInner = cy >= innerMin.y && cy <= innerMax.y;
        float innerLeft = outerRight;
        float innerRight = outerLeft;
        if (hasInner) {
            const float innerInset = capsule_inset_at_y(innerMin, innerMax, innerRounding, cy);
            innerLeft = innerMin.x + innerInset;
            innerRight = innerMax.x - innerInset;
        }

        auto edgeColorAt = [&](float x) {
            const ImVec2 sample(x, cy);
            return hoverPalette
                ? neon_capsule_hover_edge_color(style, sample, min, max, phase + yNorm * 0.035f, true)
                : neon_capsule_web_edge_color(style, sample, min, max, phase + yNorm * 0.035f, true);
        };
        auto drawSegment = [&](float x0, float x1) {
            if (x1 <= x0 + 0.2f) {
                return;
            }
            const float xNorm0 = clamp01((x0 - min.x) / width);
            const float xNorm1 = clamp01((x1 - min.x) / width);
            const float topEdge = 1.0f - smoothstep01(0.0f, 0.30f, yNorm);
            const float bottomEdge = smoothstep01(0.50f, 1.0f, yNorm);
            const float leftEdge = 1.0f - smoothstep01(0.0f, 0.24f, xNorm0);
            const float rightEdge = smoothstep01(0.76f, 1.0f, xNorm1);
            const float edgeEnergy = std::clamp(0.76f + topEdge * (hoverPalette ? 0.12f : 0.30f) +
                                                    bottomEdge * (hoverPalette ? 0.46f : 0.32f) +
                                                    leftEdge * 0.16f + rightEdge * 0.26f,
                                                0.0f,
                                                1.42f);
            ImVec4 left = edgeColorAt(x0);
            ImVec4 right = edgeColorAt(x1);
            const float alphaBase = activePresence > 0.02f
                ? (0.54f + 0.32f * activePresence)
                : (0.135f + 0.045f * hoverPresence);
            const float hoverBottomDampen = hoverPalette ? (1.0f - 0.58f * topEdge) : 1.0f;
            left.w = std::clamp(alphaBase * edgeEnergy * hoverBottomDampen,
                                0.0f,
                                activePresence > 0.02f ? 0.92f : 0.22f);
            right.w = std::clamp(alphaBase * edgeEnergy * hoverBottomDampen,
                                 0.0f,
                                 activePresence > 0.02f ? 0.94f : 0.24f);
            drawList->AddRectFilledMultiColor(ImVec2(x0, y),
                                              ImVec2(x1, y2),
                                              to_u32(left),
                                              to_u32(right),
                                              to_u32(right),
                                              to_u32(left));
        };

        if (!hasInner || innerLeft >= innerRight) {
            drawSegment(outerLeft, outerRight);
            continue;
        }

        drawSegment(outerLeft, innerLeft);
        drawSegment(innerRight, outerRight);

        const bool topOrBottom = cy < innerMin.y + 0.2f || cy > innerMax.y - 0.2f;
        if (topOrBottom) {
            drawSegment(innerLeft, innerRight);
        }

        if (innerRight > innerLeft + 0.2f) {
            const float edgeFade = std::pow(clamp01(std::fabs(yNorm - 0.5f) * 2.0f), 1.8f);
            ImVec4 fillLeft = hoverPalette
                ? ImVec4(0.010f, 0.023f, 0.060f, 0.38f * hoverPresence)
                : ImVec4(0.010f, 0.026f, 0.230f, 0.34f * activePresence);
            ImVec4 fillCenter = hoverPalette
                ? ImVec4(0.007f, 0.016f, 0.038f, 0.30f * hoverPresence)
                : ImVec4(0.012f, 0.026f, 0.205f, 0.30f * activePresence);
            ImVec4 fillRight = hoverPalette
                ? ImVec4(0.018f, 0.013f, 0.044f, 0.30f * hoverPresence)
                : ImVec4(0.010f, 0.046f, 0.285f, 0.38f * activePresence);
            fillLeft.w += edgeFade * (0.04f * hoverPresence + 0.08f * activePresence);
            fillCenter.w += edgeFade * (0.02f * hoverPresence + 0.05f * activePresence);
            fillRight.w += edgeFade * (0.04f * hoverPresence + 0.08f * activePresence);
            const float splitA = lerp(innerLeft, innerRight, 0.36f);
            const float splitB = lerp(innerLeft, innerRight, 0.68f);
            drawList->AddRectFilledMultiColor(ImVec2(innerLeft, y),
                                              ImVec2(splitA, y2),
                                              to_u32(fillLeft),
                                              to_u32(fillCenter),
                                              to_u32(fillCenter),
                                              to_u32(fillLeft));
            drawList->AddRectFilledMultiColor(ImVec2(splitA, y),
                                              ImVec2(splitB, y2),
                                              to_u32(fillCenter),
                                              to_u32(fillCenter),
                                              to_u32(fillCenter),
                                              to_u32(fillCenter));
            drawList->AddRectFilledMultiColor(ImVec2(splitB, y),
                                              ImVec2(innerRight, y2),
                                              to_u32(fillCenter),
                                              to_u32(fillRight),
                                              to_u32(fillRight),
                                              to_u32(fillCenter));
        }
    }
}

void draw_neon_capsule_side_cap_volume(ImDrawList* drawList,
                                       ImVec2 min,
                                       ImVec2 max,
                                       float rounding,
                                       const NeonCapsuleGlowStyle& style,
                                       float active,
                                       float hover,
                                       float phase,
                                       float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.68f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const ImVec4 warm = resolve_neon_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
    const float height = std::max(1.0f, max.y - min.y);
    const float width = std::max(1.0f, max.x - min.x);
    const float verticalInset = std::max(2.0f * scale, height * 0.18f);
    const ImVec2 capMin(min.x + height * 0.13f, min.y + verticalInset);
    const ImVec2 capMax(max.x - height * 0.13f, max.y - verticalInset);
    if (capMax.y <= capMin.y || capMax.x <= capMin.x) {
        return;
    }

    const float leftWidth = std::min(width * (0.092f + 0.034f * activePresence), height * 0.96f);
    const float rightWidth = std::min(width * (0.108f + 0.040f * activePresence), height * 1.08f);
    const float shimmer = 0.5f + 0.5f * std::sin(phase * kPi * 2.0f);

    const bool hoverPalette = hoverPresence > activePresence;
    ImVec4 leftEdge = hoverPalette ? cyan : mix_color(violet, warm, 0.08f);
    leftEdge = mix_color(leftEdge, ImVec4(1.0f, 0.82f, 1.0f, 1.0f), 0.22f * activePresence);
    leftEdge.w = std::clamp((0.098f * hoverPresence + 0.260f * activePresence) * (0.92f + 0.16f * shimmer),
                            0.0f,
                            0.46f);
    ImVec4 leftTransparent = leftEdge;
    leftTransparent.w = 0.0f;

    ImVec4 rightEdge = hoverPalette
        ? mix_color(violet, warm, 0.05f)
        : mix_color(cyan, ImVec4(0.90f, 0.98f, 1.0f, 1.0f), 0.18f * activePresence);
    rightEdge.w = std::clamp((0.108f * hoverPresence + 0.320f * activePresence) * (0.94f + 0.14f * shimmer),
                             0.0f,
                             0.50f);
    ImVec4 rightTransparent = rightEdge;
    rightTransparent.w = 0.0f;

    if (leftWidth > 0.5f) {
        drawList->AddRectFilledMultiColor(capMin,
                                          ImVec2(capMin.x + leftWidth, capMax.y),
                                          to_u32(leftEdge),
                                          to_u32(leftTransparent),
                                          to_u32(leftTransparent),
                                          to_u32(leftEdge));
    }
    if (rightWidth > 0.5f) {
        drawList->AddRectFilledMultiColor(ImVec2(capMax.x - rightWidth, capMin.y),
                                          capMax,
                                          to_u32(rightTransparent),
                                          to_u32(rightEdge),
                                          to_u32(rightEdge),
                                          to_u32(rightTransparent));
    }
}

void draw_neon_capsule_hover_top_sheen(ImDrawList* drawList,
                                       ImVec2 min,
                                       ImVec2 max,
                                       float rounding,
                                       const NeonCapsuleGlowStyle& style,
                                       float active,
                                       float hover,
                                       float phase,
                                       float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    if (hoverPresence <= 0.002f) {
        return;
    }

    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const float height = std::max(1.0f, max.y - min.y);
    const float radius = std::clamp(rounding, 0.0f, height * 0.5f);
    const float x0 = min.x + radius * 0.72f;
    const float x1 = max.x - radius * 0.72f;
    if (x1 <= x0 + 1.0f) {
        return;
    }

    const float y = min.y + std::max(1.05f * scale, height * 0.080f);
    constexpr int kSegments = 28;
    for (int pass = 0; pass < 2; ++pass) {
        const float thickness = (pass == 0 ? 1.75f : 0.82f) * scale;
        const float alpha = pass == 0 ? 0.018f * hoverPresence : 0.042f * hoverPresence;
        for (int i = 0; i < kSegments; ++i) {
            const float t0 = static_cast<float>(i) / static_cast<float>(kSegments);
            const float t1 = static_cast<float>(i + 1) / static_cast<float>(kSegments);
            const float tm = (t0 + t1) * 0.5f;
            const float pulse = 0.5f + 0.5f * std::sin((phase + tm * 0.16f) * kPi * 2.0f);
            ImVec4 color = mix_color(cyan, violet, std::clamp(0.16f + tm * 0.86f, 0.0f, 1.0f));
            color = mix_color(color, ImVec4(0.88f, 0.98f, 1.0f, 1.0f), (pass == 0 ? 0.16f : 0.30f) * (0.98f - 0.26f * tm));
            color.w = std::clamp(alpha * (0.86f + 0.18f * pulse), 0.0f, pass == 0 ? 0.32f : 0.62f);
            drawList->AddLine(ImVec2(lerp(x0, x1, t0), y),
                              ImVec2(lerp(x0, x1, t1), y),
                              to_u32(color),
                              thickness);
        }
    }
}

void draw_neon_capsule_inner_css_glow(ImDrawList* drawList,
                                      ImVec2 min,
                                      ImVec2 max,
                                      float rounding,
                                      const NeonCapsuleGlowStyle& style,
                                      float active,
                                      float hover,
                                      float phase,
                                      float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.70f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);
    constexpr int kRingCount = 4;
    for (int ring = 0; ring < kRingCount; ++ring) {
        const float inset = (1.0f + static_cast<float>(ring) * 1.05f) * scale;
        if (width <= inset * 2.0f || height <= inset * 2.0f) {
            continue;
        }

        const ImVec2 ringMin(min.x + inset, min.y + inset);
        const ImVec2 ringMax(max.x - inset, max.y - inset);
        const float ringRounding = std::max(0.0f, rounding - inset);
        const std::vector<ImVec2> outline = make_neon_capsule_outline(ringMin, ringMax, ringRounding, scale);
        const int segmentCount = static_cast<int>(outline.size()) - 1;
        if (segmentCount <= 0) {
            continue;
        }

        const float ringFalloff = 1.0f - static_cast<float>(ring) / static_cast<float>(kRingCount);
        const float alphaBase = (activePresence * (0.410f * ringFalloff * ringFalloff + 0.035f) +
                                 hoverPresence * (0.118f * ringFalloff * ringFalloff)) * presence;
        if (alphaBase <= 0.002f) {
            continue;
        }

        const float thickness = (ring == 0 ? 1.18f : 1.42f + static_cast<float>(ring) * 0.18f) * scale;
        for (int i = 0; i < segmentCount; ++i) {
            const ImVec2 mid((outline[i].x + outline[i + 1].x) * 0.5f,
                             (outline[i].y + outline[i + 1].y) * 0.5f);
            const float x = clamp01((mid.x - min.x) / width);
            const float y = clamp01((mid.y - min.y) / height);
            const float topEdge = 1.0f - smoothstep01(0.0f, 0.32f, y);
            const float bottomEdge = smoothstep01(0.50f, 1.0f, y);
            const float rightEdge = smoothstep01(0.58f, 1.0f, x);
            const float leftEdge = 1.0f - smoothstep01(0.0f, 0.30f, x);
            const bool hoverPalette = hoverPresence > activePresence;
            ImVec4 color = hoverPalette
                ? neon_capsule_hover_edge_color(style,
                                                mid,
                                                min,
                                                max,
                                                phase + x * 0.045f - y * 0.025f,
                                                activePresence > 0.04f)
                : neon_capsule_web_edge_color(style,
                                              mid,
                                              min,
                                              max,
                                              phase + x * 0.045f - y * 0.025f,
                                              activePresence > 0.04f);
            const float hotEdge = std::clamp(std::max(std::max(rightEdge, topEdge * (0.40f + 0.60f * x)),
                                                       std::max(bottomEdge * 0.86f, leftEdge * 0.58f)),
                                             0.0f,
                                             1.0f);
            color = mix_color(color,
                              ImVec4(0.94f, 0.98f, 1.0f, 1.0f),
                              (0.18f + 0.20f * activePresence) * hotEdge * ringFalloff);
            const float edgeBias = std::clamp(0.45f + topEdge * 0.34f + bottomEdge * 0.40f +
                                                  rightEdge * 0.42f + leftEdge * 0.26f,
                                              0.0f,
                                              1.25f);
            color.w = std::clamp(alphaBase * edgeBias, 0.0f, 0.66f);
            drawList->AddLine(outline[i], outline[i + 1], to_u32(color), thickness);
        }
    }
}

void draw_neon_capsule_web_core_rim(ImDrawList* drawList,
                                    const std::vector<ImVec2>& outline,
                                    ImVec2 min,
                                    ImVec2 max,
                                    const NeonCapsuleGlowStyle& style,
                                    float active,
                                    float hover,
                                    float phase,
                                    float scale) {
    const int segmentCount = static_cast<int>(outline.size()) - 1;
    if (segmentCount <= 0) {
        return;
    }

    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.72f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const ImVec4 warm = resolve_neon_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);

    for (int pass = 0; pass < 3; ++pass) {
        const float thickness =
            pass == 0 ? 3.60f * scale :
            pass == 1 ? 2.10f * scale :
                        1.16f * scale;
        const float alphaBase =
            pass == 0 ? (0.092f * activePresence + 0.006f * hoverPresence) :
            pass == 1 ? (0.520f * activePresence + 0.026f * hoverPresence) :
                        (1.000f * activePresence + 0.082f * hoverPresence);
        if (alphaBase <= 0.002f) {
            continue;
        }

        for (int i = 0; i < segmentCount; ++i) {
            const ImVec2 mid((outline[i].x + outline[i + 1].x) * 0.5f,
                             (outline[i].y + outline[i + 1].y) * 0.5f);
            const float x = clamp01((mid.x - min.x) / width);
            const float y = clamp01((mid.y - min.y) / height);
            const float topEdge = 1.0f - smoothstep01(0.0f, 0.34f, y);
            const float bottomEdge = smoothstep01(0.52f, 1.0f, y);
            const float rightEdge = smoothstep01(0.58f, 1.0f, x);
            const float leftEdge = 1.0f - smoothstep01(0.0f, 0.30f, x);
            const float pulse = 0.5f + 0.5f * std::sin((phase + x * 0.18f + y * 0.10f) * kPi * 2.0f);

            const float cyanWeight = std::clamp(0.24f + rightEdge * 1.35f + topEdge * (0.42f + 0.28f * x),
                                                0.0f,
                                                2.2f);
            const float violetWeight = std::clamp(0.24f + leftEdge * 1.10f + bottomEdge * 1.30f,
                                                  0.0f,
                                                  2.16f);
            const float warmWeight = 0.04f + 0.12f * bottomEdge * (1.0f - rightEdge) * pulse;
            const float total = std::max(0.001f, cyanWeight + violetWeight + warmWeight);
            ImVec4 color((cyan.x * cyanWeight + violet.x * violetWeight + warm.x * warmWeight) / total,
                         (cyan.y * cyanWeight + violet.y * violetWeight + warm.y * warmWeight) / total,
                         (cyan.z * cyanWeight + violet.z * violetWeight + warm.z * warmWeight) / total,
                         1.0f);

            const float topRightHot = std::max(rightEdge, topEdge * (0.34f + 0.66f * x));
            const float bottomHot = bottomEdge * (0.82f + 0.18f * (1.0f - rightEdge * 0.22f));
            const float bottomLeftHot = std::max(bottomHot, leftEdge * 0.76f);
            const float whiteHot = std::clamp(std::max(topRightHot, bottomLeftHot * 0.98f), 0.0f, 1.0f);
            const float whiteMix =
                pass == 0 ? 0.20f * activePresence * whiteHot :
                pass == 1 ? 0.60f * activePresence * whiteHot :
                            0.94f * activePresence * whiteHot;
            color = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), whiteMix);

            const bool hoverPalette = hoverPresence > activePresence;
            const float edgeBias = std::clamp(0.72f + topEdge * (hoverPalette ? 0.08f : 0.30f) +
                                                  rightEdge * 0.42f +
                                                  bottomEdge * (hoverPalette ? 0.68f : 0.54f) +
                                                  leftEdge * 0.28f,
                                              0.0f,
                                              1.64f);
            color.w = std::clamp(alphaBase * edgeBias, 0.0f, pass == 2 ? 1.0f : 0.70f);
            drawList->AddLine(outline[i], outline[i + 1], to_u32(color), thickness);
        }
    }

    if (activePresence <= 0.002f) {
        return;
    }

    for (int i = 0; i < segmentCount; ++i) {
        const ImVec2 mid((outline[i].x + outline[i + 1].x) * 0.5f,
                         (outline[i].y + outline[i + 1].y) * 0.5f);
        const float x = clamp01((mid.x - min.x) / width);
        const float y = clamp01((mid.y - min.y) / height);
        const float topEdge = 1.0f - smoothstep01(0.0f, 0.30f, y);
        const float bottomEdge = smoothstep01(0.56f, 1.0f, y);
        const float rightEdge = smoothstep01(0.60f, 1.0f, x);
        const float leftEdge = 1.0f - smoothstep01(0.0f, 0.28f, x);
        const float topRightHot = std::max(rightEdge, topEdge * (0.28f + 0.72f * x));
        const float bottomLeftHot = std::max(bottomEdge * (0.88f + 0.12f * (1.0f - rightEdge)), leftEdge * 0.72f);
        const float hot = std::clamp(std::max(topRightHot, bottomLeftHot * 0.92f), 0.0f, 1.0f);
        if (hot <= 0.025f) {
            continue;
        }

        ImVec4 tint = mix_color(violet, cyan, std::clamp(0.16f + rightEdge * 0.70f + topEdge * 0.28f, 0.0f, 1.0f));
        tint = mix_color(tint, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.82f);
        tint.w = std::clamp(activePresence * hot * (0.70f + 0.24f * topRightHot), 0.0f, 0.96f);
        drawList->AddLine(outline[i], outline[i + 1], to_u32(tint), 0.92f * scale);
    }
}

ImVec4 neon_capsule_css_button_color(const NeonCapsuleGlowStyle& style,
                                     ImVec2 point,
                                     ImVec2 min,
                                     ImVec2 max,
                                     float phase,
                                     bool activeMode,
                                     bool hotLine) {
    const ImVec4 cyan = resolve_neon_color(style.colorA, ImVec4(0.10f, 0.88f, 0.96f, 1.0f));
    const ImVec4 violet = resolve_neon_color(style.colorB, ImVec4(0.78f, 0.26f, 1.0f, 1.0f));
    const ImVec4 warm = resolve_neon_color(style.colorC, ImVec4(0.95f, 0.58f, 0.20f, 1.0f));
    const float width = std::max(1.0f, max.x - min.x);
    const float height = std::max(1.0f, max.y - min.y);
    const float x = clamp01((point.x - min.x) / width);
    const float y = clamp01((point.y - min.y) / height);
    const float top = 1.0f - smoothstep01(0.0f, 0.34f, y);
    const float bottom = smoothstep01(0.48f, 1.0f, y);
    const float right = smoothstep01(0.58f, 1.0f, x);
    const float left = 1.0f - smoothstep01(0.0f, 0.34f, x);
    const float shimmer = 0.5f + 0.5f * std::sin((phase + x * 0.15f - y * 0.08f) * kPi * 2.0f);
    const float topRightTube = std::clamp(std::max(right * 0.94f, top * smoothstep01(0.28f, 1.0f, x)) +
                                              0.18f * top * right,
                                          0.0f,
                                          1.0f);
    const float violetTube = std::clamp(std::max(left * (0.40f + 0.44f * bottom),
                                                 bottom * (1.0f - 0.58f * right)),
                                        0.0f,
                                        1.0f);

    ImVec4 color = activeMode
        ? mix_color(violet, cyan, smoothstep01(0.20f, 0.92f, x))
        : mix_color(cyan, violet, smoothstep01(0.48f, 1.0f, x));

    if (activeMode) {
        color = mix_color(color, cyan, 0.42f * topRightTube);
        color = mix_color(color, violet, 0.34f * violetTube);
        color = mix_color(color, warm, 0.045f * bottom * (1.0f - right));
        if (hotLine) {
            const float hot = std::clamp(std::max(topRightTube * 1.08f, violetTube * 0.72f), 0.0f, 1.0f);
            color = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), (0.28f + 0.10f * shimmer) * hot);
        }
    } else {
        color = mix_color(color, cyan, 0.26f * top + 0.20f * left);
        color = mix_color(color, violet, 0.30f * bottom + 0.28f * right);
        color = mix_color(color, warm, 0.030f * bottom * (0.35f + 0.65f * shimmer));
        if (hotLine) {
            color = mix_color(color, ImVec4(0.88f, 0.96f, 1.0f, 1.0f), 0.055f + 0.050f * shimmer);
        }
    }

    color.w = 1.0f;
    return color;
}

void draw_neon_capsule_css_shadow(ImDrawList* drawList,
                                  ImVec2 min,
                                  ImVec2 max,
                                  float rounding,
                                  const NeonCapsuleGlowStyle& style,
                                  float active,
                                  float hover,
                                  float phase,
                                  float glowScale,
                                  float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.82f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const bool activeMode = activePresence >= hoverPresence;
    const float height = std::max(1.0f, max.y - min.y);
    const float width = std::max(1.0f, max.x - min.x);
    const float maxRange = std::min(height * (activeMode ? 0.072f : 0.040f),
                                    (activeMode ? 1.72f : 0.92f) * scale) *
                           std::clamp(glowScale, 0.35f, 1.0f);
    if (maxRange <= 0.25f) {
        return;
    }

    const ImVec2 fieldMin(min.x - maxRange, min.y - maxRange);
    const ImVec2 fieldMax(max.x + maxRange, max.y + maxRange);
    const float rowStep = std::max(0.66f, 0.72f * scale);
    const float colStep = std::max(0.66f, 0.72f * scale);
    for (float y = fieldMin.y; y < fieldMax.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.12f * scale, fieldMax.y);
        for (float x = fieldMin.x; x < fieldMax.x; x += colStep) {
            const float x2 = std::min(x + colStep + 0.12f * scale, fieldMax.x);
            const ImVec2 sample((x + x2) * 0.5f, (y + y2) * 0.5f);
            const float distance = rounded_rect_signed_distance(sample, min, max, rounding);
            if (distance < -0.42f * scale || distance > maxRange) {
                continue;
            }

            const float xNorm = clamp01((sample.x - min.x) / width);
            const float yNorm = clamp01((sample.y - min.y) / height);
            const float outside = std::max(distance, 0.0f);
            const float falloff = std::pow(1.0f - outside / std::max(0.001f, maxRange), activeMode ? 3.60f : 4.20f);
            const float top = 1.0f - smoothstep01(0.0f, 0.34f, yNorm);
            const float bottom = smoothstep01(0.52f, 1.0f, yNorm);
            const float right = smoothstep01(0.60f, 1.0f, xNorm);
            const float left = 1.0f - smoothstep01(0.0f, 0.32f, xNorm);
            const float cssBias = activeMode
                ? std::clamp(0.44f + right * 0.58f + left * 0.36f + bottom * 0.42f + top * 0.22f, 0.0f, 1.34f)
                : std::clamp(0.34f + right * 0.46f + left * 0.36f + bottom * 0.28f + top * 0.16f, 0.0f, 1.02f);
            float alpha = presence * falloff * cssBias * (activeMode ? 0.056f : 0.018f);
            if (distance < 0.0f) {
                alpha *= 0.08f;
            }
            if (!activeMode) {
                alpha *= 1.0f - 0.54f * top;
            }
            if (alpha <= 0.002f) {
                continue;
            }

            ImVec4 color = neon_capsule_css_button_color(style, sample, min, max, phase, activeMode, false);
            color.w = std::clamp(alpha, 0.0f, activeMode ? 0.070f : 0.024f);
            drawList->AddRectFilled(ImVec2(x, y), ImVec2(x2, y2), to_u32(color));
        }
    }
}

void draw_neon_capsule_css_outline_glow(ImDrawList* drawList,
                                        ImVec2 min,
                                        ImVec2 max,
                                        float rounding,
                                        const NeonCapsuleGlowStyle& style,
                                        float active,
                                        float hover,
                                        float phase,
                                        float glowScale,
                                        float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.84f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const bool activeMode = activePresence >= hoverPresence;
    constexpr int kLayers = 3;
    for (int layer = kLayers; layer >= 1; --layer) {
        const float layerNorm = static_cast<float>(layer) / static_cast<float>(kLayers);
        const float innerBias = 1.0f - layerNorm;
        const float expand = (0.10f + layerNorm * (activeMode ? 1.36f : 0.72f)) *
                             std::clamp(glowScale, 0.35f, 1.0f) * scale;
        const ImVec2 layerMin(min.x - expand, min.y - expand);
        const ImVec2 layerMax(max.x + expand, max.y + expand);
        const float layerRounding = rounding + expand;
        const std::vector<ImVec2> outline = make_neon_capsule_outline(layerMin, layerMax, layerRounding, scale);
        const int segmentCount = static_cast<int>(outline.size()) - 1;
        if (segmentCount <= 0) {
            continue;
        }

        const float alphaBase = presence * (activeMode ? 0.116f : 0.034f) *
                                (0.06f + innerBias * innerBias * 1.18f);
        const float thickness = (activeMode ? 1.18f : 0.74f) * scale * (0.56f + innerBias * 0.44f);
        for (int i = 0; i < segmentCount; ++i) {
            const ImVec2 mid((outline[i].x + outline[i + 1].x) * 0.5f,
                             (outline[i].y + outline[i + 1].y) * 0.5f);
            const float x = clamp01((mid.x - min.x) / std::max(1.0f, max.x - min.x));
            const float y = clamp01((mid.y - min.y) / std::max(1.0f, max.y - min.y));
            const float top = 1.0f - smoothstep01(0.0f, 0.34f, y);
            const float bottom = smoothstep01(0.50f, 1.0f, y);
            const float right = smoothstep01(0.60f, 1.0f, x);
            const float left = 1.0f - smoothstep01(0.0f, 0.34f, x);
            const float sideBias = activeMode
                ? std::clamp(0.58f + right * 0.58f + left * 0.28f + bottom * 0.40f + top * 0.18f, 0.0f, 1.38f)
                : std::clamp(0.42f + right * 0.42f + left * 0.34f + bottom * 0.30f + top * 0.10f, 0.0f, 1.05f);
            ImVec4 color = neon_capsule_css_button_color(style, mid, min, max, phase + static_cast<float>(i) * 0.0007f, activeMode, false);
            color.w = std::clamp(alphaBase * sideBias, 0.0f, activeMode ? 0.155f : 0.040f);
            drawList->AddLine(outline[i], outline[i + 1], to_u32(color), thickness);
        }
    }
}

void draw_neon_capsule_css_inner_fill(ImDrawList* drawList,
                                      ImVec2 min,
                                      ImVec2 max,
                                      float rounding,
                                      float active,
                                      float hover,
                                      float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.72f, 0.0f, 1.0f);
    if (presence <= 0.002f || max.x <= min.x || max.y <= min.y) {
        return;
    }

    const float height = std::max(1.0f, max.y - min.y);
    const float width = std::max(1.0f, max.x - min.x);
    const float rowStep = std::max(0.62f, 0.66f * scale);
    for (float y = min.y; y < max.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.12f * scale, max.y);
        const float cy = (y + y2) * 0.5f;
        const float inset = capsule_inset_at_y(min, max, rounding, cy);
        const float leftX = min.x + inset;
        const float rightX = max.x - inset;
        if (rightX <= leftX) {
            continue;
        }

        const float yNorm = clamp01((cy - min.y) / height);
        const float top = 1.0f - smoothstep01(0.0f, 0.34f, yNorm);
        const float bottom = smoothstep01(0.52f, 1.0f, yNorm);
        const float splitA = leftX + (rightX - leftX) * 0.38f;
        const float splitB = leftX + (rightX - leftX) * 0.70f;
        const bool activeMode = activePresence >= hoverPresence;
        ImVec4 left = activeMode
            ? ImVec4(0.038f, 0.018f, 0.245f, 0.92f * activePresence)
            : ImVec4(0.006f, 0.014f, 0.033f, 0.62f * hoverPresence);
        ImVec4 center = activeMode
            ? ImVec4(0.012f, 0.032f, 0.205f, 0.88f * activePresence)
            : ImVec4(0.004f, 0.010f, 0.026f, 0.58f * hoverPresence);
        ImVec4 right = activeMode
            ? ImVec4(0.006f, 0.064f, 0.320f, 0.92f * activePresence)
            : ImVec4(0.014f, 0.012f, 0.038f, 0.54f * hoverPresence);
        left.w += (0.042f * bottom + 0.018f * top) * presence;
        center.w += (0.032f * top + 0.018f * bottom) * presence;
        right.w += (0.054f * top + 0.022f * bottom) * presence;
        drawList->AddRectFilledMultiColor(ImVec2(leftX, y),
                                          ImVec2(splitA, y2),
                                          to_u32(left),
                                          to_u32(center),
                                          to_u32(center),
                                          to_u32(left));
        drawList->AddRectFilledMultiColor(ImVec2(splitA, y),
                                          ImVec2(splitB, y2),
                                          to_u32(center),
                                          to_u32(center),
                                          to_u32(center),
                                          to_u32(center));
        drawList->AddRectFilledMultiColor(ImVec2(splitB, y),
                                          ImVec2(rightX, y2),
                                          to_u32(center),
                                          to_u32(right),
                                          to_u32(right),
                                          to_u32(center));
    }
}

void draw_neon_capsule_css_ring(ImDrawList* drawList,
                                ImVec2 min,
                                ImVec2 max,
                                float rounding,
                                const NeonCapsuleGlowStyle& style,
                                float active,
                                float hover,
                                float phase,
                                float scale) {
    const float activePresence = clamp01(active);
    const float hoverPresence = clamp01(hover * (1.0f - activePresence));
    const float presence = std::clamp(activePresence + hoverPresence * 0.86f, 0.0f, 1.0f);
    if (presence <= 0.002f) {
        return;
    }

    const bool activeMode = activePresence >= hoverPresence;
    const float height = std::max(1.0f, max.y - min.y);
    const float ringWidth = (activeMode ? 3.05f + 0.30f * activePresence : 1.04f + 0.08f * hoverPresence) * scale;
    if (height <= ringWidth * 2.0f) {
        return;
    }

    const ImVec2 innerMin(min.x + ringWidth, min.y + ringWidth);
    const ImVec2 innerMax(max.x - ringWidth, max.y - ringWidth);
    const float innerRounding = std::max(0.0f, rounding - ringWidth);
    const float rowStep = std::max(0.46f, 0.52f * scale);
    for (float y = min.y; y < max.y; y += rowStep) {
        const float y2 = std::min(y + rowStep + 0.12f * scale, max.y);
        const float cy = (y + y2) * 0.5f;
        const float yNorm = clamp01((cy - min.y) / height);
        const float outerInset = capsule_inset_at_y(min, max, rounding, cy);
        const float outerLeft = min.x + outerInset;
        const float outerRight = max.x - outerInset;
        if (outerRight <= outerLeft) {
            continue;
        }

        const bool hasInner = cy >= innerMin.y && cy <= innerMax.y;
        float innerLeft = outerRight;
        float innerRight = outerLeft;
        if (hasInner) {
            const float innerInset = capsule_inset_at_y(innerMin, innerMax, innerRounding, cy);
            innerLeft = innerMin.x + innerInset;
            innerRight = innerMax.x - innerInset;
        }

        auto drawSegment = [&](float x0, float x1) {
            if (x1 <= x0 + 0.2f) {
                return;
            }
            const ImVec2 sampleA(x0, cy);
            const ImVec2 sampleB(x1, cy);
            ImVec4 left = neon_capsule_css_button_color(style, sampleA, min, max, phase + yNorm * 0.025f, activeMode, true);
            ImVec4 right = neon_capsule_css_button_color(style, sampleB, min, max, phase + yNorm * 0.025f, activeMode, true);
            const float top = 1.0f - smoothstep01(0.0f, 0.34f, yNorm);
            const float bottom = smoothstep01(0.50f, 1.0f, yNorm);
            const float xMid = clamp01((((x0 + x1) * 0.5f) - min.x) / std::max(1.0f, max.x - min.x));
            const float topRightTube = std::clamp(std::max(smoothstep01(0.58f, 1.0f, xMid) * 0.96f,
                                                           top * smoothstep01(0.28f, 1.0f, xMid)) +
                                                      0.18f * top * smoothstep01(0.58f, 1.0f, xMid),
                                                  0.0f,
                                                  1.0f);
            const float violetTube = std::clamp(std::max((1.0f - smoothstep01(0.0f, 0.34f, xMid)) *
                                                             (0.36f + 0.48f * bottom),
                                                         bottom * (1.0f - 0.58f * smoothstep01(0.58f, 1.0f, xMid))),
                                                0.0f,
                                                1.0f);
            float alpha = activeMode
                ? (0.72f + 0.50f * topRightTube + 0.34f * violetTube + 0.10f * activePresence)
                : (0.34f + 0.10f * hoverPresence);
            if (!activeMode) {
                alpha *= 1.0f - 0.38f * top + 0.34f * bottom;
            }
            left.w = std::clamp(alpha * presence, 0.0f, activeMode ? 1.0f : 0.42f);
            right.w = std::clamp(alpha * presence, 0.0f, activeMode ? 1.0f : 0.44f);
            drawList->AddRectFilledMultiColor(ImVec2(x0, y),
                                              ImVec2(x1, y2),
                                              to_u32(left),
                                              to_u32(right),
                                              to_u32(right),
                                              to_u32(left));
        };

        if (!hasInner || innerLeft >= innerRight) {
            drawSegment(outerLeft, outerRight);
        } else {
            drawSegment(outerLeft, innerLeft);
            drawSegment(innerRight, outerRight);
            if (cy < innerMin.y + 0.2f || cy > innerMax.y - 0.2f) {
                drawSegment(innerLeft, innerRight);
            }
        }
    }

    const std::vector<ImVec2> outline = make_neon_capsule_outline(min, max, rounding, scale);
    const int segmentCount = static_cast<int>(outline.size()) - 1;
    if (segmentCount <= 0) {
        return;
    }

    const float crispThickness = (activeMode ? 1.48f : 0.58f) * scale;
    for (int i = 0; i < segmentCount; ++i) {
        const ImVec2 mid((outline[i].x + outline[i + 1].x) * 0.5f,
                         (outline[i].y + outline[i + 1].y) * 0.5f);
        const float x = clamp01((mid.x - min.x) / std::max(1.0f, max.x - min.x));
        const float y = clamp01((mid.y - min.y) / std::max(1.0f, max.y - min.y));
        const float top = 1.0f - smoothstep01(0.0f, 0.34f, y);
        const float bottom = smoothstep01(0.50f, 1.0f, y);
        const float right = smoothstep01(0.58f, 1.0f, x);
        const float left = 1.0f - smoothstep01(0.0f, 0.34f, x);
        const float topRightTube = std::clamp(std::max(right * 0.96f, top * smoothstep01(0.28f, 1.0f, x)) +
                                                  0.18f * top * right,
                                              0.0f,
                                              1.0f);
        const float violetTube = std::clamp(std::max(left * (0.36f + 0.48f * bottom),
                                                     bottom * (1.0f - 0.58f * right)),
                                            0.0f,
                                            1.0f);
        ImVec4 color = neon_capsule_css_button_color(style, mid, min, max, phase + static_cast<float>(i) * 0.0009f, activeMode, true);
        color.w = activeMode
            ? (0.72f + 0.44f * topRightTube + 0.30f * violetTube + 0.10f * activePresence)
            : (0.30f + 0.08f * hoverPresence);
        drawList->AddLine(outline[i], outline[i + 1], to_u32(color), crispThickness);

        if (activeMode) {
            const float hotTube = std::clamp(std::max(topRightTube * 1.04f, violetTube * 0.68f), 0.0f, 1.0f);
            if (hotTube > 0.035f) {
                ImVec4 hotCore = mix_color(color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.58f + 0.16f * topRightTube);
                hotCore.w = std::clamp((0.16f + 0.42f * topRightTube + 0.20f * violetTube) *
                                            activePresence,
                                        0.0f,
                                        0.78f);
                drawList->AddLine(outline[i],
                                  outline[i + 1],
                                  to_u32(hotCore),
                                  std::max(0.92f * scale, crispThickness * 0.58f));
            }
        }
    }

    if (activeMode) {
        const float innerInset = std::max(1.0f, 2.12f * scale);
        const ImVec2 innerMin(min.x + innerInset, min.y + innerInset);
        const ImVec2 innerMax(max.x - innerInset, max.y - innerInset);
        if (innerMax.x > innerMin.x && innerMax.y > innerMin.y) {
            const float innerRounding = std::max(0.0f, rounding - innerInset);
            const std::vector<ImVec2> innerOutline = make_neon_capsule_outline(innerMin, innerMax, innerRounding, scale);
            const int innerSegments = static_cast<int>(innerOutline.size()) - 1;
            for (int i = 0; i < innerSegments; ++i) {
                const ImVec2 mid((innerOutline[i].x + innerOutline[i + 1].x) * 0.5f,
                                 (innerOutline[i].y + innerOutline[i + 1].y) * 0.5f);
                const float x = clamp01((mid.x - min.x) / std::max(1.0f, max.x - min.x));
                const float y = clamp01((mid.y - min.y) / std::max(1.0f, max.y - min.y));
                const float top = 1.0f - smoothstep01(0.0f, 0.34f, y);
                const float right = smoothstep01(0.58f, 1.0f, x);
                const float bottom = smoothstep01(0.55f, 1.0f, y);
                const float left = 1.0f - smoothstep01(0.0f, 0.34f, x);
                ImVec4 innerColor = neon_capsule_css_button_color(style, mid, min, max, phase + static_cast<float>(i) * 0.0009f, true, true);
                innerColor = mix_color(innerColor, ImVec4(0.70f, 0.88f, 1.0f, 1.0f), 0.14f * right + 0.08f * top);
                innerColor.w = std::clamp((0.13f + 0.18f * right + 0.10f * top + 0.07f * bottom * left) *
                                              activePresence,
                                          0.0f,
                                          0.34f);
                drawList->AddLine(innerOutline[i], innerOutline[i + 1], to_u32(innerColor), 0.64f * scale);
            }
        }
    }
}

void draw_neon_capsule_css_button(ImDrawList* drawList,
                                  ImVec2 min,
                                  ImVec2 max,
                                  float rounding,
                                  const NeonCapsuleGlowStyle& style,
                                  float activePresence,
                                  float hoverPresence,
                                  float phase,
                                  float glowScale,
                                  float scale) {
    const float active = clamp01(activePresence);
    const float hover = clamp01(hoverPresence * (1.0f - active));
    if (active <= 0.002f && hover <= 0.002f) {
        return;
    }

    draw_neon_capsule_css_shadow(drawList, min, max, rounding, style, active, hover, phase, glowScale, scale);
    draw_neon_capsule_css_outline_glow(drawList, min, max, rounding, style, active, hover, phase, glowScale, scale);
    draw_neon_capsule_css_ring(drawList, min, max, rounding, style, active, hover, phase, scale);

    const float innerInset = (active > hover ? 3.28f : 1.70f) * scale;
    if (max.x > min.x + innerInset * 2.0f && max.y > min.y + innerInset * 2.0f) {
        draw_neon_capsule_css_inner_fill(drawList,
                                         ImVec2(min.x + innerInset, min.y + innerInset),
                                         ImVec2(max.x - innerInset, max.y - innerInset),
                                         std::max(0.0f, rounding - innerInset),
                                         active,
                                         hover,
                                         scale);
    }
}

} // namespace

void draw_glow_rect(ImDrawList* drawList,
                    ImVec2 min,
                    ImVec2 max,
    float rounding,
    const Theme& theme,
    float alpha) {
    const Palette& palette = theme.palette;
    const float pulseAlpha = alpha * animate(theme.motion.glowPulse);
    const ImU32 soft = to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, pulseAlpha * 0.35f));
    const ImU32 hard = to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, pulseAlpha));
    drawList->AddRect(min, max, soft, rounding, 0, 4.0f);
    drawList->AddRect(min, max, hard, rounding, 0, 1.0f);
}

void draw_flow_line(ImDrawList* drawList,
                    ImVec2 from,
                    ImVec2 to,
                    const Theme& theme,
                    float alpha) {
    const Palette& palette = theme.palette;
    const float len = to.x - from.x;
    if (std::fabs(len) < 1.0f) {
        return;
    }

    const float direction = len >= 0.0f ? 1.0f : -1.0f;
    const float absLen = std::fabs(len);
    const float sparkLen = std::min(absLen * 0.08f, 42.0f);
    const float travel = std::max(0.0f, absLen - sparkLen);
    const float phase = imgui_onguoin::travel(theme.motion.flowLine, std::max(1.0f, travel));
    const float startOffset = direction > 0.0f ? phase : (travel - phase);
    const float sparkStart = from.x + startOffset * direction;
    const float y = std::floor(from.y) + 0.5f;
    const ImU32 base = to_u32(
        ImVec4(palette.border.x, palette.border.y, palette.border.z, std::clamp(alpha * 0.95f, 0.0f, 0.22f)));
    drawList->AddLine(ImVec2(from.x, y), ImVec2(to.x, y), base, 1.0f);

    constexpr int kSegments = 10;
    for (int i = 0; i < kSegments; ++i) {
        const float t0 = static_cast<float>(i) / static_cast<float>(kSegments);
        const float t1 = static_cast<float>(i + 1) / static_cast<float>(kSegments);
        const float mid = (t0 + t1) * 0.5f;
        const float falloff = 1.0f - std::fabs(mid - 0.5f) * 2.0f;
        const float shaped = falloff * falloff;
        const float x0 = sparkStart + sparkLen * t0 * direction;
        const float x1 = sparkStart + sparkLen * t1 * direction;
        const float segmentAlpha = std::clamp(alpha * 0.95f * shaped, 0.0f, 0.18f);
        if (segmentAlpha <= 0.002f) {
            continue;
        }

        drawList->AddLine(ImVec2(x0, y),
                          ImVec2(x1, y),
                          to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, segmentAlpha)),
                          1.0f);
    }
}

void draw_static_glow_rect(ImDrawList* drawList,
                           ImVec2 min,
                           ImVec2 max,
                           float rounding,
                           const Theme& theme,
                           float alpha,
                           float thickness) {
    if (alpha <= 0.0f) {
        return;
    }

    const Palette& palette = theme.palette;
    const float scale = current_scale();
    for (int layer = 8; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 8.0f;
        const float feather = 1.0f - t;
        const float expand = (0.65f + t * 11.0f) * scale;
        const float layerAlpha = alpha * (0.008f + 0.125f * feather * feather);
        drawList->AddRect(ImVec2(min.x - expand, min.y - expand),
                          ImVec2(max.x + expand, max.y + expand),
                          to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, layerAlpha)),
                          rounding + expand,
                          0,
                          std::max(1.0f, thickness * (0.55f + t * 1.35f)));
    }

    for (int layer = 0; layer < 3; ++layer) {
        const float inset = (1.0f + static_cast<float>(layer) * 1.25f) * scale;
        if (max.x - min.x <= inset * 2.0f || max.y - min.y <= inset * 2.0f) {
            continue;
        }
        const float layerAlpha = alpha * (0.070f - static_cast<float>(layer) * 0.018f);
        drawList->AddRect(ImVec2(min.x + inset, min.y + inset),
                          ImVec2(max.x - inset, max.y - inset),
                          to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, layerAlpha)),
                          std::max(0.0f, rounding - inset),
                          0,
                          std::max(1.0f, thickness * (0.55f + static_cast<float>(layer) * 0.20f)));
    }
}

void draw_neon_capsule_glow(ImDrawList* drawList,
                            ImVec2 min,
                            ImVec2 max,
                            float rounding,
                            NeonCapsuleGlowStyle style,
                            float hoverProgress,
                            float activationProgress) {
    if (drawList == nullptr) {
        return;
    }

    const float scale = current_scale();
    const float hover = clamp01(hoverProgress);
    const float active = ease_out_cubic(clamp01(activationProgress));
    const float activePresence = std::clamp(active * style.intensity, 0.0f, 1.0f);
    const float hoverPresence = std::clamp(hover * (1.0f - active) * style.intensity, 0.0f, 1.0f);
    const float bodyPresence = std::clamp((0.030f * hover * (1.0f - active) + 0.86f * active) * style.intensity,
                                           0.0f,
                                           1.0f);
    const float exteriorGlowPresence = std::clamp((0.030f * hover * (1.0f - active) + 0.185f * active) * style.intensity,
                                                   0.0f,
                                                   0.26f);
    if (bodyPresence <= 0.002f && exteriorGlowPresence <= 0.002f) {
        return;
    }

    const float glowScale = std::max(0.25f, style.glowScale);
    const float time = static_cast<float>(ImGui::GetTime());
    const float travel = std::fmod(time * std::max(0.02f, style.speed), 1.0f);
    draw_neon_capsule_css_button(drawList,
                                 min,
                                 max,
                                 rounding,
                                 style,
                                 activePresence,
                                 hoverPresence,
                                 travel,
                                 glowScale,
                                 scale);
    return;

    const std::vector<ImVec2> outline = make_neon_capsule_outline(min, max, rounding, scale);
    const bool hasOutline = outline.size() >= 2;

    draw_neon_capsule_ambient_aura(drawList,
                                   min,
                                   max,
                                   rounding,
                                   style,
                                   activePresence,
                                   hoverPresence,
                                   travel,
                                   glowScale,
                                   scale);

    if (hasOutline && exteriorGlowPresence > 0.002f) {
        draw_neon_capsule_css_halo(drawList,
                                   min,
                                   max,
                                   rounding,
                                   style,
                                   std::clamp(activePresence * 0.20f + hoverPresence * 0.16f, 0.0f, 0.28f),
                                   travel,
                                   glowScale,
                                   scale);
        draw_neon_capsule_css_outline(drawList,
                                      outline,
                                      min,
                                      max,
                                      style,
                                      activePresence * 0.20f,
                                      hoverPresence * 0.26f,
                                      travel,
                                      glowScale,
                                      scale,
                                      false);
    }
    draw_neon_capsule_row_blur(drawList,
                               min,
                               max,
                               rounding,
                               style,
                               hoverPresence * 1.28f,
                               travel,
                               glowScale * 1.52f,
                               scale,
                               true);
    draw_neon_capsule_sdf_glow(drawList,
                               min,
                               max,
                               rounding,
                               style,
                               activePresence * 0.56f,
                               hoverPresence * 0.50f,
                               travel,
                               glowScale * 0.48f,
                               scale);
    draw_neon_capsule_css_shell(drawList,
                                min,
                                max,
                                rounding,
                                style,
                                activePresence,
                                hoverPresence,
                                travel,
                                scale);
    draw_neon_capsule_border_band(drawList,
                                  min,
                                  max,
                                  rounding,
                                  style,
                                  activePresence * 0.68f,
                                  hoverPresence * 0.30f,
                                  travel,
                                  scale);

    draw_neon_capsule_inner_volume(drawList,
                                   min,
                                   max,
                                   rounding,
                                   style,
                                   bodyPresence,
                                   travel,
                                   scale);
    draw_neon_capsule_core_depth(drawList,
                                 min,
                                 max,
                                 rounding,
                                 bodyPresence,
                                 scale);
    draw_neon_capsule_center_volume(drawList,
                                    min,
                                    max,
                                    rounding,
                                    style,
                                    bodyPresence,
                                    travel,
                                    scale);
    draw_neon_capsule_side_cap_volume(drawList,
                                      min,
                                      max,
                                      rounding,
                                      style,
                                      activePresence,
                                      hoverPresence,
                                      travel,
                                      scale);
    draw_neon_capsule_inner_rim(drawList,
                                 min,
                                 max,
                                 rounding,
                                 style,
                                 bodyPresence,
                                 travel,
                                 scale);
    const float rimPresence = std::max(bodyPresence * 0.76f, hoverPresence * 0.12f);
    draw_neon_capsule_horizontal_rim(drawList,
                                     min,
                                     max,
                                     rounding,
                                     style,
                                     rimPresence,
                                     travel,
                                     scale);
    draw_neon_capsule_inner_css_glow(drawList,
                                     min,
                                     max,
                                     rounding,
                                     style,
                                     activePresence,
                                     hoverPresence,
                                     travel,
                                     scale);

    if (!hasOutline) {
        return;
    }

    draw_neon_capsule_css_outline(drawList,
                                  outline,
                                  min,
                                  max,
                                  style,
                                  activePresence,
                                  hoverPresence,
                                  travel,
                                  glowScale,
                                  scale,
                                  true);
    draw_neon_capsule_web_core_rim(drawList,
                                   outline,
                                   min,
                                   max,
                                   style,
                                   activePresence,
                                   hoverPresence,
                                   travel,
                                   scale);
    draw_neon_capsule_hover_top_sheen(drawList,
                                      min,
                                      max,
                                      rounding,
                                      style,
                                      activePresence,
                                      hoverPresence,
                                      travel,
                                      scale);
}

void draw_top_highlight(ImDrawList* drawList,
                        ImVec2 min,
                        ImVec2 max,
                        float rounding,
                        float alpha,
                        float heightFraction) {
    if (alpha <= 0.0f || heightFraction <= 0.0f) {
        return;
    }

    const float scale = current_scale();
    const float inset = 1.0f * scale;
    drawList->AddRectFilled(ImVec2(min.x + inset, min.y + inset),
                            ImVec2(max.x - inset, min.y + (max.y - min.y) * heightFraction),
                            to_u32(ImVec4(1.0f, 1.0f, 1.0f, alpha)),
                            std::max(0.0f, rounding - inset),
                            ImDrawFlags_RoundCornersTop);
}

void draw_text_block(ImDrawList* drawList,
                     ImVec2 min,
                     ImVec2 padding,
                     const char* text,
                     ImU32 color,
                     float wrapWidth) {
    drawList->AddText(nullptr,
                      0.0f,
                      ImVec2(min.x + padding.x, min.y + padding.y),
                      color,
                      text != nullptr ? text : "",
                      nullptr,
                      wrapWidth > 0.0f ? wrapWidth : 0.0f);
}

void draw_accent_frame(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 max,
                       const Theme& theme,
                       const InteractionVisualState& state,
                       AccentFrameStyle style) {
    const float scale = current_scale();
    const float rounding = style.rounding > 0.0f ? style.rounding : theme.fields.frameRounding * scale;
    const float borderAlpha = emphasized_alpha(state,
                                               style.idleBorderAlpha,
                                               style.hoveredBorderAlpha,
                                               style.activeBorderAlpha,
                                               style.selectedBorderAlpha);
    const float glowAlpha = emphasized_alpha(state,
                                             style.idleGlowAlpha,
                                             style.hoveredGlowAlpha,
                                             style.activeGlowAlpha,
                                             style.selectedGlowAlpha);
    const float thickness = emphasized_thickness(state,
                                                 style.idleThickness,
                                                 style.activeThickness,
                                                 style.selectedThickness) * scale;
    const ImVec4 accentColor = style.color.x >= 0.0f
        ? style.color
        : theme.palette.accent;

    if (glowAlpha > 0.0f) {
        draw_static_glow_rect(drawList, min, max, rounding, theme, glowAlpha, thickness);
    }

    drawList->AddRect(min,
                      max,
                      to_u32(ImVec4(accentColor.x,
                                    accentColor.y,
                                    accentColor.z,
                                    borderAlpha)),
                      rounding,
                      0,
                      thickness);
}

void draw_ambient_glow(ImDrawList* drawList,
                       ImVec2 center,
                       float radius,
                       const Theme& theme,
                       float alphaScale) {
    const Palette& palette = theme.palette;
    for (int i = 4; i >= 1; --i) {
        const float scale = 1.0f + 0.26f * static_cast<float>(i);
        const float alpha = (0.015f + 0.010f * static_cast<float>(i)) * alphaScale / static_cast<float>(i);
        drawList->AddCircleFilled(center,
                                  radius * scale,
                                  to_u32(ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, alpha)),
                                  64);
    }
}

void draw_cursor_aura(ImDrawList* drawList,
                      ImVec2 center,
                      float radius,
                      const Theme& theme,
                      float alphaScale) {
    if (drawList == nullptr || radius <= 0.0f || alphaScale <= 0.0f) {
        return;
    }

    const Palette& palette = theme.palette;
    const ImVec4 cyan = mix_color(palette.accent, ImVec4(0.22f, 0.96f, 1.0f, 1.0f), 0.64f);
    for (int layer = 7; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 7.0f;
        const float shaped = t * t;
        const float layerRadius = radius * (0.22f + 0.88f * t);
        const float alpha = alphaScale * 0.030f * (1.0f - shaped * 0.72f);
        drawList->AddCircleFilled(center,
                                  layerRadius,
                                  to_u32(ImVec4(cyan.x, cyan.y, cyan.z, alpha)),
                                  72);
    }
}

void draw_circle_badge(ImDrawList* drawList,
                       ImVec2 center,
                       float radius,
                       ImU32 fillColor,
                       ImU32 borderColor,
                       int segments,
                       float borderThickness) {
    drawList->AddCircleFilled(center, radius, fillColor, segments);
    drawList->AddCircle(center, radius, borderColor, segments, borderThickness);
}

ImVec2 point_on_circle(ImVec2 center, float radius, float visualDegrees) {
    const float radians = visualDegrees * 3.1415926535f / 180.0f;
    return ImVec2(center.x + std::cos(radians) * radius,
                  center.y + std::sin(radians) * radius);
}

void draw_arc(ImDrawList* drawList,
              ImVec2 center,
              float radius,
              float startDegrees,
              float endDegrees,
              ImU32 color,
              float thickness,
              int segments) {
    drawList->PathClear();
    for (int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float degrees = startDegrees + (endDegrees - startDegrees) * t;
        drawList->PathLineTo(point_on_circle(center, radius, degrees));
    }
    drawList->PathStroke(color, 0, thickness);
}

void draw_wedge(ImDrawList* drawList,
                ImVec2 center,
                float innerRadius,
                float outerRadius,
                float startDegrees,
                float endDegrees,
                ImU32 color,
                int segments) {
    drawList->PathClear();
    for (int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float degrees = startDegrees + (endDegrees - startDegrees) * t;
        drawList->PathLineTo(point_on_circle(center, outerRadius, degrees));
    }
    for (int i = segments; i >= 0; --i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float degrees = startDegrees + (endDegrees - startDegrees) * t;
        drawList->PathLineTo(point_on_circle(center, innerRadius, degrees));
    }
    drawList->PathFillConvex(color);
}

void draw_capsule(ImDrawList* drawList,
                  ImVec2 min,
                  ImVec2 max,
                  const char* text,
                  const Theme& theme,
                  const InteractionVisualState& state,
                  CapsuleStyle style) {
    const float scale = current_scale();
    const float rounding = style.rounding > 0.0f ? style.rounding : theme.radii.control * scale;
    const ImVec4 fillColor = style.fill;
    drawList->AddRectFilled(min, max, to_u32(fillColor), rounding);
    draw_accent_frame(drawList, min, max, theme, state, style.frame);

    const ImVec2 textSize = measure_text_block(text).size;
    const ImVec4 resolvedTextColor = style.textColor.x >= 0.0f ? style.textColor : theme.palette.text;
    drawList->AddText(centered_text_pos(min, max, textSize, style.textOffset),
                      to_u32(resolvedTextColor),
                      text != nullptr ? text : "");
}

void draw_interactive_surface(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const char* text,
                              const Theme& theme,
                              const InteractionVisualState& state,
                              InteractiveSurfaceStyle style) {
    const float scale = current_scale();
    const float rounding = style.rounding > 0.0f ? style.rounding : theme.radii.button * scale;
    drawList->AddRectFilled(min, max, to_u32(style.fill), rounding);

    if (style.topHighlightAlpha > 0.0f) {
        draw_top_highlight(drawList,
                           min,
                           max,
                           rounding,
                           style.topHighlightAlpha,
                           style.topHighlightHeightFraction);
    }

    draw_accent_frame(drawList, min, max, theme, state, style.frame);

    if (style.topLineColor.x >= 0.0f && style.topLineColor.w > 0.0f && style.topLineInset > 0.0f) {
        drawList->AddLine(ImVec2(min.x + style.topLineInset * scale, min.y + style.topLineOffsetY * scale),
                          ImVec2(max.x - style.topLineInset * scale, min.y + style.topLineOffsetY * scale),
                          to_u32(style.topLineColor),
                          1.0f * scale);
    }

    if (style.innerPulseAlpha > 0.0f && style.innerPulseColor.x >= 0.0f) {
        drawList->AddRect(ImVec2(min.x + 1.0f * scale, min.y + 1.0f * scale),
                          ImVec2(max.x - 1.0f * scale, max.y - 1.0f * scale),
                          to_u32(ImVec4(style.innerPulseColor.x,
                                        style.innerPulseColor.y,
                                        style.innerPulseColor.z,
                                        style.innerPulseAlpha)),
                          std::max(0.0f, rounding - 1.0f * scale),
                          0,
                          1.0f * scale);
    }

    const ImVec2 textSize = measure_text_block(text).size;
    const ImVec4 textColor = style.textColor.x >= 0.0f ? style.textColor : theme.palette.text;
    drawList->AddText(centered_text_pos(min, max, textSize, style.textOffset),
                      to_u32(textColor),
                      text != nullptr ? text : "");
}

void draw_notice_surface(ImDrawList* drawList,
                         ImVec2 min,
                         ImVec2 max,
                         const char* text,
                         float wrapWidth,
                         NoticeSurfaceStyle style) {
    drawList->AddRectFilled(min, max, to_u32(style.fill), style.rounding);
    drawList->AddRect(min, max, to_u32(style.border), style.rounding);
    draw_text_block(drawList,
                    min,
                    style.padding,
                    text,
                    to_u32(style.textColor),
                    wrapWidth);
}

void draw_status_pill_surface(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 max,
                              const char* label,
                              const char* value,
                              StatusPillStyle style) {
    drawList->AddRectFilled(min, max, to_u32(style.fill), style.rounding);
    drawList->AddRectFilled(ImVec2(min.x, min.y + style.indicatorInsetY),
                            ImVec2(min.x + style.indicatorWidth, max.y - style.indicatorInsetY),
                            to_u32(style.indicatorColor),
                            1.0f);
    drawList->AddLine(ImVec2(min.x + style.baselineInsetStart, max.y - 1.0f),
                      ImVec2(max.x - style.baselineInsetEnd, max.y - 1.0f),
                      to_u32(style.baselineColor),
                      1.0f);

    const ImVec2 labelSize = measure_text_block(label).size;
    const ImVec2 valueSize = measure_text_block(value).size;
    drawList->AddText(leading_text_pos(min, max, labelSize, style.labelLead),
                      to_u32(style.labelColor),
                      label != nullptr ? label : "");
    drawList->AddText(trailing_text_pos(min, max, valueSize, style.valueInset),
                      to_u32(style.valueColor),
                      value != nullptr ? value : "");
}

void draw_select_frame(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 max,
                       bool hovered,
                       bool open,
                       const Theme& theme,
                       float rounding) {
    InteractionVisualState state;
    state.hovered = hovered;
    state.active = open;

    const float focusPulse = open ? animate(theme.motion.focusPulse) : 0.0f;
    AccentFrameStyle style;
    style.rounding = rounding;
    style.color = theme.palette.accent;
    style.idleBorderAlpha = theme.fields.selectIdleBorderAlpha;
    style.hoveredBorderAlpha = theme.fields.selectHoverBorderAlpha;
    style.activeBorderAlpha = theme.fields.selectOpenBorderAlpha + theme.fields.selectOpenBorderPulse * focusPulse;
    style.idleGlowAlpha = 0.0f;
    style.hoveredGlowAlpha = theme.fields.selectHoverGlowAlpha;
    style.activeGlowAlpha = theme.fields.selectOpenGlowAlpha;
    style.idleThickness = theme.fields.selectIdleBorderThickness;
    style.activeThickness = theme.fields.selectOpenBorderThickness;

    draw_accent_frame(drawList, min, max, theme, state, style);
}

} // namespace imgui_onguoin

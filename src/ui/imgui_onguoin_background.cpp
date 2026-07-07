// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#include <algorithm>
#include <cmath>

namespace imgui_onguoin {

namespace {

struct BackgroundRenderContext {
    ImDrawList* drawList = nullptr;
    ImVec2 min{};
    ImVec2 max{};
    const Theme* theme = nullptr;
    const BackgroundStyle* background = nullptr;
    BackgroundLayerMask enabledLayers = 0u;
    bool authenticated = false;
    float authenticatedAccentBoost = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

using BackgroundLayerRenderFn = void (*)(const BackgroundRenderContext&);

struct BackgroundLayerDescriptor {
    BackgroundLayerKind kind = BackgroundLayerKind::Gradient;
    BackgroundLayerRenderFn render = nullptr;
};

void draw_gradient_layer(ImDrawList* drawList,
                        ImVec2 min,
                        ImVec2 max,
                        const BackgroundGradientLayer& gradient) {
    if (!gradient.enabled || gradient.bandCount <= 0) {
        return;
    }

    const float height = max.y - min.y;
    const int bandCount = std::max(1, gradient.bandCount);
    for (int band = 0; band < bandCount; ++band) {
        const float y0 = min.y + height * static_cast<float>(band) / static_cast<float>(bandCount);
        const float y1 = min.y + height * static_cast<float>(band + 1) / static_cast<float>(bandCount) + 1.0f;
        const float u = bandCount == 1 ? 0.0f : static_cast<float>(band) / static_cast<float>(bandCount - 1);
        const float eased = u * u * (3.0f - 2.0f * u);
        const ImVec4 base = eased < 0.52f
            ? mix_color(gradient.topColor, gradient.midColor, eased / 0.52f)
            : mix_color(gradient.midColor, gradient.bottomColor, (eased - 0.52f) / 0.48f);
        drawList->AddRectFilled(ImVec2(min.x, y0), ImVec2(max.x, y1), to_u32(base));
    }
}

void draw_guide_layer(ImDrawList* drawList,
                      ImVec2 min,
                      ImVec2 max,
                      const Theme& theme,
                      const BackgroundGuideLayer& guides,
                      float authenticatedAccentBoost) {
    if (!guides.enabled || guides.lineCount <= 0) {
        return;
    }

    const float height = max.y - min.y;
    for (int i = 0; i < guides.lineCount; ++i) {
        const float y = min.y + height * (guides.topStart + guides.verticalStep * static_cast<float>(i));
        const float alpha = std::max(0.0f,
                                     guides.alpha -
                                         guides.falloffPerLine * static_cast<float>(i) +
                                         authenticatedAccentBoost * guides.authenticatedAlphaScale);
        drawList->AddLine(ImVec2(min.x + guides.horizontalInset, y),
                          ImVec2(max.x - guides.horizontalInset, y),
                          to_u32(ImVec4(theme.palette.border.x, theme.palette.border.y, theme.palette.border.z, alpha)),
                          1.0f);
    }
}

void draw_beam_layer(ImDrawList* drawList,
                     ImVec2 min,
                     ImVec2 max,
                     const Theme& theme,
                     const BackgroundBeamLayer& beamLayer,
                     bool authenticated) {
    if (!beamLayer.enabled || beamLayer.alpha <= 0.0f) {
        return;
    }

    const float width = max.x - min.x;
    const float sweep = travel(beamLayer.sweepMotion, width + beamLayer.travelPadding) - 300.0f;
    const ImVec2 beamPolygon[] = {
        ImVec2(min.x + sweep - beamLayer.width * 0.62f, min.y - beamLayer.overflow),
        ImVec2(min.x + sweep + beamLayer.width * 0.38f, min.y - beamLayer.overflow),
        ImVec2(min.x + sweep + beamLayer.shear + beamLayer.width * 1.20f, max.y + beamLayer.overflow),
        ImVec2(min.x + sweep + beamLayer.shear + beamLayer.width * 0.20f, max.y + beamLayer.overflow),
    };
    const float alpha = authenticated
        ? beamLayer.alpha
        : beamLayer.alpha * beamLayer.unauthenticatedAlphaScale;
    drawList->AddConvexPolyFilled(beamPolygon,
                                  4,
                                  to_u32(ImVec4(theme.palette.accent.x,
                                                theme.palette.accent.y,
                                                theme.palette.accent.z,
                                                alpha)));
}

void draw_grid_overlay(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 max,
                       const Theme& theme,
                       const BackgroundGridLayer& grid) {
    if (!grid.enabled || grid.alpha <= 0.0f || grid.spacing <= 1.0f) {
        return;
    }

    const float spacing = grid.spacing * current_scale();
    const ImU32 gridColor = to_u32(ImVec4(theme.palette.border.x,
                                          theme.palette.border.y,
                                          theme.palette.border.z,
                                          grid.alpha));
    for (float x = min.x; x <= max.x + spacing; x += spacing) {
        drawList->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), gridColor, 1.0f);
    }
    for (float y = min.y; y <= max.y + spacing; y += spacing) {
        drawList->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), gridColor, 1.0f);
    }
}

void draw_starfield_overlay(ImDrawList* drawList,
                            ImVec2 min,
                            ImVec2 max,
                            const Theme& theme,
                            const BackgroundStarFieldLayer& starField,
                            float authenticatedAccentBoost) {
    if (!starField.enabled || starField.pointCount <= 0 || starField.alpha <= 0.0f) {
        return;
    }

    const float width = max.x - min.x;
    const float height = max.y - min.y;
    const float baseAlpha = starField.alpha + authenticatedAccentBoost * starField.authenticatedAlphaScale;
    const float time = static_cast<float>(ImGui::GetTime());

    for (int i = 0; i < starField.pointCount; ++i) {
        const float seed = static_cast<float>(i + 1);
        const float xNorm = std::fmod(seed * 0.6180339887f, 1.0f);
        const float yNorm = std::fmod(seed * 0.4142135623f, 1.0f);
        const float radiusNorm = std::fmod(seed * 0.2795084972f, 1.0f);
        PulseMotionStyle twinkleMotion = starField.twinkleMotion;
        twinkleMotion.phase += seed * 0.73f;
        const float twinkle = animate(twinkleMotion, time);
        const float alpha = std::max(0.0f, baseAlpha * (0.55f + 0.45f * radiusNorm) * twinkle);
        const float radius = lerp(starField.minRadius, starField.maxRadius, radiusNorm) * current_scale();
        const ImVec2 center(min.x + xNorm * width, min.y + yNorm * height);
        drawList->AddCircleFilled(center,
                                  radius,
                                  to_u32(ImVec4(theme.palette.text.x,
                                                theme.palette.text.y,
                                                theme.palette.accent.z,
                                                alpha)),
                                  12);
    }
}

void render_gradient_background_layer(const BackgroundRenderContext& context) {
    draw_gradient_layer(context.drawList,
                        context.min,
                        context.max,
                        context.background->gradient);
}

void render_ambient_background_layer(const BackgroundRenderContext& context) {
    const BackgroundAmbientLayer& ambient = context.background->ambient;
    if (!ambient.enabled) {
        return;
    }

    auto drawAmbientSpot = [&](const BackgroundGlowSpot& spot) {
        if (spot.alpha <= 0.0f || spot.radius <= 0.0f) {
            return;
        }
        draw_ambient_glow(context.drawList,
                          ImVec2(context.min.x + context.width * spot.anchor.x,
                                 context.min.y + context.height * spot.anchor.y),
                          spot.radius,
                          *context.theme,
                          spot.alpha + context.authenticatedAccentBoost * spot.authenticatedBoostScale);
    };

    drawAmbientSpot(ambient.primary);
    drawAmbientSpot(ambient.secondary);
    drawAmbientSpot(ambient.tertiary);
}

void render_beam_background_layer(const BackgroundRenderContext& context) {
    draw_beam_layer(context.drawList,
                    context.min,
                    context.max,
                    *context.theme,
                    context.background->beam,
                    context.authenticated);
}

void render_grid_background_layer(const BackgroundRenderContext& context) {
    draw_grid_overlay(context.drawList,
                      context.min,
                      context.max,
                      *context.theme,
                      context.background->grid);
}

void render_starfield_background_layer(const BackgroundRenderContext& context) {
    draw_starfield_overlay(context.drawList,
                           context.min,
                           context.max,
                           *context.theme,
                           context.background->starField,
                           context.authenticatedAccentBoost);
}

void render_guides_background_layer(const BackgroundRenderContext& context) {
    draw_guide_layer(context.drawList,
                     context.min,
                     context.max,
                     *context.theme,
                     context.background->guides,
                     context.authenticatedAccentBoost);
}

void render_cursor_aura_background_layer(const BackgroundRenderContext& context) {
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse = io.MousePos;
    static ImVec2 auraCenter{};
    static bool auraInitialized = false;
    if (mouse.x < context.min.x || mouse.x > context.max.x ||
        mouse.y < context.min.y || mouse.y > context.max.y) {
        auraInitialized = false;
        return;
    }

    if (!auraInitialized) {
        auraCenter = mouse;
        auraInitialized = true;
    } else {
        const float follow = follow_factor(5.2f, io.DeltaTime, EasingCurve::OutCubic);
        auraCenter.x += (mouse.x - auraCenter.x) * follow;
        auraCenter.y += (mouse.y - auraCenter.y) * follow;
    }

    const float scale = current_scale();
    const float radius = (context.authenticated ? 178.0f : 142.0f) * scale;
    const float alpha = context.authenticated ? 0.80f : 0.58f;
    draw_cursor_aura(context.drawList, auraCenter, radius, *context.theme, alpha);
}

constexpr BackgroundLayerDescriptor kBackgroundLayerPipeline[] = {
    {BackgroundLayerKind::Gradient, &render_gradient_background_layer},
    {BackgroundLayerKind::Ambient, &render_ambient_background_layer},
    {BackgroundLayerKind::Beam, &render_beam_background_layer},
    {BackgroundLayerKind::Grid, &render_grid_background_layer},
    {BackgroundLayerKind::StarField, &render_starfield_background_layer},
    {BackgroundLayerKind::Guides, &render_guides_background_layer},
    {BackgroundLayerKind::CursorAura, &render_cursor_aura_background_layer},
};

} // namespace

void draw_app_background(ImDrawList* drawList,
                         ImVec2 min,
                         ImVec2 max,
                         bool authenticated,
                         const Theme& theme) {
    const BackgroundStyle& background = theme.background;
    BackgroundRenderContext context;
    context.drawList = drawList;
    context.min = min;
    context.max = max;
    context.theme = &theme;
    context.background = &background;
    context.enabledLayers = enabled_background_layers(background);
    context.authenticated = authenticated;
    context.authenticatedAccentBoost = authenticated ? background.authenticatedAccentBoost : 0.0f;
    context.width = max.x - min.x;
    context.height = max.y - min.y;

    for (const BackgroundLayerDescriptor& descriptor : kBackgroundLayerPipeline) {
        if (!background_layer_mask_has(context.enabledLayers, descriptor.kind)) {
            continue;
        }
        if (descriptor.render != nullptr) {
            descriptor.render(context);
        }
    }
}

} // namespace imgui_onguoin

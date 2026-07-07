// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#include <array>
#include <algorithm>
#include <cstring>

namespace imgui_onguoin {

namespace {

using ThemeRecipeFn = void (*)(ThemeSpec&);
using BackgroundRecipeFn = void (*)(BackgroundStyle&, const Palette&);

struct ThemeFlavorDescriptor {
    ThemeFlavorInfo info{};
    ThemeRecipeFn applyRecipe = nullptr;
};

struct BackgroundKindDescriptor {
    BackgroundKindInfo info{};
    BackgroundRecipeFn applyRecipe = nullptr;
};

struct ThemePresetDescriptor {
    ThemePresetInfo info{};
};

constexpr BackgroundLayerMask layer_mask(std::initializer_list<BackgroundLayerKind> kinds) {
    BackgroundLayerMask mask = 0u;
    for (BackgroundLayerKind kind : kinds) {
        mask |= background_layer_flag(kind);
    }
    return mask;
}

void apply_aurora_background_recipe(BackgroundStyle& style, const Palette& palette);
void apply_gradient_bands_background_recipe(BackgroundStyle& style, const Palette& palette);
void apply_grid_background_recipe(BackgroundStyle& style, const Palette& palette);
void apply_flat_background_recipe(BackgroundStyle& style, const Palette& palette);
void apply_starfield_background_recipe(BackgroundStyle& style, const Palette& palette);

void apply_onguoin_theme_recipe(ThemeSpec& spec);
void apply_graphite_theme_recipe(ThemeSpec& spec);
void apply_spectrum_theme_recipe(ThemeSpec& spec);

template <std::size_t N>
constexpr std::array<ThemeFlavorInfo, N> build_theme_flavor_infos(const ThemeFlavorDescriptor (&descriptors)[N]) {
    std::array<ThemeFlavorInfo, N> infos{};
    for (std::size_t i = 0; i < N; ++i) {
        infos[i] = descriptors[i].info;
    }
    return infos;
}

template <std::size_t N>
constexpr std::array<BackgroundKindInfo, N> build_background_kind_infos(
    const BackgroundKindDescriptor (&descriptors)[N]) {
    std::array<BackgroundKindInfo, N> infos{};
    for (std::size_t i = 0; i < N; ++i) {
        infos[i] = descriptors[i].info;
    }
    return infos;
}

constexpr BackgroundLayerInfo kBackgroundLayerInfos[] = {
    {BackgroundLayerKind::Gradient, "gradient", "Gradient", "Gradient"},
    {BackgroundLayerKind::Ambient, "ambient", "Ambient Glow", "Ambient Glow"},
    {BackgroundLayerKind::Beam, "beam", "Light Beam", "Light Beam"},
    {BackgroundLayerKind::Grid, "grid", "Grid", "Grid"},
    {BackgroundLayerKind::Guides, "guides", "Guides", "Guides"},
    {BackgroundLayerKind::StarField, "starfield", "Star Field", "Star Field"},
    {BackgroundLayerKind::CursorAura, "cursor_aura", "Cursor Aura", "Cursor Aura"},
};

constexpr BackgroundKindDescriptor kBackgroundKindDescriptors[] = {
    {{BackgroundKind::Aurora,
      "aurora",
      "Aurora",
      "Aurora",
      layer_mask({BackgroundLayerKind::Gradient,
                  BackgroundLayerKind::Ambient,
                  BackgroundLayerKind::Beam,
                  BackgroundLayerKind::Guides,
                  BackgroundLayerKind::CursorAura}),
      layer_mask({BackgroundLayerKind::Beam,
                  BackgroundLayerKind::CursorAura}),
      true,
      BackgroundComplexity::Rich,
      BackgroundMotionCharacter::Expressive,
      "Layered glow with a traveling accent beam",
      "Layered glow with a traveling accent beam"},
     &apply_aurora_background_recipe},
    {{BackgroundKind::GradientBands,
      "gradient_bands",
      "Gradient Bands",
      "Gradient Bands",
      layer_mask({BackgroundLayerKind::Gradient,
                  BackgroundLayerKind::Ambient,
                  BackgroundLayerKind::Guides,
                  BackgroundLayerKind::CursorAura}),
      layer_mask({BackgroundLayerKind::CursorAura}),
      true,
      BackgroundComplexity::Balanced,
      BackgroundMotionCharacter::Static,
      "Structured gradient with soft ambient depth",
      "Structured gradient with soft ambient depth"},
     &apply_gradient_bands_background_recipe},
    {{BackgroundKind::Grid,
      "grid",
      "Grid",
      "Grid",
      layer_mask({BackgroundLayerKind::Gradient,
                  BackgroundLayerKind::Ambient,
                  BackgroundLayerKind::Grid,
                  BackgroundLayerKind::Guides,
                  BackgroundLayerKind::CursorAura}),
      layer_mask({BackgroundLayerKind::CursorAura}),
      true,
      BackgroundComplexity::Balanced,
      BackgroundMotionCharacter::Static,
      "Technical grid overlay with restrained depth",
      "Technical grid overlay with restrained depth"},
     &apply_grid_background_recipe},
    {{BackgroundKind::Flat,
      "flat",
      "Flat",
      "Flat",
      layer_mask({BackgroundLayerKind::Gradient,
                  BackgroundLayerKind::CursorAura}),
      layer_mask({BackgroundLayerKind::CursorAura}),
      false,
      BackgroundComplexity::Minimal,
      BackgroundMotionCharacter::Static,
      "Minimal single-plane backdrop",
      "Minimal single-plane backdrop"},
     &apply_flat_background_recipe},
    {{BackgroundKind::Starfield,
      "starfield",
      "Starfield",
      "Starfield",
      layer_mask({BackgroundLayerKind::Gradient,
                  BackgroundLayerKind::Ambient,
                  BackgroundLayerKind::Guides,
                  BackgroundLayerKind::StarField,
                  BackgroundLayerKind::CursorAura}),
      layer_mask({BackgroundLayerKind::StarField,
                  BackgroundLayerKind::CursorAura}),
      true,
      BackgroundComplexity::Rich,
      BackgroundMotionCharacter::Gentle,
      "Atmospheric field with animated star twinkle",
      "Atmospheric field with animated star twinkle"},
     &apply_starfield_background_recipe},
};

constexpr ThemeFlavorDescriptor kThemeFlavorDescriptors[] = {
    {{ThemeFlavor::Onguoin,
      "onguoin",
      "Onguoin",
      "Onguoin",
      BackgroundKind::Aurora,
      "Clear neon control deck",
      "Clear neon control deck",
      false},
     &apply_onguoin_theme_recipe},
    {{ThemeFlavor::Graphite,
      "graphite",
      "Graphite",
      "Graphite",
      BackgroundKind::Grid,
      "Industrial low-glare shell",
      "Industrial low-glare shell",
      false},
     &apply_graphite_theme_recipe},
    {{ThemeFlavor::Spectrum,
      "spectrum",
      "Spectrum",
      "Spectrum",
      BackgroundKind::GradientBands,
      "Lively chroma-driven shell",
      "Lively chroma-driven shell",
      true},
     &apply_spectrum_theme_recipe},
};

constexpr auto kThemeFlavorInfos = build_theme_flavor_infos(kThemeFlavorDescriptors);
constexpr auto kBackgroundKindInfos = build_background_kind_infos(kBackgroundKindDescriptors);
constexpr ThemePresetDescriptor kThemePresetDescriptors[] = {
    {{ThemePreset::Onguoin, "onguoin", "Onguoin", "Onguoin", ThemeFlavor::Onguoin, BackgroundKind::Aurora}},
    {{ThemePreset::Graphite, "graphite", "Graphite", "Graphite", ThemeFlavor::Graphite, BackgroundKind::Grid}},
    {{ThemePreset::Spectrum, "spectrum", "Spectrum", "Spectrum", ThemeFlavor::Spectrum, BackgroundKind::GradientBands}},
};

template <std::size_t N>
constexpr std::array<ThemePresetInfo, N> build_theme_preset_infos(const ThemePresetDescriptor (&descriptors)[N]) {
    std::array<ThemePresetInfo, N> infos{};
    for (std::size_t i = 0; i < N; ++i) {
        infos[i] = descriptors[i].info;
    }
    return infos;
}

constexpr auto kThemePresetInfos = build_theme_preset_infos(kThemePresetDescriptors);

template <typename Info, std::size_t N, typename Key, typename Member>
const Info* find_info_by_key(const std::array<Info, N>& infos, Key key, Member member) {
    for (const Info& info : infos) {
        if (info.*member == key) {
            return &info;
        }
    }
    return nullptr;
}

template <typename Info, std::size_t N, typename Key, typename Member>
const Info* find_info_by_key(const Info (&infos)[N], Key key, Member member) {
    for (const Info& info : infos) {
        if (info.*member == key) {
            return &info;
        }
    }
    return nullptr;
}

template <typename Descriptor, std::size_t N, typename Key, typename InfoMember>
const Descriptor* find_descriptor_by_info_key(const Descriptor (&descriptors)[N],
                                              Key key,
                                              InfoMember infoMember) {
    for (const Descriptor& descriptor : descriptors) {
        if (descriptor.info.*infoMember == key) {
            return &descriptor;
        }
    }
    return nullptr;
}

template <typename Info>
const char* select_localized_text(UiTextLanguage language,
                                  const Info& info,
                                  const char* Info::*englishMember,
                                  const char* Info::*chineseMember) {
    const char* chinese = info.*chineseMember;
    if (language == UiTextLanguage::ChineseSimplified && chinese != nullptr && chinese[0] != '\0') {
        return chinese;
    }
    const char* english = info.*englishMember;
    return english != nullptr ? english : "";
}

template <typename Info, std::size_t N, typename Enum>
Enum parse_catalog_id(const char* value,
                      Enum fallback,
                      const Info (&infos)[N],
                      Enum Info::*enumMember) {
    if (value == nullptr || value[0] == '\0') {
        return fallback;
    }
    for (const Info& info : infos) {
        if (info.id != nullptr && std::strcmp(info.id, value) == 0) {
            return info.*enumMember;
        }
    }
    return fallback;
}

template <typename Info, std::size_t N, typename Enum>
Enum parse_catalog_id(const char* value,
                      Enum fallback,
                      const std::array<Info, N>& infos,
                      Enum Info::*enumMember) {
    if (value == nullptr || value[0] == '\0') {
        return fallback;
    }
    for (const Info& info : infos) {
        if (info.id != nullptr && std::strcmp(info.id, value) == 0) {
            return info.*enumMember;
        }
    }
    return fallback;
}

const ThemeFlavorDescriptor* find_theme_flavor_descriptor(ThemeFlavor flavor) {
    return find_descriptor_by_info_key(kThemeFlavorDescriptors, flavor, &ThemeFlavorInfo::flavor);
}

const BackgroundKindDescriptor* find_background_kind_descriptor(BackgroundKind kind) {
    return find_descriptor_by_info_key(kBackgroundKindDescriptors, kind, &BackgroundKindInfo::kind);
}

BackgroundStyle make_base_background_style(const Palette& palette) {
    BackgroundStyle style;
    style.gradient.enabled = true;
    style.gradient.bandCount = 28;
    style.gradient.topColor = mix_color(palette.background, palette.accentMuted, 0.18f);
    style.gradient.midColor = mix_color(palette.background, palette.panel, 0.45f);
    style.gradient.bottomColor = mix_color(palette.sidebar, ImVec4(0.0f, 0.0f, 0.0f, 1.0f), 0.20f);
    style.ambient.enabled = true;
    style.ambient.primary = BackgroundGlowSpot{ImVec2(0.20f, 0.17f), 170.0f, 0.11f, 1.0f};
    style.ambient.secondary = BackgroundGlowSpot{ImVec2(0.77f, 0.25f), 150.0f, 0.08f, 0.7f};
    style.ambient.tertiary = BackgroundGlowSpot{ImVec2(0.58f, 0.75f), 190.0f, 0.06f, 0.45f};
    style.beam.enabled = false;
    style.beam.alpha = 0.0f;
    style.beam.sweepMotion = TravelMotionStyle{18.0f, 0.0f};
    style.grid.enabled = false;
    style.grid.alpha = 0.0f;
    style.guides.enabled = true;
    style.guides.lineCount = 4;
    style.starField.enabled = false;
    style.starField.alpha = 0.0f;
    style.starField.twinkleMotion = PulseMotionStyle{0.90f, 0.82f, 1.18f, 0.0f, EasingCurve::InOutSine};
    return style;
}

float intensity_factor(BackgroundIntensity intensity, float subtle, float balanced, float vivid) {
    switch (intensity) {
    case BackgroundIntensity::Subtle:
        return subtle;
    case BackgroundIntensity::Vivid:
        return vivid;
    case BackgroundIntensity::Balanced:
    default:
        return balanced;
    }
}

void apply_aurora_background_recipe(BackgroundStyle& style, const Palette& palette) {
    (void)palette;
    style.gradient.bandCount = 28;
    style.guides.enabled = true;
    style.guides.lineCount = 4;
    style.beam.enabled = true;
    style.beam.alpha = 0.027f;
    style.grid.enabled = false;
    style.grid.spacing = 84.0f;
    style.grid.alpha = 0.016f;
    style.starField.enabled = false;
    style.starField.alpha = 0.0f;
}

void apply_gradient_bands_background_recipe(BackgroundStyle& style, const Palette& palette) {
    (void)palette;
    style.gradient.bandCount = 42;
    style.guides.enabled = true;
    style.guides.lineCount = 5;
    style.beam.enabled = false;
    style.beam.alpha = 0.0f;
    style.grid.enabled = false;
    style.grid.alpha = 0.0f;
    style.ambient.primary.alpha = 0.08f;
    style.ambient.secondary.alpha = 0.06f;
    style.ambient.tertiary.alpha = 0.05f;
    style.starField.enabled = false;
    style.starField.alpha = 0.0f;
}

void apply_grid_background_recipe(BackgroundStyle& style, const Palette& palette) {
    (void)palette;
    style.gradient.bandCount = 20;
    style.guides.enabled = true;
    style.guides.lineCount = 3;
    style.beam.enabled = false;
    style.beam.alpha = 0.0f;
    style.grid.enabled = true;
    style.grid.spacing = 68.0f;
    style.grid.alpha = 0.030f;
    style.ambient.primary.alpha = 0.06f;
    style.ambient.secondary.alpha = 0.04f;
    style.ambient.tertiary.alpha = 0.03f;
    style.starField.enabled = false;
    style.starField.alpha = 0.0f;
}

void apply_flat_background_recipe(BackgroundStyle& style, const Palette& palette) {
    style.gradient.enabled = true;
    style.gradient.bandCount = 1;
    style.guides.enabled = false;
    style.guides.lineCount = 0;
    style.beam.enabled = false;
    style.beam.alpha = 0.0f;
    style.grid.enabled = false;
    style.grid.alpha = 0.0f;
    style.ambient.enabled = false;
    style.ambient.primary.alpha = 0.0f;
    style.ambient.secondary.alpha = 0.0f;
    style.ambient.tertiary.alpha = 0.0f;
    style.gradient.topColor = palette.background;
    style.gradient.midColor = palette.background;
    style.gradient.bottomColor = mix_color(palette.background, palette.sidebar, 0.35f);
    style.starField.enabled = false;
    style.starField.alpha = 0.0f;
}

void apply_starfield_background_recipe(BackgroundStyle& style, const Palette& palette) {
    style.gradient.enabled = true;
    style.gradient.bandCount = 24;
    style.gradient.topColor = mix_color(palette.background, palette.accentMuted, 0.12f);
    style.gradient.midColor = mix_color(palette.background, palette.panel, 0.26f);
    style.gradient.bottomColor = mix_color(palette.sidebar, palette.background, 0.44f);
    style.guides.enabled = true;
    style.guides.lineCount = 2;
    style.guides.alpha = 0.016f;
    style.guides.topStart = 0.22f;
    style.guides.verticalStep = 0.26f;
    style.beam.enabled = false;
    style.beam.alpha = 0.0f;
    style.grid.enabled = false;
    style.grid.alpha = 0.0f;
    style.ambient.enabled = true;
    style.ambient.primary = BackgroundGlowSpot{ImVec2(0.24f, 0.18f), 150.0f, 0.07f, 0.80f};
    style.ambient.secondary = BackgroundGlowSpot{ImVec2(0.78f, 0.28f), 130.0f, 0.05f, 0.60f};
    style.ambient.tertiary = BackgroundGlowSpot{ImVec2(0.60f, 0.74f), 165.0f, 0.04f, 0.40f};
    style.starField.enabled = true;
    style.starField.pointCount = 78;
    style.starField.minRadius = 0.8f;
    style.starField.maxRadius = 1.7f;
    style.starField.alpha = 0.16f;
    style.starField.twinkleMotion = PulseMotionStyle{0.75f, 0.78f, 1.22f, 0.0f, EasingCurve::InOutSine};
}

SurfaceTokens make_surface_tokens(const Palette& palette) {
    SurfaceTokens tokens;
    tokens.frameBg = mix_color(palette.background, palette.surfaceRaised, 0.55f);
    tokens.frameBgHovered = mix_color(tokens.frameBg, palette.surfaceRaised, 0.32f);
    tokens.frameBgActive = palette.surfaceRaised;
    tokens.buttonBg = mix_color(palette.background, palette.surface, 0.60f);
    tokens.buttonBgHovered = mix_color(tokens.buttonBg, palette.surfaceRaised, 0.34f);
    tokens.buttonBgActive = palette.surfaceRaised;
    tokens.headerBg = palette.surface;
    tokens.headerBgHovered = mix_color(palette.surface, palette.surfaceRaised, 0.34f);
    tokens.headerBgActive = palette.surfaceRaised;
    tokens.scrollbarBg = ImVec4(0.012f, 0.014f, 0.020f, 0.55f);
    tokens.scrollbarGrab = mix_color(palette.surface, palette.accentMuted, 0.48f);
    tokens.scrollbarGrabHovered = mix_color(tokens.scrollbarGrab, palette.accent, 0.20f);
    tokens.dockOuter = mix_color(palette.background, palette.panel, 0.35f);
    tokens.dockOuter.w = 0.98f;
    tokens.dockInner = mix_color(palette.background, palette.surface, 0.50f);
    tokens.dockInner.w = 0.72f;
    tokens.statusPillBg = ImVec4(0.010f, 0.014f, 0.021f, 0.34f);
    return tokens;
}

FieldTokens make_field_tokens(const Palette& palette) {
    FieldTokens tokens;
    tokens.frameBg = mix_color(palette.background, palette.surfaceRaised, 0.40f);
    tokens.frameBgHovered = mix_color(tokens.frameBg, palette.surfaceRaised, 0.26f);
    tokens.frameBgActive = mix_color(palette.surfaceRaised, palette.accentMuted, 0.14f);
    tokens.sliderGrab = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.72f);
    tokens.sliderGrabActive = mix_color(palette.accent, ImVec4(0.280f, 0.960f, 1.000f, 1.0f), 0.42f);
    tokens.rangeTrack = ImVec4(palette.border.x, palette.border.y, palette.border.z, 0.22f);
    tokens.rangeFill = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.72f);
    tokens.rangeHandleRing = ImVec4(0.280f, 0.960f, 1.000f, 1.0f);
    return tokens;
}

void assign_notice_tone_styles(ThemeSpec& spec) {
    spec.noticeInfo = NoticeToneStyle{
        ImVec4(0.120f, 0.260f, 0.420f, 0.38f),
        ImVec4(0.350f, 0.620f, 0.980f, 0.54f),
        spec.palette.text,
    };
    spec.noticeSuccess = NoticeToneStyle{
        ImVec4(0.120f, 0.360f, 0.220f, 0.34f),
        ImVec4(0.320f, 0.760f, 0.480f, 0.52f),
        spec.palette.success,
    };
    spec.noticeWarning = NoticeToneStyle{
        ImVec4(0.430f, 0.290f, 0.100f, 0.32f),
        ImVec4(0.920f, 0.650f, 0.220f, 0.48f),
        spec.palette.warning,
    };
    spec.noticeDanger = NoticeToneStyle{
        ImVec4(0.460f, 0.140f, 0.150f, 0.34f),
        ImVec4(0.900f, 0.310f, 0.340f, 0.50f),
        spec.palette.danger,
    };
}

ThemeSpec make_base_theme_spec(const Palette& palette) {
    ThemeSpec spec;
    spec.palette = palette;
    spec.surfaces = make_surface_tokens(palette);
    spec.fields = make_field_tokens(palette);
    assign_notice_tone_styles(spec);
    return spec;
}

void apply_onguoin_theme_recipe(ThemeSpec& spec) {
    spec.motion.focusPulse.speed = 4.2f;
}

void apply_graphite_theme_recipe(ThemeSpec& spec) {
    spec.palette = Palette{
        ImVec4(0.040f, 0.044f, 0.052f, 1.0f),
        ImVec4(0.028f, 0.031f, 0.038f, 1.0f),
        ImVec4(0.052f, 0.058f, 0.068f, 1.0f),
        ImVec4(0.065f, 0.072f, 0.086f, 1.0f),
        ImVec4(0.090f, 0.100f, 0.118f, 1.0f),
        ImVec4(0.360f, 0.840f, 0.940f, 1.0f),
        ImVec4(0.120f, 0.280f, 0.340f, 1.0f),
        ImVec4(0.940f, 0.950f, 0.965f, 1.0f),
        ImVec4(0.600f, 0.640f, 0.700f, 1.0f),
        ImVec4(0.320f, 0.820f, 0.560f, 1.0f),
        ImVec4(0.980f, 0.690f, 0.340f, 1.0f),
        ImVec4(0.920f, 0.310f, 0.330f, 1.0f),
        ImVec4(0.780f, 0.910f, 0.990f, 0.080f),
    };
    spec.surfaces = make_surface_tokens(spec.palette);
    spec.widgets.action.primaryGlowHoverAlpha = 0.20f;
    spec.surfaces.frameBg = mix_color(spec.palette.background, spec.palette.surfaceRaised, 0.48f);
    spec.surfaces.frameBgHovered = mix_color(spec.surfaces.frameBg, spec.palette.surfaceRaised, 0.26f);
    spec.surfaces.buttonBg = mix_color(spec.palette.background, spec.palette.surface, 0.52f);
    spec.surfaces.buttonBgHovered = mix_color(spec.surfaces.buttonBg, spec.palette.surfaceRaised, 0.28f);
    spec.surfaces.dockOuter = mix_color(spec.palette.background, spec.palette.panel, 0.24f);
    spec.surfaces.dockInner = mix_color(spec.palette.background, spec.palette.surface, 0.38f);
    spec.surfaces.statusPillBg = ImVec4(0.012f, 0.015f, 0.021f, 0.30f);
    spec.fields = make_field_tokens(spec.palette);
    spec.noticeInfo.border = ImVec4(0.420f, 0.700f, 0.980f, 0.48f);
    spec.noticeInfo.fill = ImVec4(0.120f, 0.220f, 0.340f, 0.34f);
    spec.fields.rangeTrack = ImVec4(spec.palette.border.x, spec.palette.border.y, spec.palette.border.z, 0.18f);
    spec.fields.rangeHandleHaloAlpha = 0.20f;
}

void apply_spectrum_theme_recipe(ThemeSpec& spec) {
    spec.palette = Palette{
        ImVec4(0.020f, 0.024f, 0.046f, 1.0f),
        ImVec4(0.018f, 0.020f, 0.034f, 1.0f),
        ImVec4(0.042f, 0.032f, 0.072f, 1.0f),
        ImVec4(0.056f, 0.044f, 0.090f, 1.0f),
        ImVec4(0.086f, 0.072f, 0.132f, 1.0f),
        ImVec4(0.170f, 0.930f, 0.780f, 1.0f),
        ImVec4(0.130f, 0.180f, 0.360f, 1.0f),
        ImVec4(0.965f, 0.968f, 0.990f, 1.0f),
        ImVec4(0.640f, 0.650f, 0.760f, 1.0f),
        ImVec4(0.280f, 0.860f, 0.580f, 1.0f),
        ImVec4(1.000f, 0.620f, 0.280f, 1.0f),
        ImVec4(0.930f, 0.290f, 0.420f, 1.0f),
        ImVec4(0.720f, 0.860f, 1.000f, 0.100f),
    };
    spec.surfaces = make_surface_tokens(spec.palette);
    spec.motion.glowPulse.speed = 2.8f;
    spec.motion.primaryActionPulse.speed = 2.6f;
    spec.motion.togglePulse.speed = 2.6f;
    spec.surfaces.frameBg = mix_color(spec.palette.background, spec.palette.surfaceRaised, 0.62f);
    spec.surfaces.frameBgHovered = mix_color(spec.surfaces.frameBg, spec.palette.accentMuted, 0.22f);
    spec.surfaces.buttonBg = mix_color(spec.palette.background, spec.palette.surface, 0.66f);
    spec.surfaces.buttonBgHovered = mix_color(spec.surfaces.buttonBg, spec.palette.accentMuted, 0.24f);
    spec.surfaces.scrollbarGrab = mix_color(spec.palette.surface, spec.palette.accentMuted, 0.62f);
    spec.surfaces.scrollbarGrabHovered = mix_color(spec.surfaces.scrollbarGrab, spec.palette.accent, 0.28f);
    spec.surfaces.statusPillBg = ImVec4(0.020f, 0.018f, 0.040f, 0.36f);
    spec.surfaces.buttonTopHighlightAlpha = 0.040f;
    spec.fields = make_field_tokens(spec.palette);
    spec.fields.frameBg = mix_color(spec.palette.background, spec.palette.surfaceRaised, 0.50f);
    spec.fields.frameBgHovered = mix_color(spec.fields.frameBg, spec.palette.accentMuted, 0.18f);
    spec.fields.frameBgActive = mix_color(spec.palette.surfaceRaised, spec.palette.accentMuted, 0.20f);
    spec.fields.rangeTrack = ImVec4(spec.palette.border.x, spec.palette.border.y, spec.palette.border.z, 0.24f);
    spec.fields.rangeFill = ImVec4(spec.palette.accent.x, spec.palette.accent.y, spec.palette.accent.z, 0.82f);
    spec.noticeInfo.fill = ImVec4(0.120f, 0.220f, 0.420f, 0.40f);
    spec.noticeInfo.border = ImVec4(0.400f, 0.760f, 0.980f, 0.56f);
    spec.noticeSuccess.fill = ImVec4(0.100f, 0.300f, 0.220f, 0.36f);
}

} // namespace

BackgroundStyle make_background_style(BackgroundKind kind, const Palette& palette) {
    BackgroundStyle style = make_base_background_style(palette);
    style.kind = kind;

    if (const BackgroundKindDescriptor* descriptor = find_background_kind_descriptor(kind)) {
        descriptor->applyRecipe(style, palette);
    }

    return style;
}

BackgroundTuning make_background_tuning(BackgroundKind kind,
                                        BackgroundIntensity density,
                                        BackgroundIntensity motion) {
    BackgroundTuning tuning;
    tuning.density = density;
    tuning.motion = motion;

    if (kind == BackgroundKind::Flat) {
        tuning.motion = BackgroundIntensity::Subtle;
    }

    return tuning;
}

BackgroundStyle apply_background_tuning(const BackgroundStyle& style,
                                        const BackgroundTuning& tuning) {
    BackgroundStyle tuned = style;

    const float densityAlphaFactor = intensity_factor(tuning.density, 0.78f, 1.0f, 1.22f);
    const float densityCountFactor = intensity_factor(tuning.density, 0.82f, 1.0f, 1.20f);
    const float densitySpacingFactor = intensity_factor(tuning.density, 1.18f, 1.0f, 0.84f);
    const float motionAlphaFactor = intensity_factor(tuning.motion, 0.72f, 1.0f, 1.30f);
    const float motionSpeedFactor = intensity_factor(tuning.motion, 0.72f, 1.0f, 1.30f);
    const float accentBoostFactor = intensity_factor(tuning.motion, 0.75f, 1.0f, 1.25f);

    tuned.authenticatedAccentBoost *= accentBoostFactor;

    tuned.ambient.primary.alpha *= densityAlphaFactor;
    tuned.ambient.secondary.alpha *= densityAlphaFactor;
    tuned.ambient.tertiary.alpha *= densityAlphaFactor;

    tuned.beam.alpha *= motionAlphaFactor;
    tuned.beam.sweepMotion.speed *= motionSpeedFactor;

    tuned.grid.alpha *= densityAlphaFactor;
    tuned.grid.spacing *= densitySpacingFactor;

    tuned.guides.alpha *= densityAlphaFactor;
    tuned.guides.lineCount = std::max(0, static_cast<int>(std::lround(static_cast<double>(tuned.guides.lineCount) * densityCountFactor)));

    tuned.starField.alpha *= densityAlphaFactor * motionAlphaFactor;
    tuned.starField.pointCount = std::max(0, static_cast<int>(std::lround(static_cast<double>(tuned.starField.pointCount) * densityCountFactor)));
    tuned.starField.twinkleMotion.speed *= motionSpeedFactor;

    tuned.gradient.bandCount = std::max(1, static_cast<int>(std::lround(static_cast<double>(tuned.gradient.bandCount) * densityCountFactor)));

    return tuned;
}

ThemeSpec make_theme_spec(ThemeFlavor flavor) {
    ThemeSpec spec = make_base_theme_spec(default_palette());

    if (const ThemeFlavorDescriptor* descriptor = find_theme_flavor_descriptor(flavor)) {
        descriptor->applyRecipe(spec);
    } else {
        apply_onguoin_theme_recipe(spec);
    }

    return spec;
}

BackgroundSpec make_background_spec(BackgroundKind kind, const Palette& palette) {
    BackgroundSpec spec;
    spec.style = make_background_style(kind, palette);
    return spec;
}

Theme compose_theme(const ThemeSpec& themeSpec, const BackgroundSpec& backgroundSpec) {
    Theme theme;
    theme.palette = themeSpec.palette;
    theme.radii = themeSpec.radii;
    theme.spacing = themeSpec.spacing;
    theme.motion = themeSpec.motion;
    theme.surfaces = themeSpec.surfaces;
    theme.noticeInfo = themeSpec.noticeInfo;
    theme.noticeSuccess = themeSpec.noticeSuccess;
    theme.noticeWarning = themeSpec.noticeWarning;
    theme.noticeDanger = themeSpec.noticeDanger;
    theme.widgets = themeSpec.widgets;
    theme.fields = themeSpec.fields;
    theme.layout = themeSpec.layout;
    theme.background = backgroundSpec.style;
    return theme;
}

Theme make_theme(const Palette& palette, BackgroundKind backgroundKind) {
    ThemeSpec spec = make_base_theme_spec(palette);
    return compose_theme(spec, make_background_spec(backgroundKind, palette));
}

Theme make_theme(ThemeFlavor flavor, BackgroundKind backgroundKind) {
    const ThemeSpec spec = make_theme_spec(flavor);
    return compose_theme(spec, make_background_spec(backgroundKind, spec.palette));
}

Theme with_accent_color(const Theme& theme, ImVec4 accentColor) {
    Theme result = theme;
    accentColor.w = 1.0f;
    result.palette.accent = accentColor;
    result.palette.accentMuted = mix_color(result.palette.background, accentColor, 0.38f);
    result.palette.border = ImVec4(accentColor.x, accentColor.y, accentColor.z, theme.palette.border.w);
    result.surfaces = make_surface_tokens(result.palette);
    result.fields = make_field_tokens(result.palette);
    result.noticeInfo.fill = ImVec4(accentColor.x, accentColor.y, accentColor.z, std::max(theme.noticeInfo.fill.w, 0.26f));
    result.noticeInfo.border = ImVec4(accentColor.x, accentColor.y, accentColor.z, std::max(theme.noticeInfo.border.w, 0.42f));
    result.noticeInfo.text = mix_color(result.palette.text, accentColor, 0.28f);
    result.background = make_background_style(theme.background.kind, result.palette);
    return result;
}

Theme with_ui_opacity(const Theme& theme, float opacity) {
    Theme result = theme;
    const float alpha = std::clamp(opacity, 0.0f, 1.0f);
    auto setAlpha = [&](ImVec4& color, float floor = 0.0f) {
        color.w = std::clamp(color.w * alpha, floor, 1.0f);
    };
    auto scaleAlpha = [&](float& value, float floor = 0.0f) {
        value = std::clamp(value * alpha, floor, 1.0f);
    };

    setAlpha(result.palette.sidebar);
    setAlpha(result.palette.panel);
    setAlpha(result.palette.surface);
    setAlpha(result.palette.surfaceRaised);
    setAlpha(result.palette.accentMuted);
    setAlpha(result.palette.border);
    setAlpha(result.surfaces.frameBg);
    setAlpha(result.surfaces.frameBgHovered);
    setAlpha(result.surfaces.frameBgActive);
    setAlpha(result.surfaces.buttonBg);
    setAlpha(result.surfaces.buttonBgHovered);
    setAlpha(result.surfaces.buttonBgActive);
    setAlpha(result.surfaces.headerBg);
    setAlpha(result.surfaces.headerBgHovered);
    setAlpha(result.surfaces.headerBgActive);
    setAlpha(result.surfaces.scrollbarBg);
    setAlpha(result.surfaces.dockOuter);
    setAlpha(result.surfaces.dockInner);
    setAlpha(result.surfaces.statusPillBg);
    setAlpha(result.fields.frameBg);
    setAlpha(result.fields.frameBgHovered);
    setAlpha(result.fields.frameBgActive);
    setAlpha(result.fields.sliderGrab, result.fields.controlGrabAlphaFloor);
    setAlpha(result.fields.sliderGrabActive, std::max(result.fields.controlGrabAlphaFloor, 0.72f));
    setAlpha(result.fields.rangeTrack, result.fields.controlBorderAlphaFloor);
    setAlpha(result.fields.rangeFill, result.fields.controlGrabAlphaFloor);
    setAlpha(result.fields.rangeHandleRing, result.fields.controlGrabAlphaFloor);
    scaleAlpha(result.fields.rangeHandleHaloAlpha);
    scaleAlpha(result.fields.rangeHandleFillAlpha, result.fields.controlGrabAlphaFloor);
    scaleAlpha(result.fields.rangeHandleFillActiveAlpha, std::max(result.fields.controlGrabAlphaFloor, 0.72f));
    scaleAlpha(result.fields.rangeHandleRingIdleAlpha, result.fields.controlGrabAlphaFloor);
    scaleAlpha(result.fields.rangeHandleRingHoverAlpha, result.fields.controlHoverBorderAlphaFloor);
    scaleAlpha(result.fields.rangeHandleRingActiveAlpha, result.fields.controlActiveBorderAlphaFloor);
    scaleAlpha(result.fields.rangeValueTextAlpha, result.fields.controlTextAlphaFloor);
    scaleAlpha(result.fields.selectIdleBorderAlpha, result.fields.controlBorderAlphaFloor);
    scaleAlpha(result.fields.selectHoverBorderAlpha, result.fields.controlHoverBorderAlphaFloor);
    scaleAlpha(result.fields.selectOpenBorderAlpha, result.fields.controlActiveBorderAlphaFloor);
    scaleAlpha(result.fields.selectOpenBorderPulse);
    scaleAlpha(result.fields.selectHoverGlowAlpha);
    scaleAlpha(result.fields.selectOpenGlowAlpha);
    setAlpha(result.widgets.helpMarker.collapsedFill);
    scaleAlpha(result.widgets.helpMarker.collapsedBorderAlpha, result.fields.controlBorderAlphaFloor);
    scaleAlpha(result.widgets.helpMarker.hoveredGlowAlpha);
    scaleAlpha(result.widgets.helpMarker.collapsedGlowAlpha);
    scaleAlpha(result.widgets.helpMarker.collapsedInnerHighlightAlpha);
    scaleAlpha(result.widgets.helpMarker.hoveredInnerHighlightAlpha);
    scaleAlpha(result.surfaces.cardGlowAlpha);
    scaleAlpha(result.surfaces.innerHighlightAlpha);
    scaleAlpha(result.surfaces.buttonTopHighlightAlpha);
    scaleAlpha(result.surfaces.buttonTopHighlightDisabledAlpha);
    return result;
}

const ThemePresetInfo* theme_preset_infos(int* count) {
    if (count != nullptr) {
        *count = static_cast<int>(std::size(kThemePresetInfos));
    }
    return kThemePresetInfos.data();
}

const ThemePresetInfo* find_theme_preset_info(ThemePreset preset) {
    return find_info_by_key(kThemePresetInfos, preset, &ThemePresetInfo::preset);
}

const char* theme_preset_label(const ThemePresetInfo& info, UiTextLanguage language) {
    return select_localized_text(language, info, &ThemePresetInfo::englishLabel, &ThemePresetInfo::chineseLabel);
}

const char* theme_preset_label(ThemePreset preset, UiTextLanguage language) {
    if (const ThemePresetInfo* info = find_theme_preset_info(preset)) {
        return theme_preset_label(*info, language);
    }
    return "";
}

ThemeFlavor theme_preset_flavor(ThemePreset preset) {
    if (const ThemePresetInfo* info = find_theme_preset_info(preset)) {
        return info->flavor;
    }
    return ThemeFlavor::Onguoin;
}

BackgroundKind theme_preset_background_kind(ThemePreset preset) {
    if (const ThemePresetInfo* info = find_theme_preset_info(preset)) {
        return info->backgroundKind;
    }
    return BackgroundKind::Aurora;
}

ThemeSelection theme_preset_selection(ThemePreset preset) {
    return ThemeSelection{theme_preset_flavor(preset), theme_preset_background_kind(preset)};
}

bool theme_selection_matches_preset(const ThemeSelection& selection, ThemePreset preset) {
    const ThemeSelection presetSelection = theme_preset_selection(preset);
    return selection.flavor == presetSelection.flavor &&
           selection.backgroundKind == presetSelection.backgroundKind;
}

ThemeSelection make_theme_selection(ThemePreset preset) {
    return theme_preset_selection(preset);
}

Theme make_theme_preset(ThemePreset preset) {
    return make_theme(make_theme_selection(preset));
}

BackgroundKind default_background_kind_for_flavor(ThemeFlavor flavor) {
    if (const ThemeFlavorDescriptor* descriptor = find_theme_flavor_descriptor(flavor)) {
        return descriptor->info.preferredBackgroundKind;
    }
    return BackgroundKind::Aurora;
}

const ThemeFlavorInfo* theme_flavor_infos(int* count) {
    if (count != nullptr) {
        *count = static_cast<int>(std::size(kThemeFlavorInfos));
    }
    return kThemeFlavorInfos.data();
}

const ThemeFlavorInfo* find_theme_flavor_info(ThemeFlavor flavor) {
    return find_info_by_key(kThemeFlavorInfos, flavor, &ThemeFlavorInfo::flavor);
}

const BackgroundKindInfo* background_kind_infos(int* count) {
    if (count != nullptr) {
        *count = static_cast<int>(std::size(kBackgroundKindInfos));
    }
    return kBackgroundKindInfos.data();
}

const BackgroundKindInfo* find_background_kind_info(BackgroundKind kind) {
    return find_info_by_key(kBackgroundKindInfos, kind, &BackgroundKindInfo::kind);
}

const BackgroundLayerInfo* background_layer_infos(int* count) {
    if (count != nullptr) {
        *count = static_cast<int>(std::size(kBackgroundLayerInfos));
    }
    return kBackgroundLayerInfos;
}

const BackgroundLayerInfo* find_background_layer_info(BackgroundLayerKind kind) {
    return find_info_by_key(kBackgroundLayerInfos, kind, &BackgroundLayerInfo::kind);
}

const char* background_layer_id(BackgroundLayerKind kind) {
    if (const BackgroundLayerInfo* info = find_background_layer_info(kind)) {
        return info->id;
    }
    return "gradient";
}

const char* theme_flavor_label(const ThemeFlavorInfo& info, UiTextLanguage language) {
    return select_localized_text(language, info, &ThemeFlavorInfo::englishLabel, &ThemeFlavorInfo::chineseLabel);
}

const char* theme_flavor_label(ThemeFlavor flavor, UiTextLanguage language) {
    if (const ThemeFlavorInfo* info = find_theme_flavor_info(flavor)) {
        return theme_flavor_label(*info, language);
    }
    return "";
}

const char* theme_flavor_tone(const ThemeFlavorInfo& info, UiTextLanguage language) {
    return select_localized_text(language, info, &ThemeFlavorInfo::englishTone, &ThemeFlavorInfo::chineseTone);
}

const char* theme_flavor_tone(ThemeFlavor flavor, UiTextLanguage language) {
    if (const ThemeFlavorInfo* info = find_theme_flavor_info(flavor)) {
        return theme_flavor_tone(*info, language);
    }
    return "";
}

bool theme_flavor_has_expressive_motion(ThemeFlavor flavor) {
    if (const ThemeFlavorInfo* info = find_theme_flavor_info(flavor)) {
        return info->motionExpressive;
    }
    return false;
}

const char* background_kind_label(const BackgroundKindInfo& info, UiTextLanguage language) {
    return select_localized_text(language, info, &BackgroundKindInfo::englishLabel, &BackgroundKindInfo::chineseLabel);
}

const char* background_kind_label(BackgroundKind kind, UiTextLanguage language) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return background_kind_label(*info, language);
    }
    return "";
}

const char* background_layer_label(const BackgroundLayerInfo& info, UiTextLanguage language) {
    return select_localized_text(language, info, &BackgroundLayerInfo::englishLabel, &BackgroundLayerInfo::chineseLabel);
}

const char* background_layer_label(BackgroundLayerKind kind, UiTextLanguage language) {
    if (const BackgroundLayerInfo* info = find_background_layer_info(kind)) {
        return background_layer_label(*info, language);
    }
    return "";
}

BackgroundLayerMask background_supported_layers(BackgroundKind kind) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return info->supportedLayers;
    }
    return 0u;
}

BackgroundLayerMask background_dynamic_layers(BackgroundKind kind) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return info->dynamicLayers;
    }
    return 0u;
}

bool background_supports_layer(BackgroundKind kind, BackgroundLayerKind layer) {
    return background_layer_mask_has(background_supported_layers(kind), layer);
}

bool background_layer_is_dynamic(BackgroundKind kind, BackgroundLayerKind layer) {
    return background_layer_mask_has(background_dynamic_layers(kind), layer);
}

int background_supported_layer_count(BackgroundKind kind) {
    return background_layer_mask_count(background_supported_layers(kind));
}

int background_dynamic_layer_count(BackgroundKind kind) {
    return background_layer_mask_count(background_dynamic_layers(kind));
}

bool background_recommended_for_authenticated_shell(BackgroundKind kind) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return info->recommendedForAuthenticatedShell;
    }
    return true;
}

BackgroundComplexity background_complexity(BackgroundKind kind) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return info->complexity;
    }
    return BackgroundComplexity::Balanced;
}

BackgroundMotionCharacter background_motion_character(BackgroundKind kind) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return info->motionCharacter;
    }
    return BackgroundMotionCharacter::Static;
}

const char* background_character(const BackgroundKindInfo& info, UiTextLanguage language) {
    return select_localized_text(language,
                                 info,
                                 &BackgroundKindInfo::englishCharacter,
                                 &BackgroundKindInfo::chineseCharacter);
}

const char* background_character(BackgroundKind kind, UiTextLanguage language) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return background_character(*info, language);
    }
    return "";
}

BackgroundLayerMask enabled_background_layers(const BackgroundStyle& style) {
    BackgroundLayerMask mask = 0u;
    if (style.gradient.enabled && style.gradient.bandCount > 0) {
        mask |= background_layer_flag(BackgroundLayerKind::Gradient);
    }
    if (style.ambient.enabled) {
        mask |= background_layer_flag(BackgroundLayerKind::Ambient);
    }
    if (style.beam.enabled && style.beam.alpha > 0.0f) {
        mask |= background_layer_flag(BackgroundLayerKind::Beam);
    }
    if (style.grid.enabled && style.grid.alpha > 0.0f && style.grid.spacing > 1.0f) {
        mask |= background_layer_flag(BackgroundLayerKind::Grid);
    }
    if (style.guides.enabled && style.guides.lineCount > 0) {
        mask |= background_layer_flag(BackgroundLayerKind::Guides);
    }
    if (style.starField.enabled && style.starField.alpha > 0.0f && style.starField.pointCount > 0) {
        mask |= background_layer_flag(BackgroundLayerKind::StarField);
    }
    mask |= background_layer_flag(BackgroundLayerKind::CursorAura);
    return mask;
}

int enabled_background_layer_count(const BackgroundStyle& style) {
    return background_layer_mask_count(enabled_background_layers(style));
}

bool background_layer_enabled(const BackgroundStyle& style, BackgroundLayerKind layer) {
    return background_layer_mask_has(enabled_background_layers(style), layer);
}

namespace {

ThemeSelection& current_theme_selection_storage() {
    static ThemeSelection selection = make_theme_selection(ThemePreset::Onguoin);
    return selection;
}

Theme& current_theme_storage() {
    static Theme theme = make_theme(current_theme_selection_storage());
    return theme;
}

} // namespace

const ThemeSelection& current_theme_selection() {
    return current_theme_selection_storage();
}

ThemeFlavor current_theme_flavor() {
    return current_theme_selection().flavor;
}

BackgroundKind current_background_kind() {
    return current_theme_selection().backgroundKind;
}

const Theme& current_theme() {
    return current_theme_storage();
}

const NoticeToneStyle& notice_tone_style(const Theme& theme, NoticeTone tone) {
    switch (tone) {
    case NoticeTone::Success:
        return theme.noticeSuccess;
    case NoticeTone::Warning:
        return theme.noticeWarning;
    case NoticeTone::Danger:
        return theme.noticeDanger;
    case NoticeTone::Info:
    default:
        return theme.noticeInfo;
    }
}

void set_current_theme_style(ThemeFlavor flavor, BackgroundKind backgroundKind) {
    ThemeSelection& selection = current_theme_selection_storage();
    selection.flavor = flavor;
    selection.backgroundKind = backgroundKind;
    current_theme_storage() = make_theme(selection);
}

void set_current_theme(const Theme& theme) {
    current_theme_storage() = theme;
}

void set_current_theme(ThemePreset preset) {
    const ThemeSelection selection = make_theme_selection(preset);
    set_current_theme_style(selection.flavor, selection.backgroundKind);
}

const Theme& default_theme() {
    return current_theme();
}

const char* theme_preset_id(ThemePreset preset) {
    if (const ThemePresetInfo* info = find_theme_preset_info(preset)) {
        return info->id;
    }
    return "onguoin";
}

const char* theme_flavor_id(ThemeFlavor flavor) {
    if (const ThemeFlavorInfo* info = find_theme_flavor_info(flavor)) {
        return info->id;
    }
    return "onguoin";
}

const char* background_kind_id(BackgroundKind kind) {
    if (const BackgroundKindInfo* info = find_background_kind_info(kind)) {
        return info->id;
    }
    return "aurora";
}

BackgroundLayerKind parse_background_layer_id(const char* value, BackgroundLayerKind fallback) {
    return parse_catalog_id(value, fallback, kBackgroundLayerInfos, &BackgroundLayerInfo::kind);
}

ThemePreset parse_theme_preset_id(const char* value, ThemePreset fallback) {
    return parse_catalog_id(value, fallback, kThemePresetInfos, &ThemePresetInfo::preset);
}

ThemeFlavor parse_theme_flavor_id(const char* value, ThemeFlavor fallback) {
    return parse_catalog_id(value, fallback, kThemeFlavorInfos, &ThemeFlavorInfo::flavor);
}

BackgroundKind parse_background_kind_id(const char* value, BackgroundKind fallback) {
    return parse_catalog_id(value, fallback, kBackgroundKindInfos, &BackgroundKindInfo::kind);
}

void apply_theme(const Theme& theme) {
    const Palette& palette = theme.palette;
    const RadiusTokens& radii = theme.radii;
    const SpacingTokens& spacing = theme.spacing;
    const SurfaceTokens& surfaces = theme.surfaces;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = radii.window;
    style.ChildRounding = radii.child;
    style.FrameRounding = radii.frame;
    style.PopupRounding = radii.popup;
    style.GrabRounding = radii.grab;
    style.ScrollbarRounding = radii.scrollbar;
    style.WindowBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = spacing.windowPadding;
    style.FramePadding = spacing.framePadding;
    style.ItemSpacing = spacing.itemSpacing;
    style.ItemInnerSpacing = spacing.itemInnerSpacing;
    style.ScrollbarSize = spacing.scrollbarSize;
    style.GrabMinSize = 18.0f;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;
    style.CurveTessellationTol = 0.90f;
    style.CircleTessellationMaxError = 0.18f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = palette.background;
    colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_PopupBg] = palette.surface;
    colors[ImGuiCol_PopupBg].w = std::max(colors[ImGuiCol_PopupBg].w, 0.75f);
    colors[ImGuiCol_Border] = palette.border;
    colors[ImGuiCol_Border].w = std::max(colors[ImGuiCol_Border].w, theme.fields.controlBorderAlphaFloor);
    colors[ImGuiCol_FrameBg] = surfaces.frameBg;
    colors[ImGuiCol_FrameBgHovered] = surfaces.frameBgHovered;
    colors[ImGuiCol_FrameBgActive] = surfaces.frameBgActive;
    colors[ImGuiCol_CheckMark] = palette.accent;
    colors[ImGuiCol_SliderGrab] = theme.fields.sliderGrab;
    colors[ImGuiCol_SliderGrabActive] = theme.fields.sliderGrabActive;
    colors[ImGuiCol_Button] = surfaces.buttonBg;
    colors[ImGuiCol_ButtonHovered] = surfaces.buttonBgHovered;
    colors[ImGuiCol_ButtonActive] = surfaces.buttonBgActive;
    colors[ImGuiCol_Header] = surfaces.headerBg;
    colors[ImGuiCol_HeaderHovered] = surfaces.headerBgHovered;
    colors[ImGuiCol_HeaderActive] = surfaces.headerBgActive;
    colors[ImGuiCol_ScrollbarBg] = surfaces.scrollbarBg;
    colors[ImGuiCol_ScrollbarGrab] = surfaces.scrollbarGrab;
    colors[ImGuiCol_ScrollbarGrabHovered] = surfaces.scrollbarGrabHovered;
    colors[ImGuiCol_ScrollbarGrabActive] = palette.accentMuted;
    colors[ImGuiCol_Separator] = palette.border;
    colors[ImGuiCol_Text] = palette.text;
    colors[ImGuiCol_TextDisabled] = palette.textMuted;
}

void load_default_font(float pixelSize) {
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.OversampleH = 4;
    cfg.OversampleV = 2;
    cfg.PixelSnapH = false;
    cfg.RasterizerMultiply = 1.05f;

#ifdef _WIN32
    const char* fonts[] = {
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\msyhbd.ttc",
        "C:\\Windows\\Fonts\\simhei.ttf",
    };

    for (const char* font : fonts) {
        if (GetFileAttributesA(font) != INVALID_FILE_ATTRIBUTES) {
            io.Fonts->AddFontFromFileTTF(font, pixelSize, &cfg, io.Fonts->GetGlyphRangesChineseFull());
            return;
        }
    }
#else
    (void)pixelSize;
#endif

    io.Fonts->AddFontDefault();
}

} // namespace imgui_onguoin

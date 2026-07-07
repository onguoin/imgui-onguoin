// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_types.h"

namespace imgui_onguoin {

BackgroundStyle make_background_style(BackgroundKind kind, const Palette& palette = default_palette());
BackgroundTuning make_background_tuning(BackgroundKind kind,
                                        BackgroundIntensity density = BackgroundIntensity::Balanced,
                                        BackgroundIntensity motion = BackgroundIntensity::Balanced);
BackgroundStyle apply_background_tuning(const BackgroundStyle& style,
                                        const BackgroundTuning& tuning);
ThemeSpec make_theme_spec(ThemeFlavor flavor);
BackgroundSpec make_background_spec(BackgroundKind kind, const Palette& palette = default_palette());
Theme compose_theme(const ThemeSpec& themeSpec, const BackgroundSpec& backgroundSpec);
Theme make_theme(const Palette& palette = default_palette(),
                 BackgroundKind backgroundKind = BackgroundKind::Aurora);
Theme make_theme(ThemeFlavor flavor, BackgroundKind backgroundKind);
Theme with_accent_color(const Theme& theme, ImVec4 accentColor);
Theme with_ui_opacity(const Theme& theme, float opacity);
inline Theme make_theme(const ThemeSelection& selection) {
    return make_theme(selection.flavor, selection.backgroundKind);
}
const ThemePresetInfo* theme_preset_infos(int* count = nullptr);
const ThemePresetInfo* find_theme_preset_info(ThemePreset preset);
const char* theme_preset_label(ThemePreset preset,
                               UiTextLanguage language = UiTextLanguage::English);
const char* theme_preset_label(const ThemePresetInfo& info,
                               UiTextLanguage language = UiTextLanguage::English);
ThemeFlavor theme_preset_flavor(ThemePreset preset);
BackgroundKind theme_preset_background_kind(ThemePreset preset);
ThemeSelection theme_preset_selection(ThemePreset preset);
bool theme_selection_matches_preset(const ThemeSelection& selection, ThemePreset preset);
ThemeSelection make_theme_selection(ThemePreset preset);
Theme make_theme_preset(ThemePreset preset);
BackgroundKind default_background_kind_for_flavor(ThemeFlavor flavor);
const ThemeFlavorInfo* theme_flavor_infos(int* count = nullptr);
const ThemeFlavorInfo* find_theme_flavor_info(ThemeFlavor flavor);
const BackgroundKindInfo* background_kind_infos(int* count = nullptr);
const BackgroundKindInfo* find_background_kind_info(BackgroundKind kind);
const BackgroundLayerInfo* background_layer_infos(int* count = nullptr);
const BackgroundLayerInfo* find_background_layer_info(BackgroundLayerKind kind);
const char* background_layer_id(BackgroundLayerKind kind);
BackgroundLayerKind parse_background_layer_id(const char* value,
                                              BackgroundLayerKind fallback = BackgroundLayerKind::Gradient);
const char* theme_flavor_label(ThemeFlavor flavor,
                               UiTextLanguage language = UiTextLanguage::English);
const char* theme_flavor_label(const ThemeFlavorInfo& info,
                               UiTextLanguage language = UiTextLanguage::English);
const char* theme_flavor_tone(ThemeFlavor flavor,
                              UiTextLanguage language = UiTextLanguage::English);
const char* theme_flavor_tone(const ThemeFlavorInfo& info,
                              UiTextLanguage language = UiTextLanguage::English);
bool theme_flavor_has_expressive_motion(ThemeFlavor flavor);
const char* background_kind_label(BackgroundKind kind,
                                  UiTextLanguage language = UiTextLanguage::English);
const char* background_kind_label(const BackgroundKindInfo& info,
                                  UiTextLanguage language = UiTextLanguage::English);
const char* background_layer_label(BackgroundLayerKind kind,
                                   UiTextLanguage language = UiTextLanguage::English);
const char* background_layer_label(const BackgroundLayerInfo& info,
                                   UiTextLanguage language = UiTextLanguage::English);
BackgroundLayerMask background_supported_layers(BackgroundKind kind);
BackgroundLayerMask background_dynamic_layers(BackgroundKind kind);
bool background_supports_layer(BackgroundKind kind, BackgroundLayerKind layer);
bool background_layer_is_dynamic(BackgroundKind kind, BackgroundLayerKind layer);
int background_supported_layer_count(BackgroundKind kind);
int background_dynamic_layer_count(BackgroundKind kind);
bool background_recommended_for_authenticated_shell(BackgroundKind kind);
BackgroundComplexity background_complexity(BackgroundKind kind);
BackgroundMotionCharacter background_motion_character(BackgroundKind kind);
const char* background_character(BackgroundKind kind,
                                 UiTextLanguage language = UiTextLanguage::English);
const char* background_character(const BackgroundKindInfo& info,
                                 UiTextLanguage language = UiTextLanguage::English);
BackgroundLayerMask enabled_background_layers(const BackgroundStyle& style);
int enabled_background_layer_count(const BackgroundStyle& style);
bool background_layer_enabled(const BackgroundStyle& style, BackgroundLayerKind layer);

const ThemeSelection& current_theme_selection();
ThemeFlavor current_theme_flavor();
BackgroundKind current_background_kind();
const Theme& current_theme();
const NoticeToneStyle& notice_tone_style(const Theme& theme, NoticeTone tone);
void set_current_theme_style(ThemeFlavor flavor, BackgroundKind backgroundKind);
void set_current_theme(const Theme& theme);
void set_current_theme(ThemePreset preset);
const Theme& default_theme();

const char* theme_preset_id(ThemePreset preset);
const char* theme_flavor_id(ThemeFlavor flavor);
const char* background_kind_id(BackgroundKind kind);
ThemePreset parse_theme_preset_id(const char* value, ThemePreset fallback = ThemePreset::Onguoin);
ThemeFlavor parse_theme_flavor_id(const char* value, ThemeFlavor fallback = ThemeFlavor::Onguoin);
BackgroundKind parse_background_kind_id(const char* value, BackgroundKind fallback = BackgroundKind::Aurora);

void apply_theme(const Theme& theme);
inline void apply_theme() {
    apply_theme(default_theme());
}
inline void apply_theme(const Palette& palette) {
    apply_theme(make_theme(palette));
}

void load_default_font(float pixelSize = 18.0f);

} // namespace imgui_onguoin

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_theme.h"

#include <utility>

namespace imgui_onguoin {

FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       float preferredColumnX = -1.0f);
FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       FormLayoutPreset preset,
                                       float preferredColumnX = -1.0f);
FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       UiTextLanguage language,
                                       FormLayoutPreset preset);
FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       UiTextLanguage language,
                                       float chinesePreferredColumnX,
                                       float englishPreferredColumnX);
inline FormLayoutStyle make_form_layout_style(float preferredColumnX = -1.0f) {
    return make_form_layout_style(default_theme(), preferredColumnX);
}
inline FormLayoutStyle make_form_layout_style(FormLayoutPreset preset,
                                              float preferredColumnX = -1.0f) {
    return make_form_layout_style(default_theme(), preset, preferredColumnX);
}
inline FormLayoutStyle make_form_layout_style(UiTextLanguage language,
                                              FormLayoutPreset preset) {
    return make_form_layout_style(default_theme(), language, preset);
}
inline FormLayoutStyle make_form_layout_style(UiTextLanguage language,
                                              float chinesePreferredColumnX,
                                              float englishPreferredColumnX) {
    return make_form_layout_style(default_theme(), language, chinesePreferredColumnX, englishPreferredColumnX);
}
inline FormLayoutStyle make_form_layout_style(const Palette& palette,
                                              float preferredColumnX = -1.0f) {
    return make_form_layout_style(make_theme(palette), preferredColumnX);
}
inline FormLayoutStyle make_form_layout_style(const Palette& palette,
                                              FormLayoutPreset preset,
                                              float preferredColumnX = -1.0f) {
    return make_form_layout_style(make_theme(palette), preset, preferredColumnX);
}
inline FormLayoutStyle make_form_layout_style(const Palette& palette,
                                              UiTextLanguage language,
                                              FormLayoutPreset preset) {
    return make_form_layout_style(make_theme(palette), language, preset);
}
inline FormLayoutStyle make_form_layout_style(const Palette& palette,
                                              UiTextLanguage language,
                                              float chinesePreferredColumnX,
                                              float englishPreferredColumnX) {
    return make_form_layout_style(make_theme(palette), language, chinesePreferredColumnX, englishPreferredColumnX);
}

void set_scaled_cursor_x(float x);
void add_vertical_space(float height);
float form_control_column_x(const char* label,
                            const FormLayoutStyle& style,
                            float insetX = -1.0f);
float form_control_column_x(const char* label,
                            const Theme& theme,
                            FormLayoutPreset preset,
                            float insetX = -1.0f);
float form_control_column_x(const char* label,
                            const Theme& theme,
                            UiTextLanguage language,
                            FormLayoutPreset preset,
                            float insetX = -1.0f);
inline float form_control_column_x(const char* label,
                                   FormLayoutPreset preset,
                                   float insetX = -1.0f) {
    return form_control_column_x(label, default_theme(), preset, insetX);
}
inline float form_control_column_x(const char* label,
                                   const Palette& palette,
                                   FormLayoutPreset preset,
                                   float insetX = -1.0f) {
    return form_control_column_x(label, make_theme(palette), preset, insetX);
}
inline float form_control_column_x(const char* label,
                                   UiTextLanguage language,
                                   FormLayoutPreset preset,
                                   float insetX = -1.0f) {
    return form_control_column_x(label, default_theme(), language, preset, insetX);
}
inline float form_control_column_x(const char* label,
                                   const Palette& palette,
                                   UiTextLanguage language,
                                   FormLayoutPreset preset,
                                   float insetX = -1.0f) {
    return form_control_column_x(label, make_theme(palette), language, preset, insetX);
}
void begin_form_row(const char* label,
                    const FormLayoutStyle& style,
                    float insetX = -1.0f);
void begin_form_row(const char* label,
                    const Theme& theme,
                    FormLayoutPreset preset,
                    float insetX = -1.0f);
inline void begin_form_row(const char* label,
                           FormLayoutPreset preset,
                           float insetX = -1.0f) {
    begin_form_row(label, default_theme(), preset, insetX);
}
inline void begin_form_row(const char* label,
                           const Palette& palette,
                           FormLayoutPreset preset,
                           float insetX = -1.0f) {
    begin_form_row(label, make_theme(palette), preset, insetX);
}

template <typename DrawControlFn>
inline void draw_form_row(const char* label,
                          const FormLayoutStyle& style,
                          DrawControlFn&& drawControl,
                          float insetX = -1.0f) {
    begin_form_row(label, style, insetX);
    drawControl();
}

template <typename DrawControlFn>
inline void draw_form_row(const char* label,
                          const Theme& theme,
                          FormLayoutPreset preset,
                          DrawControlFn&& drawControl,
                          float insetX = -1.0f) {
    begin_form_row(label, theme, preset, insetX);
    drawControl();
}

template <typename DrawControlFn>
inline void draw_form_row(const char* label,
                          FormLayoutPreset preset,
                          DrawControlFn&& drawControl,
                          float insetX = -1.0f) {
    draw_form_row(label, default_theme(), preset, std::forward<DrawControlFn>(drawControl), insetX);
}

template <typename DrawControlFn>
inline void draw_form_row(const char* label,
                          const Palette& palette,
                          FormLayoutPreset preset,
                          DrawControlFn&& drawControl,
                          float insetX = -1.0f) {
    draw_form_row(label, make_theme(palette), preset, std::forward<DrawControlFn>(drawControl), insetX);
}

} // namespace imgui_onguoin

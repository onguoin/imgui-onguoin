// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "ui/imgui_onguoin.h"

#include <algorithm>

namespace imgui_onguoin {

namespace {

float resolve_metric(float value, float fallback) {
    return value >= 0.0f ? value : fallback;
}

float preset_preferred_column_x(FormLayoutPreset preset) {
    switch (preset) {
    case FormLayoutPreset::Standard:
        return 260.0f;
    case FormLayoutPreset::Wide:
        return 320.0f;
    case FormLayoutPreset::ThemeDefault:
    default:
        return -1.0f;
    }
}

float resolve_inset_x(const FormLayoutStyle& style, float insetX) {
    return insetX >= 0.0f ? insetX : resolve_metric(style.insetX, default_theme().layout.formInsetX);
}

float localized_preset_column_x(UiTextLanguage language, FormLayoutPreset preset) {
    if (language == UiTextLanguage::English) {
        return -1.0f;
    }
    switch (preset) {
    case FormLayoutPreset::Wide:
        return 208.0f;
    case FormLayoutPreset::Standard:
        return 160.0f;
    case FormLayoutPreset::ThemeDefault:
    default:
        return -1.0f;
    }
}

} // namespace

FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       float preferredColumnX) {
    FormLayoutStyle style;
    style.insetX = theme.layout.formInsetX;
    style.nestedInsetX = theme.layout.formNestedInsetX;
    style.preferredColumnX = preferredColumnX >= 0.0f ? preferredColumnX : theme.layout.formPreferredColumnX;
    style.minimumLabelGap = theme.layout.formMinimumLabelGap;
    style.rowSpacing = theme.layout.formRowSpacing;
    style.sectionSpacing = theme.layout.formSectionSpacing;
    style.footerSpacing = theme.layout.formFooterSpacing;
    return style;
}

FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       FormLayoutPreset preset,
                                       float preferredColumnX) {
    const float presetColumnX = preferredColumnX >= 0.0f
        ? preferredColumnX
        : preset_preferred_column_x(preset);
    return make_form_layout_style(theme, presetColumnX);
}

FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       UiTextLanguage language,
                                       FormLayoutPreset preset) {
    return make_form_layout_style(theme,
                                  preset,
                                  localized_preset_column_x(language, preset));
}

FormLayoutStyle make_form_layout_style(const Theme& theme,
                                       UiTextLanguage language,
                                       float chinesePreferredColumnX,
                                       float englishPreferredColumnX) {
    return make_form_layout_style(theme,
                                  language == UiTextLanguage::English ? englishPreferredColumnX : chinesePreferredColumnX);
}

void set_scaled_cursor_x(float x) {
    ImGui::SetCursorPosX(x * current_scale());
}

void add_vertical_space(float height) {
    ImGui::Dummy(ImVec2(0.0f, height * current_scale()));
}

float form_control_column_x(const char* label,
                            const FormLayoutStyle& style,
                            float insetX) {
    const float scale = current_scale();
    const float rowInset = resolve_inset_x(style, insetX) * scale;
    const float minimumLabelGap = resolve_metric(style.minimumLabelGap, default_theme().layout.formMinimumLabelGap);
    float columnX = resolve_metric(style.preferredColumnX, 260.0f) * scale;
    if (label != nullptr && label[0] != '\0') {
        columnX = std::max(columnX,
                           rowInset + ImGui::CalcTextSize(label).x + minimumLabelGap * scale);
    }
    return columnX;
}

float form_control_column_x(const char* label,
                            const Theme& theme,
                            FormLayoutPreset preset,
                            float insetX) {
    return form_control_column_x(label,
                                 make_form_layout_style(theme, preset),
                                 insetX);
}

float form_control_column_x(const char* label,
                            const Theme& theme,
                            UiTextLanguage language,
                            FormLayoutPreset preset,
                            float insetX) {
    return form_control_column_x(label,
                                 make_form_layout_style(theme, language, preset),
                                 insetX);
}

void begin_form_row(const char* label,
                    const FormLayoutStyle& style,
                    float insetX) {
    const float rowInset = resolve_inset_x(style, insetX);
    set_scaled_cursor_x(rowInset);
    if (label == nullptr || label[0] == '\0') {
        return;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::SameLine(form_control_column_x(label, style, rowInset));
}

void begin_form_row(const char* label,
                    const Theme& theme,
                    FormLayoutPreset preset,
                    float insetX) {
    begin_form_row(label,
                   make_form_layout_style(theme, preset),
                   insetX);
}

} // namespace imgui_onguoin

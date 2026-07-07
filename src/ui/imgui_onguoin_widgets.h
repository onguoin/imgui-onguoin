// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_effects.h"

#include <functional>

namespace imgui_onguoin {

void draw_page_hero(const PageHeroText& hero, const Theme& theme);
inline void draw_page_hero(const PageHeroText& hero) {
    draw_page_hero(hero, default_theme());
}
inline void draw_page_hero(const PageHeroText& hero, const Palette& palette) {
    draw_page_hero(hero, make_theme(palette));
}

void draw_footer_signature(ImDrawList* drawList,
                           ImVec2 min,
                           float width,
                           const char* text,
                           const Theme& theme,
                           FooterSignatureStyle style = {});
inline void draw_footer_signature(ImDrawList* drawList,
                                  ImVec2 min,
                                  float width,
                                  const char* text,
                                  FooterSignatureStyle style = {}) {
    draw_footer_signature(drawList, min, width, text, default_theme(), style);
}
inline void draw_footer_signature(ImDrawList* drawList,
                                  ImVec2 min,
                                  float width,
                                  const char* text,
                                  const Palette& palette,
                                  FooterSignatureStyle style = {}) {
    draw_footer_signature(drawList, min, width, text, make_theme(palette), style);
}

float draw_identity_capsule(const IdentityCapsuleData& data,
                            const Theme& theme,
                            IdentityCapsuleStyle style = {});
inline float draw_identity_capsule(const IdentityCapsuleData& data,
                                   IdentityCapsuleStyle style = {}) {
    return draw_identity_capsule(data, default_theme(), style);
}
inline float draw_identity_capsule(const IdentityCapsuleData& data,
                                   const Palette& palette,
                                   IdentityCapsuleStyle style = {}) {
    return draw_identity_capsule(data, make_theme(palette), style);
}

WidgetFrameLayout draw_compact_overlay_shell(ImDrawList* drawList,
                                             ImVec2 rootPos,
                                             ImVec2 windowSize,
                                             const CompactOverlayShellData& data,
                                             const Theme& theme,
                                             CompactOverlayShellStyle style = {});
inline WidgetFrameLayout draw_compact_overlay_shell(ImDrawList* drawList,
                                                    ImVec2 rootPos,
                                                    ImVec2 windowSize,
                                                    const CompactOverlayShellData& data,
                                                    CompactOverlayShellStyle style = {}) {
    return draw_compact_overlay_shell(drawList, rootPos, windowSize, data, default_theme(), style);
}
inline WidgetFrameLayout draw_compact_overlay_shell(ImDrawList* drawList,
                                                    ImVec2 rootPos,
                                                    ImVec2 windowSize,
                                                    const CompactOverlayShellData& data,
                                                    const Palette& palette,
                                                    CompactOverlayShellStyle style = {}) {
    return draw_compact_overlay_shell(drawList, rootPos, windowSize, data, make_theme(palette), style);
}

void draw_compact_input_matrix(ImVec2 frameMin,
                               ImVec2 frameMax,
                               const CompactInputMatrixData& data,
                               const Theme& theme,
                               CompactInputMatrixStyle style = {});
inline void draw_compact_input_matrix(ImVec2 frameMin,
                                      ImVec2 frameMax,
                                      const CompactInputMatrixData& data,
                                      CompactInputMatrixStyle style = {}) {
    draw_compact_input_matrix(frameMin, frameMax, data, default_theme(), style);
}
inline void draw_compact_input_matrix(ImVec2 frameMin,
                                      ImVec2 frameMax,
                                      const CompactInputMatrixData& data,
                                      const Palette& palette,
                                      CompactInputMatrixStyle style = {}) {
    draw_compact_input_matrix(frameMin, frameMax, data, make_theme(palette), style);
}

void draw_compact_rhythm_capsule(ImDrawList* drawList,
                                 ImVec2 min,
                                 ImVec2 size,
                                 const CompactRhythmCapsuleData& data,
                                 const Theme& theme,
                                 CompactRhythmCapsuleStyle style = {});
inline void draw_compact_rhythm_capsule(ImDrawList* drawList,
                                        ImVec2 min,
                                        ImVec2 size,
                                        const CompactRhythmCapsuleData& data,
                                        CompactRhythmCapsuleStyle style = {}) {
    draw_compact_rhythm_capsule(drawList, min, size, data, default_theme(), style);
}
inline void draw_compact_rhythm_capsule(ImDrawList* drawList,
                                        ImVec2 min,
                                        ImVec2 size,
                                        const CompactRhythmCapsuleData& data,
                                        const Palette& palette,
                                        CompactRhythmCapsuleStyle style = {}) {
    draw_compact_rhythm_capsule(drawList, min, size, data, make_theme(palette), style);
}

void draw_stick_visualizer(const StickVisualizerData& data,
                           ImVec2 size,
                           const Theme& theme,
                           StickVisualizerStyle style = {});
inline void draw_stick_visualizer(const StickVisualizerData& data,
                                  ImVec2 size,
                                  StickVisualizerStyle style = {}) {
    draw_stick_visualizer(data, size, default_theme(), style);
}
inline void draw_stick_visualizer(const StickVisualizerData& data,
                                  ImVec2 size,
                                  const Palette& palette,
                                  StickVisualizerStyle style = {}) {
    draw_stick_visualizer(data, size, make_theme(palette), style);
}

void draw_key_binding_row(ImDrawList* drawList,
                          ImVec2 min,
                          ImVec2 size,
                          const KeyBindingRowData& data,
                          const Theme& theme,
                          KeyBindingRowStyle style = {});
inline void draw_key_binding_row(ImDrawList* drawList,
                                 ImVec2 min,
                                 ImVec2 size,
                                 const KeyBindingRowData& data,
                                 KeyBindingRowStyle style = {}) {
    draw_key_binding_row(drawList, min, size, data, default_theme(), style);
}
inline void draw_key_binding_row(ImDrawList* drawList,
                                 ImVec2 min,
                                 ImVec2 size,
                                 const KeyBindingRowData& data,
                                 const Palette& palette,
                                 KeyBindingRowStyle style = {}) {
    draw_key_binding_row(drawList, min, size, data, make_theme(palette), style);
}

void draw_notice_toast(ImDrawList* drawList,
                       ImVec2 min,
                       ImVec2 size,
                       const NoticeToastData& data,
                       const Theme& theme,
                       NoticeToastStyle style = {});
inline void draw_notice_toast(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 size,
                              const NoticeToastData& data,
                              NoticeToastStyle style = {}) {
    draw_notice_toast(drawList, min, size, data, default_theme(), style);
}
inline void draw_notice_toast(ImDrawList* drawList,
                              ImVec2 min,
                              ImVec2 size,
                              const NoticeToastData& data,
                              const Palette& palette,
                              NoticeToastStyle style = {}) {
    draw_notice_toast(drawList, min, size, data, make_theme(palette), style);
}

void draw_dynamic_island(ImDrawList* drawList,
                         ImVec2 displaySize,
                         const DynamicIslandData& data,
                         const Theme& theme,
                         DynamicIslandStyle style = {});
inline void draw_dynamic_island(ImDrawList* drawList,
                                ImVec2 displaySize,
                                const DynamicIslandData& data,
                                DynamicIslandStyle style = {}) {
    draw_dynamic_island(drawList, displaySize, data, default_theme(), style);
}
inline void draw_dynamic_island(ImDrawList* drawList,
                                ImVec2 displaySize,
                                const DynamicIslandData& data,
                                const Palette& palette,
                                DynamicIslandStyle style = {}) {
    draw_dynamic_island(drawList, displaySize, data, make_theme(palette), style);
}

ConfigExchangeControlResult draw_config_exchange_control(const char* id,
                                                         const ConfigExchangeControlData& data,
                                                         const Theme& theme,
                                                         ConfigExchangeControlStyle style = {});
inline ConfigExchangeControlResult draw_config_exchange_control(const char* id,
                                                                const ConfigExchangeControlData& data,
                                                                ConfigExchangeControlStyle style = {}) {
    return draw_config_exchange_control(id, data, default_theme(), style);
}
inline ConfigExchangeControlResult draw_config_exchange_control(const char* id,
                                                                const ConfigExchangeControlData& data,
                                                                const Palette& palette,
                                                                ConfigExchangeControlStyle style = {}) {
    return draw_config_exchange_control(id, data, make_theme(palette), style);
}

void draw_notice_banner(const char* text,
                        NoticeTone tone,
                        float width,
                        const Theme& theme,
                        NoticeBannerStyle style = {});
inline void draw_notice_banner(const char* text,
                               NoticeTone tone,
                               float width,
                               const Theme& theme,
                               float rounding,
                               float spacingAfter = -1.0f) {
    NoticeBannerStyle style;
    style.rounding = rounding;
    style.spacingAfter = spacingAfter;
    draw_notice_banner(text, tone, width, theme, style);
}
inline void draw_notice_banner(const char* text,
                               NoticeTone tone,
                               float width,
                               NoticeBannerStyle style = {}) {
    draw_notice_banner(text, tone, width, default_theme(), style);
}
inline void draw_notice_banner(const char* text,
                               NoticeTone tone,
                               float width,
                               float rounding,
                               float spacingAfter = -1.0f) {
    NoticeBannerStyle style;
    style.rounding = rounding;
    style.spacingAfter = spacingAfter;
    draw_notice_banner(text, tone, width, default_theme(), style);
}
inline void draw_notice_banner(const char* text,
                               NoticeTone tone,
                               float width,
                               const Palette& palette,
                               NoticeBannerStyle style = {}) {
    draw_notice_banner(text, tone, width, make_theme(palette), style);
}
inline void draw_notice_banner(const char* text,
                               NoticeTone tone,
                               float width,
                               const Palette& palette,
                               float rounding,
                               float spacingAfter = -1.0f) {
    NoticeBannerStyle style;
    style.rounding = rounding;
    style.spacingAfter = spacingAfter;
    draw_notice_banner(text, tone, width, make_theme(palette), style);
}

int draw_two_option_segmented_control(const char* id,
                                      const SegmentedOption& first,
                                      const SegmentedOption& second,
                                      int selectedIndex,
                                      const Theme& theme,
                                      SegmentedControlStyle style = {});
inline int draw_two_option_segmented_control(const char* id,
                                             const SegmentedOption& first,
                                             const SegmentedOption& second,
                                             int selectedIndex,
                                             SegmentedControlStyle style = {}) {
    return draw_two_option_segmented_control(id, first, second, selectedIndex, default_theme(), style);
}
inline int draw_two_option_segmented_control(const char* id,
                                             const SegmentedOption& first,
                                             const SegmentedOption& second,
                                             int selectedIndex,
                                             const Palette& palette,
                                             SegmentedControlStyle style = {}) {
    return draw_two_option_segmented_control(id, first, second, selectedIndex, make_theme(palette), style);
}

int draw_three_option_segmented_control(const char* id,
                                        const SegmentedOption& first,
                                        const SegmentedOption& second,
                                        const SegmentedOption& third,
                                        int selectedIndex,
                                        const Theme& theme,
                                        SegmentedControlStyle style = {});
inline int draw_three_option_segmented_control(const char* id,
                                               const SegmentedOption& first,
                                               const SegmentedOption& second,
                                               const SegmentedOption& third,
                                               int selectedIndex,
                                               SegmentedControlStyle style = {}) {
    return draw_three_option_segmented_control(id, first, second, third, selectedIndex, default_theme(), style);
}
inline int draw_three_option_segmented_control(const char* id,
                                               const SegmentedOption& first,
                                               const SegmentedOption& second,
                                               const SegmentedOption& third,
                                               int selectedIndex,
                                               const Palette& palette,
                                               SegmentedControlStyle style = {}) {
    return draw_three_option_segmented_control(id, first, second, third, selectedIndex, make_theme(palette), style);
}

WindowChromeControlsResult draw_window_chrome_controls(const char* id,
                                                       const WindowChromeControlsData& data,
                                                       const Theme& theme,
                                                       WindowChromeControlsStyle style = {});
inline WindowChromeControlsResult draw_window_chrome_controls(const char* id,
                                                              const WindowChromeControlsData& data,
                                                              WindowChromeControlsStyle style = {}) {
    return draw_window_chrome_controls(id, data, default_theme(), style);
}
inline WindowChromeControlsResult draw_window_chrome_controls(const char* id,
                                                              const WindowChromeControlsData& data,
                                                              const Palette& palette,
                                                              WindowChromeControlsStyle style = {}) {
    return draw_window_chrome_controls(id, data, make_theme(palette), style);
}

void draw_status_pill(const char* label,
                      const char* value,
                      ImVec4 valueColor,
                      const Theme& theme);
ImVec2 status_pill_size(const char* label,
                        const char* value,
                        const Theme& theme);
inline void draw_status_pill(const char* label,
                             const char* value,
                             ImVec4 valueColor) {
    draw_status_pill(label, value, valueColor, default_theme());
}
inline void draw_status_pill(const char* label,
                             const char* value,
                             ImVec4 valueColor,
                             const Palette& palette) {
    draw_status_pill(label, value, valueColor, make_theme(palette));
}
inline ImVec2 status_pill_size(const char* label,
                               const char* value) {
    return status_pill_size(label, value, default_theme());
}
inline ImVec2 status_pill_size(const char* label,
                               const char* value,
                               const Palette& palette) {
    return status_pill_size(label, value, make_theme(palette));
}

WidgetFrameLayout draw_status_pill_group(const StatusPillData* items,
                                         int count,
                                         const Theme& theme,
                                         StatusPillGroupStyle style = {});
inline WidgetFrameLayout draw_status_pill_group(const StatusPillData* items,
                                                int count,
                                                StatusPillGroupStyle style = {}) {
    return draw_status_pill_group(items, count, default_theme(), style);
}
inline WidgetFrameLayout draw_status_pill_group(const StatusPillData* items,
                                                int count,
                                                const Palette& palette,
                                                StatusPillGroupStyle style = {}) {
    return draw_status_pill_group(items, count, make_theme(palette), style);
}

WidgetFrameLayout draw_status_info_list(const StatusInfoRowData* items,
                                        int count,
                                        const Theme& theme,
                                        StatusInfoListStyle style = {});
inline WidgetFrameLayout draw_status_info_list(const StatusInfoRowData* items,
                                               int count,
                                               StatusInfoListStyle style = {}) {
    return draw_status_info_list(items, count, default_theme(), style);
}
inline WidgetFrameLayout draw_status_info_list(const StatusInfoRowData* items,
                                               int count,
                                               const Palette& palette,
                                               StatusInfoListStyle style = {}) {
    return draw_status_info_list(items, count, make_theme(palette), style);
}

WidgetFrameLayout draw_theme_summary(const ThemeSummaryData& data,
                                     const Theme& theme,
                                     ThemeSummaryStyle style = {});
inline WidgetFrameLayout draw_theme_summary(const ThemeSummaryData& data,
                                            ThemeSummaryStyle style = {}) {
    return draw_theme_summary(data, default_theme(), style);
}
inline WidgetFrameLayout draw_theme_summary(const ThemeSummaryData& data,
                                            const Palette& palette,
                                            ThemeSummaryStyle style = {}) {
    return draw_theme_summary(data, make_theme(palette), style);
}

WidgetFrameLayout draw_theme_selection_summary(const ThemeSelectionSummaryData& data,
                                               const Theme& theme,
                                               ThemeSelectionSummaryStyle style = {});
inline WidgetFrameLayout draw_theme_selection_summary(const ThemeSelectionSummaryData& data,
                                                      ThemeSelectionSummaryStyle style = {}) {
    return draw_theme_selection_summary(data, default_theme(), style);
}
inline WidgetFrameLayout draw_theme_selection_summary(const ThemeSelectionSummaryData& data,
                                                      const Palette& palette,
                                                      ThemeSelectionSummaryStyle style = {}) {
    return draw_theme_selection_summary(data, make_theme(palette), style);
}

WidgetFrameLayout draw_background_summary(const BackgroundSummaryData& data,
                                          const Theme& theme,
                                          BackgroundSummaryStyle style = {});
inline WidgetFrameLayout draw_background_summary(const BackgroundSummaryData& data,
                                                 BackgroundSummaryStyle style = {}) {
    return draw_background_summary(data, default_theme(), style);
}
inline WidgetFrameLayout draw_background_summary(const BackgroundSummaryData& data,
                                                 const Palette& palette,
                                                 BackgroundSummaryStyle style = {}) {
    return draw_background_summary(data, make_theme(palette), style);
}

WidgetFrameLayout draw_background_preview(const BackgroundPreviewData& data,
                                          const Theme& theme,
                                          BackgroundPreviewStyle style = {});
inline WidgetFrameLayout draw_background_preview(const BackgroundPreviewData& data,
                                                 BackgroundPreviewStyle style = {}) {
    return draw_background_preview(data, default_theme(), style);
}
inline WidgetFrameLayout draw_background_preview(const BackgroundPreviewData& data,
                                                 const Palette& palette,
                                                 BackgroundPreviewStyle style = {}) {
    return draw_background_preview(data, make_theme(palette), style);
}

bool draw_theme_selection_section(const char* id,
                                  const ThemeSelectionSectionData& data,
                                  const Theme& theme,
                                  const std::function<void(ThemeFlavor)>& activateFlavor,
                                  const std::function<void(BackgroundKind)>& activateBackgroundKind,
                                  ThemeSelectionSectionStyle style = {});
inline bool draw_theme_selection_section(const char* id,
                                         const ThemeSelectionSectionData& data,
                                         const std::function<void(ThemeFlavor)>& activateFlavor,
                                         const std::function<void(BackgroundKind)>& activateBackgroundKind,
                                         ThemeSelectionSectionStyle style = {}) {
    return draw_theme_selection_section(id,
                                        data,
                                        default_theme(),
                                        activateFlavor,
                                        activateBackgroundKind,
                                        style);
}
inline bool draw_theme_selection_section(const char* id,
                                         const ThemeSelectionSectionData& data,
                                         const Palette& palette,
                                         const std::function<void(ThemeFlavor)>& activateFlavor,
                                         const std::function<void(BackgroundKind)>& activateBackgroundKind,
                                         ThemeSelectionSectionStyle style = {}) {
    return draw_theme_selection_section(id,
                                        data,
                                        make_theme(palette),
                                        activateFlavor,
                                        activateBackgroundKind,
                                        style);
}

void draw_status_row(const char* label,
                     const char* value,
                     ImVec4 valueColor,
                     float labelWidth = 102.0f);
void draw_signal_pill(const char* label,
                      bool active,
                      bool live,
                      const Theme& theme);
inline void draw_signal_pill(const char* label,
                             bool active,
                             bool live) {
    draw_signal_pill(label, active, live, default_theme());
}
inline void draw_signal_pill(const char* label,
                             bool active,
                             bool live,
                             const Palette& palette) {
    draw_signal_pill(label, active, live, make_theme(palette));
}

WidgetFrameLayout draw_signal_pill_group(const SignalPillData* items,
                                         int count,
                                         bool live,
                                         const Theme& theme,
                                         SignalPillGroupStyle style = {});
inline WidgetFrameLayout draw_signal_pill_group(const SignalPillData* items,
                                                int count,
                                                bool live,
                                                SignalPillGroupStyle style = {}) {
    return draw_signal_pill_group(items, count, live, default_theme(), style);
}
inline WidgetFrameLayout draw_signal_pill_group(const SignalPillData* items,
                                                int count,
                                                bool live,
                                                const Palette& palette,
                                                SignalPillGroupStyle style = {}) {
    return draw_signal_pill_group(items, count, live, make_theme(palette), style);
}

void draw_state_box(const char* label,
                    bool active,
                    ImVec4 accent,
                    ImVec2 size,
                    const Theme& theme);
inline void draw_state_box(const char* label,
                           bool active,
                           ImVec4 accent,
                           ImVec2 size) {
    draw_state_box(label, active, accent, size, default_theme());
}
inline void draw_state_box(const char* label,
                           bool active,
                           ImVec4 accent,
                           ImVec2 size,
                           const Palette& palette) {
    draw_state_box(label, active, accent, size, make_theme(palette));
}

bool draw_outline_action_button(const char* id,
                                const char* label,
                                ImVec2 size,
                                ImVec4 accent,
                                const Theme& theme,
                                float fillAlpha = 0.14f,
                                float borderAlpha = 1.0f,
                                ImVec4 textColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f));
inline bool draw_outline_action_button(const char* id,
                                       const char* label,
                                       ImVec2 size,
                                       ImVec4 accent,
                                       float fillAlpha = 0.14f,
                                       float borderAlpha = 1.0f,
                                       ImVec4 textColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f)) {
    return draw_outline_action_button(id, label, size, accent, default_theme(), fillAlpha, borderAlpha, textColor);
}
inline bool draw_outline_action_button(const char* id,
                                       const char* label,
                                       ImVec2 size,
                                       ImVec4 accent,
                                       const Palette& palette,
                                       float fillAlpha = 0.14f,
                                       float borderAlpha = 1.0f,
                                       ImVec4 textColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f)) {
    return draw_outline_action_button(id, label, size, accent, make_theme(palette), fillAlpha, borderAlpha, textColor);
}

bool draw_primary_action_button(const char* id,
                                const char* label,
                                ImVec2 size,
                                bool enabled,
                                const Theme& theme);
inline bool draw_primary_action_button(const char* id,
                                       const char* label,
                                       ImVec2 size,
                                       bool enabled) {
    return draw_primary_action_button(id, label, size, enabled, default_theme());
}
inline bool draw_primary_action_button(const char* id,
                                       const char* label,
                                       ImVec2 size,
                                       bool enabled,
                                       const Palette& palette) {
    return draw_primary_action_button(id, label, size, enabled, make_theme(palette));
}

bool draw_option_toggle_button(const char* id,
                               const char* label,
                               bool& value,
                               float width,
                               const Theme& theme,
                               OptionToggleStyle style = {});
bool draw_option_toggle_button(const char* id,
                               const char* label,
                               bool& value,
                               const Theme& theme,
                               OptionToggleStyle style = {});
inline bool draw_option_toggle_button(const char* id,
                                      const char* label,
                                      bool& value,
                                      float width) {
    return draw_option_toggle_button(id, label, value, width, default_theme(), {});
}
inline bool draw_option_toggle_button(const char* id,
                                      const char* label,
                                      bool& value,
                                      OptionToggleStyle style = {}) {
    return draw_option_toggle_button(id, label, value, default_theme(), style);
}
inline bool draw_option_toggle_button(const char* id,
                                      const char* label,
                                      bool& value,
                                      float width,
                                      const Palette& palette) {
    return draw_option_toggle_button(id, label, value, width, make_theme(palette), {});
}
inline bool draw_option_toggle_button(const char* id,
                                      const char* label,
                                      bool& value,
                                                const Palette& palette,
                                                OptionToggleStyle style = {}) {
    return draw_option_toggle_button(id, label, value, make_theme(palette), style);
}

bool draw_direction_toggle_button(const char* id,
                                  const char* clockwiseLabel,
                                  const char* counterclockwiseLabel,
                                  bool& clockwise,
                                  const Theme& theme,
                                  DirectionToggleStyle style = {});
inline bool draw_direction_toggle_button(const char* id,
                                         const char* clockwiseLabel,
                                         const char* counterclockwiseLabel,
                                         bool& clockwise,
                                         DirectionToggleStyle style = {}) {
    return draw_direction_toggle_button(id, clockwiseLabel, counterclockwiseLabel, clockwise, default_theme(), style);
}
inline bool draw_direction_toggle_button(const char* id,
                                         const char* clockwiseLabel,
                                         const char* counterclockwiseLabel,
                                         bool& clockwise,
                                         const Palette& palette,
                                         DirectionToggleStyle style = {}) {
    return draw_direction_toggle_button(id,
                                        clockwiseLabel,
                                        counterclockwiseLabel,
                                        clockwise,
                                        make_theme(palette),
                                        style);
}

bool draw_option_toggle_button_with_help(const char* id,
                                         const char* label,
                                         bool& value,
                                         const char* helpText,
                                         float width,
                                         const Theme& theme);
inline bool draw_option_toggle_button_with_help(const char* id,
                                                const char* label,
                                                bool& value,
                                                const Theme& theme,
                                                OptionToggleStyle style = {}) {
    return draw_option_toggle_button(id, label, value, theme, style);
}
inline bool draw_option_toggle_button_with_help(const char* id,
                                                const char* label,
                                                bool& value,
                                                const char* helpText,
                                                float width) {
    return draw_option_toggle_button_with_help(id, label, value, helpText, width, default_theme());
}
inline bool draw_option_toggle_button_with_help(const char* id,
                                                const char* label,
                                                bool& value,
                                                OptionToggleStyle style = {}) {
    return draw_option_toggle_button(id, label, value, default_theme(), style);
}
inline bool draw_option_toggle_button_with_help(const char* id,
                                                const char* label,
                                                bool& value,
                                                const char* helpText,
                                                float width,
                                                const Palette& palette) {
    return draw_option_toggle_button_with_help(id, label, value, helpText, width, make_theme(palette));
}
inline bool draw_option_toggle_button_with_help(const char* id,
                                                const char* label,
                                                bool& value,
                                                const Palette& palette,
                                                OptionToggleStyle style = {}) {
    return draw_option_toggle_button(id, label, value, make_theme(palette), style);
}

void draw_help_marker(const char* text, const Theme& theme);
inline void draw_help_marker(const char* text) {
    draw_help_marker(text, default_theme());
}
inline void draw_help_marker(const char* text, const Palette& palette) {
    draw_help_marker(text, make_theme(palette));
}

void same_line_help_marker_aligned_to_last_item(const char* text, const Theme& theme);
inline void same_line_help_marker_aligned_to_last_item(const char* text) {
    same_line_help_marker_aligned_to_last_item(text, default_theme());
}
inline void same_line_help_marker_aligned_to_last_item(const char* text, const Palette& palette) {
    same_line_help_marker_aligned_to_last_item(text, make_theme(palette));
}

void flush_help_marker_overlays();

} // namespace imgui_onguoin

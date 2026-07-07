// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_theme_types.h"

#include <cstddef>

namespace imgui_onguoin {

struct SurfaceStyle {
    float rounding = -1.0f;
    float glowAlpha = -1.0f;
    float borderBoost = 1.0f;
};

struct SurfacePanelStyle {
    SurfaceStyle surface{};
    ImVec2 padding = ImVec2(-1.0f, -1.0f);
    bool topHighlight = false;
    float topHighlightAlpha = 0.018f;
    float topHighlightHeight = 0.34f;
    ImGuiChildFlags childLayoutFlags = ImGuiChildFlags_None;
    ImGuiWindowFlags childFlags =
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;
};

struct NoticeBannerStyle {
    ImVec2 padding = ImVec2(-1.0f, -1.0f);
    float rounding = -1.0f;
    float spacingAfter = 8.0f;
};

enum class OptionToggleVisualEffect {
    None = 0,
    NeonFlow,
};

struct NeonCapsuleGlowStyle {
    ImVec4 colorA = ImVec4(0.10f, 0.88f, 0.96f, 1.0f);
    ImVec4 colorB = ImVec4(0.78f, 0.26f, 1.0f, 1.0f);
    ImVec4 colorC = ImVec4(0.95f, 0.58f, 0.20f, 1.0f);
    float intensity = 1.0f;
    float speed = 0.34f;
    float glowScale = 1.0f;
    float idleFlowAlpha = 0.16f;
};

struct OptionToggleStyle {
    float width = 0.0f;
    const char* helpText = nullptr;
    ImVec4 borderColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 selectedBorderColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 selectedFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 selectedTextColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    float selectedBorderAlpha = -1.0f;
    OptionToggleVisualEffect visualEffect = OptionToggleVisualEffect::None;
    ImVec4 flowColorA = ImVec4(0.10f, 0.88f, 0.96f, 1.0f);
    ImVec4 flowColorB = ImVec4(0.78f, 0.26f, 1.0f, 1.0f);
    ImVec4 flowColorC = ImVec4(0.95f, 0.58f, 0.20f, 1.0f);
    float flowIntensity = 1.0f;
    float flowSpeed = 0.34f;
    float flowGlowScale = 1.0f;
    float flowIdleAlpha = 0.16f;
};

struct DirectionToggleStyle {
    float width = 0.0f;
    float height = -1.0f;
    float knobDiameter = -1.0f;
    const char* helpText = nullptr;
};

struct SegmentedOption {
    const char* id = nullptr;
    const char* label = nullptr;
};

struct SegmentedControlStyle {
    float height = -1.0f;
    float gap = -1.0f;
    float rounding = -1.0f;
    float borderThickness = -1.0f;
    ImVec2 firstSize = ImVec2(-1.0f, -1.0f);
    ImVec2 secondSize = ImVec2(-1.0f, -1.0f);
    ImVec2 thirdSize = ImVec2(-1.0f, -1.0f);
    ImVec4 baseFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 hoveredFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 selectedFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 middleFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 borderColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 baseTextColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 selectedTextColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 accentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct WindowChromeControlsData {
    SegmentedOption firstOption{};
    SegmentedOption secondOption{};
    int selectedIndex = 0;
    bool showLanguageSegment = true;
};

struct WindowChromeControlsStyle {
    SegmentedControlStyle languageSegment{};
    float gap = -1.0f;
    ImVec2 minimizeButtonSize = ImVec2(-1.0f, -1.0f);
    ImVec2 closeButtonSize = ImVec2(-1.0f, -1.0f);
    ImVec4 minimizeAccentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 closeAccentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    float buttonFillAlpha = 0.05f;
    float buttonBorderAlpha = 1.0f;
};

struct TitledPanelStyle {
    SurfacePanelStyle panel{};
    ImVec2 titleOffset = ImVec2(-1.0f, -1.0f);
    float titleSpacing = -1.0f;
};

struct AutoPanelStyle {
    TitledPanelStyle titled{};
    float fallbackHeight = -1.0f;
    float minHeight = -1.0f;
    float maxHeight = -1.0f;
    float bottomPadding = -1.0f;
    bool freezeHeightWhileParentScrolling = false;
    bool measureHeightWhenClipped = false;
};

struct AutoSurfacePanelStyle {
    SurfacePanelStyle panel{};
    float fallbackHeight = -1.0f;
    float minHeight = -1.0f;
    float maxHeight = -1.0f;
    float bottomPadding = -1.0f;
    bool freezeHeightWhileParentScrolling = false;
    bool measureHeightWhenClipped = false;
};

enum class FormLayoutPreset {
    ThemeDefault = 0,
    Standard,
    Wide,
};

struct FormLayoutStyle {
    float insetX = -1.0f;
    float nestedInsetX = -1.0f;
    float preferredColumnX = -1.0f;
    float minimumLabelGap = -1.0f;
    float rowSpacing = -1.0f;
    float sectionSpacing = -1.0f;
    float footerSpacing = -1.0f;
};

struct BufferedInputState {
    char* buffer = nullptr;
    int bufferSize = 0;
    bool* editing = nullptr;
};

struct BufferedRangeInputState {
    BufferedInputState min{};
    BufferedInputState max{};
};

template <std::size_t N>
struct BufferedInputStorage {
    char buffer[N] = {};
    bool editing = false;
};

template <std::size_t N>
struct BufferedRangeInputStorage {
    BufferedInputStorage<N> min{};
    BufferedInputStorage<N> max{};
};

enum class SliderFieldSpectralEffect {
    None = 0,
    TextSheen,
    RainbowText,
    PixelBloom,
};

enum class SliderFieldHandleShape {
    Default = 0,
    Circle,
};

struct SliderFieldStyle {
    float sliderWidth = -1.0f;
    float inputWidth = -1.0f;
    float frameRounding = -1.0f;
    ImVec4 labelColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    bool showRangeValueLabels = true;
    SliderFieldSpectralEffect spectralEffect = SliderFieldSpectralEffect::None;
    const char* spectralText = nullptr;
    float spectralIntensity = 1.0f;
    bool drawCustomSlider = false;
    SliderFieldHandleShape handleShape = SliderFieldHandleShape::Default;
    float handleRadius = -1.0f;
    ImVec4 handleFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 handleBorderColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 handleGlowColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    float handleGlowScale = 1.0f;
    ImVec4 valueTextColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 valueTextShadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.38f);
    const int* tickValues = nullptr;
    int tickValueCount = 0;
    ImVec4 tickColor = ImVec4(0.96f, 0.98f, 1.0f, 0.92f);
    ImVec4 tickGlowColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    float tickRadius = -1.0f;
    float tickGlowScale = 1.0f;
    float magneticSnapResistance = 0.0f;
    int magneticSnapWindow = 0;
    ImVec4 spectralColorA = ImVec4(0.104f, 0.048f, 0.255f, 1.0f);
    ImVec4 spectralColorB = ImVec4(0.50f, 0.14f, 0.92f, 1.0f);
    ImVec4 spectralColorC = ImVec4(0.70f, 0.91f, 1.0f, 1.0f);
};

struct FieldInteractionState {
    bool sliderHovered = false;
    bool sliderActive = false;
    bool inputHovered = false;
    bool inputActive = false;
    bool hovered = false;
    bool active = false;
};

struct SelectFieldStyle {
    float comboWidth = -1.0f;
    float frameRounding = -1.0f;
    float popupRounding = -1.0f;
    ImVec2 popupPadding = ImVec2(-1.0f, -1.0f);
};

struct AnimatedComboScope {
    bool open = false;
    bool contentStylePushed = false;
    int styleVarCount = 0;
};

struct ThemeFlavorFieldText {
    const char* label = nullptr;
    const char* helpText = nullptr;
};

struct ThemeFlavorFieldData {
    UiTextLanguage language = UiTextLanguage::English;
    ThemeFlavorFieldText text{};
};

struct BackgroundKindFieldText {
    const char* label = nullptr;
    const char* helpText = nullptr;
};

struct BackgroundKindFieldData {
    UiTextLanguage language = UiTextLanguage::English;
    BackgroundKindFieldText text{};
};

struct PageHeroText {
    const char* eyebrow = nullptr;
    const char* title = nullptr;
    const char* subtitle = nullptr;
    const char* status = nullptr;
};

struct FooterSignatureStyle {
    float alpha = 0.78f;
    float fontScale = -1.0f;
    float lineWidth = -1.0f;
    float gap = -1.0f;
    ImVec4 lineColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 textColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct IdentityCapsuleData {
    const char* label = nullptr;
    const char* value = nullptr;
};

struct IdentityCapsuleStyle {
    float height = -1.0f;
    float minWidth = -1.0f;
    float maxWidth = -1.0f;
    float rounding = -1.0f;
    float horizontalPadding = -1.0f;
    float textGap = -1.0f;
    float labelFontScale = -1.0f;
    float valueFontScale = -1.0f;
    float textOffsetY = -1.0f;
    ImVec4 fillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 borderColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 innerBorderColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 labelColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 valueColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct CompactOverlayShellData {
    const char* title = nullptr;
    const char* statusLabel = nullptr;
    const char* statusValue = nullptr;
    const char* centerStatusText = nullptr;
};

struct CompactOverlayShellStyle {
    float frameInset = -1.0f;
    float frameRounding = -1.0f;
    float frameBorderThickness = -1.0f;
    float frameGlowAlpha = -1.0f;
    float headerInsetX = -1.0f;
    float headerTitleOffsetY = -1.0f;
    float headerValueOffsetY = -1.0f;
    float headerGap = -1.0f;
    float headerDividerY = -1.0f;
    float sectionTopY = -1.0f;
    float centerStatusOffsetY = -1.0f;
    float centerStatusPaddingX = -1.0f;
    float centerStatusPaddingY = -1.0f;
    float centerStatusRounding = -1.0f;
    float centerStatusFillAlpha = -1.0f;
    float centerStatusBorderAlpha = -1.0f;
    ImVec4 frameColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 titleColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 statusLabelColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 statusValueColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 centerStatusColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 dividerColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct CompactInputMatrixKey {
    const char* label = nullptr;
    bool active = false;
    ImVec4 accentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct CompactInputMatrixData {
    CompactInputMatrixKey topLeft{};
    CompactInputMatrixKey bottomLeft{};
    CompactInputMatrixKey topCenter{};
    CompactInputMatrixKey bottomCenterLeft{};
    CompactInputMatrixKey bottomCenter{};
    CompactInputMatrixKey bottomCenterRight{};
};

struct CompactInputMatrixStyle {
    float horizontalInset = -1.0f;
    float topRowY = -1.0f;
    float bottomRowY = -1.0f;
    float keyHeight = -1.0f;
    float keyGap = -1.0f;
    float sideWidthRatio = -1.0f;
    float sideWidthMin = -1.0f;
    float sideWidthMax = -1.0f;
};

struct CompactRhythmBlock {
    float ageSeconds = 0.0f;
    float laneOffset = 0.0f;
    float strength = 1.0f;
};

struct CompactRhythmCapsuleData {
    float speed01 = 0.0f;
    const CompactRhythmBlock* blocks = nullptr;
    int blockCount = 0;
};

struct CompactRhythmCapsuleStyle {
    float rounding = -1.0f;
    float borderThickness = -1.0f;
    float pulseRadius = -1.0f;
    float pulseGlowScale = 1.0f;
    float blockWidth = -1.0f;
    float blockHeight = -1.0f;
    float blockGap = -1.0f;
    float blockTravelSpeed = -1.0f;
    float waveformSpikeMinHeight = -1.0f;
    float waveformSpikeMaxHeight = -1.0f;
    float waveformSpikeThickness = -1.0f;
    float waveformGlowScale = 1.0f;
    float innerPaddingX = -1.0f;
    ImVec4 fillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 borderColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 pulseColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 blockColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 waveformCoreColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 waveformGlowColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct StatusPillData {
    const char* label = nullptr;
    const char* value = nullptr;
    ImVec4 valueColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

enum class StatusPillGroupLayout {
    Horizontal = 0,
    Vertical,
    Wrap,
};

struct StatusPillGroupStyle {
    StatusPillGroupLayout layout = StatusPillGroupLayout::Horizontal;
    float gap = -1.0f;
    float rowGap = -1.0f;
    float wrapWidth = -1.0f;
};

struct StatusInfoRowData {
    const char* label = nullptr;
    const char* value = nullptr;
    ImVec4 valueColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct StatusInfoListStyle {
    float labelWidth = -1.0f;
    float rowGap = -1.0f;
};

struct ThemeSummaryText {
    const char* toneLabel = nullptr;
    const char* backgroundLabel = nullptr;
    const char* motionLabel = nullptr;
    const char* expressiveMotion = nullptr;
    const char* restrainedMotion = nullptr;
};

struct ThemeSummaryData {
    ThemeFlavor flavor = ThemeFlavor::Onguoin;
    UiTextLanguage language = UiTextLanguage::English;
    ThemeSummaryText text{};
};

struct ThemeSummaryStyle {
    float labelWidth = -1.0f;
    float rowGap = -1.0f;
};

struct ThemeSelectionSummaryText {
    const char* presetLabel = nullptr;
    const char* modeLabel = nullptr;
    const char* customPreset = nullptr;
    const char* matchedPresetSuffix = nullptr;
};

struct ThemeSelectionSummaryData {
    ThemeSelection selection{};
    UiTextLanguage language = UiTextLanguage::English;
    ThemeSelectionSummaryText text{};
};

struct ThemeSelectionSummaryStyle {
    float labelWidth = -1.0f;
    float rowGap = -1.0f;
};

struct BackgroundSummaryText {
    const char* shellRecommended = nullptr;
    const char* shellMinimal = nullptr;
    const char* shellLabel = nullptr;
    const char* characterLabel = nullptr;
    const char* capabilityLabel = nullptr;
    const char* layersLabel = nullptr;
    const char* dynamicSuffix = nullptr;
    const char* noLayersText = nullptr;
};

struct BackgroundSummaryData {
    BackgroundKind kind = BackgroundKind::Aurora;
    const BackgroundStyle* style = nullptr;
    UiTextLanguage language = UiTextLanguage::English;
    BackgroundSummaryText text{};
};

struct BackgroundSummaryStyle {
    float labelWidth = -1.0f;
    float rowGap = -1.0f;
};

struct BackgroundPreviewText {
    const char* title = nullptr;
    const char* subtitle = nullptr;
};

struct BackgroundPreviewData {
    ThemeFlavor flavor = ThemeFlavor::Onguoin;
    BackgroundKind kind = BackgroundKind::Aurora;
    const BackgroundStyle* style = nullptr;
    const BackgroundTuning* tuning = nullptr;
    UiTextLanguage language = UiTextLanguage::English;
    bool authenticated = true;
    BackgroundPreviewText text{};
};

struct BackgroundPreviewStyle {
    ImVec2 size = ImVec2(-1.0f, -1.0f);
    SurfaceStyle surface{};
    float previewInset = -1.0f;
    float overlayInset = -1.0f;
    float overlayGap = -1.0f;
    float overlayScrimAlpha = 0.16f;
};

struct ThemeSelectionSectionData {
    ThemeSelection selection{};
    const BackgroundStyle* backgroundStyle = nullptr;
    UiTextLanguage language = UiTextLanguage::English;
    bool showBackgroundPreview = true;
    bool showDetails = true;
    bool previewAuthenticated = true;
    ThemeFlavorFieldText themeFlavorFieldText{};
    BackgroundKindFieldText backgroundKindFieldText{};
    ThemeSummaryText themeSummaryText{};
    ThemeSelectionSummaryText selectionSummaryText{};
    BackgroundSummaryText backgroundSummaryText{};
    BackgroundPreviewText backgroundPreviewText{};
};

struct ThemeSelectionSectionStyle {
    FormLayoutStyle layout{};
    SelectFieldStyle field{};
    BackgroundPreviewStyle backgroundPreview{};
    ThemeSummaryStyle themeSummary{};
    ThemeSelectionSummaryStyle selectionSummary{};
    BackgroundSummaryStyle backgroundSummary{};
    float fieldGap = 8.0f;
    float previewGap = 8.0f;
    float summaryStartGap = 6.0f;
    float summaryGap = 8.0f;
    float insetX = -1.0f;
};

struct SignalPillData {
    const char* label = nullptr;
    bool active = false;
};

enum class SignalPillGroupLayout {
    Horizontal = 0,
    Wrap,
};

struct SignalPillGroupStyle {
    SignalPillGroupLayout layout = SignalPillGroupLayout::Wrap;
    float gap = -1.0f;
    float rowGap = -1.0f;
    float wrapWidth = -1.0f;
};

struct StickVisualizerPoint {
    float x = 0.0f;
    float y = 0.0f;
};

struct StickVisualizerData {
    const char* title = nullptr;
    const char* statusLabel = nullptr;
    const char* statusValue = nullptr;
    const char* sectorText = nullptr;
    const char* targetText = nullptr;
    const char* currentText = nullptr;
    ImVec4 statusColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    StickVisualizerPoint target{};
    StickVisualizerPoint current{};
    float innerDeadZoneRatio = 0.0f;
    float outerDeadZoneRatio = 0.0f;
    float diagonalRangeDegrees = 0.0f;
};

struct StickVisualizerStyle {
    float rounding = -1.0f;
    float radiusRatio = -1.0f;
    float centerYRatio = -1.0f;
    ImVec4 targetColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 currentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct KeyBindingRowData {
    const char* title = nullptr;
    const char* description = nullptr;
    const char* keyText = nullptr;
    const char* hintText = nullptr;
    bool selected = false;
    bool blocked = false;
    bool hovered = false;
};

struct KeyBindingRowStyle {
    float rounding = -1.0f;
    float keyPillWidth = -1.0f;
    ImVec4 accentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 baseFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 raisedFillColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct NoticeToastData {
    const char* title = nullptr;
    const char* body = nullptr;
    const char* footer = nullptr;
    const char* iconText = nullptr;
    float alpha = 1.0f;
    float dismissProgress = 1.0f;
};

struct NoticeToastStyle {
    float rounding = -1.0f;
    ImVec4 accentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct DynamicIslandData {
    const char* collapsedText = nullptr;
    const char* title = nullptr;
    const char* body = nullptr;
    const char* footer = nullptr;
    float expanded = 0.0f;
};

struct DynamicIslandStyle {
    ImVec4 accentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct ConfigExchangeControlData {
    char* buffer = nullptr;
    int bufferSize = 0;
    const char* placeholder = nullptr;
};

struct ConfigExchangeControlStyle {
    float collapsedSize = -1.0f;
    float expandedWidth = -1.0f;
    float height = -1.0f;
    float rounding = -1.0f;
    ImVec4 accentColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct ConfigExchangeControlResult {
    bool edited = false;
    bool committed = false;
    bool hovered = false;
    bool active = false;
    bool expanded = false;
};

struct InteractionVisualState {
    bool hovered = false;
    bool active = false;
    bool selected = false;
    bool enabled = true;
};

struct AccentFrameStyle {
    float rounding = 0.0f;
    ImVec4 color = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    float idleBorderAlpha = 0.10f;
    float hoveredBorderAlpha = 0.25f;
    float activeBorderAlpha = 0.46f;
    float selectedBorderAlpha = 0.70f;
    float idleGlowAlpha = 0.08f;
    float hoveredGlowAlpha = 0.20f;
    float activeGlowAlpha = 0.36f;
    float selectedGlowAlpha = 0.24f;
    float idleThickness = 1.0f;
    float activeThickness = 1.6f;
    float selectedThickness = 1.4f;
};

struct CapsuleStyle {
    float rounding = 0.0f;
    ImVec4 fill = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImVec4 textColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec2 textOffset = ImVec2(0.0f, 0.0f);
    AccentFrameStyle frame{};
};

struct InteractiveSurfaceStyle {
    float rounding = 0.0f;
    ImVec4 fill = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImVec4 textColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec2 textOffset = ImVec2(0.0f, 0.0f);
    AccentFrameStyle frame{};
    float topHighlightAlpha = 0.0f;
    float topHighlightHeightFraction = 0.45f;
    float topLineInset = 0.0f;
    float topLineOffsetY = 0.0f;
    ImVec4 topLineColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    float innerPulseAlpha = 0.0f;
    ImVec4 innerPulseColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
};

struct NoticeSurfaceStyle {
    float rounding = 0.0f;
    ImVec4 fill = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImVec4 border = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 textColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec2 padding = ImVec2(0.0f, 0.0f);
};

struct StatusPillStyle {
    float rounding = 0.0f;
    ImVec4 fill = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImVec4 labelColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 valueColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 indicatorColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    ImVec4 baselineColor = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    float indicatorInsetY = 0.0f;
    float indicatorWidth = 0.0f;
    float baselineInsetStart = 0.0f;
    float baselineInsetEnd = 0.0f;
    float labelLead = 0.0f;
    float valueInset = 0.0f;
};

struct TextBlockMetrics {
    ImVec2 size{};
    float wrapWidth = 0.0f;
};

struct WidgetFrameLayout {
    ImVec2 min{};
    ImVec2 max{};
};

struct WindowChromeControlsResult {
    int selectedIndex = 0;
    bool minimizePressed = false;
    bool closePressed = false;
};

} // namespace imgui_onguoin

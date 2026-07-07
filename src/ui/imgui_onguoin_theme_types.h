// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_background_types.h"

namespace imgui_onguoin {

struct RadiusTokens {
    float window = 14.0f;
    float child = 12.0f;
    float frame = 10.0f;
    float popup = 12.0f;
    float grab = 8.0f;
    float scrollbar = 8.0f;
    float surface = 12.0f;
    float button = 12.0f;
    float control = 8.0f;
    float dock = 12.0f;
};

struct SpacingTokens {
    ImVec2 windowPadding = ImVec2(0.0f, 0.0f);
    ImVec2 framePadding = ImVec2(12.0f, 8.0f);
    ImVec2 itemSpacing = ImVec2(12.0f, 10.0f);
    ImVec2 itemInnerSpacing = ImVec2(8.0f, 6.0f);
    float scrollbarSize = 12.0f;
    float heroAccentWidth = 4.0f;
    float heroLeadGap = 6.0f;
    float heroTailGap = 10.0f;
    float heroFlowGap = 18.0f;
};

struct MotionTokens {
    PulseMotionStyle glowPulse{2.2f, 0.70f, 1.0f, 0.0f, EasingCurve::InOutSine};
    PulseMotionStyle dockPulse{2.0f, 0.55f, 1.0f, 0.0f, EasingCurve::InOutSine};
    PulseMotionStyle primaryActionPulse{2.3f, 0.58f, 1.0f, 0.0f, EasingCurve::InOutSine};
    PulseMotionStyle togglePulse{2.3f, 0.65f, 1.0f, 0.35f, EasingCurve::InOutSine};
    PulseMotionStyle signalPulse{5.0f, 0.72f, 1.0f, 0.0f, EasingCurve::InOutSine};
    PulseMotionStyle focusPulse{4.2f, 0.0f, 1.0f, 0.0f, EasingCurve::InOutSine};
    PresenceMotionStyle bindingIslandPresence{
        ProgressMotionStyle{5.8f, 0.010f, EasingCurve::Linear},
        ProgressMotionStyle{4.8f, 0.010f, EasingCurve::Linear},
    };
    PresenceMotionStyle noticeToastPresence{
        ProgressMotionStyle{10.0f, 0.010f, EasingCurve::Linear},
        ProgressMotionStyle{7.0f, 0.010f, EasingCurve::Linear},
    };
    FollowMotionStyle sidebarSelectionFollow{8.5f, 0.001f, EasingCurve::Linear};
    FollowMotionStyle sidebarIndicatorFollow{12.0f, 0.5f, EasingCurve::OutCubic};
    ProgressMotionStyle pageReveal{5.5f, 0.001f, EasingCurve::Linear};
    FollowMotionStyle loginEntranceFollow{6.0f, 0.005f, EasingCurve::Linear};
    TravelMotionStyle flowLine{24.0f, 0.0f};
};

struct SurfaceTokens {
    ImVec4 frameBg{};
    ImVec4 frameBgHovered{};
    ImVec4 frameBgActive{};
    ImVec4 buttonBg{};
    ImVec4 buttonBgHovered{};
    ImVec4 buttonBgActive{};
    ImVec4 headerBg{};
    ImVec4 headerBgHovered{};
    ImVec4 headerBgActive{};
    ImVec4 scrollbarBg{};
    ImVec4 scrollbarGrab{};
    ImVec4 scrollbarGrabHovered{};
    ImVec4 dockOuter{};
    ImVec4 dockInner{};
    ImVec4 statusPillBg{};
    float cardGlowAlpha = 0.08f;
    float innerHighlightAlpha = 0.018f;
    float buttonTopHighlightAlpha = 0.030f;
    float buttonTopHighlightDisabledAlpha = 0.012f;
};

struct NoticeToneStyle {
    ImVec4 fill{};
    ImVec4 border{};
    ImVec4 text{};
};

struct HeroWidgetTokens {
    float heroTitleScale = 1.55f;
    float heroAccentInset = 16.0f;
    float heroAccentHeight = 34.0f;
    float heroTitleOffsetY = -2.0f;
    float heroStatusOffsetY = 8.0f;
};

struct StatusPillWidgetTokens {
    float statusPillHeight = 24.0f;
    float statusPillMinWidth = 84.0f;
    float statusPillHorizontalPadding = 9.0f;
    float statusPillValueGap = 10.0f;
    float statusPillIndicatorInsetY = 4.0f;
    float statusPillIndicatorWidth = 2.0f;
    float statusPillLabelLead = 18.0f;
    float statusPillValueInset = 8.0f;
    float statusPillLineInsetStart = 8.0f;
    float statusPillLineInsetEnd = 5.0f;
};

struct StatusPillGroupWidgetTokens {
    float gap = 8.0f;
    float rowGap = 8.0f;
};

struct StatusInfoListWidgetTokens {
    float rowGap = 8.0f;
};

struct SignalPillWidgetTokens {
    float height = 28.0f;
    float minWidth = 44.0f;
    float horizontalPadding = 18.0f;
};

struct SignalPillGroupWidgetTokens {
    float gap = 8.0f;
    float rowGap = 8.0f;
};

struct NoticeWidgetTokens {
    ImVec2 noticeBannerPadding = ImVec2(12.0f, 10.0f);
    float noticeBannerRounding = 8.0f;
    float noticeBannerSpacingAfter = 8.0f;
};

struct HelpMarkerWidgetTokens {
    float helpInlineGap = 6.0f;
    float helpMarkerSize = 28.0f;
    float helpMarkerRounding = 8.0f;
    float helpMarkerGlyphOffsetY = -0.5f;
    ImVec4 collapsedFill = ImVec4(0.018f, 0.024f, 0.038f, 1.0f);
    float collapsedBorderAlpha = 0.34f;
    float collapsedGlowAlpha = 0.008f;
    float collapsedInnerHighlightAlpha = 0.014f;
    float hoveredGlowAlpha = 0.040f;
    float hoveredInnerHighlightAlpha = 0.050f;
    float tooltipWrapWidth = 26.0f;
    float tooltipCollapsedSize = 28.0f;
    float tooltipExpandedMinWidth = 220.0f;
    float tooltipExpandedMaxWidth = 360.0f;
    float tooltipPaddingX = 18.0f;
    float tooltipPaddingY = 14.0f;
    float tooltipIconTextGap = 10.0f;
    float tooltipTextOffsetY = -1.0f;
    float tooltipGap = 8.0f;
    float tooltipFollowSpeed = 13.5f;
};

struct FooterSignatureWidgetTokens {
    float fontScale = 0.64f;
    float lineWidth = 16.0f;
    float gap = 8.0f;
    float lineAlpha = 0.78f;
};

struct IdentityCapsuleWidgetTokens {
    float height = 24.0f;
    float minWidth = 144.0f;
    float maxWidth = 246.0f;
    float rounding = 5.0f;
    float horizontalPadding = 11.0f;
    float textGap = 8.0f;
    float labelFontScale = 0.72f;
    float valueFontScale = 0.82f;
    float textOffsetY = -0.5f;
    float borderAlpha = 0.24f;
    float innerBorderAlpha = 0.03f;
    float labelAlpha = 0.62f;
    float valueAlpha = 0.96f;
};

struct CompactOverlayShellWidgetTokens {
    float frameInset = 8.0f;
    float frameRounding = 12.0f;
    float frameBorderThickness = 1.2f;
    float frameGlowAlpha = 0.08f;
    float headerInsetX = 14.0f;
    float headerTitleOffsetY = 10.0f;
    float headerValueOffsetY = 12.0f;
    float headerGap = 8.0f;
    float headerDividerY = 36.0f;
    float sectionTopY = 48.0f;
    float centerStatusOffsetY = 8.0f;
    float centerStatusPaddingX = 9.0f;
    float centerStatusPaddingY = 3.5f;
    float centerStatusRounding = 8.0f;
    float centerStatusFillAlpha = 0.42f;
    float centerStatusBorderAlpha = 0.32f;
};

struct CompactInputMatrixWidgetTokens {
    float horizontalInset = 14.0f;
    float topRowY = 48.0f;
    float bottomRowY = 86.0f;
    float keyHeight = 30.0f;
    float keyGap = 8.0f;
    float sideWidthRatio = 0.33f;
    float sideWidthMin = 88.0f;
    float sideWidthMax = 102.0f;
    float inactiveFillAlpha = 0.16f;
    float activeFillAlpha = 0.34f;
    float inactiveBorderAlpha = 0.72f;
    float activeBorderAlpha = 0.92f;
    float inactiveTextAlpha = 0.92f;
    float activeTextAlpha = 1.0f;
    float inactiveBorderAccentMix = 0.10f;
    float inactiveTextAccentMix = 0.24f;
    float inactiveBorderThickness = 1.15f;
    float activeBorderThickness = 1.45f;
    float activeGlowAlpha = 0.10f;
};

struct StickVisualizerWidgetTokens {
    float frameRounding = 10.0f;
    float glowAlpha = 0.13f;
    float radiusRatio = 0.29f;
    float centerYRatio = 0.46f;
    float targetPointRadius = 5.0f;
    float currentPointRadius = 7.0f;
    float currentRingRadius = 9.5f;
    float infoPanelInsetX = 12.0f;
    float infoPanelBottomInset = 12.0f;
    float infoPanelRounding = 8.0f;
};

struct KeyBindingRowWidgetTokens {
    float rowRounding = 10.0f;
    float rowGlowAlpha = 0.16f;
    float keyPillWidth = 172.0f;
    float keyPillHeight = 32.0f;
    float keyPillRounding = 8.0f;
    float keyPillInsetX = 16.0f;
};

struct NoticeToastWidgetTokens {
    float rounding = 12.0f;
    float glowAlpha = 0.22f;
    float progressHeight = 2.4f;
    float progressInsetX = 16.0f;
    float iconRadius = 15.0f;
};

struct DynamicIslandWidgetTokens {
    float collapsedHeight = 36.0f;
    float expandedHeight = 120.0f;
    float collapsedMinWidth = 168.0f;
    float expandedMinWidth = 240.0f;
    float expandedMaxWidth = 360.0f;
    float topInset = 18.0f;
};

struct ConfigExchangeWidgetTokens {
    float collapsedSize = 36.0f;
    float expandedWidth = 360.0f;
    float height = 36.0f;
    float rounding = 10.0f;
    float iconInsetX = 10.0f;
    float iconLineGap = 5.8f;
    float iconLineLength = 15.0f;
    float inputLeadGap = 10.0f;
    float inputTrailingPadding = 13.0f;
    float inputInsetY = 5.0f;
    float inputPaddingX = 14.0f;
    float inputFillAlpha = 0.34f;
    float inputBorderAlpha = 0.18f;
    float inputHoveredBorderAlpha = 0.38f;
    float inputActiveBorderAlpha = 0.70f;
    float inputInnerHighlightAlpha = 0.045f;
    float fillAlpha = 0.92f;
    float expandedFillAlpha = 0.96f;
    float borderAlpha = 0.18f;
    float hoveredBorderAlpha = 0.44f;
    float glowAlpha = 0.05f;
    float hoveredGlowAlpha = 0.16f;
};

struct ActionWidgetTokens {
    float primaryGlowIdleAlpha = 0.13f;
    float primaryGlowHoverAlpha = 0.24f;
};

struct ToggleWidgetTokens {
    float height = 40.0f;
    float toggleGlowIdleAlpha = 0.22f;
    float toggleGlowHoverAlpha = 0.34f;
};

struct DirectionToggleWidgetTokens {
    float height = 40.0f;
    float knobDiameter = 30.0f;
    float horizontalPadding = 5.0f;
    float textInset = 12.0f;
    float arrowRadius = 7.4f;
    float arrowStroke = 1.8f;
    float arrowHeadSize = 4.2f;
    float knobShadowAlpha = 0.11f;
    float knobBorderAlpha = 0.34f;
    float trackFillAlpha = 0.86f;
    float trackBorderAlpha = 0.16f;
    float hoverGlowAlpha = 0.045f;
    float selectedGlowAlpha = 0.060f;
    FollowMotionStyle knobFollow{13.0f, 0.002f, EasingCurve::OutCubic};
};

struct SegmentedControlWidgetTokens {
    float height = 28.0f;
    float gap = 8.0f;
    float rounding = 8.0f;
    float idleFillAlpha = 1.0f;
    float hoveredFillAlpha = 1.0f;
    float selectedBorderAlpha = 1.0f;
    float idleBorderAlpha = 0.0f;
};

struct WindowChromeControlsWidgetTokens {
    float gap = 8.0f;
    float buttonHeight = 28.0f;
    float minimizeWidth = 30.0f;
    float closeWidth = 30.0f;
};

struct WidgetTokens {
    HeroWidgetTokens hero{};
    StatusPillWidgetTokens statusPill{};
    StatusPillGroupWidgetTokens statusPillGroup{};
    StatusInfoListWidgetTokens statusInfoList{};
    SignalPillWidgetTokens signalPill{};
    SignalPillGroupWidgetTokens signalPillGroup{};
    NoticeWidgetTokens notice{};
    HelpMarkerWidgetTokens helpMarker{};
    FooterSignatureWidgetTokens footerSignature{};
    IdentityCapsuleWidgetTokens identityCapsule{};
    CompactOverlayShellWidgetTokens compactOverlayShell{};
    CompactInputMatrixWidgetTokens compactInputMatrix{};
    StickVisualizerWidgetTokens stickVisualizer{};
    KeyBindingRowWidgetTokens keyBindingRow{};
    NoticeToastWidgetTokens noticeToast{};
    DynamicIslandWidgetTokens dynamicIsland{};
    ConfigExchangeWidgetTokens configExchange{};
    ActionWidgetTokens action{};
    ToggleWidgetTokens toggle{};
    DirectionToggleWidgetTokens directionToggle{};
    SegmentedControlWidgetTokens segmentedControl{};
    WindowChromeControlsWidgetTokens windowChromeControls{};
};

struct FieldTokens {
    float sliderWidth = 300.0f;
    float compactSliderWidth = 280.0f;
    float valueInputWidth = 96.0f;
    float frameRounding = 8.0f;
    float helpGap = 6.0f;
    ImVec2 selectPopupPadding = ImVec2(8.0f, 8.0f);
    ImVec4 frameBg{};
    ImVec4 frameBgHovered{};
    ImVec4 frameBgActive{};
    ImVec4 sliderGrab{};
    ImVec4 sliderGrabActive{};
    ImVec4 rangeTrack{};
    ImVec4 rangeFill{};
    ImVec4 rangeHandleRing{};
    float rangeHandleHaloAlpha = 0.25f;
    float rangeHandleFillAlpha = 0.72f;
    float rangeHandleFillActiveAlpha = 1.0f;
    float rangeHandleRingIdleAlpha = 0.50f;
    float rangeHandleRingHoverAlpha = 0.80f;
    float rangeHandleRingActiveAlpha = 1.0f;
    float rangeValueTextAlpha = 0.70f;
    float selectIdleBorderAlpha = 0.045f;
    float selectHoverBorderAlpha = 0.14f;
    float selectOpenBorderAlpha = 0.18f;
    float selectOpenBorderPulse = 0.08f;
    float selectIdleBorderThickness = 1.1f;
    float selectOpenBorderThickness = 1.7f;
    float selectHoverGlowAlpha = 0.08f;
    float selectOpenGlowAlpha = 0.16f;
    float controlBorderAlphaFloor = 0.22f;
    float controlHoverBorderAlphaFloor = 0.38f;
    float controlActiveBorderAlphaFloor = 0.58f;
    float controlGrabAlphaFloor = 0.42f;
    float controlTextAlphaFloor = 0.68f;
};

struct LayoutTokens {
    ImVec2 surfacePanelPadding = ImVec2(16.0f, 14.0f);
    ImVec2 titledPanelPadding = ImVec2(18.0f, 16.0f);
    ImVec2 titledPanelTitleOffset = ImVec2(18.0f, 16.0f);
    float titledPanelTitleSpacing = 10.0f;
    float formInsetX = 18.0f;
    float formNestedInsetX = 22.0f;
    float formPreferredColumnX = 260.0f;
    float formMinimumLabelGap = 18.0f;
    float formRowSpacing = 10.0f;
    float formSectionSpacing = 12.0f;
    float formFooterSpacing = 18.0f;
};

struct ThemeSpec {
    Palette palette{};
    RadiusTokens radii{};
    SpacingTokens spacing{};
    MotionTokens motion{};
    SurfaceTokens surfaces{};
    NoticeToneStyle noticeInfo{};
    NoticeToneStyle noticeSuccess{};
    NoticeToneStyle noticeWarning{};
    NoticeToneStyle noticeDanger{};
    WidgetTokens widgets{};
    FieldTokens fields{};
    LayoutTokens layout{};
};

struct BackgroundSpec {
    BackgroundStyle style{};
};

struct ThemeSelection {
    ThemeFlavor flavor = ThemeFlavor::Onguoin;
    BackgroundKind backgroundKind = BackgroundKind::Aurora;
};

struct ThemePresetInfo {
    ThemePreset preset = ThemePreset::Onguoin;
    const char* id = "";
    const char* englishLabel = "";
    const char* chineseLabel = "";
    ThemeFlavor flavor = ThemeFlavor::Onguoin;
    BackgroundKind backgroundKind = BackgroundKind::Aurora;
};

struct ThemeFlavorInfo {
    ThemeFlavor flavor = ThemeFlavor::Onguoin;
    const char* id = "";
    const char* englishLabel = "";
    const char* chineseLabel = "";
    BackgroundKind preferredBackgroundKind = BackgroundKind::Aurora;
    const char* englishTone = "";
    const char* chineseTone = "";
    bool motionExpressive = false;
};

struct BackgroundKindInfo {
    BackgroundKind kind = BackgroundKind::Aurora;
    const char* id = "";
    const char* englishLabel = "";
    const char* chineseLabel = "";
    BackgroundLayerMask supportedLayers = 0u;
    BackgroundLayerMask dynamicLayers = 0u;
    bool recommendedForAuthenticatedShell = true;
    BackgroundComplexity complexity = BackgroundComplexity::Balanced;
    BackgroundMotionCharacter motionCharacter = BackgroundMotionCharacter::Static;
    const char* englishCharacter = "";
    const char* chineseCharacter = "";
};

struct Theme {
    Palette palette{};
    RadiusTokens radii{};
    SpacingTokens spacing{};
    MotionTokens motion{};
    SurfaceTokens surfaces{};
    NoticeToneStyle noticeInfo{};
    NoticeToneStyle noticeSuccess{};
    NoticeToneStyle noticeWarning{};
    NoticeToneStyle noticeDanger{};
    WidgetTokens widgets{};
    FieldTokens fields{};
    LayoutTokens layout{};
    BackgroundStyle background{};
};

} // namespace imgui_onguoin

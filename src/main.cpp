// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <Windows.h>
#include <d3d11.h>
#include <tchar.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_device_context = nullptr;
static IDXGISwapChain* g_swap_chain = nullptr;
static ID3D11RenderTargetView* g_main_render_target = nullptr;
static bool g_swap_chain_occluded = false;
static UINT g_resize_width = 0;
static UINT g_resize_height = 0;

static bool CreateDeviceD3D(HWND window);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();
static LRESULT WINAPI WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

namespace {

constexpr float kBaseUiScale = 1.0f;
constexpr int kPlotSamples = 96;

struct Palette {
    static constexpr ImVec4 bg_top = ImVec4(0.018f, 0.025f, 0.040f, 1.0f);
    static constexpr ImVec4 bg_mid = ImVec4(0.012f, 0.018f, 0.029f, 1.0f);
    static constexpr ImVec4 bg_bottom = ImVec4(0.006f, 0.010f, 0.017f, 1.0f);
    static constexpr ImVec4 sidebar = ImVec4(0.016f, 0.019f, 0.026f, 0.92f);
    static constexpr ImVec4 panel = ImVec4(0.028f, 0.032f, 0.042f, 0.94f);
    static constexpr ImVec4 card = ImVec4(0.036f, 0.041f, 0.053f, 0.97f);
    static constexpr ImVec4 card_hot = ImVec4(0.060f, 0.070f, 0.095f, 1.0f);
    static constexpr ImVec4 accent = ImVec4(0.020f, 0.890f, 0.980f, 1.0f);
    static constexpr ImVec4 accent_soft = ImVec4(0.090f, 0.460f, 0.560f, 1.0f);
    static constexpr ImVec4 text = ImVec4(0.955f, 0.965f, 0.985f, 1.0f);
    static constexpr ImVec4 text_dim = ImVec4(0.500f, 0.545f, 0.615f, 1.0f);
    static constexpr ImVec4 good = ImVec4(0.250f, 0.850f, 0.540f, 1.0f);
    static constexpr ImVec4 warn = ImVec4(1.000f, 0.650f, 0.250f, 1.0f);
    static constexpr ImVec4 bad = ImVec4(0.900f, 0.240f, 0.300f, 1.0f);
    static constexpr ImVec4 border = ImVec4(0.730f, 0.900f, 1.000f, 0.090f);
};

struct AppState {
    int active_page = 0;
    int mode_index = 1;
    int focus_index = 0;
    int selected_notice = 1;
    int status_queue = 4;
    int signal_quality = 92;
    bool live_preview = true;
    bool show_scanlines = true;
    bool gentle_motion = false;
    bool island_pinned = false;
    bool sync_clock = true;
    bool seeded = false;
    float density = 0.64f;
    float glow = 0.82f;
    float drift = 0.58f;
    float cadence = 0.71f;
    float island_expand = 0.0f;
    std::array<float, kPlotSamples> frame_plot{};
    std::array<float, kPlotSamples> flow_plot{};
    std::array<float, kPlotSamples> depth_plot{};
    int plot_offset = 0;
    std::vector<std::string> notices = {
        "Preview channels synced",
        "Tone route updated",
        "Island pulse is active",
        "Layout snapshot refreshed",
        "Surface metrics look stable"
    };
};

float Saturate(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float EaseOutCubic(float value) {
    const float t = Saturate(value);
    const float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

float Approach(float current, float target, float speed, float delta_time) {
    return current + (target - current) * Saturate(delta_time * speed);
}

ImVec4 Mix(const ImVec4& a, const ImVec4& b, float t) {
    const float u = Saturate(t);
    return ImVec4(
        a.x + (b.x - a.x) * u,
        a.y + (b.y - a.y) * u,
        a.z + (b.z - a.z) * u,
        a.w + (b.w - a.w) * u
    );
}

ImVec4 WithAlpha(ImVec4 color, float alpha) {
    color.w = alpha;
    return color;
}

ImU32 ToU32(const ImVec4& color) {
    return ImGui::ColorConvertFloat4ToU32(color);
}

void DrawSoftGlow(ImDrawList* draw_list, ImVec2 center, float radius, const ImVec4& color, float strength) {
    for (int layer = 6; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 6.0f;
        const float spread = radius * (0.58f + t * 0.84f);
        const float alpha = strength * (0.05f + 0.04f * (1.0f - t));
        draw_list->AddCircleFilled(center, spread, ToU32(WithAlpha(color, alpha)), 72);
    }
}

void ApplyTheme(float dpi_scale) {
    ImGuiStyle& style = ImGui::GetStyle();
    style = ImGuiStyle();
    style.WindowRounding = 16.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 12.0f;
    style.GrabRounding = 10.0f;
    style.ScrollbarRounding = 12.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.ScrollbarSize = 13.0f;
    style.WindowPadding = ImVec2(18.0f, 18.0f);
    style.FramePadding = ImVec2(12.0f, 8.0f);
    style.ItemSpacing = ImVec2(12.0f, 12.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.ScaleAllSizes(dpi_scale);
    style.FontScaleDpi = dpi_scale;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = Palette::text;
    colors[ImGuiCol_TextDisabled] = Palette::text_dim;
    colors[ImGuiCol_WindowBg] = Palette::panel;
    colors[ImGuiCol_ChildBg] = Palette::card;
    colors[ImGuiCol_PopupBg] = Palette::card;
    colors[ImGuiCol_Border] = Palette::border;
    colors[ImGuiCol_FrameBg] = ImVec4(0.050f, 0.055f, 0.072f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.065f, 0.074f, 0.096f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = Palette::card_hot;
    colors[ImGuiCol_TitleBg] = Palette::panel;
    colors[ImGuiCol_TitleBgActive] = Palette::panel;
    colors[ImGuiCol_MenuBarBg] = Palette::panel;
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.012f, 0.014f, 0.020f, 0.55f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.090f, 0.122f, 0.170f, 0.90f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.120f, 0.170f, 0.230f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = Palette::accent_soft;
    colors[ImGuiCol_CheckMark] = Palette::accent;
    colors[ImGuiCol_SliderGrab] = Palette::accent;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.280f, 0.940f, 1.000f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.044f, 0.050f, 0.066f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.062f, 0.071f, 0.092f, 1.0f);
    colors[ImGuiCol_ButtonActive] = Palette::card_hot;
    colors[ImGuiCol_Header] = Palette::card;
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.063f, 0.072f, 0.097f, 1.0f);
    colors[ImGuiCol_HeaderActive] = Palette::card_hot;
    colors[ImGuiCol_Separator] = Palette::border;
    colors[ImGuiCol_ResizeGrip] = WithAlpha(Palette::accent, 0.16f);
    colors[ImGuiCol_ResizeGripHovered] = WithAlpha(Palette::accent, 0.34f);
    colors[ImGuiCol_ResizeGripActive] = WithAlpha(Palette::accent, 0.52f);
}

void LoadFont() {
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 2;
    config.PixelSnapH = false;
    config.RasterizerMultiply = 1.04f;

    const char* candidates[] = {
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf"
    };

    for (const char* path : candidates) {
        if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
            io.Fonts->AddFontFromFileTTF(path, 18.0f, &config, io.Fonts->GetGlyphRangesChineseFull());
            return;
        }
    }

    io.Fonts->AddFontDefault();
}

void SeedPlots(AppState& state) {
    if (state.seeded) {
        return;
    }

    for (int i = 0; i < kPlotSamples; ++i) {
        const float x = static_cast<float>(i) * 0.14f;
        state.frame_plot[i] = 14.0f + std::sin(x) * 1.8f + std::cos(x * 0.35f) * 1.2f;
        state.flow_plot[i] = 0.56f + std::sin(x * 0.82f) * 0.24f;
        state.depth_plot[i] = 0.48f + std::cos(x * 0.62f) * 0.20f;
    }

    state.seeded = true;
}

void UpdateState(AppState& state) {
    SeedPlots(state);

    ImGuiIO& io = ImGui::GetIO();
    const float time = static_cast<float>(ImGui::GetTime());
    const float motion_factor = state.gentle_motion ? 0.42f : 1.0f;

    state.plot_offset = (state.plot_offset + 1) % kPlotSamples;
    state.frame_plot[state.plot_offset] =
        12.5f + std::sin(time * 1.55f * motion_factor) * 1.7f + std::cos(time * 0.45f) * 0.9f + state.glow * 2.8f;
    state.flow_plot[state.plot_offset] =
        0.50f + std::sin(time * 1.85f * motion_factor) * 0.24f + state.cadence * 0.18f;
    state.depth_plot[state.plot_offset] =
        0.46f + std::cos(time * 1.25f * motion_factor) * 0.18f + state.drift * 0.17f;

    if (state.live_preview) {
        const int phase = static_cast<int>(time / 5.0f);
        state.selected_notice = phase % static_cast<int>(state.notices.size());
    }

    state.signal_quality = 88 + static_cast<int>(std::sin(time * 0.9f) * 5.0f + state.glow * 5.0f);
    state.status_queue = 3 + static_cast<int>(std::fabs(std::sin(time * 0.6f)) * 3.0f);
    state.island_expand = Approach(state.island_expand, state.island_pinned ? 1.0f : 0.0f, 5.8f, io.DeltaTime);
}

void DrawPillText(ImDrawList* draw_list, ImVec2 cursor, const char* text, const ImVec4& tint, float rounding, float pad_x, float pad_y) {
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    const ImVec2 min = cursor;
    const ImVec2 max = ImVec2(cursor.x + text_size.x + pad_x * 2.0f, cursor.y + text_size.y + pad_y * 2.0f);
    draw_list->AddRectFilled(min, max, ToU32(WithAlpha(tint, 0.14f)), rounding);
    draw_list->AddRect(min, max, ToU32(WithAlpha(tint, 0.24f)), rounding, 0, 1.0f);
    draw_list->AddText(ImVec2(min.x + pad_x, min.y + pad_y), ToU32(Palette::text), text);
}

bool NavButton(const char* label, const char* note, bool active, float width) {
    const float height = 58.0f;
    ImGui::PushID(label);
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const ImVec2 size(width, height);
    const ImVec2 end(start.x + size.x, start.y + size.y);
    const bool pressed = ImGui::InvisibleButton("##nav_item", size);
    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec4 bg = active
        ? ImVec4(0.040f, 0.120f, 0.132f, 0.96f)
        : (hovered ? ImVec4(0.040f, 0.047f, 0.062f, 0.98f) : ImVec4(0.030f, 0.035f, 0.046f, 0.92f));
    const ImVec4 border = active
        ? WithAlpha(Palette::accent, 0.36f)
        : (hovered ? WithAlpha(Palette::border, 0.18f) : WithAlpha(Palette::border, 0.10f));

    draw_list->AddRectFilled(start, end, ToU32(bg), 12.0f);
    draw_list->AddRect(start, end, ToU32(border), 12.0f, 0, 1.0f);
    if (active) {
        draw_list->AddRectFilled(ImVec2(start.x + 1.0f, start.y + 8.0f),
                                 ImVec2(start.x + 4.0f, end.y - 8.0f),
                                 ToU32(WithAlpha(Palette::accent, 0.95f)),
                                 6.0f);
    }

    draw_list->AddText(ImVec2(start.x + 16.0f, start.y + 10.0f),
                       ToU32(active ? Palette::text : WithAlpha(Palette::text, hovered ? 0.96f : 0.90f)),
                       label);
    if (note != nullptr && note[0] != '\0') {
        draw_list->AddText(ImVec2(start.x + 16.0f, start.y + 31.0f),
                           ToU32(active ? WithAlpha(Palette::accent, 0.92f) : Palette::text_dim),
                           note);
    }

    ImGui::PopID();
    return pressed;
}

void BeginCard(const char* id, const char* title, const char* subtitle, ImVec2 size, ImGuiWindowFlags flags = 0) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Palette::card);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));
    ImGui::BeginChild(id, size, true, flags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextUnformatted(title);
    if (subtitle != nullptr && subtitle[0] != '\0') {
        ImGui::PushStyleColor(ImGuiCol_Text, Palette::text_dim);
        ImGui::TextWrapped("%s", subtitle);
        ImGui::PopStyleColor();
    }
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Separator, WithAlpha(Palette::border, 0.16f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
}

void EndCard() {
    const ImVec2 min = ImGui::GetWindowPos();
    const ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);
    const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(min, max, ToU32(WithAlpha(Palette::border, hovered ? 0.24f : 0.12f)), 16.0f, 0, 1.0f);
    if (hovered) {
        draw_list->AddRect(ImVec2(min.x - 1.0f, min.y - 1.0f),
                           ImVec2(max.x + 1.0f, max.y + 1.0f),
                           ToU32(WithAlpha(Palette::accent, 0.18f)),
                           17.0f,
                           0,
                           1.0f);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void DrawMetricTile(const char* label, const char* value, const char* note, float fill, const ImVec4& tint) {
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    const float height = 68.0f;
    const ImVec2 end = ImVec2(start.x + width, start.y + height);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRectFilled(start, end, ToU32(ImVec4(0.028f, 0.032f, 0.043f, 0.96f)), 12.0f);
    draw_list->AddRect(start, end, ToU32(WithAlpha(tint, 0.16f)), 12.0f);
    draw_list->AddRectFilled(ImVec2(start.x + 14.0f, end.y - 10.0f),
                             ImVec2(end.x - 14.0f, end.y - 6.0f),
                             ToU32(ImVec4(0.080f, 0.090f, 0.112f, 1.0f)),
                             4.0f);
    draw_list->AddRectFilled(ImVec2(start.x + 14.0f, end.y - 10.0f),
                             ImVec2(start.x + 14.0f + (width - 28.0f) * Saturate(fill), end.y - 6.0f),
                             ToU32(WithAlpha(tint, 0.88f)),
                             4.0f);

    ImGui::InvisibleButton(label, ImVec2(width, height));
    ImGui::SetCursorScreenPos(ImVec2(start.x + 14.0f, start.y + 12.0f));
    ImGui::TextUnformatted(label);
    const float value_width = ImGui::CalcTextSize(value).x;
    ImGui::SetCursorScreenPos(ImVec2(end.x - 14.0f - value_width, start.y + 12.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, tint);
    ImGui::TextUnformatted(value);
    ImGui::PopStyleColor();
    ImGui::SetCursorScreenPos(ImVec2(start.x + 14.0f, start.y + 34.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, Palette::text_dim);
    ImGui::TextWrapped("%s", note);
    ImGui::PopStyleColor();
}

void DrawInlineChip(const char* text, const ImVec4& tint) {
    ImGui::PushID(text);
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    const ImVec2 size(text_size.x + 22.0f, 26.0f);
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const ImVec2 end(start.x + size.x, start.y + size.y);
    ImGui::InvisibleButton("##chip", size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(start, end, ToU32(WithAlpha(tint, 0.12f)), 13.0f);
    draw_list->AddRect(start, end, ToU32(WithAlpha(tint, 0.24f)), 13.0f, 0, 1.0f);
    draw_list->AddText(ImVec2(start.x + 11.0f, start.y + 5.0f), ToU32(Palette::text), text);
    ImGui::PopID();
}

struct ColumnSplit {
    float left = 0.0f;
    float right = 0.0f;
    float gap = 0.0f;
};

ColumnSplit MakeColumns(float left_ratio) {
    const float gap = ImGui::GetStyle().ItemSpacing.x;
    const float total = ImGui::GetContentRegionAvail().x;
    const float left = std::floor((total - gap) * left_ratio);
    return ColumnSplit{ left, std::max(0.0f, total - gap - left), gap };
}

void DrawMotionField(AppState& state, ImVec2 canvas_size) {
    const ImVec2 cursor = ImGui::GetCursorScreenPos();
    const ImVec2 size = ImVec2(std::max(canvas_size.x, 120.0f), std::max(canvas_size.y, 120.0f));
    const ImVec2 end = ImVec2(cursor.x + size.x, cursor.y + size.y);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(cursor, end, ToU32(ImVec4(0.018f, 0.021f, 0.030f, 1.0f)), 14.0f);
    draw_list->AddRect(cursor, end, ToU32(WithAlpha(Palette::border, 0.18f)), 14.0f);

    const float time = static_cast<float>(ImGui::GetTime());
    const ImVec2 center(cursor.x + size.x * 0.5f, cursor.y + size.y * 0.54f);
    const float base_radius = std::min(size.x, size.y) * 0.28f;
    const float pulse = 0.78f + std::sin(time * 2.2f) * 0.12f;

    for (int ring = 0; ring < 4; ++ring) {
        const float t = static_cast<float>(ring) / 3.0f;
        const float radius = base_radius * (0.52f + t * 1.02f) * pulse;
        draw_list->AddCircle(center,
                             radius,
                             ToU32(WithAlpha(Palette::accent, 0.14f - t * 0.02f)),
                             72,
                             1.0f);
    }

    for (int spoke = 0; spoke < 10; ++spoke) {
        const float angle = (time * 0.12f) + static_cast<float>(spoke) * 0.628f;
        const float inner = base_radius * 0.18f;
        const float outer = base_radius * (0.78f + 0.18f * std::sin(time * 1.3f + static_cast<float>(spoke)));
        const ImVec2 from(center.x + std::cos(angle) * inner, center.y + std::sin(angle) * inner);
        const ImVec2 to(center.x + std::cos(angle) * outer, center.y + std::sin(angle) * outer);
        draw_list->AddLine(from, to, ToU32(WithAlpha(Palette::accent, 0.28f)), 1.0f);
    }

    for (int bar = 0; bar < 5; ++bar) {
        const float sample = 0.35f + 0.35f * std::sin(time * 1.4f + static_cast<float>(bar) * 0.85f) + state.glow * 0.12f;
        const float bar_height = size.y * 0.34f * Saturate(sample);
        const float bar_width = 18.0f;
        const float gap = 12.0f;
        const float total = 5.0f * bar_width + 4.0f * gap;
        const float x = cursor.x + (size.x - total) * 0.5f + static_cast<float>(bar) * (bar_width + gap);
        const float y1 = end.y - 22.0f;
        const float y0 = y1 - bar_height;
        draw_list->AddRectFilled(ImVec2(x, y0), ImVec2(x + bar_width, y1), ToU32(WithAlpha(Palette::accent, 0.72f)), 6.0f);
    }

    ImGui::InvisibleButton("motion_canvas", size);
}

void DrawAnimatedBackground(const AppState& state) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    const ImVec2 min = viewport->Pos;
    const ImVec2 max = ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y);
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    const float time = static_cast<float>(ImGui::GetTime());

    constexpr int band_count = 30;
    for (int band = 0; band < band_count; ++band) {
        const float y0 = min.y + height * static_cast<float>(band) / static_cast<float>(band_count);
        const float y1 = min.y + height * static_cast<float>(band + 1) / static_cast<float>(band_count) + 1.0f;
        const float u = static_cast<float>(band) / static_cast<float>(band_count - 1);
        const float eased = u * u * (3.0f - 2.0f * u);
        const ImVec4 base = eased < 0.5f
            ? Mix(Palette::bg_top, Palette::bg_mid, eased / 0.5f)
            : Mix(Palette::bg_mid, Palette::bg_bottom, (eased - 0.5f) / 0.5f);
        draw_list->AddRectFilled(ImVec2(min.x, y0), ImVec2(max.x, y1), ToU32(base));
    }

    DrawSoftGlow(draw_list,
                 ImVec2(min.x + width * 0.18f, min.y + height * 0.17f),
                 180.0f + state.glow * 35.0f,
                 Palette::accent,
                 0.50f);
    DrawSoftGlow(draw_list,
                 ImVec2(min.x + width * 0.76f, min.y + height * 0.24f),
                 150.0f + state.drift * 30.0f,
                 ImVec4(0.19f, 0.45f, 0.94f, 1.0f),
                 0.34f);
    DrawSoftGlow(draw_list,
                 ImVec2(min.x + width * 0.58f, min.y + height * 0.76f),
                 200.0f + state.cadence * 36.0f,
                 ImVec4(0.08f, 0.92f, 0.88f, 1.0f),
                 0.24f);

    const float sweep = std::fmod(time * (state.gentle_motion ? 10.0f : 18.0f), width + 460.0f) - 260.0f;
    const ImVec2 beam_a[] = {
        ImVec2(min.x + sweep - 90.0f, min.y - 80.0f),
        ImVec2(min.x + sweep + 60.0f, min.y - 80.0f),
        ImVec2(min.x + sweep + 320.0f, max.y + 80.0f),
        ImVec2(min.x + sweep + 170.0f, max.y + 80.0f),
    };
    draw_list->AddConvexPolyFilled(beam_a, 4, IM_COL32(44, 150, 255, 11));

    const float sweep_b = std::fmod(time * (state.gentle_motion ? 6.0f : 12.0f), width + 560.0f) - 320.0f;
    const ImVec2 beam_b[] = {
        ImVec2(min.x + sweep_b - 40.0f, min.y - 50.0f),
        ImVec2(min.x + sweep_b + 50.0f, min.y - 50.0f),
        ImVec2(min.x + sweep_b + 260.0f, max.y + 50.0f),
        ImVec2(min.x + sweep_b + 170.0f, max.y + 50.0f),
    };
    draw_list->AddConvexPolyFilled(beam_b, 4, IM_COL32(10, 225, 245, 7));

    for (int i = 0; i < 5; ++i) {
        const float y = min.y + height * (0.14f + 0.12f * static_cast<float>(i));
        const float alpha = 0.018f - static_cast<float>(i) * 0.0025f;
        draw_list->AddLine(ImVec2(min.x + 26.0f, y),
                           ImVec2(max.x - 26.0f, y),
                           ToU32(WithAlpha(Palette::border, alpha)),
                           1.0f);
    }

    if (state.show_scanlines) {
        for (int row = 0; row < 24; ++row) {
            const float t = static_cast<float>(row) / 23.0f;
            const float y = min.y + t * height;
            const float alpha = 0.020f + 0.008f * std::sin(time * 1.6f + static_cast<float>(row) * 0.6f);
            draw_list->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), ToU32(WithAlpha(Palette::accent, alpha)), 1.0f);
        }
    }
}

void DrawSidebar(AppState& state, float ui_scale) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float outer_pad = 24.0f * ui_scale;
    const float top_gap = 82.0f * ui_scale;
    const float dock_gap = 88.0f * ui_scale;
    const float width = 268.0f * ui_scale;
    const ImVec2 pos(viewport->WorkPos.x + outer_pad, viewport->WorkPos.y + outer_pad + top_gap);
    const ImVec2 size(width, viewport->WorkSize.y - outer_pad * 2.0f - top_gap - dock_gap);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, WithAlpha(Palette::sidebar, 0.96f));
    ImGui::Begin("Sidebar", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 header_min = ImGui::GetCursorScreenPos();
    const ImVec2 header_max(header_min.x + ImGui::GetContentRegionAvail().x, header_min.y + 78.0f * ui_scale);
    draw_list->AddRectFilled(header_min, header_max, ToU32(ImVec4(0.028f, 0.033f, 0.046f, 0.88f)), 14.0f);
    draw_list->AddRect(header_min, header_max, ToU32(WithAlpha(Palette::border, 0.16f)), 14.0f);
    DrawSoftGlow(draw_list, ImVec2(header_min.x + 34.0f * ui_scale, header_min.y + 28.0f * ui_scale), 38.0f * ui_scale, Palette::accent, 0.32f);
    draw_list->AddCircleFilled(ImVec2(header_min.x + 34.0f * ui_scale, header_min.y + 28.0f * ui_scale), 8.0f * ui_scale, ToU32(Palette::accent), 32);
    draw_list->AddCircle(ImVec2(header_min.x + 34.0f * ui_scale, header_min.y + 28.0f * ui_scale), 17.0f * ui_scale, ToU32(WithAlpha(Palette::accent, 0.40f)), 32, 1.0f);
    ImGui::InvisibleButton("brand_header", ImVec2(header_max.x - header_min.x, header_max.y - header_min.y));
    ImGui::SetCursorScreenPos(ImVec2(header_min.x + 58.0f * ui_scale, header_min.y + 14.0f * ui_scale));
    ImGui::TextUnformatted("imgui-onguoin");
    ImGui::SetCursorScreenPos(ImVec2(header_min.x + 58.0f * ui_scale, header_min.y + 38.0f * ui_scale));
    ImGui::PushStyleColor(ImGuiCol_Text, Palette::text_dim);
    ImGui::TextUnformatted("public UI reference");
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0.0f, 16.0f));
    ImGui::SeparatorText("Browse");

    const char* pages[][2] = {
        { "Overview", "theme pulse and summary cards" },
        { "Controls", "sliders, toggles, and mode chips" },
        { "Panels", "content density and data framing" },
        { "Visuals", "motion field and layered previews" },
        { "Notices", "island and toast presentation" }
    };

    for (int i = 0; i < 5; ++i) {
        if (NavButton(pages[i][0], pages[i][1], state.active_page == i, ImGui::GetContentRegionAvail().x)) {
            state.active_page = i;
        }
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
    }

    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::SeparatorText("Quick switches");
    ImGui::Checkbox("Live preview", &state.live_preview);
    ImGui::Checkbox("Scan lines", &state.show_scanlines);
    ImGui::Checkbox("Gentle motion", &state.gentle_motion);
    ImGui::Checkbox("Pin island", &state.island_pinned);
    ImGui::Checkbox("Clock sync", &state.sync_clock);

    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::SeparatorText("Accent mix");
    ImGui::SliderFloat("Glow", &state.glow, 0.15f, 1.0f, "%.2f");
    ImGui::SliderFloat("Density", &state.density, 0.20f, 1.0f, "%.2f");
    ImGui::SliderFloat("Drift", &state.drift, 0.10f, 1.0f, "%.2f");
    ImGui::SliderFloat("Cadence", &state.cadence, 0.10f, 1.0f, "%.2f");

    ImGui::End();
    ImGui::PopStyleColor();
}

void DrawOverviewPage(AppState& state, float, float ui_scale) {
    const ColumnSplit split = MakeColumns(0.56f);

    BeginCard("overview_metrics",
              "Status summary",
              "Compact summary modules read best when they stay shallow and visually even.",
              ImVec2(split.left, 314.0f * ui_scale));
    DrawMetricTile("Frame blend", "14.8 ms", "steady motion budget", 0.78f, Palette::accent);
    ImGui::Spacing();
    DrawMetricTile("Signal mix", "92%", "cyan route is healthy", 0.92f, Palette::good);
    ImGui::Spacing();
    DrawMetricTile("Queue depth", "04", "small batch of active panels", 0.40f, Palette::warn);
    EndCard();

    ImGui::SameLine();

    BeginCard("overview_plot",
              "Live plot",
              "Lead with a larger chart when the page needs motion and rhythm, then keep annotations short.",
              ImVec2(split.right, 314.0f * ui_scale));
    ImGui::PushStyleColor(ImGuiCol_PlotLines, Palette::accent);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.024f, 0.028f, 0.038f, 1.0f));
    ImGui::PlotLines("##frames", state.frame_plot.data(), kPlotSamples, state.plot_offset, nullptr, 9.0f, 20.0f, ImVec2(-1.0f, 96.0f * ui_scale));
    ImGui::PopStyleColor(2);
    ImGui::PushStyleColor(ImGuiCol_Text, Palette::text_dim);
    ImGui::TextWrapped("Use this card to preview motion density, chart treatment, and compact annotations.");
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::ProgressBar(Saturate(state.glow), ImVec2(-1.0f, 7.0f * ui_scale), "");
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Text("Active notice");
    ImGui::PushStyleColor(ImGuiCol_Text, Palette::accent);
    ImGui::TextWrapped("%s", state.notices[state.selected_notice].c_str());
    ImGui::PopStyleColor();
    EndCard();

    BeginCard("overview_surface",
              "Surface notes",
              "Use one larger narrative card to anchor the lower half of the page.",
              ImVec2(split.left, 308.0f * ui_scale),
              ImGuiWindowFlags_NoScrollbar);
    ImGui::BulletText("Top island keeps time, state, and a short note.");
    ImGui::BulletText("Sidebar stays dense but still readable.");
    ImGui::BulletText("Cyan glow is limited to focal elements.");
    ImGui::BulletText("Background remains animated without stealing attention.");
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::SeparatorText("Tags");
    DrawInlineChip("layout", Palette::accent);
    ImGui::SameLine();
    DrawInlineChip("motion", Palette::good);
    ImGui::SameLine();
    DrawInlineChip("overlay", Palette::warn);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, Palette::text_dim);
    ImGui::TextWrapped("This kind of card works best when it combines a few bullets, a tiny cluster of chips, and one short paragraph instead of many unrelated widgets.");
    ImGui::PopStyleColor();
    EndCard();

    ImGui::SameLine();

    BeginCard("overview_route",
              "Route board",
              "Buttons and segmented choices should sit together in a narrower support column.",
              ImVec2(split.right, 308.0f * ui_scale));
    if (ImGui::Button("Refresh notice", ImVec2(-1.0f, 34.0f * ui_scale))) {
        state.selected_notice = (state.selected_notice + 1) % static_cast<int>(state.notices.size());
    }
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::SeparatorText("Mode group");
    const char* modes[] = { "Quiet", "Balanced", "Bright" };
    for (int i = 0; i < 3; ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        ImGui::PushStyleColor(ImGuiCol_Button, state.mode_index == i ? WithAlpha(Palette::accent, 0.20f) : ImVec4(0.042f, 0.048f, 0.063f, 1.0f));
        if (ImGui::Button(modes[i], ImVec2((split.right - 36.0f * ui_scale - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f, 32.0f * ui_scale))) {
            state.mode_index = i;
        }
        ImGui::PopStyleColor();
    }
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::SeparatorText("Routing");
    ImGui::Text("Queue depth");
    ImGui::ProgressBar(static_cast<float>(state.status_queue) / 6.0f, ImVec2(-1.0f, 7.0f * ui_scale), "");
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::Text("Signal quality");
    ImGui::ProgressBar(static_cast<float>(state.signal_quality) / 100.0f, ImVec2(-1.0f, 7.0f * ui_scale), "");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextWrapped("This panel is intentionally generic so the repository can be shared safely as a UI idea source.");
    EndCard();
}

void DrawControlsPage(AppState& state, float, float ui_scale) {
    const ColumnSplit split = MakeColumns(0.54f);

    BeginCard("controls_tuning",
              "Tuning rack",
              "Place primary sliders together so the page reads top to bottom without hunting.",
              ImVec2(split.left, 288.0f * ui_scale));
    ImGui::SliderFloat("Density", &state.density, 0.20f, 1.0f, "%.2f");
    ImGui::SliderFloat("Glow", &state.glow, 0.15f, 1.0f, "%.2f");
    ImGui::SliderFloat("Drift", &state.drift, 0.10f, 1.0f, "%.2f");
    ImGui::SliderFloat("Cadence", &state.cadence, 0.10f, 1.0f, "%.2f");
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Checkbox("Live preview", &state.live_preview);
    ImGui::Checkbox("Gentle motion", &state.gentle_motion);
    EndCard();

    ImGui::SameLine();

    BeginCard("controls_modes",
              "Mode chips",
              "State toggles feel cleaner when they are grouped by intent instead of mixed into slider stacks.",
              ImVec2(split.right, 288.0f * ui_scale));
    const char* modes[] = { "Studio", "Field", "Night" };
    for (int i = 0; i < 3; ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        ImGui::PushStyleColor(ImGuiCol_Button, state.mode_index == i ? WithAlpha(Palette::accent, 0.18f) : ImVec4(0.042f, 0.048f, 0.063f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, state.mode_index == i ? WithAlpha(Palette::accent, 0.26f) : ImVec4(0.060f, 0.070f, 0.090f, 1.0f));
        if (ImGui::Button(modes[i], ImVec2((split.right - 36.0f * ui_scale - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f, 34.0f * ui_scale))) {
            state.mode_index = i;
        }
        ImGui::PopStyleColor(2);
    }
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::SeparatorText("Switches");
    ImGui::Checkbox("Island pinned", &state.island_pinned);
    ImGui::Checkbox("Clock sync", &state.sync_clock);
    ImGui::Checkbox("Scan lines", &state.show_scanlines);
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::TextWrapped("These labels stay generic on purpose so the public repo shares component ideas instead of product behavior.");
    EndCard();

    BeginCard("controls_plot",
              "Response curves",
              "Put the supporting chart underneath the primary tuning controls so cause and effect stay close.",
              ImVec2(split.left, 224.0f * ui_scale));
    ImGui::PushStyleColor(ImGuiCol_PlotLines, Palette::accent);
    ImGui::PlotLines("##flow_plot", state.flow_plot.data(), kPlotSamples, state.plot_offset, nullptr, 0.0f, 1.0f, ImVec2(-1.0f, 62.0f * ui_scale));
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_PlotLines, WithAlpha(Palette::good, 1.0f));
    ImGui::PlotLines("##depth_plot", state.depth_plot.data(), kPlotSamples, state.plot_offset, nullptr, 0.0f, 1.0f, ImVec2(-1.0f, 62.0f * ui_scale));
    ImGui::PopStyleColor();
    EndCard();

    ImGui::SameLine();

    BeginCard("controls_focus",
              "Focus set",
              "Keep selection lists compact and aligned with other secondary controls.",
              ImVec2(split.right, 224.0f * ui_scale));
    const char* items[] = { "Surface", "Overlay", "Channel", "Dock" };
    for (int i = 0; i < 4; ++i) {
        if (ImGui::Selectable(items[i], state.focus_index == i)) {
            state.focus_index = i;
        }
    }
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::Text("Selected: %s", items[state.focus_index]);
    EndCard();
}

void DrawPanelsPage(AppState& state, float, float ui_scale) {
    const ColumnSplit split = MakeColumns(0.57f);

    BeginCard("panels_catalog",
              "Layer catalog",
              "Use the broader column for the most structured content on the page.",
              ImVec2(split.left, 296.0f * ui_scale));
    if (ImGui::BeginTable("layer_table", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableSetupColumn("Layer");
        ImGui::TableSetupColumn("Weight");
        ImGui::TableSetupColumn("State");
        ImGui::TableHeadersRow();

        const char* rows[][3] = {
            { "Backdrop", "0.64", "steady" },
            { "Island", "0.82", "focused" },
            { "Dock", "0.58", "ready" },
            { "Cards", "0.71", "active" }
        };

        for (const auto& row : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(row[0]);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(row[1]);
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(row[2]);
        }

        ImGui::EndTable();
    }
    EndCard();

    ImGui::SameLine();

    BeginCard("panels_notes",
              "Panel study",
              "This support column is better for bullets, tiny charts, and short supporting notes.",
              ImVec2(split.right, 296.0f * ui_scale));
    ImGui::BulletText("Keep page backgrounds darker than cards.");
    ImGui::BulletText("Use cyan only for emphasis and motion.");
    ImGui::BulletText("Reserve strong glow for overlays and active states.");
    ImGui::BulletText("Dense sidebars need breathing room between groups.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::ProgressBar(Saturate(state.density), ImVec2(-1.0f, 8.0f * ui_scale), "");
    ImGui::Text("Density profile");
    EndCard();

    BeginCard("panels_mix",
              "Preview stacks",
              "Metric tiles read better in a short, even stack than in a crowded text block.",
              ImVec2(split.left, 184.0f * ui_scale));
    DrawMetricTile("Stack A", "soft", "wide spacing and cooler glow", 0.46f, Palette::accent);
    ImGui::Spacing();
    DrawMetricTile("Stack B", "dense", "short labels and fast scanning", 0.71f, Palette::warn);
    EndCard();

    ImGui::SameLine();

    BeginCard("panels_short",
              "Notes",
              "Small cards should carry one clear job instead of trying to host many tiny elements.",
              ImVec2(split.right, 184.0f * ui_scale));
    ImGui::TextWrapped("This page is a public-facing shell: it demonstrates structure, spacing, and treatment while leaving the private product outside the repository.");
    EndCard();
}

void DrawVisualsPage(AppState& state, float, float ui_scale) {
    const ColumnSplit split = MakeColumns(0.60f);

    BeginCard("visuals_field",
              "Motion field",
              "Give the hero visual more width so it feels intentional rather than squeezed into a card grid.",
              ImVec2(split.left, 338.0f * ui_scale));
    DrawMotionField(state, ImVec2(-1.0f, 210.0f * ui_scale));
    EndCard();

    ImGui::SameLine();

    BeginCard("visuals_background",
              "Background profile",
              "Controls sit best in a companion column next to the hero visual.",
              ImVec2(split.right, 338.0f * ui_scale));
    ImGui::Checkbox("Scan lines", &state.show_scanlines);
    ImGui::Checkbox("Gentle motion", &state.gentle_motion);
    ImGui::Checkbox("Live preview", &state.live_preview);
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::SliderFloat("Glow", &state.glow, 0.15f, 1.0f, "%.2f");
    ImGui::SliderFloat("Drift", &state.drift, 0.10f, 1.0f, "%.2f");
    ImGui::SliderFloat("Cadence", &state.cadence, 0.10f, 1.0f, "%.2f");
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::TextWrapped("Animated bands, soft glows, and diagonal sweeps keep the desktop atmosphere from feeling static.");
    EndCard();

    BeginCard("visuals_tone",
              "Tone map",
              "Swatches should be large enough to read as a system, not just as decoration.",
              ImVec2(split.left, 188.0f * ui_scale));
    const ImVec2 swatch = ImVec2((split.left - 36.0f * ui_scale - ImGui::GetStyle().ItemSpacing.x * 3.0f) / 4.0f, 72.0f * ui_scale);
    const ImVec4 tones[] = { Palette::sidebar, Palette::panel, Palette::card, Palette::accent };
    for (int i = 0; i < 4; ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        const ImVec2 start = ImGui::GetCursorScreenPos();
        const ImVec2 end = ImVec2(start.x + swatch.x, start.y + swatch.y);
        ImGui::GetWindowDrawList()->AddRectFilled(start, end, ToU32(tones[i]), 10.0f);
        ImGui::GetWindowDrawList()->AddRect(start, end, ToU32(WithAlpha(Palette::border, 0.18f)), 10.0f);
        ImGui::InvisibleButton(("tone_" + std::to_string(i)).c_str(), swatch);
    }
    EndCard();

    ImGui::SameLine();

    BeginCard("visuals_layers",
              "Layer notes",
              "Support notes can stay small if they do not compete with the hero area.",
              ImVec2(split.right, 188.0f * ui_scale));
    ImGui::BulletText("Most surfaces stay matte.");
    ImGui::BulletText("Accent pulses are brief and focused.");
    ImGui::BulletText("Black overlays keep the island crisp.");
    EndCard();
}

void DrawNoticesPage(AppState& state, float, float ui_scale) {
    const ColumnSplit split = MakeColumns(0.55f);

    BeginCard("notices_island",
              "Island preview",
              "The main interaction summary deserves the wider column.",
              ImVec2(split.left, 282.0f * ui_scale));
    ImGui::Checkbox("Pin island", &state.island_pinned);
    ImGui::Checkbox("Clock sync", &state.sync_clock);
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::SeparatorText("Current notice");
    ImGui::PushStyleColor(ImGuiCol_Text, Palette::accent);
    ImGui::TextWrapped("%s", state.notices[state.selected_notice].c_str());
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    if (ImGui::Button("Next notice", ImVec2(-1.0f, 34.0f * ui_scale))) {
        state.selected_notice = (state.selected_notice + 1) % static_cast<int>(state.notices.size());
    }
    EndCard();

    ImGui::SameLine();

    BeginCard("notices_queue",
              "Notice queue",
              "Lists should be tall enough to breathe but not so tall that they dwarf the summary area.",
              ImVec2(split.right, 282.0f * ui_scale));
    for (int i = 0; i < static_cast<int>(state.notices.size()); ++i) {
        if (ImGui::Selectable(state.notices[i].c_str(), state.selected_notice == i)) {
            state.selected_notice = i;
        }
    }
    EndCard();

    BeginCard("notices_routing",
              "Routing summary",
              "Use a shallow stats card instead of another tall list.",
              ImVec2(split.left, 178.0f * ui_scale));
    ImGui::ProgressBar(static_cast<float>(state.signal_quality) / 100.0f, ImVec2(-1.0f, 8.0f * ui_scale), "");
    ImGui::Text("Signal quality");
    ImGui::ProgressBar(static_cast<float>(state.status_queue) / 6.0f, ImVec2(-1.0f, 8.0f * ui_scale), "");
    ImGui::Text("Queue depth");
    EndCard();

    ImGui::SameLine();

    BeginCard("notices_note",
              "Public repo note",
              "Short explanatory copy belongs in the smaller companion card.",
              ImVec2(split.right, 178.0f * ui_scale));
    ImGui::TextWrapped("The repository keeps the feel of a polished desktop surface while avoiding any private naming, workflow, or product behavior.");
    EndCard();
}

void DrawMainSurface(AppState& state, float ui_scale) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float outer_pad = 24.0f * ui_scale;
    const float top_gap = 82.0f * ui_scale;
    const float dock_gap = 88.0f * ui_scale;
    const float sidebar_width = 268.0f * ui_scale;
    const ImVec2 pos(viewport->WorkPos.x + outer_pad * 2.0f + sidebar_width,
                     viewport->WorkPos.y + outer_pad + top_gap);
    const ImVec2 size(viewport->WorkSize.x - sidebar_width - outer_pad * 3.0f,
                      viewport->WorkSize.y - outer_pad * 2.0f - top_gap - dock_gap);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 18.0f));
    ImGui::Begin("MainSurface", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings);

    static const char* titles[] = {
        "Overview",
        "Controls",
        "Panels",
        "Visuals",
        "Notices"
    };
    static const char* subtitles[] = {
        "A clean first page for summary cards, route chips, and plot treatment.",
        "Generic widgets and segmented controls that can be reused in your own layouts.",
        "A reference page for framing, information density, and grouped content.",
        "Animated background ideas, glow pacing, and a generic motion canvas.",
        "Overlay presentation ideas for island states, toast text, and status routing."
    };

    ImGui::TextUnformatted(titles[state.active_page]);
    ImGui::PushStyleColor(ImGuiCol_Text, Palette::text_dim);
    ImGui::TextWrapped("%s", subtitles[state.active_page]);
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::PushStyleColor(ImGuiCol_Separator, WithAlpha(Palette::border, 0.16f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::BeginChild("MainScroll", ImVec2(0.0f, 0.0f), false, 0);

    switch (state.active_page) {
    case 0:
        DrawOverviewPage(state, 0.0f, ui_scale);
        break;
    case 1:
        DrawControlsPage(state, 0.0f, ui_scale);
        break;
    case 2:
        DrawPanelsPage(state, 0.0f, ui_scale);
        break;
    case 3:
        DrawVisualsPage(state, 0.0f, ui_scale);
        break;
    default:
        DrawNoticesPage(state, 0.0f, ui_scale);
        break;
    }

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
}

void DrawBottomDock(const AppState& state, float ui_scale) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float width = 460.0f * ui_scale;
    const float height = 46.0f * ui_scale;
    const ImVec2 min(viewport->WorkPos.x + (viewport->WorkSize.x - width) * 0.5f,
                     viewport->WorkPos.y + viewport->WorkSize.y - height - 18.0f * ui_scale);
    const ImVec2 max(min.x + width, min.y + height);
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    for (int layer = 6; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 6.0f;
        const float spread = (1.5f + 2.5f * t) * ui_scale;
        const float alpha = 0.08f * (1.0f - t * 0.75f);
        draw_list->AddRectFilled(ImVec2(min.x - spread, min.y - spread),
                                 ImVec2(max.x + spread, max.y + spread + 3.0f * ui_scale),
                                 IM_COL32(0, 0, 0, static_cast<int>(alpha * 255.0f)),
                                 22.0f * ui_scale + spread);
    }

    draw_list->AddRectFilled(min, max, IM_COL32(0, 0, 0, 210), 22.0f * ui_scale);
    draw_list->AddRect(min, max, ToU32(WithAlpha(Palette::accent, 0.22f)), 22.0f * ui_scale, 0, 1.0f);

    const char buffer_a[] = "Theme cyan";
    char buffer_b[32];
    char buffer_c[32];
    char buffer_d[32];
    std::snprintf(buffer_b, sizeof(buffer_b), "Signal %d%%", state.signal_quality);
    std::snprintf(buffer_c, sizeof(buffer_c), "Queue %d", state.status_queue);
    std::snprintf(buffer_d, sizeof(buffer_d), "Page %d", state.active_page + 1);
    const char* items[] = { buffer_a, buffer_b, buffer_c, buffer_d, "Overlay ready" };

    float cursor_x = min.x + 18.0f * ui_scale;
    for (const char* item : items) {
        DrawPillText(draw_list, ImVec2(cursor_x, min.y + 10.0f * ui_scale), item, Palette::accent, 12.0f * ui_scale, 10.0f * ui_scale, 4.0f * ui_scale);
        cursor_x += ImGui::CalcTextSize(item).x + 38.0f * ui_scale;
    }
}

void DrawDynamicIsland(AppState& state, float ui_scale) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    SYSTEMTIME local_time{};
    GetLocalTime(&local_time);

    char clock_text[16];
    std::snprintf(clock_text,
                  sizeof(clock_text),
                  "%02u:%02u",
                  static_cast<unsigned int>(local_time.wHour),
                  static_cast<unsigned int>(local_time.wMinute));

    const char* notice = state.notices[state.selected_notice].c_str();
    std::string compact = std::string(clock_text) + "  UI preview";

    const float time = static_cast<float>(ImGui::GetTime());
    const bool auto_expand = std::fmod(time, 10.0f) < 2.6f;
    const ImVec2 compact_text_size = ImGui::CalcTextSize(compact.c_str());
    const ImVec2 notice_size = ImGui::CalcTextSize(notice, nullptr, false, 280.0f * ui_scale);

    const float compact_w = std::max(190.0f * ui_scale, compact_text_size.x + 58.0f * ui_scale);
    const float compact_h = 36.0f * ui_scale;
    const float expanded_w = std::max(320.0f * ui_scale, notice_size.x + 66.0f * ui_scale);
    const float expanded_h = 116.0f * ui_scale;

    const float width = compact_w + (expanded_w - compact_w) * EaseOutCubic(state.island_expand);
    const float height = compact_h + (expanded_h - compact_h) * EaseOutCubic(state.island_expand);
    const ImVec2 min(viewport->WorkPos.x + (viewport->WorkSize.x - width) * 0.5f,
                     viewport->WorkPos.y + 18.0f * ui_scale);
    const ImVec2 max(min.x + width, min.y + height);

    const ImGuiIO& io = ImGui::GetIO();
    const bool hovered = io.MousePos.x >= min.x && io.MousePos.x <= max.x && io.MousePos.y >= min.y && io.MousePos.y <= max.y;
    if (hovered && ImGui::IsMouseClicked(0)) {
        state.island_pinned = !state.island_pinned;
    }

    const bool expanded_target = hovered || state.island_pinned || auto_expand;
    state.island_expand = Approach(state.island_expand, expanded_target ? 1.0f : 0.0f, 6.0f, io.DeltaTime);
    const float expand = EaseOutCubic(state.island_expand);
    const float rounding = compact_h * 0.5f + (24.0f * ui_scale - compact_h * 0.5f) * expand;

    for (int layer = 7; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / 7.0f;
        const float spread = (1.0f + 2.0f * static_cast<float>(layer)) * ui_scale;
        const int alpha = static_cast<int>((24.0f + 20.0f * expand) * (1.0f - t * 0.62f));
        draw_list->AddRect(ImVec2(min.x - spread, min.y - spread),
                           ImVec2(max.x + spread, max.y + spread),
                           IM_COL32(32, 175, 255, alpha),
                           rounding + spread,
                           0,
                           1.0f);
    }

    draw_list->AddRectFilled(min, max, IM_COL32(0, 0, 0, 245), rounding);
    draw_list->AddRect(min, max, ToU32(WithAlpha(Palette::accent, 0.32f + 0.18f * expand)), rounding, 0, 1.0f);

    const float compact_alpha = Saturate(1.0f - expand * 1.35f);
    if (compact_alpha > 0.001f) {
        draw_list->AddText(ImVec2(min.x + (width - compact_text_size.x) * 0.5f, min.y + (height - compact_text_size.y) * 0.5f - 1.0f * ui_scale),
                           ToU32(WithAlpha(Palette::text, compact_alpha)),
                           compact.c_str());
    }

    const float expanded_alpha = Saturate((expand - 0.18f) / 0.82f);
    if (expanded_alpha > 0.001f) {
        const float pad_x = 22.0f * ui_scale;
        draw_list->AddText(ImVec2(min.x + pad_x, min.y + 18.0f * ui_scale),
                           ToU32(WithAlpha(Palette::text, expanded_alpha)),
                           "UI preview island");
        draw_list->AddText(ImVec2(min.x + pad_x, min.y + 46.0f * ui_scale),
                           ToU32(WithAlpha(Palette::accent, 0.95f * expanded_alpha)),
                           notice);
        draw_list->AddText(ImVec2(min.x + pad_x, min.y + 82.0f * ui_scale),
                           ToU32(WithAlpha(Palette::text_dim, 0.92f * expanded_alpha)),
                           state.island_pinned ? "Pinned with left click" : "Hover or click to pin");
    }
}

void RenderUi(AppState& state, float ui_scale) {
    UpdateState(state);
    DrawAnimatedBackground(state);
    DrawSidebar(state, ui_scale);
    DrawMainSurface(state, ui_scale);
    DrawBottomDock(state, ui_scale);
    DrawDynamicIsland(state, ui_scale);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int) {
    ImGui_ImplWin32_EnableDpiAwareness();
    const float monitor_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    const float ui_scale = std::max(monitor_scale, kBaseUiScale);

    WNDCLASSEXW window_class = {
        sizeof(window_class),
        CS_CLASSDC,
        WindowProc,
        0L,
        0L,
        instance,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        L"imgui_onguoin_window",
        nullptr
    };
    ::RegisterClassExW(&window_class);

    HWND window = ::CreateWindowW(window_class.lpszClassName,
                                  L"imgui-onguoin",
                                  WS_OVERLAPPEDWINDOW,
                                  100,
                                  80,
                                  static_cast<int>(1360.0f * ui_scale),
                                  static_cast<int>(880.0f * ui_scale),
                                  nullptr,
                                  nullptr,
                                  window_class.hInstance,
                                  nullptr);

    if (!CreateDeviceD3D(window)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(window_class.lpszClassName, window_class.hInstance);
        return 1;
    }

    ::ShowWindow(window, SW_SHOWDEFAULT);
    ::UpdateWindow(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "imgui.ini";

    ApplyTheme(ui_scale);
    LoadFont();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(g_device, g_device_context);

    AppState state;
    bool done = false;
    while (!done) {
        MSG message;
        while (::PeekMessage(&message, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&message);
            ::DispatchMessage(&message);
            if (message.message == WM_QUIT) {
                done = true;
            }
        }
        if (done) {
            break;
        }

        if (g_swap_chain_occluded && g_swap_chain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            ::Sleep(10);
            continue;
        }
        g_swap_chain_occluded = false;

        if (g_resize_width != 0 && g_resize_height != 0) {
            CleanupRenderTarget();
            g_swap_chain->ResizeBuffers(0, g_resize_width, g_resize_height, DXGI_FORMAT_UNKNOWN, 0);
            g_resize_width = 0;
            g_resize_height = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderUi(state, ui_scale);

        ImGui::Render();
        const float clear_color[4] = { 0.01f, 0.01f, 0.02f, 1.0f };
        g_device_context->OMSetRenderTargets(1, &g_main_render_target, nullptr);
        g_device_context->ClearRenderTargetView(g_main_render_target, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        const HRESULT present_result = g_swap_chain->Present(1, 0);
        g_swap_chain_occluded = (present_result == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(window);
    ::UnregisterClassW(window_class.lpszClassName, window_class.hInstance);
    return 0;
}

bool CreateDeviceD3D(HWND window) {
    DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.BufferDesc.Width = 0;
    swap_chain_desc.BufferDesc.Height = 0;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = window;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.Windowed = TRUE;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT create_device_flags = 0;
    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr,
                                                   D3D_DRIVER_TYPE_HARDWARE,
                                                   nullptr,
                                                   create_device_flags,
                                                   feature_levels,
                                                   2,
                                                   D3D11_SDK_VERSION,
                                                   &swap_chain_desc,
                                                   &g_swap_chain,
                                                   &g_device,
                                                   &feature_level,
                                                   &g_device_context);

    if (result == DXGI_ERROR_UNSUPPORTED) {
        result = D3D11CreateDeviceAndSwapChain(nullptr,
                                               D3D_DRIVER_TYPE_WARP,
                                               nullptr,
                                               create_device_flags,
                                               feature_levels,
                                               2,
                                               D3D11_SDK_VERSION,
                                               &swap_chain_desc,
                                               &g_swap_chain,
                                               &g_device,
                                               &feature_level,
                                               &g_device_context);
    }

    if (result != S_OK) {
        return false;
    }

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_swap_chain != nullptr) {
        g_swap_chain->Release();
        g_swap_chain = nullptr;
    }
    if (g_device_context != nullptr) {
        g_device_context->Release();
        g_device_context = nullptr;
    }
    if (g_device != nullptr) {
        g_device->Release();
        g_device = nullptr;
    }
}

void CreateRenderTarget() {
    ID3D11Texture2D* back_buffer = nullptr;
    g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    g_device->CreateRenderTargetView(back_buffer, nullptr, &g_main_render_target);
    back_buffer->Release();
}

void CleanupRenderTarget() {
    if (g_main_render_target != nullptr) {
        g_main_render_target->Release();
        g_main_render_target = nullptr;
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

LRESULT WINAPI WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
        return true;
    }

    switch (message) {
    case WM_SIZE:
        if (w_param == SIZE_MINIMIZED) {
            return 0;
        }
        g_resize_width = static_cast<UINT>(LOWORD(l_param));
        g_resize_height = static_cast<UINT>(HIWORD(l_param));
        return 0;
    case WM_SYSCOMMAND:
        if ((w_param & 0xfff0) == SC_KEYMENU) {
            return 0;
        }
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return ::DefWindowProcW(window, message, w_param, l_param);
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_effects.h"

namespace imgui_onguoin {

void draw_app_background(ImDrawList* drawList,
                         ImVec2 min,
                         ImVec2 max,
                         bool authenticated,
                         const Theme& theme);
inline void draw_app_background(ImDrawList* drawList,
                                ImVec2 min,
                                ImVec2 max,
                                bool authenticated) {
    draw_app_background(drawList, min, max, authenticated, default_theme());
}
inline void draw_app_background(ImDrawList* drawList,
                                ImVec2 min,
                                ImVec2 max,
                                bool authenticated,
                                const Palette& palette) {
    draw_app_background(drawList, min, max, authenticated, make_theme(palette));
}

} // namespace imgui_onguoin

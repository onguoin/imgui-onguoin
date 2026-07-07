// SPDX-License-Identifier: MIT
// Copyright (c) 2026 onguoin

#pragma once

#include "ui/imgui_onguoin_effects.h"
#include "ui/imgui_onguoin_layout.h"

#include <cstddef>
#include <functional>
#include <utility>

namespace imgui_onguoin {

inline constexpr float kMillisecondsDisplayScale = 1000.0f;

inline BufferedInputState make_buffered_input_state(char* buffer,
                                                    int bufferSize,
                                                    bool* editing = nullptr) {
    return BufferedInputState{buffer, bufferSize, editing};
}

template <std::size_t N>
inline BufferedInputState make_buffered_input_state(char (&buffer)[N],
                                                    bool* editing = nullptr) {
    return make_buffered_input_state(buffer, static_cast<int>(N), editing);
}

template <std::size_t N>
inline BufferedInputState make_buffered_input_state(BufferedInputStorage<N>& storage) {
    return make_buffered_input_state(storage.buffer, &storage.editing);
}

template <std::size_t Count, std::size_t N>
inline BufferedInputState make_indexed_buffered_input_state(char (&buffers)[Count][N],
                                                            bool (&editing)[Count],
                                                            int index) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? make_buffered_input_state(buffers[index], &editing[index])
        : BufferedInputState{};
}

template <std::size_t Count, std::size_t N>
inline BufferedInputState make_indexed_buffered_input_state(char (&buffers)[Count][N],
                                                            int index) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? make_buffered_input_state(buffers[index])
        : BufferedInputState{};
}

template <std::size_t Count, std::size_t N>
inline BufferedInputState make_indexed_buffered_input_state(BufferedInputStorage<N> (&storage)[Count],
                                                            int index) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? make_buffered_input_state(storage[index])
        : BufferedInputState{};
}

inline BufferedRangeInputState make_buffered_range_input_state(BufferedInputState minInput,
                                                               BufferedInputState maxInput) {
    return BufferedRangeInputState{minInput, maxInput};
}

template <std::size_t MinN, std::size_t MaxN>
inline BufferedRangeInputState make_buffered_range_input_state(char (&minBuffer)[MinN],
                                                               bool* minEditing,
                                                               char (&maxBuffer)[MaxN],
                                                               bool* maxEditing) {
    return make_buffered_range_input_state(make_buffered_input_state(minBuffer, minEditing),
                                           make_buffered_input_state(maxBuffer, maxEditing));
}

template <std::size_t N>
inline BufferedRangeInputState make_buffered_range_input_state(BufferedRangeInputStorage<N>& storage) {
    return make_buffered_range_input_state(make_buffered_input_state(storage.min),
                                           make_buffered_input_state(storage.max));
}

template <std::size_t Count, std::size_t MinN, std::size_t MaxN>
inline BufferedRangeInputState make_indexed_buffered_range_input_state(char (&minBuffers)[Count][MinN],
                                                                       bool (&minEditing)[Count],
                                                                       char (&maxBuffers)[Count][MaxN],
                                                                       bool (&maxEditing)[Count],
                                                                       int index) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? make_buffered_range_input_state(make_buffered_input_state(minBuffers[index], &minEditing[index]),
                                          make_buffered_input_state(maxBuffers[index], &maxEditing[index]))
        : BufferedRangeInputState{};
}

template <std::size_t Count, std::size_t MinN, std::size_t MaxN>
inline BufferedRangeInputState make_indexed_buffered_range_input_state(char (&minBuffers)[Count][MinN],
                                                                       char (&maxBuffers)[Count][MaxN],
                                                                       int index) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? make_buffered_range_input_state(make_buffered_input_state(minBuffers[index]),
                                          make_buffered_input_state(maxBuffers[index]))
        : BufferedRangeInputState{};
}

template <std::size_t Count, std::size_t N>
inline BufferedRangeInputState make_indexed_buffered_range_input_state(BufferedRangeInputStorage<N> (&storage)[Count],
                                                                       int index) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? make_buffered_range_input_state(storage[index])
        : BufferedRangeInputState{};
}

void sync_int_input_buffer(BufferedInputState inputState, int value);
void sync_float_input_buffer(BufferedInputState inputState, float value, const char* format = "%.3f");
void sync_scaled_int_input_buffer(BufferedInputState inputState, int value, float displayScale, const char* format = "%.3f");
void sync_milliseconds_input_buffer(BufferedInputState inputState, int valueMicroseconds);
void sync_float_range_input_buffer(BufferedRangeInputState inputState,
                                   float minValue,
                                   float maxValue,
                                   const char* format = "%.3f");
void sync_scaled_int_range_input_buffer(BufferedRangeInputState inputState,
                                        int minValue,
                                        int maxValue,
                                        float displayScale,
                                        const char* format = "%.3f");
void sync_milliseconds_range_input_buffer(BufferedRangeInputState inputState,
                                          int minValueMicroseconds,
                                          int maxValueMicroseconds);

template <std::size_t N>
inline void sync_int_input_buffer(BufferedInputStorage<N>& storage, int value) {
    sync_int_input_buffer(make_buffered_input_state(storage), value);
}

template <std::size_t Count, std::size_t N>
inline void sync_int_input_buffer(BufferedInputStorage<N> (&storage)[Count], int index, int value) {
    if (index < 0 || index >= static_cast<int>(Count)) {
        return;
    }
    sync_int_input_buffer(make_indexed_buffered_input_state(storage, index), value);
}

template <std::size_t N>
inline void sync_milliseconds_input_buffer(BufferedInputStorage<N>& storage, int valueMicroseconds) {
    sync_milliseconds_input_buffer(make_buffered_input_state(storage), valueMicroseconds);
}

template <std::size_t Count, std::size_t N>
inline void sync_milliseconds_input_buffer(BufferedInputStorage<N> (&storage)[Count], int index, int valueMicroseconds) {
    if (index < 0 || index >= static_cast<int>(Count)) {
        return;
    }
    sync_milliseconds_input_buffer(make_indexed_buffered_input_state(storage, index), valueMicroseconds);
}

template <std::size_t N>
inline void sync_milliseconds_range_input_buffer(BufferedRangeInputStorage<N>& storage,
                                                 int minValueMicroseconds,
                                                 int maxValueMicroseconds) {
    sync_milliseconds_range_input_buffer(make_buffered_range_input_state(storage),
                                         minValueMicroseconds,
                                         maxValueMicroseconds);
}

template <std::size_t Count, std::size_t N>
inline void sync_milliseconds_range_input_buffer(BufferedRangeInputStorage<N> (&storage)[Count],
                                                 int index,
                                                 int minValueMicroseconds,
                                                 int maxValueMicroseconds) {
    if (index < 0 || index >= static_cast<int>(Count)) {
        return;
    }
    sync_milliseconds_range_input_buffer(make_indexed_buffered_range_input_state(storage, index),
                                         minValueMicroseconds,
                                         maxValueMicroseconds);
}

bool draw_int_slider_field(const char* id,
                           const char* label,
                           int& value,
                           int minValue,
                           int maxValue,
                           const char* helpText,
                           BufferedInputState inputState,
                           const Theme& theme,
                           FormLayoutStyle layout = {},
                           SliderFieldStyle style = {},
                           float insetX = -1.0f,
                           FieldInteractionState* interaction = nullptr);
inline bool draw_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  const char* helpText,
                                  BufferedInputState inputState,
                                  FormLayoutStyle layout = {},
                                  SliderFieldStyle style = {},
                                  float insetX = -1.0f) {
    return draw_int_slider_field(id, label, value, minValue, maxValue, helpText, inputState, default_theme(), layout, style, insetX, nullptr);
}
inline bool draw_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  const char* helpText,
                                  BufferedInputState inputState,
                                  const Palette& palette,
                                  FormLayoutStyle layout = {},
                                  SliderFieldStyle style = {},
                                  float insetX = -1.0f) {
    return draw_int_slider_field(id, label, value, minValue, maxValue, helpText, inputState, make_theme(palette), layout, style, insetX, nullptr);
}
template <std::size_t N>
inline bool draw_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  const char* helpText,
                                  BufferedInputStorage<N>& storage,
                                  const Theme& theme,
                                  FormLayoutStyle layout = {},
                                  SliderFieldStyle style = {},
                                  float insetX = -1.0f,
                                  FieldInteractionState* interaction = nullptr) {
    return draw_int_slider_field(id, label, value, minValue, maxValue, helpText, make_buffered_input_state(storage), theme, layout, style, insetX, interaction);
}
template <std::size_t N>
inline bool draw_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  const char* helpText,
                                  BufferedInputStorage<N>& storage,
                                  const Theme& theme,
                                  FormLayoutPreset preset,
                                  SliderFieldStyle style = {},
                                  float insetX = -1.0f,
                                  FieldInteractionState* interaction = nullptr) {
    return draw_int_slider_field(id,
                                 label,
                                 value,
                                 minValue,
                                 maxValue,
                                 helpText,
                                 storage,
                                 theme,
                                 make_form_layout_style(theme, preset),
                                 style,
                                 insetX,
                                 interaction);
}
template <std::size_t Count, std::size_t N>
inline bool draw_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  const char* helpText,
                                  BufferedInputStorage<N> (&storage)[Count],
                                  int index,
                                  const Theme& theme,
                                  FormLayoutStyle layout = {},
                                  SliderFieldStyle style = {},
                                  float insetX = -1.0f,
                                  FieldInteractionState* interaction = nullptr) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? draw_int_slider_field(id, label, value, minValue, maxValue, helpText, make_indexed_buffered_input_state(storage, index), theme, layout, style, insetX, interaction)
        : false;
}
template <std::size_t Count, std::size_t N>
inline bool draw_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  const char* helpText,
                                  BufferedInputStorage<N> (&storage)[Count],
                                  int index,
                                  const Theme& theme,
                                  FormLayoutPreset preset,
                                  SliderFieldStyle style = {},
                                  float insetX = -1.0f,
                                  FieldInteractionState* interaction = nullptr) {
    return draw_int_slider_field(id,
                                 label,
                                 value,
                                 minValue,
                                 maxValue,
                                 helpText,
                                 storage,
                                 index,
                                 theme,
                                 make_form_layout_style(theme, preset),
                                 style,
                                 insetX,
                                 interaction);
}

bool draw_float_slider_field(const char* id,
                             const char* label,
                             float& value,
                             float minValue,
                             float maxValue,
                             const char* sliderFormat,
                             const char* inputFormat,
                             const char* helpText,
                             BufferedInputState inputState,
                             const Theme& theme,
                             FormLayoutStyle layout = {},
                             SliderFieldStyle style = {},
                             float insetX = -1.0f);
inline bool draw_float_slider_field(const char* id,
                                    const char* label,
                                    float& value,
                                    float minValue,
                                    float maxValue,
                                    const char* sliderFormat,
                                    const char* inputFormat,
                                    const char* helpText,
                                    BufferedInputState inputState,
                                    FormLayoutStyle layout = {},
                                    SliderFieldStyle style = {},
                                    float insetX = -1.0f) {
    return draw_float_slider_field(id, label, value, minValue, maxValue, sliderFormat, inputFormat, helpText, inputState, default_theme(), layout, style, insetX);
}
inline bool draw_float_slider_field(const char* id,
                                    const char* label,
                                    float& value,
                                    float minValue,
                                    float maxValue,
                                    const char* sliderFormat,
                                    const char* inputFormat,
                                    const char* helpText,
                                    BufferedInputState inputState,
                                    const Palette& palette,
                                    FormLayoutStyle layout = {},
                                    SliderFieldStyle style = {},
                                    float insetX = -1.0f) {
    return draw_float_slider_field(id, label, value, minValue, maxValue, sliderFormat, inputFormat, helpText, inputState, make_theme(palette), layout, style, insetX);
}

bool draw_scaled_int_slider_field(const char* id,
                                  const char* label,
                                  int& value,
                                  int minValue,
                                  int maxValue,
                                  float displayScale,
                                  const char* sliderFormat,
                                  const char* inputFormat,
                                  const char* helpText,
                                  BufferedInputState inputState,
                                  const Theme& theme,
                                  FormLayoutStyle layout = {},
                                  SliderFieldStyle style = {},
                                  float insetX = -1.0f);
inline bool draw_scaled_int_slider_field(const char* id,
                                         const char* label,
                                         int& value,
                                         int minValue,
                                         int maxValue,
                                         float displayScale,
                                         const char* sliderFormat,
                                         const char* inputFormat,
                                         const char* helpText,
                                         BufferedInputState inputState,
                                         FormLayoutStyle layout = {},
                                         SliderFieldStyle style = {},
                                         float insetX = -1.0f) {
    return draw_scaled_int_slider_field(id,
                                        label,
                                        value,
                                        minValue,
                                        maxValue,
                                        displayScale,
                                        sliderFormat,
                                        inputFormat,
                                        helpText,
                                        inputState,
                                        default_theme(),
                                        layout,
                                        style,
                                        insetX);
}
inline bool draw_scaled_int_slider_field(const char* id,
                                         const char* label,
                                         int& value,
                                         int minValue,
                                         int maxValue,
                                         float displayScale,
                                         const char* sliderFormat,
                                         const char* inputFormat,
                                         const char* helpText,
                                         BufferedInputState inputState,
                                         const Palette& palette,
                                         FormLayoutStyle layout = {},
                                         SliderFieldStyle style = {},
                                         float insetX = -1.0f) {
    return draw_scaled_int_slider_field(id,
                                        label,
                                        value,
                                        minValue,
                                        maxValue,
                                        displayScale,
                                        sliderFormat,
                                        inputFormat,
                                        helpText,
                                        inputState,
                                        make_theme(palette),
                                        layout,
                                        style,
                                        insetX);
}

bool draw_milliseconds_slider_field(const char* id,
                                    const char* label,
                                    int& valueMicroseconds,
                                    int minValueMicroseconds,
                                    int maxValueMicroseconds,
                                    const char* helpText,
                                    BufferedInputState inputState,
                                    const Theme& theme,
                                    FormLayoutStyle layout = {},
                                    SliderFieldStyle style = {},
                                    float insetX = -1.0f);
inline bool draw_milliseconds_slider_field(const char* id,
                                           const char* label,
                                           int& valueMicroseconds,
                                           int minValueMicroseconds,
                                           int maxValueMicroseconds,
                                           const char* helpText,
                                           BufferedInputState inputState,
                                           FormLayoutStyle layout = {},
                                           SliderFieldStyle style = {},
                                           float insetX = -1.0f) {
    return draw_milliseconds_slider_field(id,
                                          label,
                                          valueMicroseconds,
                                          minValueMicroseconds,
                                          maxValueMicroseconds,
                                          helpText,
                                          inputState,
                                          default_theme(),
                                          layout,
                                          style,
                                          insetX);
}
inline bool draw_milliseconds_slider_field(const char* id,
                                           const char* label,
                                           int& valueMicroseconds,
                                           int minValueMicroseconds,
                                           int maxValueMicroseconds,
                                           const char* helpText,
                                           BufferedInputState inputState,
                                           const Palette& palette,
                                           FormLayoutStyle layout = {},
                                           SliderFieldStyle style = {},
                                           float insetX = -1.0f) {
    return draw_milliseconds_slider_field(id,
                                          label,
                                          valueMicroseconds,
                                          minValueMicroseconds,
                                          maxValueMicroseconds,
                                          helpText,
                                          inputState,
                                          make_theme(palette),
                                          layout,
                                          style,
                                          insetX);
}
template <std::size_t N>
inline bool draw_milliseconds_slider_field(const char* id,
                                           const char* label,
                                           int& valueMicroseconds,
                                           int minValueMicroseconds,
                                           int maxValueMicroseconds,
                                           const char* helpText,
                                           BufferedInputStorage<N>& storage,
                                           const Theme& theme,
                                           FormLayoutStyle layout = {},
                                           SliderFieldStyle style = {},
                                           float insetX = -1.0f) {
    return draw_milliseconds_slider_field(id,
                                          label,
                                          valueMicroseconds,
                                          minValueMicroseconds,
                                          maxValueMicroseconds,
                                          helpText,
                                          make_buffered_input_state(storage),
                                          theme,
                                          layout,
                                          style,
                                          insetX);
}
template <std::size_t N>
inline bool draw_milliseconds_slider_field(const char* id,
                                           const char* label,
                                           int& valueMicroseconds,
                                           int minValueMicroseconds,
                                           int maxValueMicroseconds,
                                           const char* helpText,
                                           BufferedInputStorage<N>& storage,
                                           const Theme& theme,
                                           FormLayoutPreset preset,
                                           SliderFieldStyle style = {},
                                           float insetX = -1.0f) {
    return draw_milliseconds_slider_field(id,
                                          label,
                                          valueMicroseconds,
                                          minValueMicroseconds,
                                          maxValueMicroseconds,
                                          helpText,
                                          storage,
                                          theme,
                                          make_form_layout_style(theme, preset),
                                          style,
                                          insetX);
}
template <std::size_t Count, std::size_t N>
inline bool draw_milliseconds_slider_field(const char* id,
                                           const char* label,
                                           int& valueMicroseconds,
                                           int minValueMicroseconds,
                                           int maxValueMicroseconds,
                                           const char* helpText,
                                           BufferedInputStorage<N> (&storage)[Count],
                                           int index,
                                           const Theme& theme,
                                           FormLayoutStyle layout = {},
                                           SliderFieldStyle style = {},
                                           float insetX = -1.0f) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? draw_milliseconds_slider_field(id,
                                         label,
                                         valueMicroseconds,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         helpText,
                                         make_indexed_buffered_input_state(storage, index),
                                         theme,
                                         layout,
                                         style,
                                         insetX)
        : false;
}
template <std::size_t Count, std::size_t N>
inline bool draw_milliseconds_slider_field(const char* id,
                                           const char* label,
                                           int& valueMicroseconds,
                                           int minValueMicroseconds,
                                           int maxValueMicroseconds,
                                           const char* helpText,
                                           BufferedInputStorage<N> (&storage)[Count],
                                           int index,
                                           const Theme& theme,
                                           FormLayoutPreset preset,
                                           SliderFieldStyle style = {},
                                           float insetX = -1.0f) {
    return draw_milliseconds_slider_field(id,
                                          label,
                                          valueMicroseconds,
                                          minValueMicroseconds,
                                          maxValueMicroseconds,
                                          helpText,
                                          storage,
                                          index,
                                          theme,
                                          make_form_layout_style(theme, preset),
                                          style,
                                          insetX);
}

bool draw_float_range_field(const char* id,
                            const char* label,
                            float& minValue,
                            float& maxValue,
                            float rangeMin,
                            float rangeMax,
                            const char* valueFormat,
                            const char* helpText,
                            BufferedRangeInputState inputState,
                            const Theme& theme,
                            FormLayoutStyle layout = {},
                            SliderFieldStyle style = {},
                            float insetX = -1.0f);
inline bool draw_float_range_field(const char* id,
                                   const char* label,
                                   float& minValue,
                                   float& maxValue,
                                   float rangeMin,
                                   float rangeMax,
                                   const char* valueFormat,
                                   const char* helpText,
                                   BufferedRangeInputState inputState,
                                   FormLayoutStyle layout = {},
                                   SliderFieldStyle style = {},
                                   float insetX = -1.0f) {
    return draw_float_range_field(id, label, minValue, maxValue, rangeMin, rangeMax, valueFormat, helpText, inputState, default_theme(), layout, style, insetX);
}
inline bool draw_float_range_field(const char* id,
                                   const char* label,
                                   float& minValue,
                                   float& maxValue,
                                   float rangeMin,
                                   float rangeMax,
                                   const char* valueFormat,
                                   const char* helpText,
                                   BufferedRangeInputState inputState,
                                   const Palette& palette,
                                   FormLayoutStyle layout = {},
                                   SliderFieldStyle style = {},
                                   float insetX = -1.0f) {
    return draw_float_range_field(id, label, minValue, maxValue, rangeMin, rangeMax, valueFormat, helpText, inputState, make_theme(palette), layout, style, insetX);
}

bool draw_scaled_int_range_field(const char* id,
                                 const char* label,
                                 int& minValue,
                                 int& maxValue,
                                 int rangeMin,
                                 int rangeMax,
                                 float displayScale,
                                 const char* valueFormat,
                                 const char* helpText,
                                 BufferedRangeInputState inputState,
                                 const Theme& theme,
                                 FormLayoutStyle layout = {},
                                 SliderFieldStyle style = {},
                                 float insetX = -1.0f);
inline bool draw_scaled_int_range_field(const char* id,
                                        const char* label,
                                        int& minValue,
                                        int& maxValue,
                                        int rangeMin,
                                        int rangeMax,
                                        float displayScale,
                                        const char* valueFormat,
                                        const char* helpText,
                                        BufferedRangeInputState inputState,
                                        FormLayoutStyle layout = {},
                                        SliderFieldStyle style = {},
                                        float insetX = -1.0f) {
    return draw_scaled_int_range_field(id,
                                       label,
                                       minValue,
                                       maxValue,
                                       rangeMin,
                                       rangeMax,
                                       displayScale,
                                       valueFormat,
                                       helpText,
                                       inputState,
                                       default_theme(),
                                       layout,
                                       style,
                                       insetX);
}
inline bool draw_scaled_int_range_field(const char* id,
                                        const char* label,
                                        int& minValue,
                                        int& maxValue,
                                        int rangeMin,
                                        int rangeMax,
                                        float displayScale,
                                        const char* valueFormat,
                                        const char* helpText,
                                        BufferedRangeInputState inputState,
                                        const Palette& palette,
                                        FormLayoutStyle layout = {},
                                        SliderFieldStyle style = {},
                                        float insetX = -1.0f) {
    return draw_scaled_int_range_field(id,
                                       label,
                                       minValue,
                                       maxValue,
                                       rangeMin,
                                       rangeMax,
                                       displayScale,
                                       valueFormat,
                                       helpText,
                                       inputState,
                                       make_theme(palette),
                                       layout,
                                       style,
                                       insetX);
}

bool draw_milliseconds_range_field(const char* id,
                                   const char* label,
                                   int& minValueMicroseconds,
                                   int& maxValueMicroseconds,
                                   int rangeMinMicroseconds,
                                   int rangeMaxMicroseconds,
                                   const char* helpText,
                                   BufferedRangeInputState inputState,
                                   const Theme& theme,
                                   FormLayoutStyle layout = {},
                                   SliderFieldStyle style = {},
                                   float insetX = -1.0f);
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputState inputState,
                                          FormLayoutStyle layout = {},
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return draw_milliseconds_range_field(id,
                                         label,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         rangeMinMicroseconds,
                                         rangeMaxMicroseconds,
                                         helpText,
                                         inputState,
                                         default_theme(),
                                         layout,
                                         style,
                                         insetX);
}
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputState inputState,
                                          const Palette& palette,
                                          FormLayoutStyle layout = {},
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return draw_milliseconds_range_field(id,
                                         label,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         rangeMinMicroseconds,
                                         rangeMaxMicroseconds,
                                         helpText,
                                         inputState,
                                         make_theme(palette),
                                         layout,
                                         style,
                                         insetX);
}
template <std::size_t N>
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputStorage<N>& storage,
                                          const Theme& theme,
                                          FormLayoutStyle layout = {},
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return draw_milliseconds_range_field(id,
                                         label,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         rangeMinMicroseconds,
                                         rangeMaxMicroseconds,
                                         helpText,
                                         make_buffered_range_input_state(storage),
                                         theme,
                                         layout,
                                         style,
                                         insetX);
}
template <std::size_t N>
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputStorage<N>& storage,
                                          const Theme& theme,
                                          FormLayoutPreset preset,
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return draw_milliseconds_range_field(id,
                                         label,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         rangeMinMicroseconds,
                                         rangeMaxMicroseconds,
                                         helpText,
                                         storage,
                                         theme,
                                         make_form_layout_style(theme, preset),
                                         style,
                                         insetX);
}
template <std::size_t N>
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputStorage<N>& storage,
                                          const Theme& theme,
                                          UiTextLanguage language,
                                          FormLayoutPreset preset,
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return draw_milliseconds_range_field(id,
                                         label,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         rangeMinMicroseconds,
                                         rangeMaxMicroseconds,
                                         helpText,
                                         storage,
                                         theme,
                                         make_form_layout_style(theme, language, preset),
                                         style,
                                         insetX);
}
template <std::size_t Count, std::size_t N>
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputStorage<N> (&storage)[Count],
                                          int index,
                                          const Theme& theme,
                                          FormLayoutStyle layout = {},
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return (index >= 0 && index < static_cast<int>(Count))
        ? draw_milliseconds_range_field(id,
                                        label,
                                        minValueMicroseconds,
                                        maxValueMicroseconds,
                                        rangeMinMicroseconds,
                                        rangeMaxMicroseconds,
                                        helpText,
                                        make_indexed_buffered_range_input_state(storage, index),
                                        theme,
                                        layout,
                                        style,
                                        insetX)
        : false;
}
template <std::size_t Count, std::size_t N>
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputStorage<N> (&storage)[Count],
                                          int index,
                                          const Theme& theme,
                                          FormLayoutPreset preset,
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return draw_milliseconds_range_field(id,
                                         label,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         rangeMinMicroseconds,
                                         rangeMaxMicroseconds,
                                         helpText,
                                         storage,
                                         index,
                                         theme,
                                         make_form_layout_style(theme, preset),
                                         style,
                                         insetX);
}
template <std::size_t Count, std::size_t N>
inline bool draw_milliseconds_range_field(const char* id,
                                          const char* label,
                                          int& minValueMicroseconds,
                                          int& maxValueMicroseconds,
                                          int rangeMinMicroseconds,
                                          int rangeMaxMicroseconds,
                                          const char* helpText,
                                          BufferedRangeInputStorage<N> (&storage)[Count],
                                          int index,
                                          const Theme& theme,
                                          UiTextLanguage language,
                                          FormLayoutPreset preset,
                                          SliderFieldStyle style = {},
                                          float insetX = -1.0f) {
    return draw_milliseconds_range_field(id,
                                         label,
                                         minValueMicroseconds,
                                         maxValueMicroseconds,
                                         rangeMinMicroseconds,
                                         rangeMaxMicroseconds,
                                         helpText,
                                         storage,
                                         index,
                                         theme,
                                         make_form_layout_style(theme, language, preset),
                                         style,
                                         insetX);
}

void draw_help_marker(const char* text, const Theme& theme);
void same_line_help_marker_aligned_to_last_item(const char* text, const Theme& theme);
AnimatedComboScope begin_animated_combo(const char* id,
                                        const char* previewValue,
                                        float comboWidth,
                                        const Theme& theme,
                                        SelectFieldStyle style = {});
void end_animated_combo(const AnimatedComboScope& scope);

template <typename OptionType, typename LabelFn, typename SelectedFn, typename ActivateFn>
bool render_select_items_from_list(const OptionType* options,
                                   int optionCount,
                                   LabelFn&& labelForOption,
                                   SelectedFn&& isSelected,
                                   ActivateFn&& activateOption) {
    if (options == nullptr || optionCount <= 0) {
        return false;
    }

    bool changed = false;
    for (int i = 0; i < optionCount; ++i) {
        const OptionType& option = options[i];
        const bool selected = isSelected(option);
        if (ImGui::Selectable(labelForOption(option), selected)) {
            activateOption(option);
            ImGui::CloseCurrentPopup();
            changed = true;
        }
        if (selected) {
            ImGui::SetItemDefaultFocus();
        }
    }
    return changed;
}

template <typename ItemRendererFn>
bool draw_select_field(const char* id,
                       const char* label,
                       const char* previewValue,
                       const char* helpText,
                       ItemRendererFn&& renderItems,
                       const Theme& theme,
                       FormLayoutStyle layout = {},
                       SelectFieldStyle style = {},
                       float insetX = -1.0f) {
    const float scale = current_scale();
    bool changed = false;
    ImGui::PushID(id);
    begin_form_row(label, layout, insetX);

    const float comboWidth = (style.comboWidth > 0.0f ? style.comboWidth : theme.fields.sliderWidth) * scale;
    const float frameRounding = (style.frameRounding > 0.0f ? style.frameRounding : theme.fields.frameRounding) * scale;
    const ImVec2 comboMin = ImGui::GetCursorScreenPos();
    const ImVec2 comboMax(comboMin.x + comboWidth, comboMin.y + ImGui::GetFrameHeight());
    AnimatedComboScope combo = begin_animated_combo("##combo",
                                                    previewValue != nullptr ? previewValue : "",
                                                    comboWidth,
                                                    theme,
                                                    style);
    if (combo.open) {
        changed = renderItems();
    }
    end_animated_combo(combo);

    const bool comboHovered = ImGui::IsMouseHoveringRect(comboMin, comboMax);
    draw_select_frame(ImGui::GetWindowDrawList(),
                      comboMin,
                      comboMax,
                      comboHovered,
                      combo.open,
                      theme,
                      frameRounding);

    if (helpText != nullptr && helpText[0] != '\0') {
        same_line_help_marker_aligned_to_last_item(helpText, theme);
    }

    ImGui::PopID();
    return changed;
}

template <typename OptionType, typename LabelFn, typename SelectedFn, typename ActivateFn>
bool draw_select_field_from_list(const char* id,
                                 const char* label,
                                 const char* previewValue,
                                 const char* helpText,
                                 const OptionType* options,
                                 int optionCount,
                                 LabelFn&& labelForOption,
                                 SelectedFn&& isSelected,
                                 ActivateFn&& activateOption,
                                 const Theme& theme,
                                 FormLayoutStyle layout = {},
                                 SelectFieldStyle style = {},
                                 float insetX = -1.0f) {
    return draw_select_field(
        id,
        label,
        previewValue,
        helpText,
        [&]() {
            return render_select_items_from_list(options,
                                                 optionCount,
                                                 labelForOption,
                                                 isSelected,
                                                 activateOption);
        },
        theme,
        layout,
        style,
        insetX);
}

template <typename OptionType, typename LabelFn, typename SelectedFn, typename ActivateFn>
inline bool draw_select_field_from_list(const char* id,
                                        const char* label,
                                        const char* previewValue,
                                        const char* helpText,
                                        const OptionType* options,
                                        int optionCount,
                                        LabelFn&& labelForOption,
                                        SelectedFn&& isSelected,
                                        ActivateFn&& activateOption,
                                        FormLayoutStyle layout = {},
                                        SelectFieldStyle style = {},
                                        float insetX = -1.0f) {
    return draw_select_field_from_list(id,
                                       label,
                                       previewValue,
                                       helpText,
                                       options,
                                       optionCount,
                                       std::forward<LabelFn>(labelForOption),
                                       std::forward<SelectedFn>(isSelected),
                                       std::forward<ActivateFn>(activateOption),
                                       default_theme(),
                                       layout,
                                       style,
                                       insetX);
}

template <typename OptionType, typename LabelFn, typename SelectedFn, typename ActivateFn>
inline bool draw_select_field_from_list(const char* id,
                                        const char* label,
                                        const char* previewValue,
                                        const char* helpText,
                                        const OptionType* options,
                                        int optionCount,
                                        LabelFn&& labelForOption,
                                        SelectedFn&& isSelected,
                                        ActivateFn&& activateOption,
                                        const Palette& palette,
                                        FormLayoutStyle layout = {},
                                        SelectFieldStyle style = {},
                                        float insetX = -1.0f) {
    return draw_select_field_from_list(id,
                                       label,
                                       previewValue,
                                       helpText,
                                       options,
                                       optionCount,
                                       std::forward<LabelFn>(labelForOption),
                                       std::forward<SelectedFn>(isSelected),
                                       std::forward<ActivateFn>(activateOption),
                                       make_theme(palette),
                                       layout,
                                       style,
                                       insetX);
}

template <typename ValueType, std::size_t N, typename LabelFn>
bool draw_select_field_from_array(const char* id,
                                  const char* label,
                                  ValueType& value,
                                  const ValueType (&options)[N],
                                  LabelFn&& labelForOption,
                                  const char* helpText,
                                  const Theme& theme,
                                  FormLayoutStyle layout = {},
                                  SelectFieldStyle style = {},
                                  float insetX = -1.0f) {
    return draw_select_field_from_list(
        id,
        label,
        labelForOption(value),
        helpText,
        options,
        static_cast<int>(N),
        labelForOption,
        [&](const ValueType& option) {
            return value == option;
        },
        [&](const ValueType& option) {
            value = option;
        },
        theme,
        layout,
        style,
        insetX);
}

template <typename ValueType, std::size_t N, typename LabelFn>
inline bool draw_select_field_from_array(const char* id,
                                         const char* label,
                                         ValueType& value,
                                         const ValueType (&options)[N],
                                         LabelFn&& labelForOption,
                                         const char* helpText,
                                         FormLayoutStyle layout = {},
                                         SelectFieldStyle style = {},
                                         float insetX = -1.0f) {
    return draw_select_field_from_array(id,
                                        label,
                                        value,
                                        options,
                                        std::forward<LabelFn>(labelForOption),
                                        helpText,
                                        default_theme(),
                                        layout,
                                        style,
                                        insetX);
}

template <typename ValueType, std::size_t N, typename LabelFn>
inline bool draw_select_field_from_array(const char* id,
                                         const char* label,
                                         ValueType& value,
                                         const ValueType (&options)[N],
                                         LabelFn&& labelForOption,
                                         const char* helpText,
                                         const Palette& palette,
                                         FormLayoutStyle layout = {},
                                         SelectFieldStyle style = {},
                                         float insetX = -1.0f) {
    return draw_select_field_from_array(id,
                                        label,
                                        value,
                                        options,
                                        std::forward<LabelFn>(labelForOption),
                                        helpText,
                                        make_theme(palette),
                                        layout,
                                        style,
                                        insetX);
}
template <typename ValueType, std::size_t N, typename LabelFn>
inline bool draw_select_field_from_array(const char* id,
                                         const char* label,
                                         ValueType& value,
                                         const ValueType (&options)[N],
                                         LabelFn&& labelForOption,
                                         const char* helpText,
                                         const Theme& theme,
                                         UiTextLanguage language,
                                         FormLayoutPreset preset,
                                         SelectFieldStyle style = {},
                                         float insetX = -1.0f) {
    return draw_select_field_from_array(id,
                                        label,
                                        value,
                                        options,
                                        std::forward<LabelFn>(labelForOption),
                                        helpText,
                                        theme,
                                        make_form_layout_style(theme, language, preset),
                                        style,
                                        insetX);
}
template <typename ValueType, std::size_t N, typename LabelFn>
inline bool draw_select_field_from_array(const char* id,
                                         const char* label,
                                         ValueType& value,
                                         const ValueType (&options)[N],
                                         LabelFn&& labelForOption,
                                         const char* helpText,
                                         UiTextLanguage language,
                                         FormLayoutPreset preset,
                                         SelectFieldStyle style = {},
                                         float insetX = -1.0f) {
    return draw_select_field_from_array(id,
                                        label,
                                        value,
                                        options,
                                        std::forward<LabelFn>(labelForOption),
                                        helpText,
                                        default_theme(),
                                        language,
                                        preset,
                                        style,
                                        insetX);
}
template <typename ValueType, std::size_t N, typename LabelFn>
inline bool draw_select_field_from_array(const char* id,
                                         const char* label,
                                         ValueType& value,
                                         const ValueType (&options)[N],
                                         LabelFn&& labelForOption,
                                         const char* helpText,
                                         const Palette& palette,
                                         UiTextLanguage language,
                                         FormLayoutPreset preset,
                                         SelectFieldStyle style = {},
                                         float insetX = -1.0f) {
    return draw_select_field_from_array(id,
                                        label,
                                        value,
                                        options,
                                        std::forward<LabelFn>(labelForOption),
                                        helpText,
                                        make_theme(palette),
                                        language,
                                        preset,
                                        style,
                                        insetX);
}

template <typename ItemRendererFn>
inline bool draw_select_field(const char* id,
                              const char* label,
                              const char* previewValue,
                              const char* helpText,
                              ItemRendererFn&& renderItems,
                              FormLayoutStyle layout = {},
                              SelectFieldStyle style = {},
                              float insetX = -1.0f) {
    return draw_select_field(id, label, previewValue, helpText, std::forward<ItemRendererFn>(renderItems), default_theme(), layout, style, insetX);
}

template <typename ItemRendererFn>
inline bool draw_select_field(const char* id,
                              const char* label,
                              const char* previewValue,
                              const char* helpText,
                              ItemRendererFn&& renderItems,
                              const Palette& palette,
                              FormLayoutStyle layout = {},
                              SelectFieldStyle style = {},
                              float insetX = -1.0f) {
    return draw_select_field(id, label, previewValue, helpText, std::forward<ItemRendererFn>(renderItems), make_theme(palette), layout, style, insetX);
}

bool draw_theme_flavor_field(const char* id,
                             ThemeFlavor currentFlavor,
                             const ThemeFlavorFieldData& data,
                             const Theme& theme,
                             const std::function<void(ThemeFlavor)>& activateFlavor,
                             FormLayoutStyle layout = {},
                             SelectFieldStyle style = {},
                             float insetX = -1.0f);
inline bool draw_theme_flavor_field(const char* id,
                                    ThemeFlavor currentFlavor,
                                    const ThemeFlavorFieldData& data,
                                    const std::function<void(ThemeFlavor)>& activateFlavor,
                                    FormLayoutStyle layout = {},
                                    SelectFieldStyle style = {},
                                    float insetX = -1.0f) {
    return draw_theme_flavor_field(id, currentFlavor, data, default_theme(), activateFlavor, layout, style, insetX);
}
inline bool draw_theme_flavor_field(const char* id,
                                    ThemeFlavor currentFlavor,
                                    const ThemeFlavorFieldData& data,
                                    const Palette& palette,
                                    const std::function<void(ThemeFlavor)>& activateFlavor,
                                    FormLayoutStyle layout = {},
                                    SelectFieldStyle style = {},
                                    float insetX = -1.0f) {
    return draw_theme_flavor_field(id, currentFlavor, data, make_theme(palette), activateFlavor, layout, style, insetX);
}

bool draw_background_kind_field(const char* id,
                                BackgroundKind currentKind,
                                const BackgroundKindFieldData& data,
                                const Theme& theme,
                                const std::function<void(BackgroundKind)>& activateKind,
                                FormLayoutStyle layout = {},
                                SelectFieldStyle style = {},
                                float insetX = -1.0f);
inline bool draw_background_kind_field(const char* id,
                                       BackgroundKind currentKind,
                                       const BackgroundKindFieldData& data,
                                       const std::function<void(BackgroundKind)>& activateKind,
                                       FormLayoutStyle layout = {},
                                       SelectFieldStyle style = {},
                                       float insetX = -1.0f) {
    return draw_background_kind_field(id, currentKind, data, default_theme(), activateKind, layout, style, insetX);
}
inline bool draw_background_kind_field(const char* id,
                                       BackgroundKind currentKind,
                                       const BackgroundKindFieldData& data,
                                       const Palette& palette,
                                       const std::function<void(BackgroundKind)>& activateKind,
                                       FormLayoutStyle layout = {},
                                       SelectFieldStyle style = {},
                                       float insetX = -1.0f) {
    return draw_background_kind_field(id, currentKind, data, make_theme(palette), activateKind, layout, style, insetX);
}

} // namespace imgui_onguoin

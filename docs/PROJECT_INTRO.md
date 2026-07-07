# 项目介绍 / Project Introduction

## GitHub About 简介 / GitHub About Short Description

中文：

```text
基于 Dear ImGui 的可复用 C++ UI kit，提供主题、动态背景、字段控件、布局、绘制特效和语义化组件。
```

English:

```text
A reusable C++ UI kit for Dear ImGui with themes, animated backgrounds, fields, layout helpers, drawing effects, and semantic widgets.
```

## 简短介绍 / Short Introduction

`imgui-onguoin` 是一个面向 Dear ImGui 应用的可复用 C++ UI kit。它将主题系统、动态背景、控件字段、表面/卡片、布局工具、绘制特效和高层语义组件整理成独立库，帮助开发者在 Dear ImGui 中更快构建统一、现代、可组合的桌面工具界面。

`imgui-onguoin` is a reusable C++ UI kit for Dear ImGui applications. It packages theme systems, animated backgrounds, fields, surfaces/cards, layout utilities, drawing effects, and higher-level semantic widgets into a standalone library for building consistent, modern, composable desktop tool interfaces with Dear ImGui.

## 长介绍 / Long Introduction

`imgui-onguoin` 起源于一个真实桌面工具的 UI 抽象层，后来被整理成不包含私有产品逻辑的通用 Dear ImGui UI kit。库本身专注于视觉和交互复用：主题 token、调色板、动画节奏、背景层、表单布局、滑块/范围/选择字段、卡片/面板、状态胶囊、提示浮层、动态岛式通知、窗口 chrome 控件和输入可视化等模块都被拆分到独立的 `imgui_onguoin_*` 文件中。

`imgui-onguoin` started as the UI abstraction layer of a real desktop tool and was later separated into a generic Dear ImGui UI kit without private product logic. The library focuses on reusable visual and interaction building blocks: theme tokens, palettes, motion timing, background layers, form layouts, slider/range/select fields, cards/panels, status capsules, help overlays, dynamic-island-style notices, window chrome controls, input visualizers, and related modules are split across focused `imgui_onguoin_*` files.

本仓库不内置 Dear ImGui。默认构建可通过 CMake FetchContent 下载 Dear ImGui，也可以通过 `IMGUI_DIR` 使用本地 Dear ImGui 源码。`imgui-onguoin` 使用 MIT License 发布；MIT 授权允许使用和再分发，但不会转移原始作品的版权归属。

This repository does not vendor Dear ImGui. The default build can download Dear ImGui through CMake FetchContent, or users can provide a local Dear ImGui checkout through `IMGUI_DIR`. `imgui-onguoin` is released under the MIT License; MIT grants permission to use and redistribute the software, but it does not transfer ownership of the original work.

## 关键词 / Topics

```text
dear-imgui
imgui
cpp
cpp17
ui-kit
desktop-ui
immediate-mode-gui
directx11
win32
themes
widgets
animation
```

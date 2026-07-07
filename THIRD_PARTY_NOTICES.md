# 第三方声明 / Third-Party Notices

## Dear ImGui

`imgui-onguoin` 构建在 Dear ImGui 之上，但本仓库不内置 Dear ImGui 源码。

`imgui-onguoin` is built on top of Dear ImGui, but this repository does not vendor Dear ImGui source code.

Dear ImGui 是独立项目，并以 MIT License 发布。

Dear ImGui is a separate project licensed under the MIT License.

- Project: https://github.com/ocornut/imgui
- Copyright: Copyright (c) 2014-2026 Omar Cornut

当 Dear ImGui 由 CMake FetchContent 下载，或通过 `IMGUI_DIR` 由使用者提供时，Dear ImGui 的许可证只适用于 Dear ImGui 本身，不会转移 `imgui-onguoin` 原始源码的版权归属。

When Dear ImGui is downloaded by CMake FetchContent or supplied through `IMGUI_DIR`, its license applies to Dear ImGui only. It does not transfer ownership of the original `imgui-onguoin` source code.

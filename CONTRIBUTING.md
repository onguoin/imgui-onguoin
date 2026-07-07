# 贡献 / Contributing

`imgui-onguoin` 由 onguoin 维护。

`imgui-onguoin` is maintained by onguoin.

欢迎提交 issue、bug report、构建修复和小范围、目标明确的 pull request。较大的功能改动请先讨论，避免破坏 UI kit 的整体方向和边界。

Issues, bug reports, build fixes, and small focused pull requests are welcome. Large feature work should be discussed first so the project keeps a coherent UI-kit direction and boundary.

## 贡献的许可证 / License of Contributions

向本仓库贡献代码，即表示你同意你的贡献以本项目相同的 MIT License 授权。

By contributing to this repository, you agree that your contribution is licensed under the same MIT License as the project.

除非另有单独书面协议，你仍然保留自己贡献内容的版权。项目官方名称、发布方向和仓库维护权仍由项目维护者控制。

You retain copyright in your own contribution unless a separate written agreement says otherwise. The official project name, release direction, and repository maintainership remain controlled by the project maintainer.

## 代码边界 / Code Boundaries

- 将可复用的视觉行为、布局模式、字段控件、表面、动画辅助、主题 token 和组件保留在 `src/ui/imgui_onguoin_*` 中。
- Keep reusable visual behavior, layout patterns, fields, surfaces, animation helpers, theme tokens, and widgets inside `src/ui/imgui_onguoin_*`.
- 不要加入私有产品逻辑、服务集成、授权流程或产品专用工作流。
- Do not add private product logic, service integrations, auth flows, or product-specific workflows.
- 不要为了某一个应用硬编码共享组件的几何布局；需要变化的值应使用 token、style field 或可复用 API 表达。
- Avoid hardcoding one application's geometry into shared widgets. Prefer tokens, style fields, or reusable APIs.
- Dear ImGui 保持为外部依赖，不要内置进本仓库。
- Keep Dear ImGui as an external dependency; do not vendor it into this repository.

## 风格 / Style

- 使用 C++17。
- Use C++17.
- 公开 API 头文件需要和 `src/ui/imgui_onguoin.h` 保持同步。
- Keep public API headers synchronized with `src/ui/imgui_onguoin.h`.
- 源码默认优先使用 ASCII；只有在现有文本或明确需求需要 UTF-8 时再使用非 ASCII。
- Prefer ASCII in source files unless existing text or a clear requirement needs UTF-8.
- 注释保持简短，只解释不明显的逻辑。
- Keep comments concise and focused on non-obvious logic.

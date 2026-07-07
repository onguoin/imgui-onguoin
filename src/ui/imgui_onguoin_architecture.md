<!-- SPDX-License-Identifier: MIT -->
<!-- Copyright (c) 2026 onguoin -->

# imgui-onguoin Architecture

`imgui-onguoin` is a shared Dear ImGui UI kit structured as a reusable library instead of a page-local style dump.

The architecture aims to make new themes, backgrounds, and widgets cheap to add without rewriting page code.

## Public entry

- `src/ui/imgui_onguoin.h`: umbrella include for external consumers

## Module layout

- `src/ui/imgui_onguoin_types.h`: umbrella include for public tokens, enums, styles, theme/background data models
- `src/ui/imgui_onguoin_core_types.h`: palette, enums, shared public primitives
- `src/ui/imgui_onguoin_background_types.h`: layered background data models
- `src/ui/imgui_onguoin_theme_types.h`: theme, token, motion, field, layout data models
- `src/ui/imgui_onguoin_widget_types.h`: public widget-facing style structs
- `src/ui/imgui_onguoin_theme.h`: theme selection, theme construction, runtime theme access
- `src/ui/imgui_onguoin_math.h`: animation/math helpers and text geometry helpers
- `src/ui/imgui_onguoin_effects.h`: low-level drawing primitives and reusable control/display surfaces
- `src/ui/imgui_onguoin_layout.h`: form and spacing helpers
- `src/ui/imgui_onguoin_surfaces.h`: cards, panels, dock frames
- `src/ui/imgui_onguoin_background.h`: app background rendering
- `src/ui/imgui_onguoin_fields.h`: sliders, ranges, select fields, buffered input-state helpers
- `src/ui/imgui_onguoin_widgets.h`: higher-level UI kit widgets

## Layering rules

- Theme recipes should live in `imgui_onguoin.cpp`, not inside individual widgets.
- Theme and background metadata should stay close to their recipe registration so ids, defaults, and behavior evolve together.
- Theme and background descriptors should carry display-ready metadata such as localized labels, so adding a new preset does not require page-local naming switches.
- Each metadata family should have one canonical active catalog in code; avoid keeping legacy or shadow descriptor arrays alongside the live registration source.
- Theme, background, and background-layer catalogs should share the same lookup, id parsing, and localization semantics internally, so metadata APIs behave like one coherent system instead of several parallel special cases.
- Preset-to-flavor relationships should also be catalog-driven, so adding a new preset does not require scattering `switch` branches across theme-selection helpers.
- Theme presets should expose first-class public metadata, so consumers can enumerate, label, parse, and map presets without rebuilding private app-side lookup tables.
- Preset catalogs should expose label and mapped-flavor helpers with the same ergonomics as theme and background catalogs, so consumers do not need to special-case preset display or preset-to-flavor lookups.
- Theme descriptors should also expose public summary metadata such as tone, preferred background, or motion character, so settings UIs and future tooling can explain a preset without reverse-reading recipe code.
- Background descriptors should also expose structure/capability metadata such as supported layers or dynamic layers, so products can inspect presets without reverse-reading recipe code.
- Background presets should configure layers; the renderer should compose enabled layers generically.
- Background rendering order should be defined by a small layer pipeline, so adding a new layer usually means adding one descriptor and one renderer instead of rewriting `draw_app_background(...)`.
- Background styles should also expose runtime layer state through shared helpers, so products and tools can inspect which layers are actually enabled after recipe composition instead of inferring it from raw fields.
- Motion behavior should be expressed through reusable token structs such as `PulseMotionStyle`, not ad-hoc constants inside widgets.
- Repeated follow/settle animation formulas should be promoted into shared math helpers such as `follow_value(...)`, `follow_factor(...)`, or `move_toward(...)` before multiple screens copy the same transition logic.
- Screen-level reveal, presence, and follow animations should also use shared motion style structs and theme tokens, so products can tune animation personality without hunting page-local magic numbers.
- Reusable visual building blocks should be added to `imgui_onguoin_effects.*` before new widgets duplicate drawing logic.
- Small branded or utility UI fragments that can appear in multiple products, such as footers or signatures, should live in `imgui_onguoin_widgets.*` with overridable style structs instead of staying app-local.
- Identity, license, session, and profile capsules should be modeled as semantic widgets with label/value data structs and theme tokens, so products can swap copy or auth sources without forking draw code.
- Overlay or floating shell chrome, such as compact monitor frames, header strips, and inline status readouts, should be promoted as semantic widgets when they combine frame drawing with title/status layout.
- Compact shell header metrics, such as a centered FPS/status readout, should be owned by the compact shell widget so app code passes data instead of hand-positioning overlay text.
- Repeated compact monitor layouts, such as side-key plus WASD input matrices, should be promoted as shared widgets so products adapt state data instead of re-owning placement math.
- Compact input matrix key contrast, including inactive borders, fills, and text colors, should be token-driven so overlays remain readable on bright or transparent backgrounds.
- Repeated binary choice controls, such as language toggles or two-state display mode pickers, should use shared segmented widgets instead of page-local button pairs.
- Repeated animated direction choices, such as clockwise/counterclockwise route selectors, should use shared direction-toggle widgets so motion, arrow drawing, and rounded track geometry stay theme-driven.
- Repeated top-right window chrome groups, such as language switching plus minimize/close actions, should be promoted as shared widgets so shell-level behavior and spacing stay consistent across pages.
- Repeated runtime/status information clusters, such as pill strips in docks, sidebars, or compact monitors, should use shared group widgets so products stop hand-authoring `SameLine` layout chains.
- Repeated label/value diagnostic summaries, such as overview runtime readouts or compact inspector facts, should use shared info-list widgets so products adapt state data instead of hand-authoring row stacks.
- Repeated trigger/key activity strips, such as physical input monitors or feature trigger dashboards, should use shared signal-pill groups so products stop hand-authoring long `SameLine` chains.
- Product-specific views that mainly adapt runtime data into a stable visual format should prefer a thin adapter layer over app-owned rendering; the reusable rendering should move into `imgui_onguoin_widgets.*`.
- Interactive rows, command items, and binding/list entries that have a stable surface pattern should be promoted into widget-level primitives so product code only decides behavior, copy, and state.
- Floating feedback UI such as warning toasts, inline notifications, and compact status callouts should share widget/effect primitives so products do not re-implement layered glow, progress, icon, and text layout every time.
- Expanded/collapsed notification surfaces such as dynamic-island style overlays should be modeled as shared widgets with explicit state data, not kept as page-local draw-list procedures.
- Compact exchange/edit controls, such as hover-expanded config-string boxes, should live as shared widgets. Products own the serialized data format and import/export rules; the library owns the animation, icon, editable input, and theme tokens.
- Untitled content cards should use shared auto surface panels instead of fixed handwritten heights when their row count or content can grow.
- Widgets and fields should prefer theme tokens and shared primitives over hard-coded colors, spacing, or animation values.
- Buffered field formatting and unit-conversion behavior should live in shared field helpers when multiple pages need the same editing pattern.
- Repeated buffer-state wiring should use shared field-state helpers so pages do not rebuild `BufferedInputState`/`BufferedRangeInputState` by hand.
- Input state storage should prefer shared `BufferedInputStorage` / `BufferedRangeInputStorage` containers so pages do not maintain parallel `char[]` and `bool` editing arrays.
- Field APIs should accept the shared storage containers directly when possible, so consumer code can stay at the storage level instead of manually re-wrapping state views for every call.
- App pages should prefer calling the shared field APIs directly once the shared storage, layout preset, and value semantics are expressive enough; keep app-local wrapper functions only for truly product-specific behavior.
- Repeated form column conventions should use named layout presets so pages describe intent such as "standard form" or "wide form" instead of repeating magic column widths.
- Language-sensitive form column behavior should live in shared layout helpers, so products do not keep app-local wrappers just to compensate for Chinese and English label density.
- Field APIs that already understand layout presets should also expose language-aware overloads, so products can request localized form behavior directly instead of prebuilding layout structs at every call site.
- Repeated select/dropdown option loops should be promoted into shared field helpers when they only differ by option data and label mapping.
- Metadata-backed selectors such as theme-flavor and background-kind pickers should expose library-owned semantic field APIs, so products wire state changes instead of rebuilding catalog enumeration, localization, and combo behavior.
- When products repeatedly combine the same semantic fields with structural summaries, such as theme/background pickers plus preset/tone/capability readouts, promote that composition into a library-owned section widget so page code stays at the state-and-behavior layer.
- Public style override structs should use explicit sentinel semantics for inheritance, rather than relying on magic default constants that happen to match the current theme.

## Current primitive families

- Interactive primitives: `draw_capsule(...)`, `draw_interactive_surface(...)`
- Display primitives: `draw_notice_surface(...)`, `draw_status_pill_surface(...)`
- Frame/effect primitives: `draw_accent_frame(...)`, `draw_top_highlight(...)`, `draw_select_frame(...)`, `draw_circle_badge(...)`, `draw_arc(...)`, `draw_wedge(...)`
- Motion helpers: `follow_value(...)`, `follow_factor(...)`, `move_toward(...)`, `ease(...)`, `animate(...)`, `travel(...)`
- Utility widgets: `draw_footer_signature(...)`, `draw_identity_capsule(...)`, `draw_compact_overlay_shell(...)`, `draw_compact_input_matrix(...)`, `draw_status_pill_group(...)`, `draw_status_info_list(...)`, `draw_signal_pill_group(...)`, `draw_two_option_segmented_control(...)`, `draw_direction_toggle_button(...)`, `draw_window_chrome_controls(...)`, `draw_help_marker(...)`, `draw_stick_visualizer(...)`, `draw_key_binding_row(...)`, `draw_notice_toast(...)`, `draw_dynamic_island(...)`, `draw_config_exchange_control(...)`
- Summary widgets: `draw_theme_summary(...)`, `draw_background_summary(...)`

## Extension strategy

When adding a new UI feature:

1. Add or adjust tokens in `imgui_onguoin_theme_types.h` if the behavior should be theme-driven.
2. Extend theme or background recipes in `imgui_onguoin.cpp` when a flavor or preset needs distinct behavior.
3. Promote repeated drawing logic into `imgui_onguoin_effects.*`.
4. Promote small reusable composition fragments into `imgui_onguoin_widgets.*` when they carry text/layout semantics in addition to drawing.
5. Keep application page code thin and focused on composition instead of rendering internals.

## Standardization direction

- Public type split: keep stable public data in the `*_types.h` headers and avoid leaking internal page-specific helpers through them.
- Theme-driven fields: slider, range, and select visuals should come from `FieldTokens` so multi-theme support does not require widget rewrites.
- Field semantics: repeated input-buffer sync, integer-display scaling, and dual-value range editing should be promoted into `imgui_onguoin_fields.*` rather than copied in app pages.
- Overlay widgets: foreground-layer widgets such as `draw_help_marker(...)` should keep a small invisible layout/hit-test item, then render the full visible component through one token-driven path so collapsed and expanded states share rounding, spacing, clipping, and animation semantics. Expanded overlays that may cover sibling widgets should be queued and flushed at the end of the frame so they draw above all normal widget instances.
- Wrapped text layout: measure text with the same wrap width and font size used for drawing. Animated containers should compute current text bounds before drawing so padding remains stable during expansion.
- Field state model: the library should own the common editing-state shapes alongside the field helpers, so reusable consumers can adopt one stable pattern for text buffers and commit state.
- Time-field semantics: when UI displays microsecond-backed values as milliseconds, prefer dedicated shared helpers over repeating scale constants and format strings in page code.
- Select semantics: repeated enum/int option pickers should use shared select helpers so app pages describe options, not dropdown control flow.
- Runtime list semantics: metadata-backed selection UIs should use shared list-based select helpers so adding a new theme/background/preset mostly means registering data, not rewriting combo behavior.
- Motion language: shared motion structs should define pacing, range, and curve so theme flavors can own their animation personality.
- Motion plumbing: screen transitions, toast settle animations, indicator follow behavior, and other scalar interpolation patterns should share the same math helpers so animation tuning stays centralized.
- Motion ownership: when a page animation is structurally generic, such as sidebar selection follow, login reveal, or notification presence, its pacing should live in `ThemeSpec::motion` instead of app-local constants.
- Background layering: new background presets should mostly be new recipes over shared layer structs, not new bespoke renderers.
- Visualization widgets: panels such as stick/state visualizers should expose a stable data struct in the library, while app code focuses on formatting and localization instead of custom draw-list internals.
- Identity widgets: top-bar labels such as card keys, license ids, or session handles should expose a reusable label/value widget instead of staying embedded in one product header.
- Overlay shell widgets: compact windows should return library-owned layout structs instead of ImGui-internal geometry types, so the public API stays stable and third-party-friendly.
- Input matrix widgets: compact key/state grids should expose stable data structs and theme tokens, so future overlays can swap layouts or visuals without copying coordinate math into app pages.
- Status group widgets: repeated pill collections should expose stable item arrays plus layout style, so future docks or overlays can switch between horizontal, vertical, and wrapping layouts without page-local spacing logic.
- Status info widgets: repeated label/value readout lists should expose stable item arrays plus list style, so future diagnostics and inspector panels stop rebuilding row stacks by hand.
- Signal group widgets: repeated key/trigger activity collections should expose stable item arrays plus wrapping layout style, so future monitors can scale across themes and widths without page-local spacing logic.
- Segmented controls: repeated button-pair selectors should expose reusable option data and style structs, so app pages only decide labels and selected index.
- Direction toggles: animated circular-knob direction selectors should expose labels and a boolean state while the shared widget owns slider travel, arrow vector drawing, and theme-token sizing.
- Window chrome widgets: reusable shell controls should expose result structs for selected segment and button presses, so products keep behavior ownership while the library owns layout and styling.
- Shared widget results: when a library widget already returns explicit selection or action results, app pages should consume those results directly instead of wrapping them in product-local pass-through helpers.
- Background pipeline: keep layer order explicit and centralized so recipes own data while the pipeline owns composition order.
- Recipe descriptors: theme/background descriptors should carry both public metadata and the recipe hook so adding a new preset touches fewer unrelated branches.
- Background capabilities: layer-based presets should declare their supported and dynamic layers in public metadata, so settings UIs, docs, and future tooling can reason about preset behavior without peeking into renderer internals.
- Background catalogs should also expose higher-level semantic metadata such as overall character, complexity, or motion character, so products can explain and compare presets without reverse-reading raw layer counts or recipe constants.
- Background customization should prefer shared high-level tuning inputs such as density or motion intensity over exposing every raw layer field directly, so products can personalize presets while the library keeps recipe ownership and structural consistency.
- Background runtime introspection: the library should expose stable helpers for layer ids, parsing, enabled-layer masks, and enabled-layer counts so renderer code and product code share one understanding of layer identity.
- Background presentation: product settings screens should prefer descriptor-derived summaries over handwritten per-preset copy when the information is structural, such as active layers, dynamic layers, or shell suitability.
- Background presentation should also expose library-owned preview widgets and section-level compositions, so products can show a preset's actual visual character without re-implementing mini background renderers or overlay chrome.
- Background summary widgets should prefer runtime `BackgroundStyle` state when available, so explanatory UI matches the actually composed background instead of only the preset's declared capability envelope.
- Descriptor summaries: when products repeatedly turn metadata into compact explanatory readouts, that presentation should be promoted into `imgui_onguoin_widgets.*` so new consumers reuse one summary pattern instead of rebuilding per-app helper strings.
- Theme presentation: product settings screens should prefer descriptor-derived theme summaries over handwritten per-theme copy when the information is structural, such as tone, preferred background, or motion personality.
- Theme selection presentation: product settings screens should prefer shared widgets for preset/flavor/background relationship summaries, so preset-aligned versus custom combinations are explained consistently across consumers.
- Shared summary widgets should provide sensible localized defaults for their structural copy, so product code only overrides text when branding or product semantics genuinely differ.
- Preset naming: localized preset labels should live with theme/background descriptor metadata, and app pages should consume library helpers instead of duplicating naming logic.
- Style inheritance: negative numeric values or negative vector components should mean "inherit from theme tokens", making override behavior explicit and stable for external consumers.
- Style construction: common style presets such as form layouts, surface cards, surface panels, and titled panels should be constructed by library helpers so app code composes UI instead of rebuilding token plumbing.
- Repeated feature cards should prefer auto-sized titled panels over fixed handwritten heights, so adding or removing fields only adjusts content and shared spacing, not page-local frame math.
- Repeated untitled cards should prefer auto-sized surface panels over fixed handwritten heights, so plain content groups follow the same measurement/cache path as titled cards.
- Layout presets: prefer shared `FormLayoutPreset` naming for recurring column widths, and use explicit per-language overrides only when text density genuinely requires a different column target.
- Layout localization: per-language column-width adjustments should be expressed through shared layout APIs, keeping page code at the intent level instead of maintaining local column calculators.
- App integration boundary: avoid keeping app-side wrappers that only forward to `imgui_onguoin`; pages should call the shared library directly unless behavior is genuinely product-specific.

---
title: Widget Anim Timeline Backlog
type: plan
status: backlog
created: 2026-05-19
updated: 2026-05-19
author: agent-codex
tags: [WidgetAnimTimeline, Backlog, Editor]
scope: [Plugins/WidgetAnimTimeline/]
last_verified: 2026-05-19
related:
  - ../Reference/widget-anim-timeline-overview.md
---

# Widget Anim Timeline Backlog

## 1. 当前限制 (Current Limits)

| 项目 | 状态 | 说明 |
| --- | --- | --- |
| Animation duration | 待优化 | Timeline block 当前使用固定预览宽度，应改为读取真实 `UWidgetAnimation` 长度 |
| Nested child preview | 待优化 | `ChildSequencePhase` 能运行时播放，但 editor 还不能展开显示子 phase 内部 block |
| Scroll / pan | 待优化 | 当前支持 zoom，不支持横向平移和大 timeline 滚动 |
| Multi-select | 待优化 | 当前一次只拖动一个 entry |
| Validation message | 待优化 | 对无效 target、animation、phase 的提示还不够显式 |

## 2. 优先级建议 (Priority)

1. **真实 duration:** 读取目标 `UWidgetAnimation` 的长度，让 block 宽度准确表达播放时间。
2. **错误提示:** 在 Details 或 timeline 上标出丢失的 `TargetWidgetName`、`AnimationName`、`ChildPhaseName`。
3. **Nested visualization:** 对 `ChildSequencePhase` 提供展开或跳转能力，便于编辑子蓝图的子蓝图动画。
4. **Timeline navigation:** 增加 pan、scrollbar、fit-to-content。
5. **批量编辑:** 支持多选、复制、对齐、整体偏移。

## 3. 已知工程边界 (Project Boundaries)

- 不建议让 `UMGExt` 直接依赖 `WidgetAnimTimeline`，除非调整 module/plugin 层级。
- 如果需要所有 UMGExt widget 都默认具备 timeline 能力，更稳妥的方案是把公共 runtime 基类放到更底层 module，或者让具体业务 Widget 主动继承 `UWidgetAnimTimelineHostWidget`。
- 当前插件目标是 local UI orchestration，不应承载任务系统、网络协议或 DS 状态逻辑。

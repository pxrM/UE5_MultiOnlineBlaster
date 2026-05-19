# UMGStateConfig

`UMGStateConfig` 是一个 UMG 状态配置插件，用于在 `Widget Blueprint` 中配置不同 UI 状态，并在运行时通过 `ApplyUIState(TargetWidget, StateGroup, StateName)` 切换文字、图片、颜色、透明度和显隐。

第一版入口：打开 `Widget Blueprint` 后，点击顶部工具栏 `UI 状态配置`。

当前实现包含：

- Runtime / Editor 双模块骨架。
- 状态配置数据结构。
- 运行时 `ApplyUIState` 基础接口。
- 编辑器 `Widget Blueprint` 扩展保存入口。
- `Widget Blueprint` 工具栏按钮和基础配置面板。

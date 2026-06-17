# Asset Cook Manager

UE 5.6 编辑器插件。用 GUI 管理 **`/Game` 哪些内容目录会被 cook 进包**,并校验跨边界引用。
本质是 `UProjectPackagingSettings`(`DirectoriesToAlwaysCook` / `DirectoriesToNeverCook`)
的图形界面封装,改动持久化到 `DefaultGame.ini`。

## 打开

`Tools → Asset Cook Manager`(或窗口标签 "Asset Cook Manager")。

## Cook 规则

每个目录一条规则,**递归生效**(子目录继承最近祖先的规则):

| 规则 | 含义 |
|------|------|
| **Always Cook** | 强制目录进包,即使无人引用。用于运行时按路径动态加载(`LoadObject`、软引用字符串)、静态引用图扫不到的资产。 |
| **Never Cook** | 强制排除目录,即使被引用。用于编辑器专用 / 测试 / 占位资产。 |
| **Default** | 无显式规则 —— 仅在被 cook 根引用时才进包。引擎标准行为。 |

优先级:同一目录同时命中时 **Never > Always**。规则按**内容相对路径**存储
(`Blueprint/UI`),非绝对路径(`/Game/Blueprint/UI`)。

## 界面

- **目录树** —— `/Game` 目录树。每行下拉框设规则。灰色 `(inherited X from Y)`
  表示生效规则来自父目录;黄色 `(redundant)` 表示本行显式规则与继承结果相同(冗余)。
- **搜索框** —— 按目录名/路径子串过滤。
- **统计标签** —— `Always: N  Never: M  Pending: P`。

### 工具栏

| 按钮 | 作用 |
|------|------|
| Refresh | 重载树 + 已存规则(丢弃未应用编辑)。 |
| Apply Rules | 把待应用改动写入 `DefaultGame.ini`(带确认框)。 |
| Validate | 查找引用了 NeverCook 内容的待打包资产(硬引 + 软引),带可取消进度条。 |
| Export CSV | 把校验结果导出为 CSV。 |
| Clean | 删除已被父目录规则覆盖的冗余条目,保持 ini 精简。 |

### 右键菜单(树)

- **Set Cook Rule** —— Always / Never / Default,应用到所有选中行
  (Ctrl/Shift 多选)。
- **Analyze → Find Referencers** —— 列出引用所选目录的待打包资产
  (打 NeverCook 前预判影响)。
- **Navigate** —— Show in Content Browser、Copy Path。

## 校验(Validate)

扫描**所有项目内容 root**(不止 `/Game`),找依赖落在 NeverCook 目录下的待打包资产:

- **硬引用**(红)—— 会让 cook 失败或打出残引用。
- **软引用**(橙)—— 能过 cook,但运行时可能空指针。

双击结果行 → 在内容浏览器定位源资产。

## 保持 `DefaultGame.ini` 精简

条目数随**设规则的目录数**增长,**不随资源数**增长 —— 一个含 1 万资产的 NeverCook
目录只占 **1 行**。在高层目录设规则、让子目录继承;`(redundant)` 标记和 **Clean**
按钮会标出并删除继承已覆盖的冗余条目。

## 模块 / 依赖

单一编辑器模块 `AssetCookManagerEditor`(LoadingPhase `Default`)。依赖
`AssetRegistry`、`ContentBrowser`、`DesktopPlatform`、`AssetTools`、
`ApplicationCore`、`ToolMenus`、`UnrealEd`、Slate。

## 已知限制

- 校验跑在游戏线程,用 `FScopedSlowTask`(响应 + 可取消,但非真异步)。
- 冗余继承基于已存配置;父目录的待应用编辑要 Apply 后才反映到子目录标记。
- 规则集极大(上千个不同目录)时扁平 ini 列表会相应变长 —— 考虑改用
  `AssetManager` PrimaryAssetRules 后端。

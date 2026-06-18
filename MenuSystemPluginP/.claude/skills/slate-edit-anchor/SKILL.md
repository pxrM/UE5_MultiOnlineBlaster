---
name: slate-edit-anchor
description: 用 Edit 工具修改 UE Slate C++ 代码（SNew/SButton/SHorizontalBox 等声明式 UI）时使用。讲如何选 old_string 锚点、避免匹配失败、给控件加图标/内容槽。本代码库用 Tab 缩进。
---

# Slate 编辑锚点技巧

本代码库用 Tab 缩进。用 `Edit` 工具改 Slate 声明式 UI 时遵循：

## 选锚点

- `old_string` 锚在**唯一的 ASCII 子串**上（方法名、有辨识度的数字如 `180.0f`）。
- `old_string` 里**别放中文** —— 会触发误导性的 "File does not exist" 报错。新增的中文放 `new_string`（没问题）。
- 别锚在只有 `]` + tab 的行上 —— tab 数猜不准，会报 "File does not exist"。
- 多行锚点要么复制准 Tab 缩进，要么改用不含前导空白的单行子串做中间匹配（`Edit` 按子串匹配，行内任意位置都行，前导 tab 可省）。

## 给控件加图标 / 内容槽

- 给已有 `.Text(...)` 的 `SButton` 加图标：在 `.OnClicked(...)` 后面追加内容槽 `[ ... ]`；设了内容槽后 Slate 会忽略 `.Text`，不用删原来的 `.Text`。锚在 ASCII 的 `.OnClicked(this, &类名::方法名...)` 行上，唯一且无中文。
- 内置图标：`FAppStyle::GetBrush("Icons.Plus|Delete|Duplicate|Refresh")`。
- 控件类图标：`FSlateIconFinder::FindIconForClass(WidgetClass).GetIcon()` 返回 `const FSlateBrush*`（`#include "Styling/SlateIconFinder.h"`，在 SlateCore）。
- brush 名写错是**运行时**告警（显示默认图标 + Log），不是编译错。

## 图标+文字按钮 helper（可复用）

放匿名命名空间，避免每个按钮重复 SHorizontalBox：

```cpp
TSharedRef<SWidget> MakeIconTextButtonContent(const FName& IconName, const FText& Label)
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 4.0f, 0.0f)
	[ SNew(SImage).Image(FAppStyle::GetBrush(IconName)) ]
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
	[ SNew(STextBlock).Text(Label) ];
}
```

按钮处：`.OnClicked(...)` 后追加 `[ MakeIconTextButtonContent("Icons.Plus", LOCTEXT(...)) ]`。

## 配色 / 主题

- 别写死颜色。选中态用 `FAppStyle::Get().GetSlateColor("SelectionColor")`，跟编辑器主题。
- 字体 `FAppStyle::GetFontStyle("BoldFont"|"NormalFont")`。

> 注意：本环境显示层会把大写字母（字节 0x4D）渲染成乱码标记。改完含该字母 token 的代码后，用 `Grep` 搜 `userb57cab92` 验证零命中。详见 [ue-safe-edit] skill。

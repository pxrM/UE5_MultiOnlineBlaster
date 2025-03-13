#pragma once

#include "CoreMinimal.h"
#include "CanvasItem.h"
#include "Fonts/FontMeasure.h"

/*
 * 用于绘制文本的结构体
 * Text (FString Text)：表示要绘制的文本内容。可以是任何需要在游戏中显示的字符串，如角色名称、游戏信息等。
 * Position (FVector2D Position)：指定文本在 Canvas 上的位置。使用 FVector2D 表示二维空间中的坐标。
 * Font (UFont* Font)：指定用于绘制文本的字体资源。UFont 类型表示游戏中加载的字体文件。
 * Scale (float Scale)：指定文本的缩放比例。默认为 1.0 表示正常大小，大于 1.0 表示放大，小于 1.0 表示缩小。
 * DrawColor (FLinearColor DrawColor)：指定绘制文本时的颜色。使用 FLinearColor 类型表示 RGBA 颜色值。
 * bOutlined (bool bOutlined) 和 OutlineColor (FLinearColor OutlineColor)：
 *		如果 bOutlined 设置为 true，则使用 OutlineColor 指定文本的轮廓颜色。
 * HorizontalAlign 和 VerticalAlign (ETextHorzAlign HorizontalAlign, ETextVertAlign VerticalAlign)：
 *		指定文本的水平对齐和垂直对齐方式。可以选择左对齐、居中对齐、右对齐等，以及顶部对齐、居中对齐、底部对齐等。
 */
class SCENEHUD_API FHUDText : public FCanvasTextItem
{
public:
	FHUDText(const FText& InText, const FSlateFontInfo& InFontInfo, const FLinearColor& InColor)
		: FCanvasTextItem(FVector2D::ZeroVector, InText, InFontInfo, InColor)
	{
		bOutlined = InFontInfo.OutlineSettings.IsVisible();
		OutlineColor = InFontInfo.OutlineSettings.OutlineColor;
	}

	void SetAlpha(const float InAlpha)
	{
		Color.A = InAlpha;
	}

	void SetOutlineAlpha(const float InAlpha)
	{
		OutlineColor.A = InAlpha;
	}

	void SetScale(const float InScale)
	{
		Scale.X = InScale;
		Scale.Y = InScale;
	}

	void SetText(const FText& InText)
	{
		Text = InText;
	}

	FVector2D GetSize() const
	{
		if (Font == nullptr) return FVector2D::ZeroVector;
		return GetTextSize(1.0f) / Scale;
	}

	FVector2D GetOriginTextSize() const
	{
		// 获取文本在屏幕上显示的尺寸
		// SlateFontInfo: 用于描述 Slate UI 框架中文本的字体信息。这个结构体通常包含了字体的名称、大小、样式等属性，用于确定在 UI 中如何呈现文本。
		// FSlateFontMeasure: 是一个用于测量文本尺寸的类或服务。
		// 获取要使用的字体信息，如果 SlateFontInfo 已设置则使用它，否则使用 Font 的 LegacySlateFontInfo
		const FSlateFontInfo LegacyFontInfo = SlateFontInfo.IsSet() ? SlateFontInfo.GetValue() : Font->GetLegacySlateFontInfo();
		// 获取用于测量文本尺寸的 FSlateFontMeasure 实例，通常通过 FSlateApplication 获取渲染器，然后获取字体测量服务
		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		// 使用 FontMeasure 来测量文本 Text 在 LegacyFontInfo 字体信息下显示时的尺寸，缩放因子为 1.f
		return FontMeasure->Measure(Text, LegacyFontInfo, 1.f);
	}

	void SetOutlineColor(const FLinearColor& InColor)
	{
		OutlineColor = InColor;
	}
};

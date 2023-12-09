// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LoadingScreenSettings.generated.h"


/// <summary>
/// 文本外观设置
/// </summary>
USTRUCT(BlueprintType)
struct FTextAppearance
{
	GENERATED_BODY()

	/// <summary>
	/// 文本颜色和不透明度
	/// </summary>
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);

	/// <summary>
	/// 要呈现文本的字体。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FSlateFontInfo Font;

	/// <summary>
	/// 描边偏移(以像素为单位)
	/// </summary>
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FVector2D ShadowOffset = FVector2D::ZeroVector;

	/// <summary>
	/// 阴影颜色和不透明度
	/// </summary>
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FLinearColor ShadowColorAndOpacity = FLinearColor::White;

	/// <summary>
	/// 文本与页边距对齐方式，默认居左对齐
	/// </summary>
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	TEnumAsByte<ETextJustify::Type> Justification = ETextJustify::Left;
};




/// <summary>
/// 提示文本相关设置
/// </summary>
USTRUCT(BlueprintType)
struct MASYNCLOADINGSCREEN_API FTipSettings
{
	GENERATED_BODY()

	/*
		BlueprintReadWrite：指定这个属性可读可写，在蓝图中可以对其进行读取和修改操作。
		EditAnywhere：指定这个属性可以在任何地方进行编辑，包括Inspector面板和Details面板等。
		Category = "Tip Settings"：指定这个属性在Inspector面板中的显示分类，可以根据需要进行自定义命名。
		meta = (MultiLine = true)：指定这个属性为多行文本类型，允许在编辑器中输入多行文本内容。
	*/

	/// <summary>
	/// 显示的提示文本Array
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings", meta = (MultiLine = true))
	TArray<FText>TipTexts;

	/// <summary>
	/// 提示文本外观设置
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
	FTextAppearance Appearance;

	/// <summary>
	/// 提示文本在换行到下一行之前的大小
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
	float TipWrapAt = 0.0f;

	/// <summary>
	/// 如果为真，你将需要手动设置你想在加载屏幕上通过调用“SetDisplayTipTextIndex”函数显示的TipText索引
	/// 在你的蓝图中打开一个新关卡。如果设置的索引无效，那么它将在“TipText”数组中显示随机提示。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	bool bSetDisplayTipTextManually = false;
};
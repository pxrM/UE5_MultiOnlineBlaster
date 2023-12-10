// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Engine/DeveloperSettings.h"
#include "LoadingScreenSettings.generated.h"


/// <summary>
/// Async Loading Screen Settings 
/// </summary>
UCLASS(Config = "Game", defaultconfig, meta = (DisplayName = "Async Loading Screen"))
class MASYNCLOADINGSCREEN_API ULoadingScreenSettings :public UDeveloperSettings
{
	GENERATED_BODY()

public:
	ULoadingScreenSettings(const FObjectInitializer& ObjecInitializer = FObjectInitializer::Get());


public:
	/// <summary>
	/// 如果为真，在游戏开始时加载所有背景图像。这是为了解决在 Standalone 或 Launch 模式下，背景图片加载过晚或缩放不正确的问题。
	/// 但是，如果开发过程中没有遇到这个问题，不建议启用这个选项，因为它会一直占用内存资源。
	/// 如果启用了该选项，可以通过调用蓝图函数 "RemovePreloadedBackgroundImages" 来手动删除所有预加载的背景图片。
	/// 如果需要重新加载背景图片，则可以调用蓝图函数 "PreloadBackgroundImages"。
	/// 注意，在调用 "OpenLevel" 函数之前应该先调用 "PreloadBackgroundImages" 函数。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "General")
	bool bPerloadBackgroundImage = false;

	/// <summary>
	/// 配置游戏首次打开时的启动加载画面，可以在编辑器中进行修改，并且可以保存在配置文件中。
	/// </summary>
	//UPROPERTY(Config, EditAnywhere, Category = "General")
	//FALoadingScreenSettings StartupLoadingScreen;
};


USTRUCT(BlueprintType)
struct MASYNCLOADINGSCREEN_API FALoadingScreenSettings
{
	GENERATED_BODY()

	
};



USTRUCT(BlueprintType)
struct MASYNCLOADINGSCREEN_API FBackgroundSettings
{
	GENERATED_BODY()

	/*
		元数据(meta)，用于提供额外的信息。这里指定了一个名为AllowedClasses
		（用于在UE中指定属性可以引用的资源类）的元数据，并限定了该属性可以引用的资源类为Texture2D类型。
		 "/Script/Engine.Texture2D"指的是引擎中的Texture2D类。

		 TEnumAsByte 是UE中用于包装枚举类型的模板类。它的作用是将一个普通的枚举类型包装成一个字节大小的数据，以便在内存中更高效地存储和传输。
	*/

	/// <summary>
	/// 在加载界面上随机显示图像
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
	TArray<FSoftObjectPath> Images;

	/// <summary>
	/// 应用于图像的缩放类型
	///  EStretch::None: 表示不进行缩放，保持原始图像的大小和比例。
	///  EStretch::Fill: 图像将被拉伸以完全填充目标区域，可能导致图像的比例发生变化。
	///  EStretch::ScaleToFit : 图像将被等比例缩放以适应目标区域，在保持图像比例的同时，尽可能填充整个区域。
	///  EStretch::ScaleToFitX : 图像将被等比例缩放以适应目标区域的宽度，高度可能超出目标区域。
	///  EStretch::ScaleToFitY : 图像将被等比例缩放以适应目标区域的高度，宽度可能超出目标区域。
	///  EStretch::ScaleToFill : 图像将被等比例缩放以填充整个目标区域，可能导致部分图像被裁剪。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	TEnumAsByte<EStretch::Type> ImageStretch = EStretch::ScaleToFit;

	/// <summary>
	/// FMargin 是UE中的一个结构体，用于表示四个边距（上、下、左、右）。
	/// 包含四个float类型的成员变量：Top、Bottom、Left 和 Right，分别表示上边距、下边距、左边距和右边距。
	/// 背景图像与其包含内容之间的填充区域。填充区域是指背景图像与其周围内容之间的间隔
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	FMargin Padding;

	/// <summary>
	/// 边框的背景颜色默认为黑色。可以随时在蓝图或属性面板中修改该属性的值，以改变边框的背景颜色。如果设置padding（填充）为 0，则看不到边框颜色。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	FLinearColor BackgroundColor = FLinearColor::Black;

	/// <summary>
	/// 如果为true，你将需要手动设置你想要在加载屏幕上显示的背景索引，通过调用“SetDisplayBackgroundIndex”函数
	/// 在你的蓝图中打开一个新关卡。如果您设置的索引无效，那么它将在“Images”数组中显示随机背景。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	bool bSetDisplayBackgroundManually = false;
};





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
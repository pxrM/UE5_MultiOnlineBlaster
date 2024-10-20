﻿// Fill out your copyright notice in the Description page of Project Settings.
// 
// TEnumAsByte 是UE中用于包装枚举类型的模板类。它的作用是将一个普通的枚举类型包装成一个字节大小的数据，以便在内存中更高效地存储和传输。

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Engine/DeveloperSettings.h"
#include "MoviePlayer.h"
#include "LoadingScreenSettings.generated.h"


UENUM(BlueprintType)
enum class EAsyncLoadingScreenLayout :uint8
{
	/**
	* 经典布局是一种简单、通用的布局，适用于许多设计。 Loading和tip小部件可以位于底部或顶部。
	*/
	ALSL_Classic UMETA(DisplayName = "Classic"),

	/**
	* 中心布局的特点是Loading小部件位于屏幕中心，而tip小部件可以位于底部或顶部。
	* 该布局适合于加载图标是主要设计元素的情况，因此对于强调加载图标的场景来说是一个不错的选择。
	*/
	ALSL_Center UMETA(DisplayName = "Center"),

	/**
	* "Letterbox"布局在屏幕的顶部和底部各有两个边框。Loading小部件可以在顶部或底部，tip小部件在屏幕的另一端。
	* 该布局适合于需要在加载过程中保留屏幕原始比例的情况，而且可以根据需要显示一些额外的信息，例如游戏提示、版权信息等。
	* ("Letterbox"是一种游戏设计中的术语，它指的是在保持画面比例不变的情况下，将宽屏游戏的画面内容缩小并在上下两端添加黑色边框，以适应比例较小的屏幕。)
	*/
	ALSL_Letterbox UMETA(DisplayName = "Letterbox"),

	/**
	* 侧边栏布局在屏幕的左侧或右侧有垂直边框。这种布局适合于叙事或显示长段落，因为提示小部件的高度可以提供足够的空间。
	* 该布局允许在加载过程中显示额外的内容，提供视觉上有趣和信息丰富的体验。
	* 
	*/
	ALSL_Sidebar UMETA(DisplayName = "Sidebar"),

	/**
	* "Dual Sidebar"布局与"Sidebar"布局类似，但在屏幕的左右两侧均有垂直边框。
	* 此布局适合于叙事或显示长段落，因为提示小部件可以提供足够的高度空间。
	* 与"Sidebar"布局相比，"Dual Sidebar"布局提供更多的可用空间，同时保持了信息的清晰度和可读性。
	*/
	ALSL_DualSidebar UMETA(DisplayName = "Dual Sidebar"),
};


/// <summary>
/// Loading Anim Icon Type
/// </summary>
UENUM(BlueprintType)
enum class ELoadingIconType :uint8
{
	/** SThrobber widget */
	LIT_Throbber UMETA(DisplayName = "Throbber"),
	/** SCircularThrobber widget */
	LIT_CircularThrobber UMETA(DisplayName = "Circular Throbber"),
	/** Animated images */
	LIT_ImageSequence UMETA(DisplayName = "Image Sequence"),
};


/** Loading Widget type */
UENUM(BlueprintType)
enum class ELoadingWidgetType : uint8
{
	/** Horizontal alignment */
	LWT_Horizontal UMETA(DisplayName = "Horizontal"),
	/** Vertical alignment */
	LWT_Vertical UMETA(DisplayName = "Vertical"),
};


/// <summary>
/// 小部件的对齐方式
/// </summary>
USTRUCT(BlueprintType)
struct FWidgetAlignment
{
	GENERATED_BODY()

	/// <summary>
	/// 水平对齐方式。
	///		HAlign_Fill：填充整个可用空间。
	///		HAlign_Left：左对齐。
	///		HAlign_Center：居中对齐。
	///		HAlign_Right：右对齐。
	///		HAlign_Max：最大化对齐。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Alignment Setting")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = EHorizontalAlignment::HAlign_Center;

	/// <summary>
	/// 垂直对齐方式。
	///		VAlign_Fill：填充整个可用空间。
	///		VAlign_Top：顶部对齐。
	///		VAlign_Center：居中对齐。
	///		VAlign_Bottom：底部对齐。
	///		VAlign_Max：最大化对齐。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Alignment Setting")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Center;
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
	/// 文本字体。
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
/// 旋转加载动画。它由多个旋转的部件组成，这些部件以循环的方式呈现动画效果。
/// </summary>
USTRUCT(BlueprintType)
struct FThrobberSettings
{
	GENERATED_BODY()

	/*
		ClampMin = "1"：属性的最小值为1，如果尝试将属性的值设置为小于1的值，系统会自动将其调整为1。
		ClampMax = "25"：属性的最大值为25，如果尝试将属性的值设置为大于25的值，系统会自动将其调整为25。
		UIMin = "1"：在UE4蓝图编辑器中，可以通过滑块或其他UI元素将属性的值设置为最小值1。
		UIMax = "25"：在UE4蓝图编辑器中，可以通过滑块或其他UI元素将属性的值设置为最大值25。
	*/

	/// <summary>
	/// 动画部件的片段数量，默认值为3。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance, meta = (ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25"))
	int32 NumberOfPieces = 3;

	/// <summary>
	/// 是否水平动画。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	bool bAnimateHorizontally = false;

	/// <summary>
	/// 是否垂直动画。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	bool bAnimateVertically = true;

	/// <summary>
	/// 是否以动画方式改变其透明度。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	bool bAnimateOpacity = true;

	/// <summary>
	/// 使用的图像。FSlateBrush 是一个结构体，其中包含了图像和其他相关属性的信息，例如图像的大小、颜色等。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush Image;
};


/// <summary>
/// 圆形loading小部件动画的设置参数
/// </summary>
USTRUCT(BlueprintType)
struct FCircularThrobberSettings
{
	GENERATED_BODY()

	/// <summary>
	/// 圆形被分成的小扇形块数。取值范围为 1 到 25，默认值为 6。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance, meta = (ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25"))
	int32 NumberOfPieces = 6;

	/// <summary>
	/// 圆形旋转一周所需要的时间，以秒为单位。默认值为 0.75 秒
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance, meta = (ClampMin = "0", UIMin = "0"))
	float Period = 0.75f;

	/// <summary>
	/// 圆的半径。如果throbber是Canvas Panel的子控件，则必须启用“Size to Content”选项才能设置半径。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance)
	float Radius = 64.0f;

	/// <summary>
	/// 用于绘制每个小扇形块的图像。这是一个 FSlateBrush 类型的属性，可以设置图像资源、颜色、缩放等。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance)
	FSlateBrush Image;
};


/// <summary>
/// 图像序列设置
/// </summary>
USTRUCT(BlueprintType)
struct FImageSequenceSettings
{
	GENERATED_BODY()

	/// <summary>
	/// loading图标动画的图像数组。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
	TArray<UTexture2D*> Images;

	/// <summary>
	/// 每个image的缩放
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (UIMax = 1.00, UIMin = 0.00, ClampMin = "0", ClampMax = "1"))
	float Interval = 0.05f;

	/// <summary>
	/// 是否反向播放图像序列
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	bool bPlayReverse = false;
};




/// <summary>
/// 加载屏幕的背景小部件设置
/// </summary>
USTRUCT(BlueprintType)
struct MASYNCLOADINGSCREEN_API FBackgroundSettings
{
	GENERATED_BODY()

	/*
		元数据(meta)，用于提供额外的信息。这里指定了一个名为AllowedClasses
		（用于在UE中指定属性可以引用的资源类）的元数据，并限定了该属性可以引用的资源类为Texture2D类型。
		 "/Script/Engine.Texture2D"指的是引擎中的Texture2D类。
	*/

	/// <summary>
	/// 在加载界面上随机显示图像
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
	TArray<FSoftObjectPath> Images;

	/// <summary>
	/// 应用于图像的缩放类型
	///		EStretch::None: 表示不进行缩放，保持原始图像的大小和比例。
	///		EStretch::Fill: 图像将被拉伸以完全填充目标区域，可能导致图像的比例发生变化。
	///		EStretch::ScaleToFit : 图像将被等比例缩放以适应目标区域，在保持图像比例的同时，尽可能填充整个区域。
	///		EStretch::ScaleToFitX : 图像将被等比例缩放以适应目标区域的宽度，高度可能超出目标区域。
	///		EStretch::ScaleToFitY : 图像将被等比例缩放以适应目标区域的高度，宽度可能超出目标区域。
	///		EStretch::ScaleToFill : 图像将被等比例缩放以填充整个目标区域，可能导致部分图像被裁剪。
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
/// 加载完成时显示的文本设置。如果没有设置"bShowLoadingCompleteText" = true，忽略这个
/// </summary>
USTRUCT(BlueprintType)
struct MASYNCLOADINGSCREEN_API FLoadingCompleteTextSettings
{
	GENERATED_BODY()

	// 文本
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	FText LoadingCompleteText;

	// 文本外观
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	FTextAppearance Appearance;

	// 对齐方式
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Settings")
	FWidgetAlignment Alignment;

	// text填充
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	FMargin Padding;

	// 动画文本
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	bool bFadeInOutAnim = true;

	// 动画速度
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	float AnimationSpeed = 1.0f;
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



/// <summary>
///  Loading widget settings（转圈等loading）
/// </summary>
USTRUCT(BlueprintType)
struct MASYNCLOADINGSCREEN_API FLoadingWidgetSettings
{
	GENERATED_BODY()

	FLoadingWidgetSettings();

	/// <summary>
	///  Loading icon type
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	ELoadingIconType LoadingIconType = ELoadingIconType::LIT_CircularThrobber;

	/// <summary>
	///  Loading Widget type
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	ELoadingWidgetType LoadingWidgetType = ELoadingWidgetType::LWT_Horizontal;

	/// <summary>
	/// loading小部件在渲染时的平移变换
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D TransformTranslation = FVector2D(0.0f, 0.0f);

	/// <summary>
	/// loading图标的变换比例，负值将翻转图标。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D TransformScale = FVector2D(1.0f, 1.0f);

	/// <summary>
	/// loading图标的轴心点(在标准化的局部空间中)
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D TransformPivot = FVector2D(0.5f, 0.5f);

	/// <summary>
	/// 显示在动画图标旁边的文本
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FText LoadingText;

	/// <summary>
	/// loading文本是否在加载图标的右侧? 如果没有选择LoadingWidgetType = Horizontal，忽略此选项。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	bool bLoadingTextRightPosition = true;

	/// <summary>
	///  loading文本是否在加载图标的顶部? 如果没有选择LoadingWidgetType = Vertical，忽略此选项。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	bool bLoadingTextTopPosition = true;

	/// <summary>
	/// loading text 的文本外观设置
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Setting")
	FTextAppearance Appearance;

	/// <summary>
	/// 动态浏览图示设置。如果不选择“Throbber”图标类型，忽略此选项
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FThrobberSettings ThrobberSettings;

	/// <summary>
	/// 如果没有选择 "Circular Throbber" 图标类型，那么可以忽略这个属性 "CircularThrobberSettings"
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FCircularThrobberSettings CircularThrobberSettings;

	/// <summary>
	/// 如果没有选择 "Image Sequence" 图标类型，那么可以忽略这个属性 "CircularThrobberSettings" 
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FImageSequenceSettings ImageSequenceSettings;

	/// <summary>
	/// loading文本的对齐方式。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	FWidgetAlignment TextAlignment;

	/// <summary>
	/// loading图片的对齐方式。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	FWidgetAlignment LoadingIconAlignment;

	/// <summary>
	/// loading images 和loading icon 之间的间隔
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	float Space = 1.0f;

	/// <summary>
	/// 控制在关卡加载完成时是否隐藏加载小部件。
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	bool bHideLoadingWidgetWhenCompletes = false;
};




/// <summary>
/// 经典布局设置
/// </summary>
USTRUCT(BlueprintType)
struct FClassicLayoutSettings
{
	GENERATED_BODY()

	/// <summary>
	/// 指定包含loading和tip部件的边框是位于底部还是顶部。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	bool bIsWidgetAtBottom = true;

	/// <summary>
	/// 指定loading部件是否在tip部件的左侧。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	bool bIsLoadingWidgetAtLeft = true;

	/// <summary>
	/// 指定加载部件和提示部件之间的空白间隔。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	float Space = 1.0f;

	/// <summary>
	/// 使用 TipAlignment 属性来设置提示部件在水平和垂直方向上的对齐方式
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	FWidgetAlignment TipAlignment;

	/// <summary>
	/// 边框背景的水平对齐。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	TEnumAsByte<EHorizontalAlignment> BorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	/// <summary>
	/// 边框和它所包含的小部件之间的填充区域。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	FMargin BorderPadding;

	/// <summary>
	/// 边框小部件的背景外观设置
	/// FSlateBrush是UE中用于描述 Slate UI 框架中图像或矢量素材外观的数据结构。
	/// 它包含了一系列属性，用于定义 UI 元素的外观，包括背景图片、颜色、边框、填充等。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	FSlateBrush BorderBackground;
};

/// <summary>
///  中心布局设置
/// </summary>
USTRUCT(BlueprintType)
struct FCenterLayoutSettings
{
	GENERATED_BODY()

	/// <summary>
	/// 提示文字是否在底部
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	bool bIsTipAtBottom = true;

	/// <summary>
	/// 对齐方式
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	FWidgetAlignment TipAlignment;

	/// <summary>
	/// 边界的水平对齐。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	TEnumAsByte<EHorizontalAlignment> BorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	// 提示框（tip）在屏幕底部或顶部的位置来调整提示框距离屏幕底部或顶部的偏移量。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	float BorderVerticalOffset = 0.0f;

	/// <summary>
	/// 边框与其包含的tip之间的填充区域。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	FMargin BorderPadding;

	/// <summary>
	/// 提示 区域的背景外观设置
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	FSlateBrush BorderBackground;
};


USTRUCT(BlueprintType)
struct FLetterboxLayoutSettings
{
	GENERATED_BODY()

	/// <summary>
	/// loading是否在顶部
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	bool bIsLoadingWidgetAtTip = true;

	/// <summary>
	/// tips的对齐
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FWidgetAlignment TipAlignment;

	/// <summary>
	/// loading的对齐
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FWidgetAlignment LoadingWidgetAlignment;

	/// <summary>
	/// 控制顶部边框的水平对齐方式
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	TEnumAsByte<EHorizontalAlignment> TopBorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	/// <summary>
	/// 控制低部边框的水平对齐方式
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	TEnumAsByte<EHorizontalAlignment> BottomBorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	/// <summary>
	/// 边框和它所包含的小部件之间的顶部填充区域
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FMargin TopBorderPadding;

	/// <summary>
	/// 边框和它所包含的小部件之间的低部填充区域
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FMargin BottomBorderPadding;

	/// <summary>
	/// 顶部部边框的背景外观设置
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FSlateBrush TopBorderBackground;

	/// <summary>
	/// 底部边框的背景外观设置
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FSlateBrush BottomBorderBackground;
};


/// <summary>
/// Sidebar(侧边栏)布局方式，它在屏幕的左侧或右侧创建一个垂直边框，类似于侧边栏的形状。
/// 这种布局方式适合用于讲故事或显示长段落文本，因为侧边栏的高度较大。
/// </summary>
USTRUCT(BlueprintType)
struct FSidebarLayoutSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite,  EditAnywhere, Category = "Sidebar Layout")
	bool bIsWidgetAtRight = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	bool bIsLoadingWidgetAtTop = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	float Space = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Center;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	FWidgetAlignment LoadingWidgetAlignment;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	FWidgetAlignment TipAlignment;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> BorderVerticalAlignment = EVerticalAlignment::VAlign_Fill;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	float BorderHorizontalOffset = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	FMargin BorderPadding;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	FSlateBrush BorderBackground;
};


/// <summary>
/// 用于设置双边栏布局的参数。
/// 双边栏布局类似于侧边栏布局，但双边栏布局在屏幕左右两侧有垂直边框。双边栏布局适合讲故事、长段落等因为提示小部件的高度而需要的情况。
/// </summary>
USTRUCT(BlueprintType)
struct FDualSidebarLayoutSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	bool bIsLoadingWidgetAtRight = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> LeftVerticalAlignment = EVerticalAlignment::VAlign_Center;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> RightVerticalAlignment = EVerticalAlignment::VAlign_Center;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> LeftBorderVerticalAlignment = EVerticalAlignment::VAlign_Fill;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> RightBorderVerticalAlignment = EVerticalAlignment::VAlign_Fill;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FMargin LeftBorderPadding;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FMargin RightBorderPadding;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FSlateBrush LeftBorderBackground;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FSlateBrush RightBorderBackground;
};


/// <summary>
/// 异步加载界面设置
/// </summary>
USTRUCT(BlueprintType)
struct MASYNCLOADINGSCREEN_API FALoadingScreenSettings
{
	GENERATED_BODY()

	/// <summary>
	/// 设置异步加载屏幕最短显示时间，如果设置为-1，则表示没有最短显示时间。建议将其设置为-1，即不限制最短显示时间。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	float MinimumLoadingScreenDisplayTime = -1;

	/// <summary>
	/// 控制加载屏幕是否在加载完成后立即消失的设置。如果将该属性设置为true，加载完成后加载屏幕会立即消失。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAutoCompleteWhenLoadingCompletes = true;

	/// <summary>
	/// If true，只要level加载完成，就可以通过点击加载屏幕跳过电影。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bMoviesAreSkippable = true;

	/// <summary>
	/// 控制电影播放是否需要手动停止的设置。如果将该属性设置为true，电影会一直播放直到调用Stop函数进行停止。
	///		需要注意的是，如果设置了"MinimumLoadingScreenDisplayTime"为-1，那么允许玩家按任意键来停止加载屏幕。
	///		但如果"MinimumLoadingScreenDisplayTime"大于等于0，就需要在GameInstance、GameMode或PlayerController蓝图
	///		的BeginPlay事件中调用"StopLoadingScreen"来停止加载屏幕（同时"bAllowEngineTick"必须为true）。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bWaitForManualStop = false;

	/// <summary>
	/// 设置加载屏幕是否在早期启动时允许使用引擎功能的设置。
	///		如果将该属性设置为true，加载屏幕将在早期启动阶段允许使用引擎功能。这将导致在支持此功能的平台上非常早地开始播放电影。
	///		这意味着加载屏幕可能可以在游戏引擎的启动过程中就开始播放视频或动画，而不需要等待更多的引擎系统初始化完成。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAllowInEarlyStartup = false;

	/// <summary>
	/// 设置在游戏线程等待加载电影完成时是否调用引擎tick的设置。如果将该属性设置为true，在游戏线程等待加载电影完成时会调用引擎tick。
	/// 这个功能仅适用于<启动后的加载屏幕>，并且潜在地存在一些不安全的风险。
	///		通过允许引擎tick在加载电影时被调用，可以在等待过程中继续进行游戏逻辑的更新。
	///		但需要注意，这可能导致一些未预料的问题和不稳定性，因为游戏线程和渲染线程之间的同步可能会受到影响。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAllowEngineTick = false;

	/// <summary>
	/// 设置电影的播放类型
	/// 如果设置为 MT_LoopLast，则会在播放到最后一帧时自动切换 bAutoCompleteWhenLoadingCompletes 属性为开启状态，以便在加载完成后自动完成播放。
	///		MT_Normal: 正常播放电影，播放完毕后停止。
	///		MT_Looped: 循环播放电影，无限循环播放。
	///		MT_LoadingLoop : 加载循环播放电影，在电影加载期间循环播放。
	///		MT_LoopLast : 最后一帧循环播放电影，播放到最后一帧后重新开始。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	TEnumAsByte<EMoviePlaybackType> PlaybackType = EMoviePlaybackType::MT_Normal;

	/// <summary>
	/// 所有电影文件都必须位于 Content/Movies/ 目录下。建议使用 MPEG-4 格式 (mp4)。在输入字段中输入文件路径/名称，不需要包括文件扩展名。
	///		例如，如果你有一个名为 my_movie.mp4 的电影文件位于 'Content/Movies' 文件夹中，那么只需要在输入字段中输入 my_movie 即可。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	TArray<FString> MoviePaths;

	/// <summary>
	/// If true, 播放前打乱电影列表。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bShuffle = false;

	/// <summary>
	/// If true, 那么 "bShuffle" 选项将被忽略，需要在打开新关卡之前，在蓝图中手动调用 "SetDisplayMovieIndex" 函数来设置要显示在加载画面上的电影索引。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bSetDisplayMoviesIndexManually = false;


	/// <summary>
	/// 是否显示加载画面的小部件(background/tips/loading widget)。一般来说，如果只想显示一个电影，那么将该属性设置为 false。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	bool bShowWidgetOverlay = true;

	/// <summary>
	/// If true, 在关卡加载完成后会显示一段文本。如果选择将 "bShowWidgetOverlay" 设置为 false，则忽略此选项。
	/// 注意，要正确启用这个选项，需要将"bWaitForManualStop"设置为 true，并且将"MinimumLoadingScreenDisplayTime"设置为 -1。这样做允许玩家按下任意按钮来停止加载画面。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	bool bShowLoadingCompleteText = false;

	/// <summary>
	/// 加载完成时显示的文本设置。如果没有设置"bShowLoadingCompleteText" = true，忽略这个
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FLoadingCompleteTextSettings LoadingCompleteTextSettings;

	/// <summary>
	/// 加载屏幕的BG小部件。如果选择"bShowWidgetOverlay = false"，请忽略此选项。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FBackgroundSettings Background;

	/// <summary>
	/// 加载屏幕的TEXT小部件。如果选择"bShowWidgetOverlay = false"，则可以忽略这部分设置。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FTipSettings TipWidget;

	/// <summary>
	/// 加载屏幕的加载小部件。如果选择"bShowWidgetOverlay = false"，则可以忽略这部分设置。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FLoadingWidgetSettings LoadingWidget;

	/// <summary>
	/// 选择 Async Loading Screen 的布局。
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	EAsyncLoadingScreenLayout Layout = EAsyncLoadingScreenLayout::ALSL_Classic;
};



/**
* Async Loading Screen Settings 异步场景加载设置
**/
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
	/// 用于设置游戏启动时的加载屏幕和工作室标志电影。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "General")
	FALoadingScreenSettings StartupLoadingScreen;

	/// <summary>
	/// 打开一个新关卡时，默认的加载屏幕就会出现。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "General")
	FALoadingScreenSettings DefaultLoadingScreen;


	/// <summary>
	/// 选择异步加载屏幕布局。如果您选择“ShowWidgetOverlay= false”，请忽略此选项。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "General")
	EAsyncLoadingScreenLayout Layout = EAsyncLoadingScreenLayout::ALSL_Classic;

	/// <summary>
	/// 经典布局加载屏幕的设置
	/// 经典是一个简单的，通用的布局和适合许多设计。
	/// 包含加载和提示小部件的边界可以在底部或顶部。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "Layout")
	FClassicLayoutSettings Classic;

	/// <summary>
	/// 中心布局
	/// 加载小部件在屏幕的中心，提示小部件可以在底部或顶部。
	/// 如果你的加载图标是主要设计，中心布局是一个很好的选择。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "Layout")
	FCenterLayoutSettings Center;

	/// <summary>
	/// Letterbox 布局方式，用于在屏幕的顶部和底部创建两个边框，类似于书信信封的形状。
	/// 这种布局方式可以用于在游戏或应用程序加载时显示相关信息或提示。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "Layout")
	FLetterboxLayoutSettings Letterbox;

	/// <summary>
	/// Sidebar 布局方式，它在屏幕的左侧或右侧创建一个垂直边框，类似于侧边栏的形状。
	/// 这种布局方式适合用于讲故事或显示长段落文本，因为侧边栏的高度较大。
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "Layout")
	FSidebarLayoutSettings Sidebar;

	/// <summary>
	/// 
	/// </summary>
	UPROPERTY(Config, EditAnywhere, Category = "Layout")
	FDualSidebarLayoutSettings DualSidebar;
};
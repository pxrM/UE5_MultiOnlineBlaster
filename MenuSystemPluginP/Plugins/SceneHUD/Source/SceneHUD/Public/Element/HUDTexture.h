#pragma once

#include "CoreMinimal.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

/*
 * 用于绘制平铺图像或纹理的结构体
 * Texture (UTexture* Texture)：表示要绘制的纹理或图像资源。可以是加载到游戏中的任何图像资源，例如角色贴图、背景图片等。
 * Position (FVector2D Position)：指定图像在 Canvas 上的位置。使用 FVector2D 来描述位置，即二维空间中的坐标。
 * Size (FVector2D Size)：指定图像的大小，以像素为单位。通过 FVector2D 来描述宽度和高度。
 * DrawColor (FLinearColor DrawColor)：指定绘制图像时的颜色。使用 FLinearColor 类型表示，可以定义 RGBA（红绿蓝透明度）颜色值。
 * Rotation (float Rotation)：指定图像的旋转角度，以度为单位。
 * UV0 和 UV1 (FVector2D UV0, FVector2D UV1)：指定纹理坐标的范围。这些坐标通常用于定义从纹理中采样的区域，允许你指定纹理的一部分或整个纹理。
 */
class SCENEHUD_API FHUDTexture : public FCanvasTileItem
{
public:
	FHUDTexture(const FTexture* InTexture, const FVector2D& InSize, const FVector2D& InUV0, const FVector2D& InUV1,
	                const FLinearColor& InColor, EBlendMode InBlendMode, const FVector2D& InAssetSize)
		: FCanvasTileItem(FVector2D::ZeroVector, InTexture, InSize, InUV0, InUV1, InColor)
		  , Scale(1), FillAmount(1, 1), TextureSize(InSize), UVSize(InUV1 - InUV0), AssetSize(InAssetSize)
	{
		// 混合模式定义了如何将新的绘制操作与已有的图像内容进行混合。不同的混合模式会影响绘制结果的外观，例如是否叠加、透明度如何计算等。
		BlendMode = FCanvas::BlendToSimpleElementBlend(InBlendMode);
	}

	FHUDTexture(const FVector2D& InSize, const FLinearColor& InColor, EBlendMode InBlendMode,
	                const FVector2D& InAssetSize)
		: FCanvasTileItem(FVector2D::ZeroVector, InSize, InColor)
		  , Scale(1), FillAmount(1, 1), TextureSize(InSize), UVSize(1, 1), AssetSize(InAssetSize)
	{
		BlendMode = FCanvas::BlendToSimpleElementBlend(InBlendMode);
	}

public:
	void SetAlpha(const float InAlpha);

	void SetScale(const float InScale);

	void SetFillAmountX(const float InFillAmount);

	void SetFillAmountY(const float InFillAmount);

	void SetTexture(const FTexture* InTexture, const FVector2D& InSize, const FVector2D& InUV0, const FVector2D& InUV1);

	void SetTextureSize(const FVector2D& InTextureSize);
	FVector2D GetTextureSize() const;

	FVector2D GetSize() const;
	FLinearColor GetColor() const;

protected:
	// 纹理的缩放
	float Scale;
	// 纹理的填充量
	FVector2D FillAmount;
	// 纹理的实际大小
	FVector2D TextureSize;
	// 纹理的 UV 坐标大小
	FVector2D UVSize;
	// 纹理的资源大小
	FVector2D AssetSize;
};

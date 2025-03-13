#include "Element/HUDTexture.h"

void FHUDTexture::SetAlpha(const float InAlpha)
{
	Color.A = InAlpha;
}

void FHUDTexture::SetScale(const float InScale)
{
	Scale = InScale;

	Size.X = TextureSize.X * FillAmount.X * Scale;
	Size.Y = TextureSize.Y * FillAmount.Y * Scale;
}

void FHUDTexture::SetFillAmountX(const float InFillAmount)
{
	FillAmount.X = InFillAmount;

	UV1.X = UV0.X + UVSize.X * FillAmount.X;
	Size.X = TextureSize.X + FillAmount.X * Scale;
}

void FHUDTexture::SetFillAmountY(const float InFillAmount)
{
	FillAmount.X = InFillAmount;
    
    UV1.Y = UV0.Y + UVSize.Y * FillAmount.Y;
    Size.Y = TextureSize.Y * FillAmount.Y * Scale;
}

void FHUDTexture::SetTexture(const FTexture* InTexture, const FVector2D& InSize, const FVector2D& InUV0,
	const FVector2D& InUV1)
{
	if(AssetSize.X <= 0 || AssetSize.Y <= 0)
	{
		TextureSize = InSize;
		Size = InSize;
	}

	UVSize = InUV1 - InUV0;

	Texture = InTexture;
	UV0 = InUV0;
	UV1 = InUV1;
}

void FHUDTexture::SetTextureSize(const FVector2D& InTextureSize)
{
	TextureSize = InTextureSize;
}

FVector2D FHUDTexture::GetTextureSize() const
{
	return TextureSize;
}

FVector2D FHUDTexture::GetSize() const
{
	return Size;
}

FLinearColor FHUDTexture::GetColor() const
{
	return Color;
}

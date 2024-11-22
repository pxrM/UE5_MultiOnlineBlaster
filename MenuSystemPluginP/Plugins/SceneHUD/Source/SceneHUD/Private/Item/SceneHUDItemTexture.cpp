
#include "Item/SceneHUDItemTexture.h"

#include "Element/HUDTexture.h"

FSceneHUDItemTexture::~FSceneHUDItemTexture()
{
	if(CanvasItem != nullptr)
	{
		delete CanvasItem;
		CanvasItem = nullptr;
	}
}

void FSceneHUDItemTexture::Create(const FSceneHUDWidgetData& InData)
{
	FSceneHUDItem::Create(InData);

	const auto Data = static_cast<const FHUDTextureData&>(InData);
	const auto PaperSprite = Data.PaperSprite;
	if(PaperSprite != nullptr)
	{
		
	}
}

void FSceneHUDItemTexture::Draw(UCanvas* InCanvas, float InScale, float InAlpha, float InX, float InY, float InZ,
	bool bDrawDebug)
{
}

FVector2D FSceneHUDItemTexture::GetContentSize() const
{
}

FVector2D FSceneHUDItemTexture::GetRotatePivot() const
{
}

void FSceneHUDItemTexture::SetRotatePivot(const FVector2D& InPivot)
{
}

float FSceneHUDItemTexture::GetAngle() const
{
}

void FSceneHUDItemTexture::SetAngle(float InAngle)
{
}

void FSceneHUDItemTexture::SetFillAmountX(const float InFillAmount)
{
}

void FSceneHUDItemTexture::SetFillAmountY(const float InFillAmount)
{
}

void FSceneHUDItemTexture::SetTexture(const FTexture* InTexture, const FVector2D& InSize, const FVector2D& InUV0,
	const FVector2D& InUV1)
{
}

void FSceneHUDItemTexture::SetTextureSize(const FVector2D& InTextureSize)
{
}

void FSceneHUDItemTexture::SetItemColorInternal(const FLinearColor& InColor)
{
}

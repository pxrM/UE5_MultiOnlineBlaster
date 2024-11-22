#pragma once

#include "CoreMinimal.h"
#include "Item/SceneHUDItem.h"


class SCENEHUD_API FSceneHUDItemTexture : public FSceneHUDItem
{
public:
	static ESceneHUDType GetSceneHUDType()
	{
		return SHT_Bits_Texture;
	}

	FSceneHUDItemTexture()
	{
		HUDTypeFlags |= GetESceneHUDType();
	}
	
	virtual ~FSceneHUDItemTexture() override;

	
public:
	virtual void Create(const FSceneHUDWidgetData& InData) override;
	virtual void Draw(UCanvas* InCanvas, float InScale, float InAlpha, float InX, float InY, float InZ, bool bDrawDebug) override;
	virtual FVector2D GetContentSize() const override;

	
public:
	FVector2D GetRotatePivot() const;
	void SetRotatePivot(const FVector2D& InPivot);
	float GetAngle() const;
	void SetAngle(float InAngle);
	void SetFillAmountX(const float InFillAmount);
	void SetFillAmountY(const float InFillAmount);
	void SetTexture(const FTexture* InTexture, const FVector2D& InSize, const FVector2D& InUV0, const FVector2D& InUV1);
	void SetTextureSize(const FVector2D& InTextureSize);
	
	
protected:
	virtual void SetItemColorInternal(const FLinearColor& InColor) override;

	
protected:
	class FHUDTexture* CanvasItem = nullptr;
	
};
 
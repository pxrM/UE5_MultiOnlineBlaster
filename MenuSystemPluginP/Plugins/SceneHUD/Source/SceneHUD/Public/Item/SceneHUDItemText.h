

#pragma once

#include "CoreMinimal.h"
#include "Item/SceneHUDItem.h"

class SCENEHUD_API FSceneHUDItemText : public FSceneHUDItem
{
public:
	static ESceneHUDType GetSceneHUDType()
	{
		return SHT_Bits_Text;
	}

	FSceneHUDItemText()
	{
		HUDTypeFlags |= GetSceneHUDType();
	}
	
	virtual ~FSceneHUDItemText() override;

	
public:
	virtual void Create(const FSceneHUDWidgetData& InData) override;
	virtual void Draw(UCanvas* InCanvas, float InScale, float InAlpha, float InX, float InY, float InZ, bool bDrawDebug) override;
	virtual FVector2D GetContentSize() const override;

	
public:
	void SetText(const FText& InText) const;
	void SetOutlineColorHex(const FString& InColorHex);

	
protected:
	virtual void SetItemColorInternal(const FLinearColor& InColor) override;

	
protected:
	class FHUDText* CanvasItem = nullptr;
	float OutlineColorAlpha = 1.f;
	
};
 
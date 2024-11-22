

#pragma once

#include "CoreMinimal.h"
#include "SceneHUDWidget.h"

class SCENEHUD_API FSceneHUDItem : public FSceneHUDWidget
{
public:
	static ESceneHUDType GetSceneHUDType()
	{
		return SHT_Bits_Item;
	}

	FSceneHUDItem()
	{
		HUDTypeFlags |= GetSceneHUDType();
	}

	void SetColorHex(const FString& InColorHex)
	{
		const FLinearColor Color = FColor::FromHex(InColorHex);
		SetItemColorInternal(Color);
		ColorAlpha = Color.A;
	}

protected:
	virtual void SetItemColorInternal(const FLinearColor& InColor) = 0;

protected:
	float ColorAlpha = 1.0f;
};

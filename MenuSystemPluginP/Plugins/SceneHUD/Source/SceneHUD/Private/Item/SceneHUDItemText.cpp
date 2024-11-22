

#include "Item/SceneHUDItemText.h"

#include "Element/HUDText.h"
#include "Engine/Canvas.h"


FSceneHUDItemText::~FSceneHUDItemText()
{
	if(CanvasItem != nullptr)
	{
		delete CanvasItem;
		CanvasItem = nullptr;
	}
}

void FSceneHUDItemText::Create(const FSceneHUDWidgetData& InData)
{
	FSceneHUDItem::Create(InData);

	const auto Data = static_cast<const FHUDTextData&>(InData);
	
	ColorAlpha = Data.Color.A;
	const FLinearColor& OutLinearColor = Data.FontInfo.OutlineSettings.OutlineColor;
	OutlineColorAlpha = OutLinearColor.A;
	
	CanvasItem = new FHUDText(Data.Text, Data.FontInfo, Data.Color);
	CanvasItem->SetOutlineColor(OutLinearColor);
}

void FSceneHUDItemText::Draw(UCanvas* InCanvas, float InScale, float InAlpha, float InX, float InY, float InZ,
                             bool bDrawDebug)
{
	if (bDrawDebug)
	{
		DrawDebug(InCanvas, FVector2D{InX, InY}, InScale, 2.f, 3.f, FLinearColor::Green * 0.25f, FLinearColor::Green);
	}
	if(CanvasItem)
	{
		CanvasItem->StereoDepth = InZ;
		CanvasItem->SetScale(InScale * Scale);
		CanvasItem->SetAlpha(InAlpha * Alpha * ColorAlpha);
		CanvasItem->SetOutlineAlpha(InAlpha * Alpha * OutlineColorAlpha);
		InCanvas->DrawItem(*Cast<FCanvasItem>(CanvasItem), GetDrawPosition(InScale, FVector2D(InX, InY)));
	}
}

FVector2D FSceneHUDItemText::GetContentSize() const
{
	return CanvasItem ? CanvasItem->GetOriginTextSize() : FVector2D::ZeroVector;
}

void FSceneHUDItemText::SetText(const FText& InText) const
{
	if(CanvasItem)
	{
		CanvasItem->SetText(InText);
	}
}

void FSceneHUDItemText::SetOutlineColorHex(const FString& InColorHex)
{
	if(CanvasItem)
	{
		const FLinearColor Color = FColor::FromHex(InColorHex);
		CanvasItem->SetOutlineColor(Color);
		OutlineColorAlpha = Color.A;
	}
}

void FSceneHUDItemText::SetItemColorInternal(const FLinearColor& InColor)
{
	if(CanvasItem)
	{
		CanvasItem->SetColor(InColor);
	}
}

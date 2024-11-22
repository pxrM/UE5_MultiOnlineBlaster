// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HUDAsset.h"


enum ESceneHUDType : uint8
{
	SHT_Bits_Widget = 1 << 0,
	SHT_Bits_Item = 1 << 1,
	SHT_Bits_Texture = 1 << 2,
	SHT_Bits_Text = 1 << 3,
	SHT_Bits_Layout = 1 << 4,
	SHT_Bits_Panel = 1 << 5,
};


/**
 * 
 */
class SCENEHUD_API FSceneHUDWidget
{
	template<typename To, typename From>
	friend To* HUDCast(From*);
	
public:
	static ESceneHUDType GetESceneHUDType() { return SHT_Bits_Widget; }
	virtual ~FSceneHUDWidget()
	{
	};
	virtual void Create(const FSceneHUDWidgetData& InData);
	virtual void Draw(UCanvas* InCanvas, float InScale, float InAlpha, float InX, float InY, float InZ,
	                  bool bDrawDebug) = 0;
	virtual void UpdateLayout();
	virtual FVector2D GetContentSize() const = 0;
	virtual void GetAllValidHandleID(TArray<int32>& OutHandles);
	virtual FSceneHUDWidget* FindWidget(const FName& InName);

	template<typename T>
	T* FindWidgetAs(const FName& InName);
	
	void SetWidgetPosition(const float InX, const float InY) { OriginPosition = {InX, InY}; }
	FVector2D GetLayoutSize() const;
	FVector2D GetLayoutSizeWithPadding() const;
	const FName& GetName() const;
	bool GetVisible() const;
	void SetVisible(const bool InVisible);
	const FMargin& GetPadding();
	void SetPadding(const FMargin& InPadding);
	void SetHandleID(const int32 InHandleID);
	int32 GetHandleID() const;

protected:
	FVector2D GetDrawPosition(const float InScale, const FVector2D& InPosition) const;
	void DrawDebug(UCanvas* InCanvas, const FVector2D& InPosition, const float InScale, const float LayoutThickness,
	               const float ContentThickness,
	               const FLinearColor& LayoutColor, const FLinearColor& ContentColor) const;

protected:
	FName Name{TEXT("")};
	FVector2D OriginPosition = FVector2D::ZeroVector;
	FVector2D AnimPosition = FVector2D::ZeroVector;
	FVector2D Pivot = FVector2D::ZeroVector;
	FVector2D OriginSize = FVector2D::ZeroVector;
	bool bSizeToContent = true;
	float Scale = 1.f;
	float Alpha = 1.f;
	FMargin Padding;
	FVector2D LayoutPosition = FVector2D::ZeroVector;
	bool Visible = true;
	int32 HandleID = 0;
	uint8 HUDTypeFlags = SHT_Bits_Widget;
};



// To：目标类型，希望将 InWidget 指针转换为这个类型。
// From：源类型，默认为 FFHUDWidget 类型。
template <typename To, typename From = FSceneHUDWidget>
To* HUDCast(From* InWidget)
{
	static_assert(!std::is_base_of_v<class FSceneHUDPanel, To> || std::is_same<FSceneHUDPanel, To>::value,
		"不支持转换为FSceneHUDPanel的子类");
	return (InWidget->HUDTypeFlags & To::GetESceneHUDType()) ? static_cast<To*>(InWidget) : nullptr;
}

template <typename T>
T* FSceneHUDWidget::FindWidgetAs(const FName& InName)
{
	return HUDCast<T>(FindWidget(InName));
}

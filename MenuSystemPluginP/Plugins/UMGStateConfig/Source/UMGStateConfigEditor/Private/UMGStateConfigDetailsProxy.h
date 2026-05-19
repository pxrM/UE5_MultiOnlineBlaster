#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "UObject/Object.h"
#include "UMGStateConfigData.h"
#include "UMGStateConfigDetailsProxy.generated.h"

UCLASS(Abstract)
class UUMGStateConfigDetailsProxyBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void FromValue(const FUMGStateConfigPropertyValue&) {}
	virtual void ToValue(FUMGStateConfigPropertyValue&) const {}

};

UCLASS()
class UUMGStateConfigVisibilityProxy : public UUMGStateConfigDetailsProxyBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Behavior")
	ESlateVisibility Visibility = ESlateVisibility::Visible;

	virtual void FromValue(const FUMGStateConfigPropertyValue& Value) override;
	virtual void ToValue(FUMGStateConfigPropertyValue& Value) const override;
};

UCLASS()
class UUMGStateConfigRenderOpacityProxy : public UUMGStateConfigDetailsProxyBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", DisplayName = "Render Opacity"))
	float RenderOpacity = 1.0f;

	virtual void FromValue(const FUMGStateConfigPropertyValue& Value) override;
	virtual void ToValue(FUMGStateConfigPropertyValue& Value) const override;
};

UCLASS()
class UUMGStateConfigTextContentProxy : public UUMGStateConfigDetailsProxyBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Content")
	FText Text;

	virtual void FromValue(const FUMGStateConfigPropertyValue& Value) override;
	virtual void ToValue(FUMGStateConfigPropertyValue& Value) const override;
};

UCLASS()
class UUMGStateConfigImageAppearanceProxy : public UUMGStateConfigDetailsProxyBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Appearance")
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (DisplayName = "Color and Opacity"))
	FLinearColor ColorAndOpacity = FLinearColor::White;

	virtual void FromValue(const FUMGStateConfigPropertyValue& Value) override;
	virtual void ToValue(FUMGStateConfigPropertyValue& Value) const override;
};

UCLASS()
class UUMGStateConfigTextAppearanceProxy : public UUMGStateConfigDetailsProxyBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (DisplayName = "Color and Opacity"))
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, Category = "Appearance")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, Category = "Appearance")
	FVector2D ShadowOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (DisplayName = "Shadow Color and Opacity"))
	FLinearColor ShadowColorAndOpacity = FLinearColor::Transparent;

	virtual void FromValue(const FUMGStateConfigPropertyValue& Value) override;
	virtual void ToValue(FUMGStateConfigPropertyValue& Value) const override;
};


#pragma once

#include "CoreMinimal.h"
#include "PaperSprite.h"
#include "HUDAsset.generated.h"


UENUM(BlueprintType)
enum class EHUDLayoutType : uint8
{
	HLT_Horizontal UMETA(DisplayName = "Horizontal"),
	HLT_Vertical UMETA(DisplayName = "Vertical"),
};


UENUM(BlueprintType)
enum class EWidgetPropertyType : uint8
{
	WPT_Text = 0,
	WPT_Image = 1,
};


UENUM(BlueprintType)
enum class EHUDAnimationType : uint8
{
	HAT_Alpha = 0,
	HAT_Scale = 1,
	HAT_TranslationX = 2,
	HAT_TranslationY = 3,
};


USTRUCT(BlueprintType)
struct FSceneHUDWidgetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Pivot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMargin Padding;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSizeToContent = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="!bSizeToContent"))
	FVector2D Size = FVector2D(100.f, 100.f);
};


USTRUCT(BlueprintType)
struct FHUDTextData : public FSceneHUDWidgetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateFontInfo FontInfo;
};


USTRUCT(BlueprintType)
struct FHUDTextureData : public FSceneHUDWidgetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EBlendMode> BlendMode = BLEND_Translucent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPaperSprite* PaperSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D TextureSize;
};


USTRUCT(BlueprintType)
struct FSceneHUDLayoutData : public FSceneHUDWidgetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LayoutInterval = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EHUDLayoutType> LayoutType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EVerticalAlignment> VerticalAlignment;
};


USTRUCT(BlueprintType)
struct SCENEHUD_API FSceneHUDPropertyLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSceneHUDLayoutData LayoutData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EWidgetPropertyType> WidgetProperties;
};


USTRUCT(BlueprintType)
struct FHUDAnimationCurve
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EHUDAnimationType AnimType;

	UPROPERTY(EditAnywhere)
	UCurveFloat* Curve;

	float GetEndTime() const
	{
		if(Curve && Curve->FloatCurve.GetNumKeys() > 0)
		{
			return Curve->FloatCurve.GetLastKey().Time;
		}
		return 0.f;
	}

	float GetValue(const float InTime, const float InDefaultValue = 0.f) const
	{
		if(Curve && Curve->FloatCurve.GetNumKeys() > 0)
		{
			return Curve->FloatCurve.Eval(InTime, InDefaultValue);
		}
		return 1.f;
	}
};


USTRUCT(BlueprintType)
struct FWidgetPropertyAnimationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName AnimName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHUDAnimationCurve> Curves;

	UPROPERTY(EditAnywhere)
	bool bWidgetAnimation = false;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bWidgetAnimation"))
	TArray<FName> Widgets;

	UPROPERTY(Transient, EditAnywhere, AdvancedDisplay)
	bool bPreviewAnimation = false;

	UPROPERTY(Transient, EditAnywhere, meta=(EditCondition="bPreviewAnimation"), AdvancedDisplay)
	bool bLoopPreviewAnimation = false;

	float GetEndTime() const
	{
		float EndTime = 0.f;
		for(auto& Curve : Curves)
		{
			if(const float Time = Curve.GetEndTime(); Time > EndTime)
			{
				EndTime = Time;
			}
		}
		return EndTime;
	}
};


UCLASS(BlueprintType)
class SCENEHUD_API UHUDAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Panel")
	FSceneHUDLayoutData LayoutData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layouts")
	TArray<FSceneHUDPropertyLayout> PropertyLayouts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	TArray<FWidgetPropertyAnimationData> AnimProperties;
};
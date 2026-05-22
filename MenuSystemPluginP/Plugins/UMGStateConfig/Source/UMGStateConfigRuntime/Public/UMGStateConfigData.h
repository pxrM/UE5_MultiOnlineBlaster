#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "UMGStateConfigData.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUMGStateConfig, Log, All);

UENUM(BlueprintType)
enum class EUMGStateConfigPropertyType : uint8
{
	Visibility            UMETA(DisplayName = "可见性"),
	RenderOpacity         UMETA(DisplayName = "渲染不透明度"),
	Text                  UMETA(DisplayName = "文本"),
	TextColor             UMETA(DisplayName = "文本颜色"),
	BrushImage            UMETA(DisplayName = "图片资源"),
	BrushTint             UMETA(DisplayName = "图片着色"),
	ImageAppearance       UMETA(DisplayName = "图片外观"),
	TextAppearance        UMETA(DisplayName = "文本外观"),
	SerializedProperty    UMETA(DisplayName = "自定义属性"),
};


USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStateConfigPropertyValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FText TextValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FLinearColor ColorValue = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FLinearColor SecondaryColorValue = FLinearColor::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FVector2D VectorValue = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FSlateFontInfo FontValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	float FloatValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TObjectPtr<UObject> ObjectValue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FSlateBrush BrushValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	ESlateVisibility VisibilityValue = ESlateVisibility::Visible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FString SerializedPropertyPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FString SerializedPropertyValue;
};


USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStatePropertyChange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName TargetWidgetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TSubclassOf<UWidget> ExpectedWidgetClass;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "UMG State Config")
	FString EditorPath;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	EUMGStateConfigPropertyType PropertyType = EUMGStateConfigPropertyType::Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FUMGStateConfigPropertyValue Value;
};

USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStateConfigState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName StateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FText DisplayName;

	// Widgets registered for state. Auto-synced with PropertyChanges by editor add sites.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TArray<FName> ConfiguredWidgetNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TArray<FUMGStatePropertyChange> PropertyChanges;
};

USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStateConfigGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName DefaultStateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TArray<FUMGStateConfigState> States;
};

USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStateConfigRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TArray<FUMGStateConfigGroup> StateGroups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName PreviewStateGroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName PreviewStateName;
};

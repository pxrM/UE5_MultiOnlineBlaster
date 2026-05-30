#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "UObject/SoftObjectPath.h"
#include "UMGStateConfigData.generated.h"

UMGSTATECONFIGRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogUMGStateConfig, Log, All);


UENUM(BlueprintType)
enum class EUMGStateConfigPropertyType : uint8
{
	SerializedProperty,
};

USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStateConfigPropertyValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FString SerializedPropertyPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FString SerializedPropertyTypeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FString SerializedPropertyValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TArray<FSoftObjectPath> SerializedReferencedAssets;
};

USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStatePropertyChange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName TargetWidgetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TSubclassOf<UWidget> ExpectedWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FString EditorPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	EUMGStateConfigPropertyType PropertyType = EUMGStateConfigPropertyType::SerializedProperty;

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
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	bool bExclusiveGroup = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TArray<FUMGStateConfigState> States;
};

USTRUCT(BlueprintType)
struct UMGSTATECONFIGRUNTIME_API FUMGStateConfigRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	int32 SchemaVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	TArray<FUMGStateConfigGroup> StateGroups;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName PreviewStateGroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FName PreviewStateName;
};

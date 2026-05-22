#pragma once

#include "CoreMinimal.h"
#include "Extensions/UserWidgetExtension.h"
#include "UMGStateConfigData.h"
#include "UMGStateConfigUserWidgetExtension.generated.h"

class UUserWidget;

struct FUMGStateConfigChangeKey
{
	FName TargetWidgetName;
	EUMGStateConfigPropertyType PropertyType = EUMGStateConfigPropertyType::Visibility;
	FString SerializedPropertyPath;

	bool operator==(const FUMGStateConfigChangeKey& Other) const
	{
		return TargetWidgetName == Other.TargetWidgetName
			&& PropertyType == Other.PropertyType
			&& SerializedPropertyPath == Other.SerializedPropertyPath;
	}
};

FORCEINLINE uint32 GetTypeHash(const FUMGStateConfigChangeKey& Key)
{
	return HashCombine(
		HashCombine(GetTypeHash(Key.TargetWidgetName), GetTypeHash(static_cast<uint8>(Key.PropertyType))),
		GetTypeHash(Key.SerializedPropertyPath));
}

struct FUMGStateConfigGroupRestoreData
{
	TMap<FUMGStateConfigChangeKey, FUMGStateConfigPropertyValue> RestoreValues;
	TArray<FUMGStateConfigChangeKey> ActiveChangeKeys;
};


UCLASS(BlueprintType)
class UMGSTATECONFIGRUNTIME_API UUMGStateConfigUserWidgetExtension : public UUserWidgetExtension
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UMG State Config")
	FUMGStateConfigRuntimeData RuntimeData;

	UFUNCTION(BlueprintCallable, Category = "UMG State Config")
	bool ApplyUIState(FName StateGroupName, FName StateName);

	UFUNCTION(BlueprintCallable, Category = "UMG State Config")
	void SetRuntimeData(const FUMGStateConfigRuntimeData& NewData);

	void ResetActiveState();

private:
	bool ApplyPropertyChange(UUserWidget* TargetUserWidget, FName StateGroupName, const FUMGStatePropertyChange& Change);
	UWidget* ResolveWidget(UUserWidget* TargetUserWidget, FName WidgetName);
	const FUMGStateConfigGroup* FindStateGroup(FName StateGroupName);
	const FUMGStateConfigState* FindState(const FUMGStateConfigGroup& Group, FName StateName);
	void BuildLookupCache();
	void InvalidateLookupCache();
	void RestorePreviousValues(UUserWidget* TargetUserWidget, FName StateGroupName);
	void RestoreAllGroups(UUserWidget* TargetUserWidget);

private:
	TMap<FName, FUMGStateConfigGroupRestoreData> GroupRestoreData;
	TMap<FName, TWeakObjectPtr<UWidget>> WidgetCache;
	TMap<FName, int32> StateGroupIndexByName;
	TMap<FName, TMap<FName, int32>> StateIndexByGroupName;
	bool bLookupCacheBuilt = false;
};

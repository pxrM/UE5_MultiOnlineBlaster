#pragma once

#include "CoreMinimal.h"
#include "Extensions/UserWidgetExtension.h"
#include "UMGStateConfigData.h"
#include "UMGStateConfigUserWidgetExtension.generated.h"

class UUserWidget;

struct FUMGStateConfigChangeKey
{
	FName TargetWidgetName;
	EUMGStateConfigPropertyType PropertyType = EUMGStateConfigPropertyType::SerializedProperty;

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

struct FUMGActiveStateGroupRuntime
{
	FName ActiveStateName;
	TMap<FUMGStateConfigChangeKey, FUMGStateConfigPropertyValue> RestoreValues;
	TArray<FUMGStateConfigChangeKey> ActiveChangeKeys;
};

UCLASS(BlueprintType)

class UMGSTATECONFIGRUNTIME_API UUMGStateConfigUserWidgetExtension : public UUserWidgetExtension
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UMG State Config")
	FUMGStateConfigRuntimeData RuntimeData;

	UFUNCTION(BlueprintCallable, Category = "UMG State Config")
	bool ApplyUIState(FName StateGroupName, FName StateName);

	void ResetActiveState();
	void SetRuntimeData(const FUMGStateConfigRuntimeData& InRuntimeData);

private:
	bool ApplyPropertyChange(UUserWidget* TargetUserWidget, const FUMGStatePropertyChange& Change, FUMGActiveStateGroupRuntime& ActiveGroupRuntime);
	bool ReapplyActiveStates(UUserWidget* TargetUserWidget);
	UWidget* ResolveWidget(UUserWidget* TargetUserWidget, FName WidgetName);
	const FUMGStateConfigGroup* FindStateGroup(FName StateGroupName);
	const FUMGStateConfigState* FindState(const FUMGStateConfigGroup& Group, FName StateName);
	void BuildLookupCache();
	void InvalidateLookupCache();
	void NormalizeRuntimeData();
	void PreloadReferencedAssets(bool bAsync = false);
	void RestoreGlobalValues(UUserWidget* TargetUserWidget);
	void QueueWidgetRefresh(UWidget* Widget);
	void FlushPendingWidgetRefreshes();


private:
	TMap<FName, FUMGActiveStateGroupRuntime> ActiveStateGroups;
	TMap<FUMGStateConfigChangeKey, FUMGStateConfigPropertyValue> GlobalRestoreValues;
	TMap<FName, TWeakObjectPtr<UWidget>> WidgetCache;
	TArray<TWeakObjectPtr<UWidget>> PendingRefreshWidgets;

	TMap<FName, int32> StateGroupIndexByName;

	TMap<FName, TMap<FName, int32>> StateIndexByGroupName;
	bool bLookupCacheBuilt = false;
};

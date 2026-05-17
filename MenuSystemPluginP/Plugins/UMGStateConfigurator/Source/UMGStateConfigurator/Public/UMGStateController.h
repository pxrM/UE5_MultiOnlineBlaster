// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "UMGStateController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUMGStateConfigurator, Log, All);

USTRUCT(BlueprintType, meta=(DisplayName="UI Property Override", Tooltip="Stores a single widget property override for a UI state"))
struct FUIPropertyOverride
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FName TargetWidgetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FName PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString ValueData; // 以字符串形式序列化存储任何属性值

	/** 由 TargetWidgetName + "." + PropertyName 派生，用于快速查找 */
	FName GetPropertyKey() const
	{
		return FName(*(TargetWidgetName.ToString() + TEXT(".") + PropertyName.ToString()));
	}
};

USTRUCT(BlueprintType, meta=(DisplayName="UI State Group", Tooltip="A named state containing property overrides"))
struct FUIStateGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString StateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TArray<FUIPropertyOverride> Overrides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Config")
	bool bRecordMode = false;
};

USTRUCT(BlueprintType, meta=(DisplayName="UI State Category", Tooltip="A category grouping related UI states together"))
struct FUIStateCategory
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString EnumName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString ActiveStateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TArray<FUIStateGroup> States;
};

USTRUCT()
struct FInitialValueSnapshot
{
	GENERATED_BODY()

	// 记录 控件名.属性名 -> 原始字符串值
	UPROPERTY()
	TMap<FString, FString> SavedValues;
};


UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta=(DisplayName="UMG State Controller", Tooltip="Manages UI widget states with property overrides"))
class UMGSTATECONFIGURATOR_API UUMGStateController : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	TArray<FUIStateCategory> StateCategories;

	UFUNCTION(BlueprintCallable, Category = "StateControl")
	void SetState(const FString& CategoryName, const FString& NewStateName);
	void ApplyAllActiveStates();
	void ApplyStateGroup(const FUIStateGroup& Group) const;

	UFUNCTION(BlueprintCallable, Category = "UMGState")
	void ResetToInitialState();

	// 供编辑器调用的录制接口
	void UpdateRecordedPropertyToCategory(int32 CategoryIndex, int32 StateIndex, FName WidgetName, FName PropName, const FString& ValueStr);

	/** 使快照失效，下次 SetState 会重新采集 */
	void InvalidateSnapshot();

	/** 使控件指针缓存失效（控件树结构变更时调用） */
	void InvalidateWidgetCache();

protected:
	// 采集当前涉及的所有属性的快照
	void TakeInitialSnapshot();
	bool HasProperty(const TArray<FUIPropertyOverride>& Overrides, FName Key);

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

private:
	// 缓存初始状态
	UPROPERTY()
	FInitialValueSnapshot InitialSnapshot;

	// 持久控件指针缓存，避免每次 Apply/Reset 都查找
	UPROPERTY()
	TMap<FName, UWidget*> WidgetPtrCache;
	bool bWidgetCacheValid = false;

	// 记录各分类上一次的活跃状态，用于增量切换
	TMap<FString, FString> PreviousActiveStateNames;

	/** 通过缓存查找控件，未命中则查找并缓存 */
	UWidget* FindCachedWidget(UUserWidget* Owner, FName WidgetName);

	/** 撤销单个分类的 override（恢复到初始快照 + 其他分类的活跃状态） */
	void RevertCategoryOverrides(const FUIStateCategory& Category);

	/** 为指定控件批量调用 SynchronizeProperties（去重） */
	static void SynchronizeWidgets(const TMap<UWidget*, TArray<const FUIPropertyOverride*>>& WidgetOverridesMap);
	static void SynchronizeWidgets(const TArray<UWidget*>& Widgets);
};

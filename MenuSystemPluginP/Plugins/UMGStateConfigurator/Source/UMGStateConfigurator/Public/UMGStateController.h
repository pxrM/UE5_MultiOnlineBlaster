#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "UMGStateController.generated.h"

USTRUCT(BlueprintType)
struct FUIPropertyOverride
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FName PropertyKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FName TargetWidgetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	FString ValueData; // 以字符串形式序列化存储任何属性值
}; 

USTRUCT(BlueprintType)
struct FUIStateGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString StateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TArray<FUIPropertyOverride> Overrides;

	UPROPERTY(EditAnywhere, Category = "State|Config")
	bool bRecordMode = false;
};

USTRUCT(BlueprintType)
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


UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class UMGSTATECONFIGURATOR_API UUMGStateController : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	TArray<FUIStateCategory> StateCategories;

	// 缓存初始状态
	UPROPERTY()
	FInitialValueSnapshot InitialSnapshot;

	UFUNCTION(BlueprintCallable, Category = "StateControl")
	void SetState(FString CategoryName, FString NewStateName);
	void ApplyAllActiveStates();
	void ApplyStateGroup(const FUIStateGroup& Group) const;
	
	UFUNCTION(BlueprintCallable, Category = "UMGState")
	void ResetToInitialState();

	// 供编辑器调用的录制接口
	void UpdateRecordedPropertyToCategory(int32 CategoryIndex, int32 StateIndex, FName WidgetName, FName PropName, FString ValueStr);

protected:
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	// 采集当前涉及的所有属性的快照
	void TakeInitialSnapshot();
	bool HasProperty(const TArray<FUIPropertyOverride>& Overrides, FName Key);
};

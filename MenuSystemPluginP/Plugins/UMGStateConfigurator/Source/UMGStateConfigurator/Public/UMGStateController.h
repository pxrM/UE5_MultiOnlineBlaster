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
	FName TargetWidgetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
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
	TArray<FUIStateGroup> StateGroups;

	UPROPERTY(BlueprintReadOnly, Category = "States")
	FString CurrentState;

	UPROPERTY(EditAnywhere, Category = "Preview")
	FString ActiveStateName;

	UFUNCTION(BlueprintCallable, Category = "StateControl")
	void SetControllerState(FString NewStateName);

	void ApplyState(int32 Index);

	// 缓存初始状态
	UPROPERTY()
	FInitialValueSnapshot InitialSnapshot;
	
	UFUNCTION(BlueprintCallable, Category = "UMGState")
	void ResetToInitialState();

	// 供编辑器调用的录制接口
	void UpdateRecordedPropertyToGroup(FUIStateGroup& TargetGroup, FName TargetWidgetName, FName PropertyName, const FString& ValueStr);

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	// 采集当前涉及的所有属性的快照
	void TakeInitialSnapshot();
};

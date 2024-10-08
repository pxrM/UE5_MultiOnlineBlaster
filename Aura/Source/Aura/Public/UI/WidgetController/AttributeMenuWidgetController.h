// P

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"


class UAttributeInfoData;
struct FAuraAttributeInfo;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FAuraAttributeInfo&, Info);


/**
 * 属性菜单ui控制器
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitValues() override;
	virtual void BindCallbacksToDependencies() override;
	
	// 升级对应的属性
	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

private:
	// 广播属性变化
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
	
public:
	// 属性信息变化委托
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FAttributeInfoSignature AttributeInfoDelegate;
	// 玩家属性点数量改变广播 ui层
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnPlayerStatChangedSignature AttributePointChangedDelegate;
	
protected:
	// 数据基础信息数据表
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfoData> AttributeInfoData; 
};

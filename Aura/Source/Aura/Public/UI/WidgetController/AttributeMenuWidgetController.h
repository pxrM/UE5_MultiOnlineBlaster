// P

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

class UAttributeInfoData;
struct FAuraAttributeInfo;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FAuraAttributeInfo&, Info);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitValues() override;
	virtual void BindCallbacksToDependencies() override;


private:
	// 广播属性变化
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
	
	
public:
	// 属性信息变化委托
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FAttributeInfoSignature AttributeInfoDelegate;
	
protected:
	// 数据基础信息数据表
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfoData> AttributeInfoData; 
};

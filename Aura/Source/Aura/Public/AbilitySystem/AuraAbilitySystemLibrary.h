// P

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:
	/*
	 * 获取一个UOverlayWidgetController
	 * @param:WorldContextObject 因为静态函数本身不属于ue的任何一个world，所以需要指定一个world上下文进行追踪该world中obj
	 * BlueprintPure 是一个标记，用于标识一个函数或方法是“纯粹”的，即该函数不会修改对象的状态，也不会对外部状态产生影响，它只依赖于输入参数来计算输出结果。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController")
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController")
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
};

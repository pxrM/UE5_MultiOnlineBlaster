// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * uibase，会和uicontroller进行绑定
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 为ui设置WidgetController
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	

protected:
	// 设置contorller后调用 供蓝图使用
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;
	
};

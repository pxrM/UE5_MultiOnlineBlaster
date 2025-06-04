// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhotoWidget.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UPhotoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FEventReply OnTouchGesture(FGeometry MyGeometry, const FPointerEvent& GestureEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FEventReply OnTouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FEventReply OnTouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FEventReply OnTouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();
	
};

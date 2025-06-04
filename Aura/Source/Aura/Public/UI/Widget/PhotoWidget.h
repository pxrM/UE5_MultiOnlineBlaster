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
	FEventReply TouchGesture(FGeometry MyGeometry, const FPointerEvent& GestureEvent);

	FEventReply TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateSelectionBox(FVector2D BoxSize);


public:
	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	bool bIsSelecting = false;
	
};

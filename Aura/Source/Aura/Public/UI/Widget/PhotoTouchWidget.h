// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhotoTouchWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhotoTouchDelegate, const FPointerEvent&, PointerEvent);
/**
 * 
 */
UCLASS()
class AURA_API UPhotoTouchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FEventReply TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	static bool CheckPointEffectiveIndex(const int32 PointIndex);

public:
	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	bool bIsSelecting = false;
	
	FPhotoTouchDelegate TouchStartedCallBack;
	FPhotoTouchDelegate TouchMovedCallBack;
	FPhotoTouchDelegate TouchEndedCallBack;
};

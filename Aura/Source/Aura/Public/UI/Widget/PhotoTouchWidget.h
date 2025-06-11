// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhotoTouchWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhotoTouchDelegate, const FPointerEvent&, PointerEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSelectAreaDelegate, const FVector2D, CentrePoint, const FVector2D, BoxSize,  bool, bIsEnd);
/**
 * 
 */
UCLASS()
class AURA_API UPhotoTouchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	UFUNCTION(BlueprintCallable)
	FEventReply TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);
	UFUNCTION(BlueprintCallable)
	FEventReply TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);
	UFUNCTION(BlueprintCallable)
	FEventReply TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	static bool CheckPointEffectiveIndex(const int32 PointIndex);

public:
	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	FVector2D LastSelectionEnd;
	bool bIsSelecting = false;
	
	UPROPERTY(BlueprintAssignable)
	FPhotoTouchDelegate TouchStartedCallBack;
	UPROPERTY(BlueprintAssignable)
	FPhotoTouchDelegate TouchMovedCallBack;
	UPROPERTY(BlueprintAssignable)
	FPhotoTouchDelegate TouchEndedCallBack;
	UPROPERTY(BlueprintAssignable)
	FSelectAreaDelegate SelectAreaCallBack;
};

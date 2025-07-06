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
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, 
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UFUNCTION(BlueprintCallable)
	FEventReply TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);
	UFUNCTION(BlueprintCallable)
	FEventReply TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);
	UFUNCTION(BlueprintCallable)
	FEventReply TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);
	FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	static bool CheckPointEffectiveIndex(const int32 PointIndex);

	void UpdateCanvasPanelSlot(const FVector2D& BoxLeftTop, const FVector2D& BoxSize);

	UFUNCTION(BlueprintCallable)
	void UpdateSizePos();

	FVector2D UpdateImgPos(FVector2D TargetScreenPos);


public:
	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	FVector2D LastSelectionEnd;
	bool bIsSelected = false;
	bool bIsSelecting = false;

	FVector2D DStartPos;
	FVector2D DDragOffset;
	bool bIsDragging = false;
	
	UPROPERTY(BlueprintAssignable)
	FPhotoTouchDelegate TouchStartedCallBack;
	UPROPERTY(BlueprintAssignable)
	FPhotoTouchDelegate TouchMovedCallBack;
	UPROPERTY(BlueprintAssignable)
	FPhotoTouchDelegate TouchEndedCallBack;
	UPROPERTY(BlueprintAssignable)
	FSelectAreaDelegate SelectAreaCallBack;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PhotoTouchWidget")
	class UImage* ImageWidget = nullptr; // Reference to the image widget to be manipulated

	//UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PhotoTouchWidget")
	class UCanvasPanel* DecorateCanvasPanel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PhotoTouchWidget")
	UImage* ImageMaskWidget;

	bool bIsMoveImg = false;
	FVector2D TouchStartImagePosition;
	FVector2D TouchStartMousePosition;
	FVector2D InitialImagePosition;
	FVector2D InitialImageSize;
	const float MinZoom = 1.f;
	const float MaxZoom = 3.0f;
	const float ZoomSpeed = 0.1f;
	float ZoomFactor = 1.0f;
};

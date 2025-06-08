// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhotoWidget.generated.h"

 // 裁剪模式枚举
enum class ECropMode { FreeDrag, FixedRatio };

/**
 * 
 */
UCLASS()
class AURA_API UPhotoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	FEventReply TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateSelectionBox(FVector2D BoxSize);

	// 执行裁剪 (支持拖拽区域和比例裁剪)
	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshot(UTexture2D* SourceTexture, FVector2D NormalizedCenter, float NormalizedWidth, float NormalizedHeight);

	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshotRatio(UTexture2D* SourceTexture, const float TargetAspectRatio);


public:
	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	bool bIsSelecting = false;
	
};

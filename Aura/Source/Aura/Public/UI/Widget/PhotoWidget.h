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
	FEventReply TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

	FEventReply TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent);

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateSelectionBox(FVector2D BoxSize);

	/**
	* @SourceWidth Դͼ��Ŀ��
	* @SourceHeight Դͼ��ĸ߶�
	* @NormalizedCenter	��һ����Ĳü����ĵ�
	* @NormalizedWidth ��һ����Ĳü����
	* @NormalizedHeight	��һ����Ĳü��߶�
	* 
	* @OutStartPoint  �ü��������Ͻ����꣨���أ�
	* @OutEndPoint  �ü��������½����꣨���أ�
	* @OutCropWidth  �ü�����Ŀ�Ⱥ͸߶ȣ����أ�
	*/
	void CalculateCropRange(
		const int32 SourceWidth,
		const int32 SourceHeight,
		const FVector2D NormalizedCenter,
		const float NormalizedWidth,
		const float NormalizedHeight,
		FVector2D& OutStartPoint,
		FVector2D& OutEndPoint,
		FIntPoint& OutCropWidth);

	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshotCropScreenshot(
		const TArray<FColor>& InImageData,
		const int32 SourceWidth,
		const int32 SourceHeight,
		const int32 StartX,
		const int32 StartY,
		const int32 CropWidth,
		const int32 CropHeight);

	UFUNCTION(BlueprintCallable)
	void NormalizeSize(const FVector2D InSelectionStart, const FVector2D InSelectionEnd, FVector2D& OutNormalizedCenter, FVector2D& OutNormalizedSize);

	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshot(UTexture2D* SourceTexture, FVector2D NormalizedCenter, float NormalizedWidth, float NormalizedHeight);

	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshotRatio(UTexture2D* SourceTexture, const float TargetAspectRatio);


public:
	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	bool bIsSelecting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CacheSourceTexture;
	
};

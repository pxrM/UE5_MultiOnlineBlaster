// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhotoWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSetPhotoTexture, int32, InSizeX, int32, InSizeY, UTexture2D*, PhotoTexture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPhotoSelectAreaDelegate, const FVector2D, CentrePoint, const FVector2D, BoxSize, bool, bIsEnd);

USTRUCT(BlueprintType)
struct FPhotoTextureElement
{
	GENERATED_BODY()

	UPROPERTY()
	UTexture2D* Image; // 贴图（可选）

	UPROPERTY()
	FString Text;      // 文字（可选）

	UPROPERTY()
	FVector2D Position;

	UPROPERTY()
	FVector2D Size;

	UPROPERTY()
	FVector2D Scale;

	UPROPERTY()
	UFont* Font;

	UPROPERTY()
	FLinearColor Color;
};


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
	UFUNCTION(BlueprintCallable)
	void OnPhotoButtonPressed();
	UFUNCTION()
	void MScreenShot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData);

	/**
	* @SourceWidth 源图像的宽度
	* @SourceHeight 源图像的高度
	* @NormalizedCenter	归一化后的裁剪中心点
	* @NormalizedWidth 归一化后的裁剪宽度
	* @NormalizedHeight	归一化后的裁剪高度
	* 
	* @OutStartPoint  裁剪区域左上角坐标（像素）
	* @OutEndPoint  裁剪区域右下角坐标（像素）
	* @OutCropWidth  裁剪区域的宽度和高度（像素）
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

	/**
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshotCropScreenshot(
		const TArray<FColor>& InImageData,
		const int32 SourceWidth,
		const int32 SourceHeight,
		const int32 StartX,
		const int32 StartY,
		const int32 CropWidth,
		const int32 CropHeight);

	void AddElementImage(UTexture2D* Image, FVector2D Position, FVector2D Size, FVector2D Scale);
	void AddElementTxt(const FString& Text, FVector2D Position, FVector2D Size, FVector2D Scale);

	/**
	 * 将裁剪范围数据进行归一化
	 */
	UFUNCTION(BlueprintCallable)
	void NormalizeSize(const FVector2D InImageWidgetSize, const FVector2D InSelectionStart, const FVector2D InSelectionEnd, FVector2D& OutNormalizedCenter, FVector2D& OutNormalizedSize);

	/**
	* 
	*/
	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshot(UTexture2D* SourceTexture, FVector2D NormalizedCenter, float NormalizedWidth, float NormalizedHeight);

	/**
	* 对图片按比例进行裁剪
	*/
	UFUNCTION(BlueprintCallable)
	UTexture2D* CropScreenshotRatio(UTexture2D* SourceTexture, const float TargetAspectRatio/*3:4*/);

	/**
	 * 基于一个已有的 UTexture2D 纹理，生成一个新的纹理，并在其上绘制额外的元素（如贴图和文字 PhotoTextureElements）。
	 */
	UTexture2D* GenerateFinalTexture(UTexture2D* SourceTexture);

	/**
	 * 将一个 UWidget 渲染为纹理，生成一个新的 UTexture2D。
	 */
	UTexture2D* GenerateFinalTexture(UWidget* InWidget, int32 InSizeX, int32 InSizeY);

	/**
	* 将一个 UWidget 渲染为纹理，然后进行裁剪，生成一个新的 UTexture2D。
	*/
	UTexture2D* GenerateFinalTexture(UWidget* InWidget, int32 InSizeX, int32 InSizeY, FVector2D NormalizedCenter, FVector2D NormalizedSize);



public:
	FDelegateHandle PhotoDelegateHandle;
	TArray<FColor> CachedImageData;
	UPROPERTY(BlueprintAssignable)
	FSetPhotoTexture SetPhotoTexture;

	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	bool bIsSelecting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CacheSourceTexture;

	UPROPERTY(BlueprintAssignable)
	FPhotoSelectAreaDelegate PhotoSelectAreaCallBack;

	TArray<FPhotoTextureElement> PhotoTextureElements;
	
};

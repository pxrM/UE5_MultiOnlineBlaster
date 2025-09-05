// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhotoNewWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPhotoTextureShoted, int32, InSizeX, int32, InSizeY, UTexture2D*, PhotoTexture);

enum class FPhotoResizeCorner
{
	None,
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};


/**
 * 
 */
UCLASS()
class AURA_API UPhotoNewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPhotoNewWidget(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable)
	void OnPhotoBtnClicked();
	UFUNCTION()
	void OnScreenShoted(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData);
	UFUNCTION(BlueprintCallable)
	void BindTouchEvents();
	UFUNCTION(BlueprintCallable)
	void OnCropBtnClicked();

private:
	UFUNCTION()
	void OnPhotoTextureCB(int32 InSizeX, int32 InSizeY, UTexture2D* Texture);
	UFUNCTION()
	void OnTouchStartedCB(const FVector2D& ScreenPos);
	UFUNCTION()
	void OnTouchMovedCB(const FVector2D& ScreenPos, const FVector2D& DeltaScreenPos);
	UFUNCTION()
	void OnTouchEndedCB(const FVector2D& ScreenPos);
	UFUNCTION()
	void OnScaleChangedCB(const FVector2D& ScreenPos, const float Scale);

	void InitDisplayScreenShot(const int32 InSizeX, const int32 InSizeY);
	FPhotoResizeCorner CalculateClickedCorner(const FVector2D& LocalPos) const;
	bool IsClickedWithFrameRange(const FVector2D& LocalPos) const;

	void UpdateFrameRange(const FVector2D& Delta, const bool bIsTryMoveImgPos);
	void UpdateFramePos(const FVector2D& Delta, const bool bIsTryMoveImgPos);
	void UpdateImagePos(const FVector2D& Delta, const bool bIsTryMoveFramePos);
	bool IsFrameWithInImage(const FVector2D& InFramePos, const FVector2D& InFrameSize) const;
	bool IsImgCanContainFrame(const FVector2D& InImgPos) const;
	static bool IsImageContainFrame(const FVector2D& InImgPos, const FVector2D& InImgSize, const FVector2D& InFramePos, const FVector2D& InFrameSize);

	void SetUpdateMaskTimer();
	void UpdatePhotoMask() const;
	void SetPhotoMaskRange(const FVector2D& InCenterPos, const FVector2D& InSize) const;


public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* Image_Mask;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Photo;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UCanvasPanel* CanvasPanel_Edit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCanvasPanel* CanvasPanel_Size;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCanvasPanel* CanvasPanel_FrameRoot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCanvasPanel* CanvasPanel_Frame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCanvasPanel* CanvasPanel_Tex;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UPhotoNewTouchWidget* PhotoNewTouch;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CornerRadius = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinFrameSize = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinFrameFixedSize = 220.f;

private:
	FDelegateHandle PhotoDelegateHandle;
	FPhotoTextureShoted PhotoTextureDelegate;

	bool bIsLimitProportion = false;
	float AspectRatio = 0.f;

	FVector2D InitFramePos = FVector2D::ZeroVector;
	FVector2D InitFrameSize = FVector2D::ZeroVector;
	FVector2D FramePos = FVector2D::ZeroVector;
	FVector2D FrameSize = FVector2D::ZeroVector;

	float InitDisplayImgScale = 1.f;
	float MaxImageScale = 1.f;
	float PhotoImgScale = 1.f;
	FVector2D InitDisplayImgSize = FVector2D::ZeroVector;
	FVector2D PhotoImgPos = FVector2D::ZeroVector;
	FVector2D PhotoImgSize = FVector2D::ZeroVector;

	bool bIsFrameZoom = false;
	bool bIsFrameMove = false;
	bool bIsImgMove = false;
	FPhotoResizeCorner CurCorner = FPhotoResizeCorner::None;
	FVector2D TouchStartPos = FVector2D::ZeroVector;
	FVector2D TouchStartFramePos = FVector2D::ZeroVector;
	FVector2D TouchStartFrameSize = FVector2D::ZeroVector;
	FVector2D TouchStartImgPos = FVector2D::ZeroVector;

	FTimerHandle UpdateMaskTimer;
};

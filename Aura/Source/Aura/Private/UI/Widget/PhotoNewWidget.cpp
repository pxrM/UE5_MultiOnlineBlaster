// P


#include "UI/Widget/PhotoNewWidget.h"
#include "UI/Widget/PhotoNewTouchWidget.h"
#include "Utils/UIBlueprintFunctionLibrary.h"
#include "Components/Widget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"

UPhotoNewWidget::UPhotoNewWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PhotoTextureDelegate.AddDynamic(this, &UPhotoNewWidget::OnPhotoTextureCB);
}

void UPhotoNewWidget::OnPhotoBtnClicked()
{
	if (PhotoDelegateHandle.IsValid())
	{
		UGameViewportClient::OnScreenshotCaptured().Remove(PhotoDelegateHandle);
	}
	PhotoDelegateHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UPhotoNewWidget::OnScreenShoted);
	FScreenshotRequest::RequestScreenshot(true);
}

void UPhotoNewWidget::OnScreenShoted(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData)
{
	constexpr EPixelFormat PixelFormat = PF_B8G8R8A8;
	UTexture2D* Texture = UTexture2D::CreateTransient(InSizeX, InSizeY, PixelFormat, FName("123"));

	int32 NumBlocksX = InSizeX / GPixelFormats[PixelFormat].BlockSizeX;
	int32 NumBlocksY = InSizeY / GPixelFormats[PixelFormat].BlockSizeY;
	NumBlocksX = FMath::Max(NumBlocksX, 1);
	NumBlocksY = FMath::Max(NumBlocksY, 1);
	const int32 PixelSize = NumBlocksX * NumBlocksY * GPixelFormats[PixelFormat].BlockBytes;

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips.Last();
	uint8* TextureData = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
	FMemory::Memcpy(TextureData, InImageData.GetData(), PixelSize);
	Mip.BulkData.Unlock();

	Texture->UpdateResource();

	PhotoTextureDelegate.Broadcast(InSizeX, InSizeY, Texture);
}

void UPhotoNewWidget::BindTouchEvents()
{
	if (PhotoNewTouch)
	{
		PhotoNewTouch->TouchStartedCallBack.AddDynamic(this, &UPhotoNewWidget::OnTouchStartedCB);
		PhotoNewTouch->TouchMovedCallBack.AddDynamic(this, &UPhotoNewWidget::OnTouchMovedCB);
		PhotoNewTouch->TouchEndedCallBack.AddDynamic(this, &UPhotoNewWidget::OnTouchEndedCB);
		PhotoNewTouch->MouseWheelCallBack.AddDynamic(this, &UPhotoNewWidget::OnScaleChangedCB);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PhotoNewTouch is null! Please check the widget blueprint."));
	}
}

void UPhotoNewWidget::OnCropBtnClicked()
{
	UTexture2D* SourceTexture = UUIBlueprintFunctionLibrary::GetTexture2DFromImage(Image_Photo);
	if (SourceTexture == nullptr) return;

	const int32 SourceWidth = SourceTexture->GetSizeX();
	const int32 SourceHeight = SourceTexture->GetSizeY();

	const FVector2D FrameLocalSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	const FVector2D FrameLocalPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	const FVector2D ImgLocalSize = UUIBlueprintFunctionLibrary::GetLocalSize(Image_Photo);
	const FVector2D ImgLocalPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(Image_Photo);

	const FVector2D Delta = FrameLocalPos - ImgLocalPos;
	
	const FVector2D UVStart = Delta / ImgLocalSize;
	const FVector2D UVEnd = (Delta + FrameLocalSize) / ImgLocalSize;

	const float CropStartPosX = FMath::Floor(UVStart.X * SourceWidth);
	const float CropStartPosY = FMath::Floor(UVStart.Y * SourceHeight);
	const float CropWidth = FMath::Floor((UVEnd.X - UVStart.X) * SourceWidth);
	const float CropHeight = FMath::Floor((UVEnd.Y - UVStart.Y) * SourceHeight);

	UTexture2D* CropTex = UUIBlueprintFunctionLibrary::CropTexture2D(SourceTexture, CropStartPosX, CropStartPosY, CropWidth, CropHeight);
	if (CropTex == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("CropTex is null !!!"));
		return;
	}
	UUIBlueprintFunctionLibrary::SaveTextureToDisk(CropTex);
}

void UPhotoNewWidget::OnPhotoTextureCB(int32 InSizeX, int32 InSizeY, UTexture2D* Texture)
{
	UUIBlueprintFunctionLibrary::SetImageBrushResourceObject(Image_Photo, Texture);
	InitDisplayScreenShot(InSizeX, InSizeY);
}

void UPhotoNewWidget::OnTouchStartedCB(const FVector2D& ScreenPos)
{
	TouchStartPos = UUIBlueprintFunctionLibrary::AbsoluteToLocal(CanvasPanel_Size, ScreenPos);
	CurCorner = CalculateClickedCorner(TouchStartPos);
	if (CurCorner != FPhotoResizeCorner::None)
	{
		TouchStartFramePos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
		TouchStartFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
		bIsFrameZoom = true;
	}
	else if (IsClickedWithFrameRange(TouchStartPos))
	{
		TouchStartFramePos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
		TouchStartFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
		bIsFrameMove = true;
	}
	else
	{
		TouchStartImgPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(Image_Photo);
		bIsImgMove = true;
	}
}

void UPhotoNewWidget::OnTouchMovedCB(const FVector2D& ScreenPos, const FVector2D& DeltaScreenPos)
{
	FVector2D LocalPos = UUIBlueprintFunctionLibrary::AbsoluteToLocal(CanvasPanel_Size, ScreenPos);
	FVector2D Delta = LocalPos - TouchStartPos;
	UE_LOG(LogTemp, Log, TEXT("[OnTouchMovedCB] Delta: %s"), *Delta.ToString());
	if (bIsFrameZoom)
	{
		UpdateFrameRange(Delta, true);
	}
	else if (bIsFrameMove)
	{
		UpdateFramePos(Delta, true);
	}
	else if (bIsImgMove)
	{
		UpdateImagePos(Delta, true);
	}
	TouchStartPos = LocalPos;
}

void UPhotoNewWidget::OnTouchEndedCB(const FVector2D& ScreenPos)
{
	CurCorner = FPhotoResizeCorner::None;
	bIsFrameZoom = false;
	bIsFrameMove = false;
	bIsImgMove = false;
	TouchStartPos = FVector2D::ZeroVector;
	TouchStartFramePos = FVector2D::ZeroVector;
	TouchStartFrameSize = FVector2D::ZeroVector;
	TouchStartImgPos = FVector2D::ZeroVector;
}

void UPhotoNewWidget::OnScaleChangedCB(const FVector2D& ScreenPos, const float Scale)
{
	const float NewImgScale = FMath::Clamp(PhotoImgScale * Scale, InitDisplayImgScale, MaxImageScale);
	if (FMath::Abs(NewImgScale - InitDisplayImgScale) < 0.01f)
	{
		return;
	}
	const FVector2D LocalPosition = UUIBlueprintFunctionLibrary::AbsoluteToLocal(Image_Photo, ScreenPos);
	const FVector2D ImgLocalSize = UUIBlueprintFunctionLibrary::GetLocalSize(Image_Photo);
	const FVector2D ImgLocalPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(Image_Photo);
	const FVector2D NewImgSize = InitDisplayImgSize * NewImgScale;

	const FVector2D RelativeMouseNormalPos = LocalPosition / ImgLocalSize;
	const FVector2D NewRelativeMouseNormalPos = RelativeMouseNormalPos * NewImgSize;
	FVector2D NewImgPos = ImgLocalPos - (NewRelativeMouseNormalPos - LocalPosition);
	const FVector2D NewImgEndPos = NewImgPos + NewImgSize;

	const FVector2D LocalFramePos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	const FVector2D LocalFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	const FVector2D LocalFrameEndPos = LocalFramePos + LocalFrameSize;

	if (NewImgPos.X > LocalFramePos.X)
	{
		NewImgPos.Set(LocalFramePos.X, NewImgPos.Y);
	}
	if (NewImgEndPos.X < LocalFrameEndPos.X)
	{
		NewImgPos.Set(LocalFrameEndPos.X - NewImgSize.X, NewImgPos.Y);
	}
	if (NewImgEndPos.Y < LocalFrameEndPos.Y)
	{
		NewImgPos.Set(NewImgPos.X, LocalFrameEndPos.Y - NewImgSize.Y);
	}

	PhotoImgScale = NewImgScale;
	UUIBlueprintFunctionLibrary::SetCanvasSlotPosition(Image_Photo, NewImgPos);
	UUIBlueprintFunctionLibrary::SetCanvasSlotSize(Image_Photo, NewImgSize);
}

void UPhotoNewWidget::InitDisplayScreenShot(const int32 InSizeX, const int32 InSizeY)
{
	const FVector2D RootSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Edit);
	UE_LOG(LogTemp, Log, TEXT("[InitDisplayScreenShot] RootSize: %s"), *RootSize.ToString());

	const float ScaleX = RootSize.X / InSizeX;
	const float ScaleY = RootSize.Y / InSizeY;
	float FinalScale = FMath::Min(ScaleX, ScaleY);
	const FVector2D NewSize = FVector2D(InSizeX * FinalScale, InSizeY * FinalScale);

	InitDisplayImgScale = FinalScale;
	PhotoImgScale = FinalScale;
	MaxImageScale = FinalScale * 2;
	InitDisplayImgSize = NewSize;

	UUIBlueprintFunctionLibrary::SetCanvasSlotSize(Image_Photo, NewSize);
	UUIBlueprintFunctionLibrary::SetCanvasSlotSize(CanvasPanel_Tex, NewSize);
	UUIBlueprintFunctionLibrary::SetCanvasSlotSize(CanvasPanel_Size, NewSize);
	UUIBlueprintFunctionLibrary::SetCanvasSlotSize(CanvasPanel_Frame, NewSize);
	UUIBlueprintFunctionLibrary::SetCanvasSlotSize(CanvasPanel_FrameRoot, NewSize);
	SetUpdateMaskTimer();
}

FPhotoResizeCorner UPhotoNewWidget::CalculateClickedCorner(const FVector2D& LocalPos) const
{
	const FVector2D LocalFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	const FVector2D FrameStartPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	const FVector2D FrameEndPos = FrameStartPos + LocalFrameSize;
	
	const bool bIsTopLeft = (LocalPos.X >= FrameStartPos.X) && (LocalPos.X <= FrameStartPos.X + CornerRadius) &&
			(LocalPos.Y >= FrameStartPos.Y) && (LocalPos.Y <= FrameStartPos.X + CornerRadius);
	
	const bool bIsTopRight = (LocalPos.X <= FrameEndPos.X) && (LocalPos.X >= FrameEndPos.X - CornerRadius) &&
			(LocalPos.Y >= FrameStartPos.Y) && (LocalPos.Y <= FrameEndPos.Y - CornerRadius);
	
	const bool bIsBottomLeft = (LocalPos.X >= FrameStartPos.X) && (LocalPos.X <= FrameStartPos.X + CornerRadius) &&
			(LocalPos.Y <= FrameEndPos.Y)  && (LocalPos.Y >= FrameEndPos.Y - CornerRadius);
	
	const bool bIsBottomRight = (LocalPos.X <= FrameEndPos.X) && (LocalPos.Y <= FrameEndPos.Y) &&
			(LocalPos.X >= FrameEndPos.X - CornerRadius) && (LocalPos.Y >= FrameEndPos.Y - CornerRadius);
	
	FPhotoResizeCorner Corner = FPhotoResizeCorner::None;
	if (bIsTopLeft)
		Corner = FPhotoResizeCorner::TopLeft;
	else if (bIsTopRight)
		Corner = FPhotoResizeCorner::TopRight;
	else if (bIsBottomLeft)
		Corner = FPhotoResizeCorner::BottomLeft;
	else if (bIsBottomRight)
		Corner = FPhotoResizeCorner::BottomRight;
	return Corner;
}

bool UPhotoNewWidget::IsClickedWithFrameRange(const FVector2D& LocalPos) const
{
	const FVector2D LocalFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	const FVector2D FrameStartPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	const FVector2D FrameEndPos = FrameStartPos + LocalFrameSize;
	if (LocalPos.X >= FrameStartPos.X && LocalPos.X <= FrameEndPos.X
		&& LocalPos.Y >= FrameStartPos.Y && LocalPos.Y <= FrameEndPos.Y)
	{
		return true;
	}
	return false;
}

void UPhotoNewWidget::UpdateFrameRange(const FVector2D& Delta, const bool bIsTryMoveImgPos)
{
	const FVector2D LocalFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	const FVector2D LocalFramePos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	FVector2D NewFrameSize = LocalFrameSize;
	FVector2D NewFramePos = LocalFramePos;

	switch (CurCorner)
	{
	case FPhotoResizeCorner::TopLeft:
		NewFrameSize = NewFrameSize - Delta;
		NewFramePos = NewFramePos + Delta;
		break;
	case FPhotoResizeCorner::TopRight:
		NewFrameSize.Set(NewFrameSize.X + Delta.X, NewFrameSize.Y - Delta.Y);
		NewFramePos.Set(NewFramePos.X, NewFramePos.Y + Delta.Y);
		break;
	case FPhotoResizeCorner::BottomLeft:
		NewFrameSize.Set(NewFrameSize.X - Delta.X, NewFrameSize.Y + Delta.Y);
		NewFramePos.Set(NewFramePos.X + Delta.X, NewFramePos.Y);
		break;
	case FPhotoResizeCorner::BottomRight:
		NewFrameSize = NewFrameSize + Delta;
		break;
	default:
		break;
	}
	const float MinSize = bIsLimitProportion ? MinFrameFixedSize : MinFrameSize;
	const FVector2D MaxFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_FrameRoot);
	NewFrameSize.Set(FMath::Clamp(NewFrameSize.X, MinSize, MaxFrameSize.X), FMath::Clamp(NewFrameSize.Y, MinSize, MaxFrameSize.Y));

	if (bIsLimitProportion && AspectRatio > 0.f)
	{
		const float CurRataio = NewFrameSize.X / NewFrameSize.Y;
		if (CurRataio > AspectRatio)
			NewFrameSize.Set(NewFrameSize.Y * AspectRatio, NewFrameSize.Y);
		else
			NewFrameSize.Set(NewFrameSize.X, NewFrameSize.X / AspectRatio);

		switch (CurCorner)
		{
		case FPhotoResizeCorner::TopLeft:
			NewFramePos.Set(LocalFramePos.X + (LocalFrameSize.X - NewFrameSize.X), LocalFramePos.Y + (LocalFrameSize.Y - NewFrameSize.Y));
			break;
		case FPhotoResizeCorner::TopRight:
			NewFramePos.Set(LocalFramePos.X, LocalFramePos.Y + (LocalFrameSize.Y - NewFrameSize.Y));
			break;
		case FPhotoResizeCorner::BottomLeft:
			NewFramePos.Set(LocalFramePos.X + (LocalFrameSize.X - NewFrameSize.X), LocalFramePos.Y);
			break;
		}
	}
	NewFramePos.Set(FMath::Clamp(NewFramePos.X, 0, MaxFrameSize.X - NewFrameSize.X), FMath::Clamp(NewFramePos.Y, 0, MaxFrameSize.Y - NewFrameSize.Y));

	bool bIsFrameWithInImage = IsFrameWithInImage(NewFramePos, NewFrameSize);
	if (!bIsFrameWithInImage && bIsTryMoveImgPos)
	{
		UpdateImagePos(Delta, false);
		bIsFrameWithInImage = IsFrameWithInImage(NewFramePos, NewFrameSize);
	}
	if (bIsFrameWithInImage)
	{
		UUIBlueprintFunctionLibrary::SetCanvasSlotPosition(CanvasPanel_Frame, NewFramePos);
		UUIBlueprintFunctionLibrary::SetCanvasSlotSize(CanvasPanel_Frame, NewFrameSize);
		SetUpdateMaskTimer();
	}
}

void UPhotoNewWidget::UpdateFramePos(const FVector2D& Delta, const bool bIsTryMoveImgPos)
{
	const FVector2D MaxFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_FrameRoot);
	const FVector2D LocalFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	const FVector2D LocalFramePos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	const FVector2D NewFramePos = FVector2D(
		FMath::Clamp(LocalFramePos.X + Delta.X, 0, MaxFrameSize.X - LocalFrameSize.X),
		FMath::Clamp(LocalFramePos.Y + Delta.Y, 0, MaxFrameSize.Y - LocalFrameSize.Y)
	);
	bool bIsFrameWithInImage = IsFrameWithInImage(NewFramePos, LocalFrameSize);
	if (!bIsFrameWithInImage && bIsTryMoveImgPos)
	{
		UpdateImagePos(Delta, false);
		bIsFrameWithInImage = IsFrameWithInImage(NewFramePos, LocalFrameSize);
	}
	if (bIsFrameWithInImage)
	{
		UUIBlueprintFunctionLibrary::SetCanvasSlotPosition(CanvasPanel_Frame, NewFramePos);
		SetUpdateMaskTimer();
	}
}

void UPhotoNewWidget::UpdateImagePos(const FVector2D& Delta, const bool bIsTryMoveFramePos)
{
	const FVector2D ImgPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(Image_Photo);
	const FVector2D NewImgPos = ImgPos + Delta;

	bool bIsFrameWithInImage = IsImgCanContainFrame(NewImgPos);
	if (!bIsFrameWithInImage && bIsTryMoveFramePos)
	{
		UpdateFramePos(Delta, false);
		bIsFrameWithInImage = IsImgCanContainFrame(NewImgPos);
	}
	if (bIsFrameWithInImage)
	{
		UUIBlueprintFunctionLibrary::SetCanvasSlotPosition(Image_Photo, NewImgPos);
		SetUpdateMaskTimer();
	}
}

bool UPhotoNewWidget::IsFrameWithInImage(const FVector2D& InFramePos, const FVector2D& InFrameSize) const
{
	const FVector2D ImgLocalSize = UUIBlueprintFunctionLibrary::GetLocalSize(Image_Photo);
	const FVector2D ImgLocalPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(Image_Photo);
	return IsImageContainFrame(ImgLocalPos, ImgLocalSize, InFramePos, InFrameSize);
}

bool UPhotoNewWidget::IsImgCanContainFrame(const FVector2D& InImgPos) const
{
	const FVector2D LocalImgSize = UUIBlueprintFunctionLibrary::GetLocalSize(Image_Photo);
	const FVector2D LocalFramePos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	const FVector2D LocalFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	return IsImageContainFrame(InImgPos, LocalImgSize, LocalFramePos, LocalFrameSize);
}

bool UPhotoNewWidget::IsImageContainFrame(const FVector2D& InImgPos, const FVector2D& InImgSize, const FVector2D& InFramePos, const FVector2D& InFrameSize)
{
	const FVector2D ImgEndBottomRightPos = InImgPos + InImgSize;
	const FVector2D FrameEndBottomRightPos = InFramePos + InFrameSize;
	if (InImgPos.X <= InFramePos.X && ImgEndBottomRightPos.X >= FrameEndBottomRightPos.X
		&& (InImgPos.Y <= InFramePos.Y && ImgEndBottomRightPos.Y >= FrameEndBottomRightPos.Y))
	{
		return true;
	}
	return false;
}

void UPhotoNewWidget::SetUpdateMaskTimer()
{
	if (UpdateMaskTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateMaskTimer);
	}
	GetWorld()->GetTimerManager().SetTimer(UpdateMaskTimer, this, &UPhotoNewWidget::UpdatePhotoMask, 0.1f, false, 0.1f);
}

void UPhotoNewWidget::UpdatePhotoMask() const
{
	const FVector2D LocalFrameSize = UUIBlueprintFunctionLibrary::GetLocalSize(CanvasPanel_Frame);
	const FVector2D LocalStartPos = UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(CanvasPanel_Frame);
	const FVector2D LocalFrameEndPos = LocalStartPos + LocalFrameSize;
	const FVector2D LocalCenterPos = (LocalFrameEndPos - LocalStartPos) * 0.5f;
	const FVector2D ScreenFrameSPos = UUIBlueprintFunctionLibrary::LocalToAbsolute(CanvasPanel_Frame, LocalStartPos);
	const FVector2D ScreenFrameEPos = UUIBlueprintFunctionLibrary::LocalToAbsolute(CanvasPanel_Frame, LocalFrameEndPos);
	const FVector2D ScreenFrameCPos = UUIBlueprintFunctionLibrary::LocalToAbsolute(CanvasPanel_Frame, LocalCenterPos);

	const FVector2D MaskSize = UUIBlueprintFunctionLibrary::GetLocalSize(Image_Mask);
	const FVector2D LocalSPos = UUIBlueprintFunctionLibrary::AbsoluteToLocal(Image_Mask, ScreenFrameSPos);
	const FVector2D LocalEPos = UUIBlueprintFunctionLibrary::AbsoluteToLocal(Image_Mask, ScreenFrameEPos);
	const FVector2D LocalCPos = UUIBlueprintFunctionLibrary::AbsoluteToLocal(Image_Mask, ScreenFrameCPos);

	const FVector2D LocalMaskSize = LocalEPos - LocalSPos;
	const FVector2D NorMaskCenter = LocalCPos / MaskSize;
	const FVector2D NorMaskSize = LocalMaskSize / MaskSize;

	SetPhotoMaskRange(NorMaskCenter, NorMaskSize);
}

void UPhotoNewWidget::SetPhotoMaskRange(const FVector2D& InCenterPos, const FVector2D& InSize) const
{
	UUIBlueprintFunctionLibrary::SetImageMaterialScalarParameterValue(Image_Mask, TEXT("CentreX"), InCenterPos.X);
	UUIBlueprintFunctionLibrary::SetImageMaterialScalarParameterValue(Image_Mask, TEXT("CentreY"), InCenterPos.Y);
	UUIBlueprintFunctionLibrary::SetImageMaterialScalarParameterValue(Image_Mask, TEXT("SizeX"), InSize.X);
	UUIBlueprintFunctionLibrary::SetImageMaterialScalarParameterValue(Image_Mask, TEXT("SizeY"), InSize.Y);
}

// P


#include "UI/Widget/PhotoWidget.h"

FEventReply UPhotoWidget::TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	SelectionStart = MyGeometry.AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
	bIsSelecting = true;
	// 初始化选择框UI（如半透明矩形）
	return Reply;
}

FEventReply UPhotoWidget::TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	if (bIsSelecting)
	{
		// 更新选择框的结束位置
		SelectionEnd = MyGeometry.AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
		// 计算选择框的大小
		FVector2D BoxSize = FVector2D(FMath::Abs(SelectionEnd.X - SelectionStart.X), FMath::Abs(SelectionEnd.Y - SelectionStart.Y));
		// 计算选择框的左上角位置
		FVector2D BoxLeftTop = FVector2D(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y)
		);
		// 获取小部件的本地大小
		FVector2D WidgetSize = MyGeometry.GetLocalSize();
		// 计算选择框的中心点（中心点坐标可以表示为：中心点x = 左上角x + 宽度/2，中心点y = 左上角y + 高度/2。）
		FVector2D CenterPoint = BoxLeftTop + BoxSize * 0.5f;
		// 将中心点归一化到0-1范围
		float NormailizedX = FMath::Clamp(CenterPoint.X / WidgetSize.X, 0.0f, 1.0f);
		float NormailizedY = FMath::Clamp(CenterPoint.Y / WidgetSize.Y, 0.0f, 1.0f);


		// 更新选择框UI（如半透明矩形）
		UpdateSelectionBox(BoxSize);
	}
	return Reply;
}

FEventReply UPhotoWidget::TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	if (bIsSelecting)
	{
		bIsSelecting = false;
		// 选择结束，更新选择框UI
		FIntRect CropArea(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y),
			FMath::Max(SelectionStart.X, SelectionEnd.X),
			FMath::Max(SelectionStart.Y, SelectionEnd.Y)
		);
	}
	return Reply;
}

UTexture2D* UPhotoWidget::CropScreenshot(UTexture2D* SourceTexture, FVector2D NormalizedCenter, float NormalizedWidth, float NormalizedHeight)
{
	if (!SourceTexture) return nullptr;
	int32 SourceWidth = SourceTexture->GetSizeX();
	int32 SourceHeight = SourceTexture->GetSizeY();

	FVector2D CenterPixel(NormalizedCenter.X * SourceWidth, NormalizedCenter.Y * SourceHeight);
	FVector2D HalfSize(NormalizedWidth * SourceWidth * 0.5f, NormalizedHeight * SourceHeight * 0.5f);
	int32 StartX = FMath::Clamp(FMath::RoundToInt(CenterPixel.X - HalfSize.X), 0, SourceWidth - 1);
	int32 StartY = FMath::Clamp(FMath::RoundToInt(CenterPixel.Y - HalfSize.Y), 0, SourceHeight - 1);
	int32 EndX = FMath::Clamp(FMath::RoundToInt(CenterPixel.X + HalfSize.X), 0, SourceWidth - 1);
	int32 EndY = FMath::Clamp(FMath::RoundToInt(CenterPixel.Y + HalfSize.Y), 0, SourceHeight - 1);
	int32 CropWidth = EndX - StartX;
	int32 CropHeight = EndY - StartY;
	TArray<FColor> CroppedPixels;
	CroppedPixels.SetNumUninitialized(CropWidth * CropHeight);
	// 锁定纹理数据
	FTexture2DMipMap& Mip = SourceTexture->GetPlatformData()->Mips[0];
	const FColor* SourceData = reinterpret_cast<const FColor*>(Mip.BulkData.LockReadOnly());
	//for (int32 Y = StartY; Y < EndY; Y++)
	//{
	//	for (int32 X = StartX; X < EndX; X++)
	//	{
	//		const int32 Index = Y * SourceWidth + X;
	//		if (Index >= 0 && Index < Mip.BulkData.GetBulkDataSize() / sizeof(FColor))
	//		{
	//			CroppedPixels.Add(SourceData[Index]);
	//		}
	//	}
	//}
	for (int32 Y = 0; Y < CropHeight; Y++)
	{
		const int32 SrcRowStart = (StartY + Y) * SourceWidth + StartX;
		const int32 DstRowStart = Y * CropWidth;
		FMemory::Memcpy(&CroppedPixels[DstRowStart], &SourceData[SrcRowStart], CropWidth * sizeof(FColor));
	}
	Mip.BulkData.Unlock();

	// 创建新纹理
	UTexture2D* NewTexture = UTexture2D::CreateTransient(CropWidth, CropHeight, PF_B8G8R8A8);
	FTexture2DMipMap& NewMip = NewTexture->PlatformData->Mips[0];
	void* TextureData = NewMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, CroppedPixels.GetData(), CroppedPixels.Num() * sizeof(FColor));
	NewMip.BulkData.Unlock();

	NewTexture->UpdateResource();

	return nullptr;
}

UTexture2D* UPhotoWidget::CropScreenshotRatio(UTexture2D* SourceTexture, const float TargetAspectRatio/*3.0f/4.0f*/)
{
	if (!SourceTexture) return nullptr;

	// 获取纹理的尺寸
	int32 SourceWidth = SourceTexture->GetSizeX();
	int32 SourceHeight = SourceTexture->GetSizeY();
	int32 CropWidth = SourceWidth;
	int32 CropHeight = SourceHeight;
	int32 StartX = 0;
	int32 StartY = 0;
	const float OriginalAspect = static_cast<float>(SourceWidth) / SourceHeight;
	if (OriginalAspect > TargetAspectRatio)
	{
		// 原始更宽：裁剪宽度
		CropWidth = static_cast<int32>(SourceHeight * TargetAspectRatio);
		StartX = (SourceWidth - CropWidth) / 2;
	}
	else
	{
		// 原始更高：裁剪高度
		CropHeight = static_cast<int32>(SourceWidth / TargetAspectRatio);
		StartY = (SourceHeight - CropHeight) / 2;
	}
	UE_LOG(LogTemp, Log, TEXT("Cropping %dx%d to %dx%d (Start: %d,%d)"), SourceWidth, SourceHeight, CropWidth, CropHeight, StartX, StartY);

	TArray<FColor> CroppedBitmap;
	CroppedBitmap.Reserve(CropWidth * CropHeight);

	FTexture2DMipMap& Mip = SourceTexture->GetPlatformData()->Mips[0];
	const FColor* SourceData = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
	/*
	int32 NumBlocksX = SourceWidth / GPixelFormats[PF_B8G8R8A8].BlockSizeX;
	int32 NumBlocksY = SourceHeight / GPixelFormats[PF_B8G8R8A8].BlockSizeY;
	NumBlocksX = FMath::Max(NumBlocksX, 1);
	NumBlocksY = FMath::Max(NumBlocksY, 1);
	const int32 PixelSize = NumBlocksX * NumBlocksY * GPixelFormats[PF_B8G8R8A8].BlockBytes;
	const void* DataPtr = Mip.BulkData.LockReadOnly();
	TArray<uint8> RawData;
	RawData.SetNumUninitialized(PixelSize);
	FMemory::Memcpy(RawData.GetData(), DataPtr, PixelSize);
	Mip.BulkData.Unlock();
	*/
	for (int32 y = StartY; y < StartY + CropHeight; y++)
	{
		for (int32 x = StartX; x < StartX + CropWidth; x++)
		{
			int32 Index = y * SourceWidth + x;
			if (Index >= 0 && Index < Mip.BulkData.GetBulkDataSize() / sizeof(FColor))
			{
				CroppedBitmap.Add(SourceData[Index]);
			}
		}
	}
	Mip.BulkData.Unlock();
	UTexture2D* CroppedTexture = UTexture2D::CreateTransient(CropWidth, CropHeight, PF_B8G8R8A8);
	void* TextureData = CroppedTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, CroppedBitmap.GetData(), CroppedBitmap.Num() * sizeof(FColor));
	CroppedTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	CroppedTexture->UpdateResource();

	return CroppedTexture;
}

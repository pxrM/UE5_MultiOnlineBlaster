// P


#include "UI/Widget/PhotoWidget.h"

#include "Components/SlateWrapperTypes.h"
#include "Engine/GameViewportClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include <Slate/WidgetRenderer.h>
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "ImageUtils.h"
#include "UI/Widget/PhotoTouchWidget.h"

// HighResShot

void UPhotoWidget::OnPhotoButtonPressed()
{
	if (PhotoDelegateHandle.IsValid())
	{
		UGameViewportClient::OnScreenshotCaptured().Remove(PhotoDelegateHandle);
	}
	PhotoDelegateHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UPhotoWidget::MScreenShot);
	FScreenshotRequest::RequestScreenshot(false);
}

void UPhotoWidget::MScreenShot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData)
{
	CachedImageData = InImageData;
	// 常用的8位BGRA格式
	constexpr EPixelFormat PixelFormat = PF_B8G8R8A8;
	// 创建一个临时的2D纹理对象，尺寸和像素格式与截图一致。
	UTexture2D* Texture = UTexture2D::CreateTransient(InSizeX, InSizeY, PixelFormat, FName("123"));

	// UE的纹理有多级 MipMap，获取纹理的最高分辨率Mip层级（即Mip链中的第0级）
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips.Last();

	// 获取指定像素格式（如 PF_B8G8R8A8）在内存中每个“块”在 X 方向上的像素数。
	// 它的作用主要体现在压缩纹理格式（如DXT、ASTC等），这些格式以“块”为单位存储像素数据，每个块通常是 4x4 像素。
	int32 NumBlocksX = InSizeX / GPixelFormats[PixelFormat].BlockSizeX;
	int32 NumBlocksY = InSizeY / GPixelFormats[PixelFormat].BlockSizeY;
	NumBlocksX = FMath::Max(NumBlocksX, 1);
	NumBlocksY = FMath::Max(NumBlocksY, 1);
	// 总字节数 = X方向块数 × Y方向块数 × 单块字节数
	const int32 PixelSize = NumBlocksX * NumBlocksY * GPixelFormats[PixelFormat].BlockBytes;

	// 获取Mip数据可读写指针
	uint8* TextureData = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
	// 高效内存拷贝（SIMD优化）
	FMemory::Memcpy(TextureData, InImageData.GetData(), PixelSize);
	// 释放内存锁并标记数据变更
	Mip.BulkData.Unlock();

	// 更新纹理资源
	Texture->UpdateResource();

	SetPhotoTexture.Broadcast(InSizeX, InSizeY, Texture);
}

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

void UPhotoWidget::CalculateCropRange(const int32 SourceWidth, const int32 SourceHeight, const FVector2D NormalizedCenter, const float NormalizedWidth, const float NormalizedHeight,
	FVector2D& OutStartPoint, FVector2D& OutEndPoint, FIntPoint& OutCropWH)
{
	// 将归一化的中心点坐标（范围 [0,1]）转换为实际像素坐标
	FVector2D CenterPixel(NormalizedCenter.X * SourceWidth, NormalizedCenter.Y * SourceHeight);
	// 计算裁剪区域的半宽和半高（便于后续计算边界）0.5f 表示除以 2，因为要从中心向两侧扩展
	// 例如：归一化宽度 0.4，源宽 1000 → 半宽 = 0.4 * 1000 * 0.5 = 200 像素
	//	FVector2D HalfSize = NormalizedSize * FVector2D(SourceWidth, NormalizedHeight) * 0.5f;
	FVector2D HalfSize(NormalizedWidth * SourceWidth * 0.5f, NormalizedHeight * SourceHeight * 0.5f);
	// 左上角X坐标：中心X - 半宽，并限制在图像范围内（0到SourceWidth-1）
	int32 StartX = FMath::Clamp(FMath::RoundToInt(CenterPixel.X - HalfSize.X), 0, SourceWidth - 1);
	// 左上角Y坐标：中心Y - 半高，并限制在图像范围内
	int32 StartY = FMath::Clamp(FMath::RoundToInt(CenterPixel.Y - HalfSize.Y), 0, SourceHeight - 1);
	// 右下角X坐标：中心X + 半宽，并限制在图像范围内
	int32 EndX = FMath::Clamp(FMath::RoundToInt(CenterPixel.X + HalfSize.X), 0, SourceWidth - 1);
	// 右下角Y坐标：中心Y + 半高，并限制在图像范围内
	int32 EndY = FMath::Clamp(FMath::RoundToInt(CenterPixel.Y + HalfSize.Y), 0, SourceHeight - 1);
	int32 CropWidth = EndX - StartX;
	int32 CropHeight = EndY - StartY;

	OutStartPoint = FVector2D(StartX, StartY);
	OutEndPoint = FVector2D(EndX, EndY);
	OutCropWH = FIntPoint(CropWidth, CropHeight);
}

UTexture2D* UPhotoWidget::CropScreenshotCropScreenshot(const TArray<FColor>& InImageData,
	const int32 SourceWidth, const int32 SourceHeight, const int32 StartX, const int32 StartY, const int32 CropWidth, const int32 CropHeight)
{
	TArray<FColor> CroppedPixels;
	CroppedPixels.SetNumUninitialized(CropWidth * CropHeight);

	/*
	* 假设：
	* •	源图像宽度为 10，高度为 5。
	* •	裁剪区域起始点为 (2, 1)，宽度为 4，高度为 3。
	* •	源图像像素数据如下（每个数字表示像素索引）：
			 0   1   2   3   4   5   6	 7	 8   9
			10  11  12  13  14  15  16  17  18  19
			20  21  22  23  24  25  26  27  28  29
			30  31  32  33  34  35  36  37  38  39
			40  41  42  43  44  45  46  47  48  49

		裁剪区域为：
		•	起始点 (2, 1)，即从像素索引 12 开始。
		•	宽度为 4，高度为 3，即裁剪出以下像素：
			12  13  14  15
			22  23  24  25
			32  33  34  35

		循环执行过程：
		1.	第一行 (Y = 0)：
			•	SrcRowStart = (1 + 0) * 10 + 2 = 12。
			•	DstRowStart = 0 * 4 = 0。
			•	拷贝源图像中从索引 12 开始的 4 个像素到目标数组的索引 0 开始。
		2.	第二行 (Y = 1)：
			•	SrcRowStart = (1 + 1) * 10 + 2 = 22。
			•	DstRowStart = 1 * 4 = 4。
			•	拷贝源图像中从索引 22 开始的 4 个像素到目标数组的索引 4 开始。
		3.	第三行 (Y = 2)：
			•	SrcRowStart = (1 + 2) * 10 + 2 = 32。
			•	DstRowStart = 2 * 4 = 8。
			•	拷贝源图像中从索引 32 开始的 4 个像素到目标数组的索引 8 开始。
	*/
	// 遍历裁剪区域的每一行
	for (int32 Y = 0; Y < CropHeight; Y++)
	{
		// 计算源图像中当前行的起始索引
		// (StartY + Y)：表示裁剪行在源图中的起始行的像素索引
		// * SourceWidth：将行号转为源图像中该行的起始像素索引
		// + StartX：加上裁剪区域的起始列号，得到裁剪区域当前行在源图像中的起始像素索引。
		const int32 SrcRowStart = (StartY + Y) * SourceWidth + StartX;
		// 计算目标图像中当前行的起始索引
		// Y* CropWidth：表示裁剪区域当前行在目标数组 CroppedPixels 中的起始像素索引。
		const int32 DstRowStart = Y * CropWidth;
		// &CroppedPixels[DstRowStart]：目标数组中当前行的起始地址。
		// &CroppedPixels[DstRowStart]：源图像中裁剪区域当前行的起始地址。
		// CropWidth * sizeof(FColor)：需要拷贝的字节数，等于裁剪区域的宽度乘以每个像素的大小（FColor 的大小）。
		FMemory::Memcpy(&CroppedPixels[DstRowStart], &InImageData[SrcRowStart], CropWidth * sizeof(FColor));
	}

	UTexture2D* NewTexture = UTexture2D::CreateTransient(CropWidth, CropHeight, PF_B8G8R8A8);
	FTexture2DMipMap& NewMip = NewTexture->GetPlatformData()->Mips[0];
	void* TextureData = NewMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, CroppedPixels.GetData(), CroppedPixels.Num() * sizeof(FColor));
	NewMip.BulkData.Unlock();

	NewTexture->UpdateResource();

	return NewTexture;
}

void UPhotoWidget::NormalizeSize(const FVector2D InImageWidgetSize, const FVector2D InSelectionStart, const FVector2D InSelectionEnd, FVector2D& OutNormalizedCenter, FVector2D& OutNormalizedSize)
{
	//FVector2D WidgetSize = FVector2D(CacheSourceTexture->GetSizeX(), CacheSourceTexture->GetSizeY());
	FVector2D WidgetSize = InImageWidgetSize;
	// 计算选择框的大小
	FVector2D BoxSize(FMath::Abs(InSelectionEnd.X - InSelectionStart.X), FMath::Abs(InSelectionEnd.Y - InSelectionStart.Y));
	// 计算选择框的左上角位置
	FVector2D BoxLeftTop(FMath::Min(InSelectionStart.X, InSelectionEnd.X), FMath::Min(InSelectionStart.Y, InSelectionEnd.Y));
	// 将选择框的中心点归一化到0-1范围
	FVector2D CenterPoint = BoxLeftTop + BoxSize * 0.5f;
	// 归一化处理（比例值）
	OutNormalizedCenter = FVector2D(FMath::Clamp(CenterPoint.X / WidgetSize.X, 0.0f, 1.0f), FMath::Clamp(CenterPoint.Y / WidgetSize.Y, 0.0f, 1.0f));
	OutNormalizedSize = FVector2D(FMath::Clamp(BoxSize.X / WidgetSize.X, 0.0f, 1.0f), FMath::Clamp(BoxSize.Y / WidgetSize.Y, 0.0f, 1.0f));

	UE_LOG(LogTemp, Log, TEXT("Normalized Center: (%f, %f), Normalized Size: (%f, %f)"), OutNormalizedCenter.X, OutNormalizedCenter.Y, OutNormalizedSize.X, OutNormalizedSize.Y);
}

UTexture2D* UPhotoWidget::CropScreenshot(UTexture2D* SourceTexture, FVector2D NormalizedCenter, float NormalizedWidth, float NormalizedHeight)
{
	if (!SourceTexture) return nullptr;
	int32 SourceWidth = SourceTexture->GetSizeX();
	int32 SourceHeight = SourceTexture->GetSizeY();

	//FVector2D CenterPixel(NormalizedCenter.X * SourceWidth, NormalizedCenter.Y * SourceHeight);
	//FVector2D HalfSize(NormalizedWidth * SourceWidth * 0.5f, NormalizedHeight * SourceHeight * 0.5f);
	//int32 StartX = FMath::Clamp(FMath::RoundToInt(CenterPixel.X - HalfSize.X), 0, SourceWidth - 1);
	//int32 StartY = FMath::Clamp(FMath::RoundToInt(CenterPixel.Y - HalfSize.Y), 0, SourceHeight - 1);
	//int32 EndX = FMath::Clamp(FMath::RoundToInt(CenterPixel.X + HalfSize.X), 0, SourceWidth - 1);
	//int32 EndY = FMath::Clamp(FMath::RoundToInt(CenterPixel.Y + HalfSize.Y), 0, SourceHeight - 1);
	//int32 CropWidth = EndX - StartX;
	//int32 CropHeight = EndY - StartY;

	FVector2D OutStartPoint;
	FVector2D OutEndPoint;
	FIntPoint OutCropWH;
	CalculateCropRange(SourceWidth, SourceHeight, NormalizedCenter, NormalizedWidth, NormalizedHeight, OutStartPoint, OutEndPoint, OutCropWH);

	TArray<FColor> CroppedPixels;
	CroppedPixels.SetNumUninitialized(OutCropWH.X * OutCropWH.Y);
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
	for (int32 Y = 0; Y < OutCropWH.Y; Y++)
	{
		const int32 SrcRowStart = (OutStartPoint.Y + Y) * SourceWidth + OutStartPoint.X;
		const int32 DstRowStart = Y * OutCropWH.X;
		FMemory::Memcpy(&CroppedPixels[DstRowStart], &SourceData[SrcRowStart], OutCropWH.X * sizeof(FColor));
	}
	Mip.BulkData.Unlock();

	// 创建新纹理
	UTexture2D* NewTexture = UTexture2D::CreateTransient(OutCropWH.X, OutCropWH.Y, PF_B8G8R8A8, FName("123_1"));
	FTexture2DMipMap& NewMip = NewTexture->GetPlatformData()->Mips.Last();
	void* TextureData = NewMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, CroppedPixels.GetData(), CroppedPixels.Num() * sizeof(FColor));
	NewMip.BulkData.Unlock();

	NewTexture->UpdateResource();

	return NewTexture;
}

void UPhotoWidget::CalculateScreenshotRatio(UTexture2D* SourceTexture, const float TargetAspectRatio, bool IsCrop)
{
	if (!SourceTexture || FMath::IsNearlyZero(TargetAspectRatio))
		return;

	// 获取纹理的尺寸
	int32 SourceWidth = SourceTexture->GetSizeX();
	int32 SourceHeight = SourceTexture->GetSizeY();
	int32 CropWidth = SourceWidth;
	int32 CropHeight = SourceHeight;
	int32 StartX = 0;
	int32 StartY = 0;

	// 计算裁剪区域
	const float OriginalAspect = static_cast<float>(SourceWidth) / SourceHeight;
	if (OriginalAspect > TargetAspectRatio)
	{
		// 原始宽高比更宽：裁剪宽度
		CropWidth = static_cast<int32>(SourceHeight * TargetAspectRatio);
		StartX = (SourceWidth - CropWidth) / 2;
	}
	else
	{
		// 原始宽高比更高：裁剪高度
		CropHeight = static_cast<int32>(SourceWidth / TargetAspectRatio);
		StartY = (SourceHeight - CropHeight) / 2;
	}

	FBox2D CropFrame;
	CropFrame.Min = FVector2D(StartX, StartY);
	CropFrame.Max = FVector2D(StartX + CropWidth, StartY + CropHeight);
	FVector2D BoxSize(FMath::Abs(CropFrame.Max.X - CropFrame.Min.X), FMath::Abs(CropFrame.Max.Y - CropFrame.Min.Y));
	FVector2D CropCenter = CropFrame.Min + (CropFrame.Max - CropFrame.Min) * 0.5f; // 计算裁剪区域的中心点
	FVector2D NormalizedCenter = FVector2D(
		FMath::Clamp(CropCenter.X / SourceWidth, 0.0f, 1.0f),
		FMath::Clamp(CropCenter.Y / SourceHeight, 0.0f, 1.0f)
	);
	FVector2D NormalizedSize = FVector2D(
		FMath::Clamp(BoxSize.X / SourceWidth, 0.0f, 1.0f),
		FMath::Clamp(BoxSize.Y / SourceHeight, 0.0f, 1.0f)
	);
	PhotoTouchWidget->bIsSelected = true;
	PhotoTouchWidget->bIsSelecting = false;
	PhotoTouchWidget->SelectionStart = CropFrame.Min;
	PhotoTouchWidget->SelectionEnd = CropFrame.Max;
	PhotoSelectAreaCallBack.Broadcast(NormalizedCenter, NormalizedSize, false);

	UE_LOG(LogTemp, Log, TEXT("Cropping %dx%d to %dx%d (Start: %d,%d)"), SourceWidth, SourceHeight, CropWidth, CropHeight, StartX, StartY);

	if (IsCrop)
	{
		CropScreenshotRatio(SourceTexture, CropWidth, CropHeight, StartX, StartY);
	}
}

UTexture2D* UPhotoWidget::CropScreenshotRatio(UTexture2D* SourceTexture, int32 CropWidth, int32 CropHeight, int32 StartX, int32 StartY)
{
	if (!SourceTexture) return nullptr;

	int32 SourceWidth = SourceTexture->GetSizeX();
	int32 SourceHeight = SourceTexture->GetSizeY();

	// 锁定纹理数据
	FTexture2DMipMap& Mip = SourceTexture->GetPlatformData()->Mips[0];
	const FColor* SourceData = reinterpret_cast<const FColor*>(Mip.BulkData.LockReadOnly());

	// 提取裁剪区域的像素数据
	TArray<FColor> CroppedPixels;
	CroppedPixels.SetNumUninitialized(CropWidth * CropHeight);
	for (int32 Y = 0; Y < CropHeight; Y++)
	{
		const int32 SrcRowStart = (StartY + Y) * SourceWidth + StartX;
		const int32 DstRowStart = Y * CropWidth;
		FMemory::Memcpy(&CroppedPixels[DstRowStart], &SourceData[SrcRowStart], CropWidth * sizeof(FColor));
	}
	Mip.BulkData.Unlock();

	// 创建新纹理
	UTexture2D* CroppedTexture = UTexture2D::CreateTransient(CropWidth, CropHeight, PF_B8G8R8A8);
	FTexture2DMipMap& NewMip = CroppedTexture->GetPlatformData()->Mips[0];
	void* TextureData = NewMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, CroppedPixels.GetData(), CroppedPixels.Num() * sizeof(FColor));
	NewMip.BulkData.Unlock();

	CroppedTexture->UpdateResource();

	return CroppedTexture;
}

void UPhotoWidget::AddElementImage(UTexture2D* Image, FVector2D Position, FVector2D Size, FVector2D Scale)
{
	FPhotoTextureElement NewElement;
	NewElement.Image = Image;
	NewElement.Position = Position;
	NewElement.Size = Size;
	NewElement.Scale = Scale;
	PhotoTextureElements.Add(NewElement);
}

void UPhotoWidget::AddElementTxt(const FString& Text, FVector2D Position, FVector2D Size, FVector2D Scale)
{
	FPhotoTextureElement NewElement;
	NewElement.Text = Text;
	NewElement.Position = Position;
	NewElement.Size = Size;
	NewElement.Scale = Scale;
	PhotoTextureElements.Add(NewElement);
}

UTexture2D* UPhotoWidget::GenerateFinalTexture(UTexture2D* SourceTexture)
{
	int32 SourceWidth = SourceTexture->GetSizeX();
	int32 SourceHeight = SourceTexture->GetSizeY();

	// 1.创建RenderTarget
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->InitAutoFormat(SourceWidth, SourceHeight);

	// 2.创建一个画布来绘制元素
	FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();
	ERHIFeatureLevel::Type FeatureLevel = GetWorld()->GetFeatureLevel();
	FCanvas Canvas(RTResource, nullptr, 0, 0, 0, FeatureLevel);

	// 3.按ZOrder排序，依次绘制
	//	3.1 绘制原图
	Canvas.DrawTile(0, 0, SourceWidth, SourceHeight, 0, 0, 1, 1, FLinearColor::White, SourceTexture->GetResource());

	//	3.2 绘制贴图和文字
	for (const FPhotoTextureElement& Element : PhotoTextureElements)
	{
		if (Element.Image)
		{
			// 贴图
			Canvas.DrawTile(Element.Position.X, Element.Position.Y, Element.Size.X, Element.Size.Y, 0, 0, 1, 1, FLinearColor::White, Element.Image->GetResource(), SE_BLEND_Translucent);
		}
		else if (!Element.Text.IsEmpty())
		{
			// 文字
			FCanvasTextItem TextItem(Element.Position, FText::FromString(Element.Text), Element.Font, Element.Color);
			TextItem.Scale = FVector2D(Element.Scale.X, Element.Scale.Y);
			Canvas.DrawItem(TextItem);
		}
	}

	Canvas.Flush_GameThread();

	// 4.读取像素并生成新纹理
	TArray<FColor> OutPixels;
	RTResource->ReadPixels(OutPixels);
	UTexture2D* NewTexture = UTexture2D::CreateTransient(SourceWidth, SourceHeight, PF_B8G8R8A8);
	void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, OutPixels.GetData(), OutPixels.Num() * sizeof(FColor));
	NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	NewTexture->UpdateResource();

	return NewTexture;
}

UTexture2D* UPhotoWidget::GenerateFinalTexture(UWidget* InWidget, int32 InSizeX, int32 InSizeY)
{
	// FWidgetRenderer 是一个工具类，用于将 UWidget 渲染到 UTextureRenderTarget2D。
	auto WRDeleter = [](FWidgetRenderer* p) {BeginCleanup(p); };
	// 使用 std::unique_ptr 管理 FWidgetRenderer 的生命周期，并在销毁时调用 WRDeleter（BeginCleanup）。
	std::unique_ptr<FWidgetRenderer, decltype(WRDeleter)> WidgetRenderer(new FWidgetRenderer(true), WRDeleter);

	// 创建一个 UTextureRenderTarget2D，用于存储渲染结果。
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	// InitAutoFormat 方法根据输入的宽度和高度初始化渲染目标。
	RenderTarget->InitAutoFormat(InSizeX, InSizeY);

	// DrawWidget 方法将 InWidget 的内容绘制到 RenderTarget。
	// InWidget->TakeWidget() 将 UWidget 转换为 SWidget（Slate 的基础类）。
	WidgetRenderer->DrawWidget(RenderTarget, InWidget->TakeWidget(), FVector2D(InSizeX, InSizeY), 0, false);

	// GameThread_GetRenderTargetResource 获取渲染目标的资源。
	FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();
	// 从渲染目标中读取像素数据，并存储到 RawData 数组中。
	TArray<FColor> RawData;
	RTResource->ReadPixels(RawData);

	// 使用 CreateTransient 创建一个新的 UTexture2D，尺寸和格式与 RenderTarget 相同。
	UTexture2D* NewTexture = UTexture2D::CreateTransient(RenderTarget->SizeX, RenderTarget->SizeY, RenderTarget->GetFormat());
	NewTexture->UpdateResource();

	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);

	FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

	// 使用 FMemory::Memcpy 将像素数据从 RawData 拷贝到纹理的 Mip 数据中。
	FMemory::Memcpy(Data, RawData.GetData(), RawData.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();

	if (UTexture* ATex = Cast<UTexture>(NewTexture))
	{
#if WITH_EDITORONLY_DATA
		ATex->MipGenSettings = TMGS_NoMipmaps; // 关闭Mipmap生成
#endif
		ATex->Filter = TF_Nearest; // 使用最近邻过滤
		ATex->CompressionSettings = TC_EditorIcon; // 对于UI纹理使用无压缩
		ATex->LODGroup = TEXTUREGROUP_UI; // 指定为UI组
	}

	NewTexture->UpdateResource();

	return NewTexture;
}

UTexture2D* UPhotoWidget::GenerateFinalTextureCrop(UWidget* InWidget, int32 InSizeX, int32 InSizeY, FVector2D NormalizedCenter, FVector2D NormalizedSize)
{
	// 1. 渲染整个Widget到RenderTarget
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->InitAutoFormat(InSizeX, InSizeY);

	auto WRDeleter = [](FWidgetRenderer* p) {BeginCleanup(p); };
	std::unique_ptr<FWidgetRenderer, decltype(WRDeleter)> WidgetRenderer(new FWidgetRenderer(true), WRDeleter);

	WidgetRenderer->DrawWidget(RenderTarget, InWidget->TakeWidget(), FVector2D(InSizeX, InSizeY), 0, false);

	FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();
	TArray<FColor> RawData;
	RTResource->ReadPixels(RawData);

	// 2. 计算裁剪区域
	FVector2D OutStartPoint;
	FVector2D OutEndPoint;
	FIntPoint OutCropWH;
	CalculateCropRange(InSizeX, InSizeY, NormalizedCenter, NormalizedSize.X, NormalizedSize.Y, OutStartPoint, OutEndPoint, OutCropWH);

	// 3. 裁剪像素
	TArray<FColor> CroppedPixels;
	CroppedPixels.SetNumUninitialized(OutCropWH.X * OutCropWH.Y);
	for (int32 Y = 0; Y < OutCropWH.Y; Y++)
	{
		const int32 SrcRowStart = (OutStartPoint.Y + Y) * InSizeX + OutStartPoint.X;
		const int32 DstRowStart = Y * OutCropWH.X;
		FMemory::Memcpy(&CroppedPixels[DstRowStart], &RawData[SrcRowStart], OutCropWH.X * sizeof(FColor));
	}

	// 4. 生成新纹理
	UTexture2D* NewTexture = UTexture2D::CreateTransient(OutCropWH.X, OutCropWH.Y, PF_B8G8R8A8);
	NewTexture->UpdateResource();
	//FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->AreDerivedMipsAvailable() ? NewTexture->GetPlatformData()->Mips[0] : NewTexture->GetPlatformData()->Mips.Last();
	FTexture2DMipMap& Mip =  NewTexture->GetPlatformData()->Mips.Last();
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, CroppedPixels.GetData(), CroppedPixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();

	if (UTexture* ATex = Cast<UTexture>(NewTexture))
	{
#if WITH_EDITORONLY_DATA
		ATex->MipGenSettings = TMGS_NoMipmaps; // 关闭Mipmap生成
#endif
		ATex->Filter = TF_Nearest; // 使用最近邻过滤
		ATex->CompressionSettings = TC_EditorIcon; // 对于UI纹理使用无压缩
		ATex->LODGroup = TEXTUREGROUP_UI; // 指定为UI组
	}

	NewTexture->UpdateResource();

	return NewTexture;
}

void UPhotoWidget::SaveCroppedTextureToDisk(UTexture2D* Texture)
{
	FDateTime Now = FDateTime::Now();
	FString TimeStr = Now.ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
	FString FilePath = FPaths::ProjectSavedDir() / FString::Printf(TEXT("CaptureImage_%s.png"), *TimeStr);
	SaveCroppedTextureToDisk(Texture, FilePath);
}

void UPhotoWidget::SaveCroppedTextureToDisk(UTexture2D* Texture, const FString& FilePath)
{
	if (!Texture) return;

	// 获取像素数据
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	const int32 Width = Texture->GetSizeX();
	const int32 Height = Texture->GetSizeY();
	const FColor* FormattedImageData = static_cast<const FColor*>(Mip.BulkData.LockReadOnly());

	TArray<FColor> Pixels;
	Pixels.AddUninitialized(Width * Height);
	FMemory::Memcpy(Pixels.GetData(), FormattedImageData, Width * Height * sizeof(FColor));
	Mip.BulkData.Unlock();

	// 压缩为PNG
	TArray64<uint8> CompressedPNG;
	FImageUtils::PNGCompressImageArray(Width, Height, Pixels, CompressedPNG);

	// 保存到本地
	FFileHelper::SaveArrayToFile(CompressedPNG, *FilePath);
}
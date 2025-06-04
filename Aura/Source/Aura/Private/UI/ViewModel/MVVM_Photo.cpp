// P


#include "UI/ViewModel/MVVM_Photo.h"

#include "Components/SlateWrapperTypes.h"
#include "Engine/GameViewportClient.h"


void UMVVM_Photo::OnPhotoButtonPressed()
{
	if(PhotoDelegateHandle.IsValid())
	{
		UGameViewportClient::OnScreenshotCaptured().Remove(PhotoDelegateHandle);
	}
	PhotoDelegateHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UMVVM_Photo::MScreenShot);
	FScreenshotRequest::RequestScreenshot(false);
}

void UMVVM_Photo::MScreenShot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData)
{
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

void UMVVM_Photo::SetEditPanelVisibility(const ESlateVisibility InEditPanelVisibility)
{
	UE_MVVM_SET_PROPERTY_VALUE(EditPanelVisibility, InEditPanelVisibility);
}

void UMVVM_Photo::SetPhotoBrush(const FSlateBrush NewBrush)
{
	UE_MVVM_SET_PROPERTY_VALUE(PhotoBrush, NewBrush);
}


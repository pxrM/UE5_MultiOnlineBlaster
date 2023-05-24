// Fill out your copyright notice in the Description page of Project Settings.

/*
	智能指针：不需要手动释放的，通过引用计数实现，当引用计数为0，自动析构
	为什么要使用ue的智能指针：1.std::shared_ptr不是所有平台都可以使用，2.可以和其它虚幻容器(TArray)以及类型完美的协作，3.线程安全及优化
*/

#include "PictureToolsLibrary.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"

bool UPictureToolsLibrary::LoadImageToTexture2D(const FString& ImagePath, UTexture2D*& InTexture, float& Width, float& Height)
{
	// 使用静态映射表
//const TMap<FString, EImageFormat> UPictureToolsLibrary::ImageFormatMap =
//{
//	{ TEXT("jpg"), EImageFormat::JPEG },
//	{ TEXT("jpeg"), EImageFormat::JPEG },
//	{ TEXT("png"), EImageFormat::PNG },
//	{ TEXT("bmp"), EImageFormat::BMP }
//};
	FString AbsImagePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), ImagePath);
	/* 将指定路径下的图像文件加载为二进制数据，并使用Image Wrapper模块将其转换为特定格式的图像数据 */
	//存储图像文件的原始二进制数据。
	TArray<uint8> ImageResultData;
	//加载指定路径下的图像文件，并将其读取到一个TArray<uint8>中
	FFileHelper::LoadFileToArray(ImageResultData, *AbsImagePath);
	//加载Image Wrapper模块，ImageWrapper模块负责处理各种图片格式的压缩和解压缩等操作。
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	//获取文件后缀，参数一路径 参数二是否带.
	FString ImageSuffix = FPaths::GetExtension(AbsImagePath, false);
	EImageFormat ImageFromt = EImageFormat::Invalid;
	if (ImageSuffix.Equals(TEXT("jpg"), ESearchCase::IgnoreCase) || ImageSuffix.Equals(TEXT("jpeg"), ESearchCase::IgnoreCase))
	{
		ImageFromt = EImageFormat::JPEG;
	}
	else if (ImageSuffix.Equals(TEXT("png"), ESearchCase::IgnoreCase))
	{
		ImageFromt = EImageFormat::PNG;
	}
	else if (ImageSuffix.Equals(TEXT("bmp"), ESearchCase::IgnoreCase))
	{
		ImageFromt = EImageFormat::BMP;
	}
	//创建了一个指向特定图像格式处理器的智能指针。EImageFormat指定了要处理的图像格式
	TSharedPtr<IImageWrapper> ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFromt);
	//进行解码和编码操作
	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetCompressed(ImageResultData.GetData(), ImageResultData.Num()))
	{
		TArray<uint8> OutRawData;
		ImageWrapperPtr->GetRaw(ERGBFormat::BGRA, 8, OutRawData);
		Width = ImageWrapperPtr->GetWidth();
		Height = ImageWrapperPtr->GetHeight();
		//创建一个临时UTexture2D对象InTexture，并指定了其大小（Width和Height）和像素格式（PF_B8G8R8A8）
		InTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		if (InTexture)
		{
			//访问InTexture对象的底层平台数据（PlatformData）该对象包含了多个MipMap等级（Mips），每个MipMap等级都有相应的原始数据（BulkData）。
			//BulkData属性为一个FBulkData类型的结构体，它封装了UE4中处理大块数据的方法和数据，允许在内存中存储大块数据或使用文件映射技术进行数据访问。
			//通过访问MipMap等级的BulkData属性，我们可以对图像的原始数据进行读写操作，以修改或更新图像的像素信息。
			void *TextureData = InTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, OutRawData.GetData(), OutRawData.Num());
			InTexture->PlatformData->Mips[0].BulkData.Unlock();
			InTexture->UpdateResource();
			return true;
		}
	}
	return false;
}

bool UPictureToolsLibrary::SaveImageFromTexture2D(UTexture2D* InTex, const FString& DesPath)
{
	if (!InTex)return false;

	FString ImageSuffix = FPaths::GetExtension(DesPath, false);
	EImageFormat ImageFromt = EImageFormat::Invalid;
	if (ImageSuffix.Equals(TEXT("jpg"), ESearchCase::IgnoreCase) || ImageSuffix.Equals(TEXT("jpeg"), ESearchCase::IgnoreCase))
	{
		ImageFromt = EImageFormat::JPEG;
	}
	else if (ImageSuffix.Equals(TEXT("png"), ESearchCase::IgnoreCase))
	{
		ImageFromt = EImageFormat::PNG;
	}
	else if (ImageSuffix.Equals(TEXT("bmp"), ESearchCase::IgnoreCase))
	{
		ImageFromt = EImageFormat::BMP;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	TSharedPtr<IImageWrapper> ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFromt);
	TArray64<uint8> OutData;
	InTex->Source.GetMipData(OutData, 0);
	float Width = InTex->GetSizeX();
	float Height = InTex->GetSizeY();
	int Depth = InTex->Source.GetFormat() == ETextureSourceFormat::TSF_RGBA16 ? 16 : 8;
	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetRaw(OutData.GetData(), OutData.Num(), Width, Height, ERGBFormat::BGRA, Depth))
	{
		if (ImageFromt == EImageFormat::BMP)
		{

		}
		else
		{
			const TArray64<uint8>CompessedData = ImageWrapperPtr->GetCompressed(100);
			FFileHelper::SaveArrayToFile(CompessedData, *DesPath);
			return true;
		}		
	}
	return false;
}

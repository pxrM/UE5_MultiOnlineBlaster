// Fill out your copyright notice in the Description page of Project Settings.

/*
	����ָ�룺����Ҫ�ֶ��ͷŵģ�ͨ�����ü���ʵ�֣������ü���Ϊ0���Զ�����
	ΪʲôҪʹ��ue������ָ�룺1.std::shared_ptr��������ƽ̨������ʹ�ã�2.���Ժ������������(TArray)�Լ�����������Э����3.�̰߳�ȫ���Ż�
*/

#include "PictureToolsLibrary.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"

bool UPictureToolsLibrary::LoadImageToTexture2D(const FString& ImagePath, UTexture2D*& InTexture, float& Width, float& Height)
{
	// ʹ�þ�̬ӳ���
//const TMap<FString, EImageFormat> UPictureToolsLibrary::ImageFormatMap =
//{
//	{ TEXT("jpg"), EImageFormat::JPEG },
//	{ TEXT("jpeg"), EImageFormat::JPEG },
//	{ TEXT("png"), EImageFormat::PNG },
//	{ TEXT("bmp"), EImageFormat::BMP }
//};
	FString AbsImagePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), ImagePath);
	/* ��ָ��·���µ�ͼ���ļ�����Ϊ���������ݣ���ʹ��Image Wrapperģ�齫��ת��Ϊ�ض���ʽ��ͼ������ */
	//�洢ͼ���ļ���ԭʼ���������ݡ�
	TArray<uint8> ImageResultData;
	//����ָ��·���µ�ͼ���ļ����������ȡ��һ��TArray<uint8>��
	FFileHelper::LoadFileToArray(ImageResultData, *AbsImagePath);
	//����Image Wrapperģ�飬ImageWrapperģ�鸺�������ͼƬ��ʽ��ѹ���ͽ�ѹ���Ȳ�����
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	//��ȡ�ļ���׺������һ·�� �������Ƿ��.
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
	//������һ��ָ���ض�ͼ���ʽ������������ָ�롣EImageFormatָ����Ҫ�����ͼ���ʽ
	TSharedPtr<IImageWrapper> ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFromt);
	//���н���ͱ������
	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetCompressed(ImageResultData.GetData(), ImageResultData.Num()))
	{
		TArray<uint8> OutRawData;
		ImageWrapperPtr->GetRaw(ERGBFormat::BGRA, 8, OutRawData);
		Width = ImageWrapperPtr->GetWidth();
		Height = ImageWrapperPtr->GetHeight();
		//����һ����ʱUTexture2D����InTexture����ָ�������С��Width��Height�������ظ�ʽ��PF_B8G8R8A8��
		InTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		if (InTexture)
		{
			//����InTexture����ĵײ�ƽ̨���ݣ�PlatformData���ö�������˶��MipMap�ȼ���Mips����ÿ��MipMap�ȼ�������Ӧ��ԭʼ���ݣ�BulkData����
			//BulkData����Ϊһ��FBulkData���͵Ľṹ�壬����װ��UE4�д��������ݵķ��������ݣ��������ڴ��д洢������ݻ�ʹ���ļ�ӳ�似���������ݷ��ʡ�
			//ͨ������MipMap�ȼ���BulkData���ԣ����ǿ��Զ�ͼ���ԭʼ���ݽ��ж�д���������޸Ļ����ͼ���������Ϣ��
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

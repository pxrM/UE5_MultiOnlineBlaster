// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PictureToolsLibrary.generated.h"

/**
 *
 */
UCLASS()
class SIMPLEPICTURETOOLS_API UPictureToolsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = PictureTool)
		static bool LoadImageToTexture2D(const FString& ImagePath, UTexture2D*& InTextue, float& Width, float& Height);

	//  π”√æ≤Ã¨”≥…‰±Ì
	//const TMap<FString, EImageFormat> UPictureToolsLibrary::ImageFormatMap =
	//{
	//	{ TEXT("jpg"), EImageFormat::JPEG },
	//	{ TEXT("jpeg"), EImageFormat::JPEG },
	//	{ TEXT("png"), EImageFormat::PNG },
	//	{ TEXT("bmp"), EImageFormat::BMP }
	//};

};

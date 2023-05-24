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
	/// <summary>
	/// �Ӵ��̼���ͼƬ�ļ�ת��Texture2D
	/// </summary>
	/// <param name="ImagePath">ͼƬ·��</param>
	/// <param name="InTextue">���ص�Texture2D</param>
	/// <param name="Width"></param>
	/// <param name="Height"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = PictureTool)
		static bool LoadImageToTexture2D(const FString& ImagePath, UTexture2D*& InTextue, float& Width, float& Height);

	/// <summary>
	/// ��UTexture2D����Ϊ����ͼƬ
	/// </summary>
	/// <param name="InTex"></param>
	/// <param name="DesPath">Ҫ�����·��</param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = PictureTool)
		static bool SaveImageFromTexture2D(UTexture2D* InTex, const FString& DesPath);
};

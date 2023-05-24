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
	/// 从磁盘加载图片文件转成Texture2D
	/// </summary>
	/// <param name="ImagePath">图片路径</param>
	/// <param name="InTextue">返回的Texture2D</param>
	/// <param name="Width"></param>
	/// <param name="Height"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = PictureTool)
		static bool LoadImageToTexture2D(const FString& ImagePath, UTexture2D*& InTextue, float& Width, float& Height);

	/// <summary>
	/// 将UTexture2D保存为本地图片
	/// </summary>
	/// <param name="InTex"></param>
	/// <param name="DesPath">要保存的路径</param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = PictureTool)
		static bool SaveImageFromTexture2D(UTexture2D* InTex, const FString& DesPath);
};

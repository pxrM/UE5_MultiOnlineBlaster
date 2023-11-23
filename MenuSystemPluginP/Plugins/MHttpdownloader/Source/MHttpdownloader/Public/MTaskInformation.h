// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MTaskInformation.generated.h"

/**
* 任务信息结构体
 */
USTRUCT(BlueprintType)
struct FMTaskInformation
{
	GENERATED_BODY()

public:
	/// <summary>
	/// 将结构体序列化为jsonstr
	/// </summary>
	/// <param name="OutJsonString"></param>
	/// <returns></returns>
	bool SerializeToJsonString(FString& OutJsonString) const;

	/// <summary>
	/// 将jsonstr序列化为结构体
	/// </summary>
	/// <param name="InJsonString"></param>
	/// <returns></returns>
	bool DeserializeFromJsonString(const FString& InJsonString);

	/// <summary>
	/// 获取唯一标识id
	/// </summary>
	/// <returns></returns>
	FORCEINLINE int32 GetGUID() const { return GUID; }


public:
	/// <summary>
	/// 文件名
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString FileName = FString("");

	/// <summary>
	/// 目标目录
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString DestDirectory = FString("");

	/// <summary>
	/// url
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString SourceUrl = FString("");

	/// <summary>
	/// ETag是一个不透明的标识符，由Web服务器根据URL上的资源的特定版本而指定。如果URL上的资源内容改变，一个新的不一样的ETag就会被生成。
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString ETag = FString("");

	/// <summary>
	/// 当前大小
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 CurrentSize = 0;

	/// <summary>
	/// 总大小
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 TotalSize = 0;

	/// <summary>
	/// 标识
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 GUID = 0;

};

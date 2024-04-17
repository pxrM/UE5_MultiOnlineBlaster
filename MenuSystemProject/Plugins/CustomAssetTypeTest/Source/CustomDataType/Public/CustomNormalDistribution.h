// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <random>
#include "CustomNormalDistribution.generated.h"

/**
 * 自定义的正态分布类型
 */
UCLASS(BlueprintType)
class CUSTOMDATATYPE_API UCustomNormalDistribution : public UObject
{
	GENERATED_BODY()
	
public:
	UCustomNormalDistribution();

	/// <summary>
	/// 生成符合正态分布的随机样本
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable)
	float DrawSample();

	UFUNCTION(BlueprintCallable)
	void LogSample();


public:
	/// <summary>
	/// 正态分布的均值
	/// </summary>
	UPROPERTY(EditAnywhere)
	float Mean;

	/// <summary>
	/// 正态分布的标准差
	/// </summary>
	UPROPERTY(EditAnywhere)
	float StandardDeviation;

	//标记仅在编辑器环境下使用的数据
#if WITH_EDITORONLY_DATA
	/// <summary>
	/// 路径变量，记录数据的导入来源。
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	FString SourceFilePath;
#endif


private:
	/// <summary>
	/// 随机数生成器
	/// </summary>
	 std::mt19937 RandomNumberGenerator;


};

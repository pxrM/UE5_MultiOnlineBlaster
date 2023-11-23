// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MTaskInformation.generated.h"

/**
* ������Ϣ�ṹ��
 */
USTRUCT(BlueprintType)
struct FMTaskInformation
{
	GENERATED_BODY()

public:
	/// <summary>
	/// ���ṹ�����л�Ϊjsonstr
	/// </summary>
	/// <param name="OutJsonString"></param>
	/// <returns></returns>
	bool SerializeToJsonString(FString& OutJsonString) const;

	/// <summary>
	/// ��jsonstr���л�Ϊ�ṹ��
	/// </summary>
	/// <param name="InJsonString"></param>
	/// <returns></returns>
	bool DeserializeFromJsonString(const FString& InJsonString);

	/// <summary>
	/// ��ȡΨһ��ʶid
	/// </summary>
	/// <returns></returns>
	FORCEINLINE int32 GetGUID() const { return GUID; }


public:
	/// <summary>
	/// �ļ���
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString FileName = FString("");

	/// <summary>
	/// Ŀ��Ŀ¼
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString DestDirectory = FString("");

	/// <summary>
	/// url
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString SourceUrl = FString("");

	/// <summary>
	/// ETag��һ����͸���ı�ʶ������Web����������URL�ϵ���Դ���ض��汾��ָ�������URL�ϵ���Դ���ݸı䣬һ���µĲ�һ����ETag�ͻᱻ���ɡ�
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString ETag = FString("");

	/// <summary>
	/// ��ǰ��С
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 CurrentSize = 0;

	/// <summary>
	/// �ܴ�С
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 TotalSize = 0;

	/// <summary>
	/// ��ʶ
	/// </summary>
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 GUID = 0;

};

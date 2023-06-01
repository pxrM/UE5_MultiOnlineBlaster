// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

/**
 * 
 */
class MHTTPDOWNTOOL_API FHttpDownloadSubTask : public TSharedFromThis<FHttpDownloadSubTask>
{
public:
	FHttpDownloadSubTask();
	~FHttpDownloadSubTask();

public:
	TSharedPtr<IHttpRequest> CreateRequest();
	void Stop();
	void SaveData();
	void LoadData();

public:
	int32 TaskID;
	FString URL;
	FString MD5Str;
	FString CurFilePath;
	FString Range;	//���ط�Χ
	bool bFinished;	//�����Ƿ����
	bool bWaitRreponse; //�Ƿ���Ҫ�ȴ�
	int32 StarPos;  //�ļ����ص��ֽ�λ��
	int32 Size; //��������ֽڴ�С
	TArray<uint8> RawData; //���غ������
	double RequestTime; //����ʱ��
	TSharedPtr<IHttpRequest> RequestPtr; //����ָ��
};

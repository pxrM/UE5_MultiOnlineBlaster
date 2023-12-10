// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMAsyncLoadingScreenModule : public IModuleInterface
{
public:
	/// <summary>
	/// �ڼ���ģ��DLL������ģ�����֮����������
	/// </summary>
	virtual void StartupModule() override;

	/// <summary>
	/// ��ģ�鱻ж��֮ǰ���ã�����ģ���������֮ǰ��
	/// </summary>
	virtual void ShutdownModule() override;

	/// <summary>
	/// ����һ���������ģ��Ľӿڡ���ֻ��Ϊ�˷���!
	/// ע���ڲ���رս׶ε������������ģ������Ѿ���ж���ˡ�
	/// </summary>
	/// <returns> ���ص���ʵ����������Ҫ����ģ�� </returns>
	static inline FMAsyncLoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMAsyncLoadingScreenModule>("MAsyncLoadingScreen");
	}


public:
	/// <summary>
	/// ��顰bPreloadBackgroundImages��ѡ���Ƿ�����
	/// </summary>
	/// <returns></returns>
	bool IsPreloadBackgroundImageEnable();

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	TArray<UTexture2D*> GetBackgroundImages();


private:
	/// <summary>
	/// ��������ͼ������
	/// </summary>
	UPROPERTY()
	TArray<class UTexture2D*> StartupBackgroundImages;
	/// <summary>
	/// Ĭ�ϱ���ͼ������
	/// </summary>
	UPROPERTY()
	TArray<UTexture2D*> DefaultBackgroundImages;
	/// <summary>
	/// �Ƿ�����������
	/// </summary>
	bool bIsStartupLoadingScreen = false;

};

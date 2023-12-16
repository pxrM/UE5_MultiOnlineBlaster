// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


struct FALoadingScreenSettings;


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
	/// �Ƿ�������Ϸ�߼�ģ�顣�ú�������һ������ֵ�����ڱ�ʾ��ǰģ���Ƿ��й���Ϸ���롣
	/// �������ֵΪ true�����ʾ��ģ������Ϸ�߼�ģ�飻�������ֵΪ false�����ʾ��ģ�����������ģ�顢������������͵�ģ�顣
	/// </summary>
	/// <returns></returns>
	virtual bool IsGameModule() const override;


public:
	/// <summary>
	/// ����һ���������ģ��Ľӿڡ���ֻ��Ϊ�˷���!
	/// ע���ڲ���رս׶ε������������ģ������Ѿ���ж���ˡ�
	/// </summary>
	/// <returns> ���ص���ʵ����������Ҫ����ģ�� </returns>
	static inline FMAsyncLoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMAsyncLoadingScreenModule>("MAsyncLoadingScreen");
	}

	/// <summary>
	/// ģ���Ƿ��Ѽ���
	/// </summary>
	/// <returns></returns>
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("MAsyncLoadingScreen");
	}

	FORCEINLINE bool IsStartupLoadingScreen() const { return bIsStartupLoadingScreen; }


public:
	/// <summary>
	/// ��ȡ����ͼƬ����
	/// </summary>
	/// <returns></returns>
	TArray<UTexture2D*> GetBackgroundImages();

	/// <summary>
	/// ��顰bPreloadBackgroundImages��ѡ���Ƿ�����
	/// </summary>
	/// <returns></returns>
	bool IsPreloadBackgroundImageEnable();

	/// <summary>
	/// �������м������б���ͼ������
	/// </summary>
	void LoadBackgroundImages();

	/// <summary>
	/// ɾ���������е�bgͼ
	/// </summary>
	void RemoveAllBackgroundImages();


private:
	/// <summary>
	/// ������Ļ�ص��������ú����ڼ��ع����б����ã�����Ԥ�����ü�����Ļ���Ա��ڼ����ڼ���ʾ��
	/// </summary>
	void PreSetupLoadingScreen();

	/// <summary>
	/// ���ü��ػ�������
	/// </summary>
	/// <param name="LoadingScreenSettings"></param>
	void SetupLoadingScreen(const FALoadingScreenSettings& LoadingScreenSettings);

	/// <summary>
	/// ϴ�Ƶ�Ӱ�б�
	/// </summary>
	/// <param name="MoviesList"></param>
	void ShuffleMovies(TArray<FString>& MoviesList);


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

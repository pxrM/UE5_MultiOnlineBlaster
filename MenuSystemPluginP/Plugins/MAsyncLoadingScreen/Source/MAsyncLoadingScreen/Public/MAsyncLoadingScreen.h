// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


struct FALoadingScreenSettings;


class FMAsyncLoadingScreenModule : public IModuleInterface
{
public:
	/// <summary>
	/// 在加载模块DLL并创建模块对象之后立即调用
	/// </summary>
	virtual void StartupModule() override;

	/// <summary>
	/// 在模块被卸载之前调用，就在模块对象被销毁之前。
	/// </summary>
	virtual void ShutdownModule() override;

	/// <summary>
	/// 是否属于游戏逻辑模块。该函数返回一个布尔值，用于表示当前模块是否托管游戏代码。
	/// 如果返回值为 true，则表示该模块是游戏逻辑模块；如果返回值为 false，则表示该模块是引擎代码模块、插件或其他类型的模块。
	/// </summary>
	/// <returns></returns>
	virtual bool IsGameModule() const override;


public:
	/// <summary>
	/// 像单例一样访问这个模块的接口。这只是为了方便!
	/// 注意在插件关闭阶段调用这个。您的模块可能已经被卸载了。
	/// </summary>
	/// <returns> 返回单例实例，根据需要加载模块 </returns>
	static inline FMAsyncLoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMAsyncLoadingScreenModule>("MAsyncLoadingScreen");
	}

	/// <summary>
	/// 模块是否已加载
	/// </summary>
	/// <returns></returns>
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("MAsyncLoadingScreen");
	}

	FORCEINLINE bool IsStartupLoadingScreen() const { return bIsStartupLoadingScreen; }


public:
	/// <summary>
	/// 获取背景图片数组
	/// </summary>
	/// <returns></returns>
	TArray<UTexture2D*> GetBackgroundImages();

	/// <summary>
	/// 检查“bPreloadBackgroundImages”选项是否启用
	/// </summary>
	/// <returns></returns>
	bool IsPreloadBackgroundImageEnable();

	/// <summary>
	/// 从设置中加载所有背景图像到数组
	/// </summary>
	void LoadBackgroundImages();

	/// <summary>
	/// 删除数组所有的bg图
	/// </summary>
	void RemoveAllBackgroundImages();


private:
	/// <summary>
	/// 加载屏幕回调函数。该函数在加载过程中被调用，用于预先设置加载屏幕，以便在加载期间显示。
	/// </summary>
	void PreSetupLoadingScreen();

	/// <summary>
	/// 设置加载画面设置
	/// </summary>
	/// <param name="LoadingScreenSettings"></param>
	void SetupLoadingScreen(const FALoadingScreenSettings& LoadingScreenSettings);

	/// <summary>
	/// 洗牌电影列表
	/// </summary>
	/// <param name="MoviesList"></param>
	void ShuffleMovies(TArray<FString>& MoviesList);


private:
	/// <summary>
	/// 启动背景图像数组
	/// </summary>
	UPROPERTY()
	TArray<class UTexture2D*> StartupBackgroundImages;
	/// <summary>
	/// 默认背景图像数组
	/// </summary>
	UPROPERTY()
	TArray<UTexture2D*> DefaultBackgroundImages;
	/// <summary>
	/// 是否是启动场景
	/// </summary>
	bool bIsStartupLoadingScreen = false;

};

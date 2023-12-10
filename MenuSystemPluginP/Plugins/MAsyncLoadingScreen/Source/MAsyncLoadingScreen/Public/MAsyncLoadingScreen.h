// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

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
	/// 像单例一样访问这个模块的接口。这只是为了方便!
	/// 注意在插件关闭阶段调用这个。您的模块可能已经被卸载了。
	/// </summary>
	/// <returns> 返回单例实例，根据需要加载模块 </returns>
	static inline FMAsyncLoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMAsyncLoadingScreenModule>("MAsyncLoadingScreen");
	}


public:
	/// <summary>
	/// 检查“bPreloadBackgroundImages”选项是否启用
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

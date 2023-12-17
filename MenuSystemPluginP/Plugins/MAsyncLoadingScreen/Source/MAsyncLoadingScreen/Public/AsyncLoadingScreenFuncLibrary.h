// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoadingScreenSettings.h"
#include "AsyncLoadingScreenFuncLibrary.generated.h"

/**
 * 异步加载屏幕函数库
 */
UCLASS()
class MASYNCLOADINGSCREEN_API UAsyncLoadingScreenFuncLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:
	static int32 DisplayBackgroundIndex;
	static int32 DisplayTipTextIndex;
	static int32 DisplayMovieIndex;
	static bool  bShowLoadingScreen;


public:
	static inline int32 GetDisplayBackgroundIndex() { return DisplayBackgroundIndex; }
	static inline int32 GetDisplayTipTextIndex() { return DisplayTipTextIndex; }
	static inline int32 GetDisplayMovieIndex() { return DisplayMovieIndex; }


public:
	/**
	*  通过索引设置加载屏幕上显示的背景。这个索引对应于背景设置中的 "Images" 数组中的有效索引。
	* 如果传入的索引无效，则会显示随机的背景图像。
	* 背景设置中的“SetDisplayBackgroundManually”选项需要为“true”才能使用此功能。
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetDisplayBackgroundIndex(int32 BackgroundIndex);

	/**
	* 通过索引设置加载屏幕上显示的文本。Tip Widget设置中的“SetDisplayTipTextManually”选项需要为“true”才能使用此功能。
	* TipTextIndex在Tip Widget设置中“TipText”数组中文本的有效索引。如果索引无效，那么它将显示随机文本。
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetDisplayTipTextIndex(int32 TipTextIndex);

	/**
	* 通过索引设置在加载屏幕上显示的电影。
	* “SetDisplayMovieIndexManually”选项需要为“true”才能使用此功能。
	* MovieIndex需要是“MoviePaths”数组中电影的有效索引。
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetDisplayMovieIndex(int32 MovieIndex);

	/**
	* 为下一个关卡设置开启/关闭加载屏幕
	* bIsEnableLoadingScreen 是否应该为下一个关卡启用加载屏幕?
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetEnableLoadingScreen(bool bIsEnableLoadingScreen);

	/**
	* 获取下一个关卡启用/禁用加载屏幕
	**/
	UFUNCTION(BlueprintPure, Category = "Async Loading Screen")
	static inline bool GetIsEnableLoadingScreen() { return bShowLoadingScreen; }

	/**
	* 通过调用该函数来停止正在显示的加载屏幕。要使用该函数，必须启用 "bAllowEngineTick" 选项，
	* 而且需要在 BeginPlay 事件中调用该函数（可以与 Delay 节点一起使用）。
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void StopLoadingScreen();

	/**
	* 将所有背景图像预加载到内存中。
	* 在调用该函数之前，需要确保已勾选 "bPreloadBackgroundImages" 选项，并且在之前调用了 "RemovePreloadedBackgroundImages" 函数。
	* 只有在满足以下条件时才需要使用该函数：
	*	已勾选 "bPreloadBackgroundImages" 选项
	*   在调用该函数之前已经调用了 "RemovePreloadedBackgroundImages" 函数
	* 如果从未调用过 "RemovePreloadedBackgroundImages" 函数，那么无需调用此函数，因为背景图像仍然在内存中。
	* 需要注意的是，在调用 "OpenLevel" 节点之前调用此函数（在加载新关卡之前，希望预加载所有的背景图像，以避免加载过程中的延迟或卡顿）。
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void PreloadBackgroundImages();

	/**
	* 通过调用该函数来移除所有已预加载的背景图像。
	* 只有在已勾选 "bPreloadBackgroundImages" 选项时才需要使用此函数。
	* 如果需要再次加载背景图像，需要再次调用 "PreloadBackgroundImages" 函数进行手动加载。
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void RemovePreloadedBackgroundImages();
};

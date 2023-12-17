// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoadingScreenSettings.h"
#include "AsyncLoadingScreenFuncLibrary.generated.h"

/**
 * �첽������Ļ������
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
	*  ͨ���������ü�����Ļ����ʾ�ı��������������Ӧ�ڱ��������е� "Images" �����е���Ч������
	* ��������������Ч�������ʾ����ı���ͼ��
	* ���������еġ�SetDisplayBackgroundManually��ѡ����ҪΪ��true������ʹ�ô˹��ܡ�
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetDisplayBackgroundIndex(int32 BackgroundIndex);

	/**
	* ͨ���������ü�����Ļ����ʾ���ı���Tip Widget�����еġ�SetDisplayTipTextManually��ѡ����ҪΪ��true������ʹ�ô˹��ܡ�
	* TipTextIndex��Tip Widget�����С�TipText���������ı�����Ч���������������Ч����ô������ʾ����ı���
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetDisplayTipTextIndex(int32 TipTextIndex);

	/**
	* ͨ�����������ڼ�����Ļ����ʾ�ĵ�Ӱ��
	* ��SetDisplayMovieIndexManually��ѡ����ҪΪ��true������ʹ�ô˹��ܡ�
	* MovieIndex��Ҫ�ǡ�MoviePaths�������е�Ӱ����Ч������
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetDisplayMovieIndex(int32 MovieIndex);

	/**
	* Ϊ��һ���ؿ����ÿ���/�رռ�����Ļ
	* bIsEnableLoadingScreen �Ƿ�Ӧ��Ϊ��һ���ؿ����ü�����Ļ?
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void SetEnableLoadingScreen(bool bIsEnableLoadingScreen);

	/**
	* ��ȡ��һ���ؿ�����/���ü�����Ļ
	**/
	UFUNCTION(BlueprintPure, Category = "Async Loading Screen")
	static inline bool GetIsEnableLoadingScreen() { return bShowLoadingScreen; }

	/**
	* ͨ�����øú�����ֹͣ������ʾ�ļ�����Ļ��Ҫʹ�øú������������� "bAllowEngineTick" ѡ�
	* ������Ҫ�� BeginPlay �¼��е��øú����������� Delay �ڵ�һ��ʹ�ã���
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void StopLoadingScreen();

	/**
	* �����б���ͼ��Ԥ���ص��ڴ��С�
	* �ڵ��øú���֮ǰ����Ҫȷ���ѹ�ѡ "bPreloadBackgroundImages" ѡ�������֮ǰ������ "RemovePreloadedBackgroundImages" ������
	* ֻ����������������ʱ����Ҫʹ�øú�����
	*	�ѹ�ѡ "bPreloadBackgroundImages" ѡ��
	*   �ڵ��øú���֮ǰ�Ѿ������� "RemovePreloadedBackgroundImages" ����
	* �����δ���ù� "RemovePreloadedBackgroundImages" ��������ô������ô˺�������Ϊ����ͼ����Ȼ���ڴ��С�
	* ��Ҫע����ǣ��ڵ��� "OpenLevel" �ڵ�֮ǰ���ô˺������ڼ����¹ؿ�֮ǰ��ϣ��Ԥ�������еı���ͼ���Ա�����ع����е��ӳٻ򿨶٣���
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void PreloadBackgroundImages();

	/**
	* ͨ�����øú������Ƴ�������Ԥ���صı���ͼ��
	* ֻ�����ѹ�ѡ "bPreloadBackgroundImages" ѡ��ʱ����Ҫʹ�ô˺�����
	* �����Ҫ�ٴμ��ر���ͼ����Ҫ�ٴε��� "PreloadBackgroundImages" ���������ֶ����ء�
	**/
	UFUNCTION(BlueprintCallable, Category = "Async Loading Screen")
	static void RemovePreloadedBackgroundImages();
};

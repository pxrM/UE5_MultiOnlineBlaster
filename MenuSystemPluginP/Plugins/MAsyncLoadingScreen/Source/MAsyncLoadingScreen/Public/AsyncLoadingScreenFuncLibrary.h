// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoadingScreenSettings.h"
#include "AsyncLoadingScreenFuncLibrary.generated.h"

/**
 * Òì²½¼ÓÔØÆÁÄ»º¯Êý¿â
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

};

// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpManagerSettings.h"

UHttpManagerSettings::UHttpManagerSettings(const FObjectInitializer& ObjectInitializer)
	: MaxParallel(5)
	, MaxTryCount(10)
	, RequestKBSize(1024)
	, ResponseTimeout(5.0f)
{
	CurFilePath = FPaths::ProjectSavedDir() + TEXT("/HttpCacheFiles");
}

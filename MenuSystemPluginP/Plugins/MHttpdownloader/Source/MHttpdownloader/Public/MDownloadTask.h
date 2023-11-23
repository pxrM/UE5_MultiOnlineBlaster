// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MTaskInformation.h"
#include "MDownloadEvent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"


/**
 * a download task, normally operated by FileDownloadManager, extreamly advise you to use FileDownloadManager.
 */
class FMDownloadTask
{
public:
	FMDownloadTask();

	FMDownloadTask(const FString& InUrl, const FString& InDirectory, const FString& InFileName);

	FMDownloadTask(const FMTaskInformation& InTaskInfo);

	~FMDownloadTask();


public:


};

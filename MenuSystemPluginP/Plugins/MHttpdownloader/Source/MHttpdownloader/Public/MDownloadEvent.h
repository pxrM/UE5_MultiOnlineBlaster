// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


/**
 * �����¼�id
 */
UENUM(BlueprintType)
enum class EMTaskEvent : uint8
{
	// start downloading task
	START_DOWNLOAD,
	// Update
	DOWNLOAD_UPDATE,
	// stop
	STOP,
	// download completed
	DOWNLOAD_COMPLETED,
	// meet error during downloading or get task information
	ERROR_OCCUR
};


/// <summary>
/// ���������״̬
/// </summary>
UENUM(BlueprintType)
enum class EMTaskState : uint8
{
	// wait for getting information
	WAIT,
	// is being downloading
	DOWNLOADING,
	// task completed
	COMPLETED,
	// error state
	ERROR
};
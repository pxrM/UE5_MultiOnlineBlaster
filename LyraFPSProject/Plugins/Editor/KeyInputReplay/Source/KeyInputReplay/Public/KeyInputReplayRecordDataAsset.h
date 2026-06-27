// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KeyInputReplayDataTypes.h"
#include "KeyInputReplayRecordDataAsset.generated.h"

UCLASS(BlueprintType,Blueprintable)
class KEYINPUTREPLAY_API UKeyInputReplayRecordDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Category = "Status", VisibleAnywhere)
	bool bRecorded = false;

	UPROPERTY(Category = "Record", EditAnywhere)
	TArray<FKeyInputReplayKeyData> KeyData;

public:
	void Reset()
	{
		bRecorded = false;
		KeyData.Empty();
	};
};

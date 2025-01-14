// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 存档名
	UPROPERTY()
	FString SlotName = FString();

	// 存档索引
	UPROPERTY()
	int32 SlotIndex = 0;

	// 存档的玩家名
	UPROPERTY()
	FString PlayerName = FString("Default Name");
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiPlayerSessionSubsystem.h"


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		const UMultiPlayerSessionSubsystem* SubSystem = GameInstance->GetSubsystem<UMultiPlayerSessionSubsystem>();
		check(SubSystem); // 检查：如果SubSystem为null，程序停止执行

		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		if (NumberOfPlayers == SubSystem->DesiredNumPublicConnections)
		{
			if (UWorld* World = GetWorld())
			{
				bUseSeamlessTravel = true;	// 使用无缝travel，蓝图里也要设置
				const FString MatchType = SubSystem->DesiredMatchType;
				if (MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/TeamBlasterMap?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlagBlasterMa?listen"));
				}
			}
		}
	}
}

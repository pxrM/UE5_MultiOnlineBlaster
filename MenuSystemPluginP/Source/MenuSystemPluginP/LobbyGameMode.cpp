// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		if (GEngine)
		{
			APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();
			if (NewPlayerState)
			{
				FString NewPlayerName = NewPlayerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Yellow, FString::Printf(TEXT("%s has joined the game"), *NewPlayerName));
			}

			int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
			GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Yellow, FString::Printf(TEXT("Players is game: %d"), NumberOfPlayers));
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GameState)
	{
		if (GEngine)
		{
			APlayerState* ExitPlayerState = Exiting->GetPlayerState<APlayerState>();
			if (ExitPlayerState)
			{
				FString ExitPlayerName = ExitPlayerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Yellow, FString::Printf(TEXT("%s has exited the game"), *ExitPlayerName));
			}

			int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
			GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Yellow, FString::Printf(TEXT("Players is game: %d"), NumberOfPlayers - 1));
		}
	}
}

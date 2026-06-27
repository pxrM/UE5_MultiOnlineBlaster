// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/Player/LyraFPSPlayerSpawningManagementComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/LyraGameState.h"
#include "GameplayAbilitySystem/LyraFPSGameplayTags.h"
#include "Player/LyraPlayerStart.h"
#include "Teams/LyraTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraFPSPlayerSpawningManagementComponent)

class AActor;

ULyraFPSPlayerSpawningManagementComponent::ULyraFPSPlayerSpawningManagementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AActor* ULyraFPSPlayerSpawningManagementComponent::OnChoosePlayerStart(AController* Player, TArray<ALyraPlayerStart*>& PlayerStarts)
{
	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	if (!ensure(TeamSubsystem))
	{
		return nullptr;
	}

	const int32 PlayerTeamId = TeamSubsystem->FindTeamFromObject(Player);

	// We should have a TeamId by now, but early login stuff before post login can try to do stuff, ignore it.
	if (!ensure(PlayerTeamId != INDEX_NONE))
	{
		return nullptr;
	}

	ALyraGameState* GameState = GetGameStateChecked<ALyraGameState>();

	ALyraPlayerStart* BestPlayerStart = nullptr;
	double MaxDistance = 0;
	ALyraPlayerStart* FallbackPlayerStart = nullptr;
	double FallbackMaxDistance = 0;

	//新方案：Tag匹配玩家还是AI
	TArray<ALyraPlayerStart*> willPlayerStarts;
	for (ALyraPlayerStart* PlayerStart : PlayerStarts)
	{
		//AIStart对AI，Player选择无AI标识的
		bool bIsPlayerStart=!PlayerStart->GetGameplayTags().HasTag(LyraFPSGameplayTags::PlayerStart_AI);
		if (Player->IsPlayerController()==bIsPlayerStart)
		{
			willPlayerStarts.Add(PlayerStart);
		}
	}
	if (!willPlayerStarts.IsEmpty())
	{
		int32 minIndex=INDEX_NONE;
		int32 minCount=0;
		// float randIndex = FMath::Rand32()%(willPlayerStarts.Num());
		for (auto tmpPlayerStart : willPlayerStarts)
		{
			auto countIndex= PlayerStartCount.Find(tmpPlayerStart);
			if (countIndex!=INDEX_NONE)
			{
				if (minCount>PlayerStartCount[countIndex].Count)
				{
					minCount=PlayerStartCount[countIndex].Count;
					minIndex=countIndex;
				}
			}
			else
			{
				PlayerStartCount.Add(tmpPlayerStart);
				return tmpPlayerStart;
			}
		}
		if (minIndex!=INDEX_NONE)
		{
			return willPlayerStarts[minIndex];
		}
		
	}
	
	
	for (APlayerState* PS : GameState->PlayerArray)
	{
		const int32 TeamId = TeamSubsystem->FindTeamFromObject(PS);
		
		// We should have a TeamId by now...
		if (PS->IsOnlyASpectator() || !ensure(TeamId != INDEX_NONE))
		{
			continue;
		}

		// If the other player isn't on the same team, lets find the furthest spawn from them.
		if (TeamId != PlayerTeamId)
		{
			
			for (ALyraPlayerStart* PlayerStart : PlayerStarts)
			{
				if (APawn* Pawn = PS->GetPawn())
				{
					const double Distance = PlayerStart->GetDistanceTo(Pawn);

					if (PlayerStart->IsClaimed())
					{
						if (FallbackPlayerStart == nullptr || Distance > FallbackMaxDistance)
						{
							FallbackPlayerStart = PlayerStart;
							FallbackMaxDistance = Distance;
						}
					}
					else if (PlayerStart->GetLocationOccupancy(Player) < ELyraPlayerStartLocationOccupancy::Full)
					{
						if (BestPlayerStart == nullptr || Distance > MaxDistance)
						{
							BestPlayerStart = PlayerStart;
							MaxDistance = Distance;
						}
					}
				}
			}
		}
	}

	if (BestPlayerStart)
	{
		return BestPlayerStart;
	}

	return FallbackPlayerStart;
}

void ULyraFPSPlayerSpawningManagementComponent::OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
{
	
}

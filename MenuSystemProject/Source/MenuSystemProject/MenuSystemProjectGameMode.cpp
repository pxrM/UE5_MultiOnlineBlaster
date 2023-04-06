// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemProjectGameMode.h"
#include "MenuSystemProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMenuSystemProjectGameMode::AMenuSystemProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

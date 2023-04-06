// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemPluginPGameMode.h"
#include "MenuSystemPluginPCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMenuSystemPluginPGameMode::AMenuSystemPluginPGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

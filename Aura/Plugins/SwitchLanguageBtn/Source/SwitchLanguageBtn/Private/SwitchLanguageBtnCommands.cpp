// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwitchLanguageBtnCommands.h"

#define LOCTEXT_NAMESPACE "FSwitchLanguageBtnModule"

void FSwitchLanguageBtnCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "SwitchLanguageBtn", "Execute SwitchLanguageBtn action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

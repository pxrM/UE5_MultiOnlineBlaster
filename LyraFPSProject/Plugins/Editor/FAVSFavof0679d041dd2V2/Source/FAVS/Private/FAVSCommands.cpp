// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSCommands.h"

#define LOCTEXT_NAMESPACE "FFAVSModule"

void FFAVSCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "FAVS", "Open the FAVS tab", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

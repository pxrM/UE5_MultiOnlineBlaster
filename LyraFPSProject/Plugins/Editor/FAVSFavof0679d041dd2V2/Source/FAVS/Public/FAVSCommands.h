// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "FAVSStyle.h"

class FFAVSCommands : public TCommands<FFAVSCommands>
{
public:

	FFAVSCommands(): TCommands<FFAVSCommands>(TEXT("FAVS"), NSLOCTEXT("Contexts", "FAVS", "FAVS Plugin"), NAME_None, FAVSStyle::GetStyleSetName()){}
	
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
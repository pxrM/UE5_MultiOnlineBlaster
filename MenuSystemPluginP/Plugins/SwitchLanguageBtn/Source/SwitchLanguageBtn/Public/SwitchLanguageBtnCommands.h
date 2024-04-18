// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SwitchLanguageBtnStyle.h"

class FSwitchLanguageBtnCommands : public TCommands<FSwitchLanguageBtnCommands>
{
public:

	FSwitchLanguageBtnCommands()
		: TCommands<FSwitchLanguageBtnCommands>(TEXT("SwitchLanguageBtn"), NSLOCTEXT("Contexts", "SwitchLanguageBtn", "SwitchLanguageBtn Plugin"), NAME_None, FSwitchLanguageBtnStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};

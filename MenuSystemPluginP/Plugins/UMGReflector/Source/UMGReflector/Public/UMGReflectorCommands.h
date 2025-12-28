#pragma once

#include "Framework/Commands/Commands.h"
#include "UMGReflectorStyle.h"

class FUMGReflectorCommands : public TCommands<FUMGReflectorCommands>
{
public:
	FUMGReflectorCommands()
		: TCommands<FUMGReflectorCommands>(TEXT("UMGReflector"), NSLOCTEXT("Contexts", "UMGReflector", "UMGReflector Plugins"), NAME_None, FUMGReflectorStyle::GetStyleName())
	{  }

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
	
};
